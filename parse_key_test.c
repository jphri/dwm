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

static struct
{
	const char *str;
	ButtonSymbol r;
} button_symbol_test[] = {
	{ "alt + super + ctrl + shift + left", .r = { .mod = Mod1Mask | Mod4Mask | ControlMask | ShiftMask , .button = Button1 } },
	{ "super + ctrl + shift + right", .r = { .mod = Mod4Mask | ControlMask | ShiftMask , .button = Button3 } },
	{ "ctrl + shift + left", .r = { .mod = ControlMask | ShiftMask , .button = Button1 } },
	{ "shift + middle", .r = { .mod = ShiftMask , .button = Button2 } },
	{ "left", .r = { .mod = 0, .button = Button1 } },
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

	for(int i = 0; i < LENGTH(button_symbol_test); i++) {
		ButtonSymbol rr = parse_buttonsymbol(button_symbol_test[i].str);

		if(rr.error) {
			printf("%d: error\n", i);
			return 1;
		}

		if(rr.mod != button_symbol_test[i].r.mod) {
			printf("%d: wrong mod\n", i);
			return 1;
		}

		if(rr.button != button_symbol_test[i].r.button) {
			printf("%d: wrong button\n", i);
			return 1;
		}
	}

	return 0;
}
