#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <assert.h>

#include "keys.h"

#define LENGTH(X) (sizeof(X) / sizeof(X)[0])

int
main()
{
	static struct {
		const char *str;
		ButtonSymbol r;
	} button_symbol_test[] = {
		{ "alt + super + ctrl + shift + left",
		  .r = { .mod = Mod1Mask | Mod4Mask | ControlMask | ShiftMask,
		         .button = Button1 } },
		{ "super + ctrl + shift + right",
		  .r = { .mod = Mod4Mask | ControlMask | ShiftMask,
		         .button = Button3 } },
		{ "ctrl + shift + left",
		  .r = { .mod = ControlMask | ShiftMask, .button = Button1 } },
		{ "shift + middle",
		  .r = { .mod = ShiftMask, .button = Button2 } },
		{ "left", .r = { .mod = 0, .button = Button1 } },
	};

	for (int i = 0; i < LENGTH(button_symbol_test); i++) {
		ButtonSymbol rr = parse_buttonsymbol(button_symbol_test[i].str);

		assert(rr.error == 0);
		assert(rr.mod == button_symbol_test[i].r.mod);
		assert(rr.button == button_symbol_test[i].r.button);
	}

	return 0;
}
