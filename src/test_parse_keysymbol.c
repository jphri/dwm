#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <assert.h>
#include <stdio.h>

#include "keys.h"

#define LENGTH(X) (sizeof(X) / sizeof(X)[0])

int
main()
{
	static struct {
		const char *str;
		KeySymbol r;
	} test_table[] = {
		{ "alt + super + ctrl + shift + p",
		  .r = { .mod = Mod1Mask | Mod4Mask | ControlMask | ShiftMask,
		         .sym = XK_p } },
		{ "super + ctrl + shift + p",
		  .r = { .mod = Mod4Mask | ControlMask | ShiftMask,
		         .sym = XK_p } },
		{ "ctrl + shift + p",
		  .r = { .mod = ControlMask | ShiftMask, .sym = XK_p } },
		{ "shift + p", .r = { .mod = ShiftMask, .sym = XK_p } },
		{ "p", .r = { .mod = 0, .sym = XK_p } },
	};

	for (int i = 0; i < LENGTH(test_table); i++) {
		KeySymbol rr = parse_keysymbol(test_table[i].str);

		assert(rr.error == 0);
		assert(rr.mod == test_table[i].r.mod);
		assert(rr.sym == test_table[i].r.sym);
	}

	return 0;
}
