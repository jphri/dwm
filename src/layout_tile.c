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
tile(Monitor *m)
{
	unsigned int i, n, h, mw, my, ty;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++)
		;
	if (n == 0) {
		return;
	}

	if (n > m->nmaster) {
		mw = m->nmaster ? m->ww * m->mfact : 0;
	} else {
		mw = m->ww;
	}
	for (i = my = ty = 0, c = nexttiled(m->clients); c;
	     c = nexttiled(c->next), i++) {
		if (i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i) -
			    2 * currentconfig->appearance.gappx;

			resize_arrange(
				c, m->wx + currentconfig->appearance.gappx,
				m->wy + my + currentconfig->appearance.gappx,
				mw - 2 * currentconfig->appearance.gappx, h);

			if (my + HEIGHT(c) < m->wh) {
				my += HEIGHT(c) +
				      2 * currentconfig->appearance.gappx;
			}
		} else {
			h = (m->wh - ty) / (n - i) -
			    2 * currentconfig->appearance.gappx;

			resize_arrange(
				c, m->wx + mw + currentconfig->appearance.gappx,
				m->wy + ty + currentconfig->appearance.gappx,
				m->ww - mw -
					2 * currentconfig->appearance.gappx,
				h);

			if (ty + HEIGHT(c) < m->wh) {
				ty += HEIGHT(c) +
				      2 * currentconfig->appearance.gappx;
			}
		}
	}
}

Layout layout_tile = {
	.name = "tile",
	.symbol = "[]=",
	.arrange = tile
};
