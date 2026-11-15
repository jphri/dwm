#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

#include "dat.h"
#include "fns.h"

void
monocle(Monitor *m)
{
	unsigned int n = 0;
	Client *c;

	for (c = m->clients; c; c = c->next) {
		if (ISVISIBLE(c)) {
			n++;
		}
	}
	if (n > 0) { /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	}

	for (c = nexttiled(m->clients); c; c = nexttiled(c->next)) {
		resize_arrange(c, m->wx + currentconfig->appearance.gappx,
		               m->wy + currentconfig->appearance.gappx,
		               m->ww - 2 * currentconfig->appearance.gappx,
		               m->wh - 2 * currentconfig->appearance.gappx);
	}
}
