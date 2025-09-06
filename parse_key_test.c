#include <X11/X.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <stdio.h>

#include "keys.h"

#define LENGTH(X) (sizeof (X) / sizeof (X)[0])

static struct
{
	const char *str;
	KeySymbol r;
} test_table[] = {
	{ "alt + super + ctrl + shift + p", .r = { .mod = Mod1Mask | Mod4Mask | ControlMask | ShiftMask , .sym = XK_p } },
	{ "super + ctrl + shift + p", .r = { .mod = Mod4Mask | ControlMask | ShiftMask , .sym = XK_p } },
	{ "ctrl + shift + p", .r = { .mod = ControlMask | ShiftMask , .sym = XK_p } },
	{ "shift + p", .r = { .mod = ShiftMask , .sym = XK_p } },
	{ "p", .r = { .mod = 0, .sym = XK_p } },
};

int
main()
{
	for(int i = 0; i < LENGTH(test_table); i++) {
		KeySymbol rr = parse_keysymbol(test_table[i].str);

		if(rr.error) {
			printf("%d: error\n", i);
			return 1;
		}

		if(rr.mod != test_table[i].r.mod) {
			printf("%d: wrong mod\n", i);
			return 1;
		}

		if(rr.sym != test_table[i].r.sym) {
			printf("%d: wrong sym\n", i);
			return 1;
		}
	}

	return 0;
}
