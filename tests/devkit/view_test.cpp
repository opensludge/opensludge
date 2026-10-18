/*
 * Image tests for the views of the floor maker, the z-buffer maker and the
 * sprite bank editor.
 *
 * We draw each view offscreen, in an OpenGL 3.2 core profile context like
 * the one that GtkGLArea creates, and compare the picture with a reference
 * image in the references folder. We count a pixel as different when one of
 * its channels differs by more than 8, and a picture as different when more
 * than 0.1% of its pixels are. We also count an OpenGL error as a failure.
 * For a picture that differs, we write it and a picture of the differences
 * into the current folder.
 *
 * Usage: view_test [--update] [--compatibility]
 *
 * With --update, we write the pictures as the new reference images. With
 * --compatibility, we draw in a compatibility profile context.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include <epoxy/egl.h>
#include <epoxy/gl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "draw2d.h"
#include "floormaker.h"
#include "interface.h"
#include "sprites.h"
#include "zbuffer.h"

// In the Dev Kit, we show a dialog in each function of interface.h. In the
// tests, we print the messages instead.

const char * getTempDir () {
	return "/tmp";
}

bool askAQuestion (const char * head, const char * msg) {
	fprintf (stderr, "Question: %s: %s\n", head, msg);
	return false;
}

bool errorBox (const char * head, const char * msg) {
	fprintf (stderr, "Error: %s: %s\n", head, msg);
	return false;
}

static const int maxChannelDifference = 8;
static const double maxDifferentPixels = 0.001;

static std::string testFolder;
static bool update = false;
static int failures = 0;

// Create an OpenGL context without a window, with Mesa's surfaceless EGL
// platform, and make it current.
static bool createContext (bool compatibility) {
	EGLDisplay display = eglGetPlatformDisplayEXT (EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, NULL);
	if (display == EGL_NO_DISPLAY || ! eglInitialize (display, NULL, NULL)) {
		fprintf (stderr, "We can't open the surfaceless EGL display.\n");
		return false;
	}
	if (! eglBindAPI (EGL_OPENGL_API)) {
		fprintf (stderr, "EGL has no OpenGL support.\n");
		return false;
	}
	const EGLint attributes[] = {
		EGL_CONTEXT_MAJOR_VERSION, 3,
		EGL_CONTEXT_MINOR_VERSION, 2,
		EGL_CONTEXT_OPENGL_PROFILE_MASK,
		compatibility ? EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT : EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
		EGL_NONE
	};
	EGLContext context = eglCreateContext (display, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, attributes);
	if (context == EGL_NO_CONTEXT || ! eglMakeCurrent (display, EGL_NO_SURFACE, EGL_NO_SURFACE, context)) {
		fprintf (stderr, "We can't create an OpenGL 3.2 context.\n");
		return false;
	}
	printf ("OpenGL %s\n", glGetString (GL_VERSION));
	return true;
}

// A frame buffer of the size of a view, in which we draw instead of in a
// window.
struct view {
	int width, height;
	GLuint framebuffer, renderbuffer;
};

static view startView (int width, int height) {
	view v = { width, height, 0, 0 };
	glGenRenderbuffers (1, &v.renderbuffer);
	glBindRenderbuffer (GL_RENDERBUFFER, v.renderbuffer);
	glRenderbufferStorage (GL_RENDERBUFFER, GL_RGBA8, width, height);
	glGenFramebuffers (1, &v.framebuffer);
	glBindFramebuffer (GL_FRAMEBUFFER, v.framebuffer);
	glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, v.renderbuffer);
	glViewport (0, 0, width, height);
	return v;
}

static GdkPixbuf * readView (const view & v) {
	GdkPixbuf * picture = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, v.width, v.height);
	int stride = gdk_pixbuf_get_rowstride (picture);
	guchar * pixels = gdk_pixbuf_get_pixels (picture);
	std::vector<unsigned char> rows (v.width * v.height * 4);
	glPixelStorei (GL_PACK_ALIGNMENT, 1);
	glReadPixels (0, 0, v.width, v.height, GL_RGBA, GL_UNSIGNED_BYTE, rows.data ());
	// glReadPixels returns the rows from the bottom up.
	for (int y = 0; y < v.height; y ++) {
		memcpy (pixels + y * stride, rows.data () + (v.height - 1 - y) * v.width * 4, v.width * 4);
	}
	return picture;
}

static void endView (const view & v) {
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers (1, &v.framebuffer);
	glDeleteRenderbuffers (1, &v.renderbuffer);
}

// Compare the picture in the view with the reference image of the test.
static void check (const char * name, const view & v) {
	GdkPixbuf * picture = readView (v);
	std::string reference = testFolder + "/references/" + name + ".png";
	GLenum error = glGetError ();

	if (update) {
		gdk_pixbuf_save (picture, reference.c_str (), "png", NULL, NULL);
		printf ("Updated %s\n", reference.c_str ());
	} else {
		GdkPixbuf * expected = gdk_pixbuf_new_from_file (reference.c_str (), NULL);
		bool same = expected
			&& gdk_pixbuf_get_width (expected) == v.width
			&& gdk_pixbuf_get_height (expected) == v.height
			&& gdk_pixbuf_get_n_channels (expected) == 4;
		int different = 0;
		if (same) {
			GdkPixbuf * difference = gdk_pixbuf_copy (picture);
			for (int y = 0; y < v.height; y ++) {
				guchar * a = gdk_pixbuf_get_pixels (picture) + y * gdk_pixbuf_get_rowstride (picture);
				guchar * b = gdk_pixbuf_get_pixels (expected) + y * gdk_pixbuf_get_rowstride (expected);
				guchar * d = gdk_pixbuf_get_pixels (difference) + y * gdk_pixbuf_get_rowstride (difference);
				for (int x = 0; x < v.width * 4; x += 4) {
					bool pixelDifferent = false;
					for (int c = 0; c < 4; c ++) {
						if (abs (a[x + c] - b[x + c]) > maxChannelDifference) pixelDifferent = true;
					}
					d[x] = pixelDifferent ? 255 : a[x] / 4;
					d[x + 1] = d[x + 2] = pixelDifferent ? 0 : a[x] / 4;
					d[x + 3] = 255;
					if (pixelDifferent) different ++;
				}
			}
			same = different <= maxDifferentPixels * v.width * v.height;
			if (! same) {
				gdk_pixbuf_save (difference, (std::string (name) + "-difference.png").c_str (), "png", NULL, NULL);
			}
			g_object_unref (difference);
		}
		if (! same) {
			gdk_pixbuf_save (picture, (std::string (name) + "-actual.png").c_str (), "png", NULL, NULL);
		}
		if (error != GL_NO_ERROR) {
			printf ("FAIL %s: OpenGL error 0x%x\n", name, error);
			failures ++;
		} else if (! same) {
			printf ("FAIL %s: %d pixels differ from %s\n", name, different, reference.c_str ());
			failures ++;
		} else {
			printf ("ok   %s\n", name);
		}
		if (expected) g_object_unref (expected);
	}
	g_object_unref (picture);
	while (glGetError () != GL_NO_ERROR) {}
}

// The floor maker, with the floor from fixtures/floor.flo, a lit vertex and
// a selection line, shown whole at half size and in part at full size.
static void testFloorMaker () {
	struct spriteBank backdrop;
	memset (&backdrop, 0, sizeof (backdrop));
	addSprite (0, &backdrop);
	struct polyList * floor = NULL;
	std::string file = testFolder + "/fixtures/floor.flo";
	if (! loadFloorFromFile ((char *) file.c_str (), &floor)) {
		printf ("FAIL floor: we can't load %s\n", file.c_str ());
		failures ++;
		return;
	}
	struct floorMakerView fmv = {
		&backdrop, floor, 1.0f, 1.0f, 1.0f,
		true, 300, 260,
		true, 60, 300, 420, 420
	};

	view v = startView (320, 240);
	setViewArea (0, 0, v.width, v.height, 2.0f);
	drawFloorMakerView (&fmv);
	check ("floormaker", v);

	setViewArea (200, -200, v.width, v.height, 1.0f);
	fmv.r = 0.4f;
	drawFloorMakerView (&fmv);
	check ("floormaker-scrolled", v);
	endView (v);

	noFloor (&floor);
	forgetSpriteBank (&backdrop);
}

// The sprite bank editor, with the sprite from fixtures/sprite.png, its
// hotspot at its bottom centre, without and with its box.
static void testSpriteBankEditor () {
	struct spriteBank sprites;
	memset (&sprites, 0, sizeof (sprites));
	sprites.type = 2;
	addSprite (0, &sprites);
	std::string file = testFolder + "/fixtures/sprite.png";
	if (! loadSpriteFromPNG (file.c_str (), &sprites, 0)) {
		printf ("FAIL spritebank: we can't load %s\n", file.c_str ());
		failures ++;
		return;
	}
	sprites.sprites[0].xhot = sprites.sprites[0].width / 2;
	sprites.sprites[0].yhot = sprites.sprites[0].height;

	view v = startView (160, 160);
	setSpriteViewArea (-80, 112, v.width, v.height, 1.0f);
	drawSpriteBankView (&sprites, 0, false);
	check ("spritebank", v);

	setSpriteViewArea (-80, 144, v.width, v.height, 0.5f);
	drawSpriteBankView (&sprites, 0, true);
	check ("spritebank-box", v);
	endView (v);

	forgetSpriteBank (&sprites);
}

// The z-buffer maker, with the z-buffers from fixtures/zbuffer.tga, without
// and with a selected z-buffer.
static void testZBufferMaker () {
	struct spriteBank backdrop;
	memset (&backdrop, 0, sizeof (backdrop));
	std::string file = testFolder + "/fixtures/zbuffer.tga";
	if (! loadZBufferFromTGA (file.c_str (), &backdrop)) {
		printf ("FAIL zbuffer: we can't load %s\n", file.c_str ());
		failures ++;
		return;
	}
	loadZTextures (&backdrop);

	view v = startView (200, 160);
	setViewArea (-20, -20, v.width, v.height, 1.0f);
	drawZBufferView (&backdrop, 0);
	check ("zbuffer", v);

	drawZBufferView (&backdrop, 2);
	check ("zbuffer-selected", v);
	endView (v);
}

int main (int argc, char ** argv) {
	testFolder = VIEW_TEST_FOLDER;
	bool compatibility = false;
	for (int i = 1; i < argc; i ++) {
		if (! strcmp (argv[i], "--update")) update = true;
		else if (! strcmp (argv[i], "--compatibility")) compatibility = true;
		else {
			fprintf (stderr, "Usage: view_test [--update] [--compatibility]\n");
			return 2;
		}
	}
	if (! createContext (compatibility)) return 1;

	testFloorMaker ();
	testSpriteBankEditor ();
	testZBufferMaker ();

	if (failures) printf ("%d tests failed\n", failures);
	return failures ? 1 : 0;
}
