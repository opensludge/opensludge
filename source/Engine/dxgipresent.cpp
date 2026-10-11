/*
 * Showing the frames on Windows through DXGI.
 *
 * In full screen, the graphics driver can show the frames of an OpenGL
 * program without the compositor of Windows, and then Windows does not map
 * their colours to an HDR display. With a DXGI flip model swap chain,
 * Windows maps the colours in full screen too, and can apply Auto HDR.
 * OpenGL has no way to present through DXGI, so we do it ourselves:
 *
 * - We create the OpenGL context in a hidden window, and we draw each frame
 *   into a frame buffer object in that context.
 * - We create a Direct3D 11 device on the same GPU as the OpenGL context,
 *   because with WGL_NV_DX_interop2 we can only share textures on one GPU.
 *   On a computer with more than one GPU, the default GPU of Direct3D can
 *   be another one than the GPU of OpenGL.
 * - With the WGL_NV_DX_interop2 extension, we share a Direct3D 11 texture
 *   with OpenGL. We copy the frame into it, upside down, because Direct3D
 *   counts the rows from the top and OpenGL from the bottom.
 * - We copy that texture into a flip model swap chain of the game window,
 *   and present it.
 *
 * We do this ourselves instead of using ANGLE (OpenGL ES on Direct3D), so
 * that we keep one renderer in the engine and ship no other DLLs. We load
 * Direct3D 11, DXGI and the OpenGL functions that we use here at run time,
 * so that the engine still starts where they are missing, for example on
 * Windows XP. There, we draw into the window with OpenGL, as on the other
 * platforms.
 */

#include <SDL3/SDL.h>

#if !defined(_WIN32)
#error "dxgipresent.cpp is only for Windows"
#endif

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>

#include "sludge_gl.h"

#include "debug.h"
#include "dxgipresent.h"

// WGL_NV_DX_interop2, from its specification.
static const GLenum WGL_ACCESS_WRITE_DISCARD_NV = 0x0002;
typedef HANDLE (WINAPI * WGLDXOPENDEVICENV) (void * dxDevice);
typedef BOOL (WINAPI * WGLDXCLOSEDEVICENV) (HANDLE device);
typedef HANDLE (WINAPI * WGLDXREGISTEROBJECTNV) (HANDLE device, void * dxObject, GLuint name, GLenum type, GLenum access);
typedef BOOL (WINAPI * WGLDXUNREGISTEROBJECTNV) (HANDLE device, HANDLE object);
typedef BOOL (WINAPI * WGLDXLOCKOBJECTSNV) (HANDLE device, GLint count, HANDLE * objects);

static WGLDXOPENDEVICENV wglDXOpenDevice;
static WGLDXCLOSEDEVICENV wglDXCloseDevice;
static WGLDXREGISTEROBJECTNV wglDXRegisterObject;
static WGLDXUNREGISTEROBJECTNV wglDXUnregisterObject;
static WGLDXLOCKOBJECTSNV wglDXLockObjects;
static WGLDXLOCKOBJECTSNV wglDXUnlockObjects;

// Frame buffer objects, from OpenGL 3.0 or ARB_framebuffer_object.
static PFNGLGENFRAMEBUFFERSPROC genFramebuffers;
static PFNGLDELETEFRAMEBUFFERSPROC deleteFramebuffers;
static PFNGLBINDFRAMEBUFFERPROC bindFramebuffer;
static PFNGLFRAMEBUFFERTEXTURE2DPROC framebufferTexture2D;
static PFNGLFRAMEBUFFERRENDERBUFFERPROC framebufferRenderbuffer;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC checkFramebufferStatus;
static PFNGLGENRENDERBUFFERSPROC genRenderbuffers;
static PFNGLDELETERENDERBUFFERSPROC deleteRenderbuffers;
static PFNGLBINDRENDERBUFFERPROC bindRenderbuffer;
static PFNGLRENDERBUFFERSTORAGEPROC renderbufferStorage;
static PFNGLBLITFRAMEBUFFERPROC blitFramebuffer;

typedef HRESULT (WINAPI * D3D11CREATEDEVICE) (IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT,
	const D3D_FEATURE_LEVEL *, UINT, UINT, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
typedef HRESULT (WINAPI * CREATEDXGIFACTORY1) (REFIID, void **);

// PCI vendor IDs of GPUs.
static const UINT VENDOR_AMD = 0x1002;
static const UINT VENDOR_INTEL = 0x8086;
static const UINT VENDOR_NVIDIA = 0x10DE;

static bool presenting = false;
static HMODULE d3d11Library = NULL, dxgiLibrary = NULL;
static D3D11CREATEDEVICE createD3D11Device = NULL;
static SDL_Window * hiddenWindow = NULL;
static SDL_GLContext glContext = NULL;

static ID3D11Device * device = NULL;
static ID3D11DeviceContext * deviceContext = NULL;
static IDXGISwapChain1 * swapChain = NULL;
static HANDLE interopDevice = NULL;

// The frame buffer that we draw into, as large as the window.
static int width = 0, height = 0;
static GLuint drawFramebuffer = 0, drawTexture = 0, drawDepthStencil = 0;

// The texture that we share with Direct3D.
static ID3D11Texture2D * sharedTexture = NULL;
static GLuint sharedGLTexture = 0, sharedFramebuffer = 0;
static HANDLE sharedObject = NULL;

template <class T> static void release (T * & object) {
	if (object) object -> Release ();
	object = NULL;
}

static bool loadFunctions () {
#define LOAD(variable, type, name) variable = (type) SDL_GL_GetProcAddress (name); if (! variable) return false;
	LOAD (wglDXOpenDevice, WGLDXOPENDEVICENV, "wglDXOpenDeviceNV")
	LOAD (wglDXCloseDevice, WGLDXCLOSEDEVICENV, "wglDXCloseDeviceNV")
	LOAD (wglDXRegisterObject, WGLDXREGISTEROBJECTNV, "wglDXRegisterObjectNV")
	LOAD (wglDXUnregisterObject, WGLDXUNREGISTEROBJECTNV, "wglDXUnregisterObjectNV")
	LOAD (wglDXLockObjects, WGLDXLOCKOBJECTSNV, "wglDXLockObjectsNV")
	LOAD (wglDXUnlockObjects, WGLDXLOCKOBJECTSNV, "wglDXUnlockObjectsNV")
	LOAD (genFramebuffers, PFNGLGENFRAMEBUFFERSPROC, "glGenFramebuffers")
	LOAD (deleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC, "glDeleteFramebuffers")
	LOAD (bindFramebuffer, PFNGLBINDFRAMEBUFFERPROC, "glBindFramebuffer")
	LOAD (framebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC, "glFramebufferTexture2D")
	LOAD (framebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC, "glFramebufferRenderbuffer")
	LOAD (checkFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC, "glCheckFramebufferStatus")
	LOAD (genRenderbuffers, PFNGLGENRENDERBUFFERSPROC, "glGenRenderbuffers")
	LOAD (deleteRenderbuffers, PFNGLDELETERENDERBUFFERSPROC, "glDeleteRenderbuffers")
	LOAD (bindRenderbuffer, PFNGLBINDRENDERBUFFERPROC, "glBindRenderbuffer")
	LOAD (renderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC, "glRenderbufferStorage")
	LOAD (blitFramebuffer, PFNGLBLITFRAMEBUFFERPROC, "glBlitFramebuffer")
#undef LOAD
	return true;
}

// Load Direct3D 11 and DXGI, and return the DXGI factory, or NULL.
static IDXGIFactory1 * loadLibraries () {
	d3d11Library = LoadLibraryW (L"d3d11.dll");
	dxgiLibrary = LoadLibraryW (L"dxgi.dll");
	if (! d3d11Library || ! dxgiLibrary) return NULL;
	createD3D11Device = (D3D11CREATEDEVICE) (void *) GetProcAddress (d3d11Library, "D3D11CreateDevice");
	CREATEDXGIFACTORY1 createFactory = (CREATEDXGIFACTORY1) (void *) GetProcAddress (dxgiLibrary, "CreateDXGIFactory1");
	IDXGIFactory1 * factory = NULL;
	if (! createD3D11Device || ! createFactory || FAILED (createFactory (__uuidof (IDXGIFactory1), (void **) &factory))) return NULL;
	return factory;
}

// Return the PCI vendor ID of the GPU of the current OpenGL context, from
// the vendor string of OpenGL, or 0 when we do not know the vendor. The
// strings are, for example, "NVIDIA Corporation", "ATI Technologies Inc."
// and "Intel". We compare with case, because "Corporation" contains "ati".
static UINT openGLVendor () {
	const char * vendor = (const char *) glGetString (GL_VENDOR);
	if (! vendor) return 0;
	if (SDL_strstr (vendor, "NVIDIA")) return VENDOR_NVIDIA;
	if (SDL_strstr (vendor, "ATI") || SDL_strstr (vendor, "AMD")) return VENDOR_AMD;
	if (SDL_strstr (vendor, "Intel")) return VENDOR_INTEL;
	return 0;
}

static bool createDevice (IDXGIAdapter1 * adapter) {
	// With a given adapter, the driver type must be UNKNOWN.
	return SUCCEEDED (createD3D11Device (adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		NULL, 0, D3D11_SDK_VERSION, &device, NULL, &deviceContext));
}

static bool createSwapChain (SDL_Window * window) {
	HWND hwnd = (HWND) SDL_GetPointerProperty (SDL_GetWindowProperties (window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	if (! hwnd) return false;

	IDXGIDevice * dxgiDevice = NULL;
	IDXGIAdapter * adapter = NULL;
	IDXGIFactory2 * factory = NULL;
	bool ok = SUCCEEDED (device -> QueryInterface (__uuidof (IDXGIDevice), (void **) &dxgiDevice))
		&& SUCCEEDED (dxgiDevice -> GetAdapter (&adapter))
		&& SUCCEEDED (adapter -> GetParent (__uuidof (IDXGIFactory2), (void **) &factory));

	if (ok) {
		SDL_GetWindowSizeInPixels (window, &width, &height);
		DXGI_SWAP_CHAIN_DESC1 desc;
		ZeroMemory (&desc, sizeof (desc));
		desc.Width = width;
		desc.Height = height;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 2;
		// FLIP_DISCARD is from Windows 10 on, and FLIP_SEQUENTIAL from
		// Windows 8 on.
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		ok = SUCCEEDED (factory -> CreateSwapChainForHwnd (device, hwnd, &desc, NULL, NULL, &swapChain));
		if (! ok) {
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			ok = SUCCEEDED (factory -> CreateSwapChainForHwnd (device, hwnd, &desc, NULL, NULL, &swapChain));
		}
		// We switch to full screen with Alt+Enter ourselves, in main.cpp.
		if (ok) factory -> MakeWindowAssociation (hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
	}
	release (factory);
	release (adapter);
	release (dxgiDevice);
	return ok;
}

static void destroyFramebuffers () {
	if (sharedObject) {
		wglDXUnlockObjects (interopDevice, 1, &sharedObject);
		wglDXUnregisterObject (interopDevice, sharedObject);
		sharedObject = NULL;
	}
	if (sharedFramebuffer) deleteFramebuffers (1, &sharedFramebuffer);
	if (sharedGLTexture) glDeleteTextures (1, &sharedGLTexture);
	release (sharedTexture);
	if (drawFramebuffer) deleteFramebuffers (1, &drawFramebuffer);
	if (drawTexture) glDeleteTextures (1, &drawTexture);
	if (drawDepthStencil) deleteRenderbuffers (1, &drawDepthStencil);
	sharedFramebuffer = sharedGLTexture = drawFramebuffer = drawTexture = drawDepthStencil = 0;
}

// Create the frame buffer that we draw into, and the texture that we share
// with Direct3D, both width x height pixels, and leave the first one bound.
static bool createFramebuffers () {
	genFramebuffers (1, &drawFramebuffer);
	glGenTextures (1, &drawTexture);
	genRenderbuffers (1, &drawDepthStencil);
	glBindTexture (GL_TEXTURE_2D, drawTexture);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	bindRenderbuffer (GL_RENDERBUFFER, drawDepthStencil);
	renderbufferStorage (GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	bindFramebuffer (GL_FRAMEBUFFER, drawFramebuffer);
	framebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, drawTexture, 0);
	framebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, drawDepthStencil);
	if (checkFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory (&desc, sizeof (desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (FAILED (device -> CreateTexture2D (&desc, NULL, &sharedTexture))) return false;

	glGenTextures (1, &sharedGLTexture);
	sharedObject = wglDXRegisterObject (interopDevice, sharedTexture, sharedGLTexture, GL_TEXTURE_2D, WGL_ACCESS_WRITE_DISCARD_NV);
	if (! sharedObject) return false;
	// We keep the texture locked for OpenGL, except while Direct3D copies
	// it.
	if (! wglDXLockObjects (interopDevice, 1, &sharedObject)) return false;
	genFramebuffers (1, &sharedFramebuffer);
	bindFramebuffer (GL_FRAMEBUFFER, sharedFramebuffer);
	framebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sharedGLTexture, 0);
	bool complete = checkFramebufferStatus (GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

	bindFramebuffer (GL_FRAMEBUFFER, drawFramebuffer);
	return complete;
}

// Release the Direct3D device and everything that we made with it. The
// OpenGL context must be current.
static void releaseDevice () {
	destroyFramebuffers ();
	if (interopDevice) wglDXCloseDevice (interopDevice);
	interopDevice = NULL;
	release (swapChain);
	release (deviceContext);
	release (device);
}

static void stop () {
	if (glContext) {
		SDL_GL_MakeCurrent (hiddenWindow, glContext);
		releaseDevice ();
		SDL_GL_DestroyContext (glContext);
	}
	glContext = NULL;
	if (hiddenWindow) SDL_DestroyWindow (hiddenWindow);
	hiddenWindow = NULL;
	if (d3d11Library) FreeLibrary (d3d11Library);
	if (dxgiLibrary) FreeLibrary (dxgiLibrary);
	d3d11Library = dxgiLibrary = NULL;
	presenting = false;
}

// Set up the presentation on the GPU of an adapter. Return NULL on
// success, or a description of the failure.
static const char * startOnAdapter (IDXGIAdapter1 * adapter, SDL_Window * window) {
	if (! createDevice (adapter)) return "no Direct3D 11 device";
	if (! createSwapChain (window)) return "no flip model swap chain";
	interopDevice = wglDXOpenDevice (device);
	if (! interopDevice) return "wglDXOpenDeviceNV failed";
	if (! createFramebuffers ()) return "the shared frame buffer is incomplete";
	return NULL;
}

// Set up the presentation on the first GPU where we can, trying the GPUs
// from the vendor of the OpenGL context first. Return NULL on success, or
// a description of the failure on the last GPU that we tried.
static const char * startOnSameGPU (IDXGIFactory1 * factory, SDL_Window * window) {
	const char * problem = "no GPU";
	UINT vendor = openGLVendor ();
	for (int sameVendor = 1; sameVendor >= 0; sameVendor --) {
		IDXGIAdapter1 * adapter = NULL;
		for (UINT i = 0; factory -> EnumAdapters1 (i, &adapter) == S_OK; i ++) {
			DXGI_ADAPTER_DESC1 desc;
			bool tryIt = SUCCEEDED (adapter -> GetDesc1 (&desc))
				&& ! (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				&& (desc.VendorId == vendor) == (sameVendor == 1);
			if (tryIt) {
				problem = startOnAdapter (adapter, window);
				char name[256];
				WideCharToMultiByte (CP_UTF8, 0, desc.Description, -1, name, sizeof (name), NULL, NULL);
				debugOut ("DXGI on %s: %s.\n", name, problem ? problem : "ready");
				if (problem) releaseDevice ();
			}
			release (adapter);
			if (tryIt && ! problem) return NULL;
		}
	}
	return problem;
}

SDL_GLContext startDXGIPresentation (SDL_Window * window) {
	const char * problem = NULL;
	hiddenWindow = SDL_CreateWindow ("", 16, 16, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
	if (! hiddenWindow) problem = "no hidden OpenGL window";
	if (! problem) {
		glContext = SDL_GL_CreateContext (hiddenWindow);
		if (! glContext) problem = "no OpenGL context";
	}
	if (! problem && ! loadFunctions ()) problem = "no WGL_NV_DX_interop2 or no frame buffer objects";
	IDXGIFactory1 * factory = problem ? NULL : loadLibraries ();
	if (! problem && ! factory) problem = "no Direct3D 11 or DXGI";
	if (! problem) problem = startOnSameGPU (factory, window);
	release (factory);

	if (problem) {
		debugOut ("We show the frames with OpenGL, not DXGI: %s.\n", problem);
		stop ();
		return NULL;
	}
	debugOut ("We show the frames through DXGI.\n");
	presenting = true;
	return glContext;
}

bool presentingWithDXGI () {
	return presenting;
}

void resizeDXGIPresentation (int newWidth, int newHeight) {
	if (! presenting || (newWidth == width && newHeight == height)) return;
	if (newWidth < 1 || newHeight < 1) return;
	destroyFramebuffers ();
	width = newWidth;
	height = newHeight;
	// ResizeBuffers fails while a buffer is bound to the device context.
	deviceContext -> ClearState ();
	if (FAILED (swapChain -> ResizeBuffers (0, width, height, DXGI_FORMAT_UNKNOWN, 0))) {
		debugOut ("We could not resize the DXGI swap chain.\n");
	}
	if (! createFramebuffers ()) debugOut ("We could not make the frame buffers for DXGI again.\n");
}

void presentWithDXGI () {
	if (! presenting) return;

	bindFramebuffer (GL_READ_FRAMEBUFFER, drawFramebuffer);
	bindFramebuffer (GL_DRAW_FRAMEBUFFER, sharedFramebuffer);
	blitFramebuffer (0, 0, width, height, 0, height, width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	bindFramebuffer (GL_FRAMEBUFFER, drawFramebuffer);
	wglDXUnlockObjects (interopDevice, 1, &sharedObject);

	ID3D11Texture2D * backBuffer = NULL;
	if (SUCCEEDED (swapChain -> GetBuffer (0, __uuidof (ID3D11Texture2D), (void **) &backBuffer))) {
		deviceContext -> CopyResource (backBuffer, sharedTexture);
		release (backBuffer);
	}
	swapChain -> Present (1, 0);

	wglDXLockObjects (interopDevice, 1, &sharedObject);
}
