#ifndef __SLUDGE_KEYS_H__
#define __SLUDGE_KEYS_H__

// SLUDGE key codes. A key that types a character has the code of that
// character. These are the codes of keys that type no character.
enum {
	KEY_TAB = 9,
	KEY_ENTER = 13,
	KEY_ESCAPE = 27,
	KEY_BACKSPACE = 127,
	KEY_UP = 63232,
	KEY_DOWN = 63233,
	KEY_LEFT = 63234,
	KEY_RIGHT = 63235,
	KEY_F1 = 63236,		// F2 to F12 follow F1.
	KEY_F12 = 63247,
	KEY_HOME = 63273,
	KEY_END = 63275,
	KEY_PAGE_UP = 63276,
	KEY_PAGE_DOWN = 63277,
};

// Return the name that we pass to games for a key that types no character,
// such as "ESCAPE" or "PAGE UP", or NULL for other keys.
const char * nameOfSpecialKey (int key);

// Return the code of a key from its name: the name of a key that types no
// character, "SPACE", or a single character. Return 0 for an unknown name.
// We ignore the case of names longer than one character.
int keyForName (const char * name);

#endif
