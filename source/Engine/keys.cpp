#include <SDL3/SDL_stdinc.h>

#include "keys.h"

static const struct {
	int key;
	const char * name;
} specialKeys[] = {
	{ KEY_BACKSPACE, "BACKSPACE" },
	{ KEY_TAB, "TAB" },
	{ KEY_ENTER, "ENTER" },
	{ KEY_ESCAPE, "ESCAPE" },
	{ KEY_PAGE_UP, "PAGE UP" },
	{ KEY_PAGE_DOWN, "PAGE DOWN" },
	{ KEY_END, "END" },
	{ KEY_HOME, "HOME" },
	{ KEY_LEFT, "LEFT" },
	{ KEY_UP, "UP" },
	{ KEY_RIGHT, "RIGHT" },
	{ KEY_DOWN, "DOWN" },
	{ KEY_F1, "F1" },
	{ KEY_F1 + 1, "F2" },
	{ KEY_F1 + 2, "F3" },
	{ KEY_F1 + 3, "F4" },
	{ KEY_F1 + 4, "F5" },
	{ KEY_F1 + 5, "F6" },
	{ KEY_F1 + 6, "F7" },
	{ KEY_F1 + 7, "F8" },
	{ KEY_F1 + 8, "F9" },
	{ KEY_F1 + 9, "F10" },
	{ KEY_F1 + 10, "F11" },
	{ KEY_F12, "F12" },
};

const char * nameOfSpecialKey (int key) {
	for (unsigned i = 0; i < SDL_arraysize (specialKeys); i ++) {
		if (specialKeys[i].key == key) return specialKeys[i].name;
	}
	return NULL;
}

int keyForName (const char * name) {
	if (! name[0]) return 0;
	if (! name[1]) return (unsigned char) name[0];
	if (SDL_strcasecmp (name, "SPACE") == 0) return ' ';
	for (unsigned i = 0; i < SDL_arraysize (specialKeys); i ++) {
		if (SDL_strcasecmp (specialKeys[i].name, name) == 0) return specialKeys[i].key;
	}
	return 0;
}
