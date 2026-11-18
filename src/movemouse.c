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
	
static int x, y, ocx, ocy, nx, ny;
static Client *c;
static Monitor *m;
static Time lasttime = 0;

static void motion_notify(XEvent *ev);
static void key_press(XEvent *ev);
static void button_release(XEvent *ev);

XEventHandlerList move_mouse_state = {
	[ButtonPress] = buttonpress,
	[ButtonRelease] = button_release,
	[ClientMessage] = clientmessage,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = key_press,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motion_notify,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};

void
movemouse(const Arg *arg)
{
	if (!(c = selmon->sel)) {
		return;
	}
	if (c->isfullscreen) { /* no support moving fullscreen windows by mouse
		                */
		return;
	}
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync,
	                 GrabModeAsync, None, cursor[CurMove]->cursor,
	                 CurrentTime) != GrabSuccess) {
		return;
	}
	if (!getrootptr(&x, &y)) {
		return;
	}

	handler = &move_mouse_state;
}

void
motion_notify(XEvent *ev)
{
	if ((ev->xmotion.time - lasttime) <=
	    (1000 / currentconfig->lsettings.refreshrate)) {
	    return;
	}
	lasttime = ev->xmotion.time;

	nx = ocx + (ev->xmotion.x - x);
	ny = ocy + (ev->xmotion.y - y);
	if (abs(selmon->wx - nx) <
	    currentconfig->appearance.snap) {
		nx = selmon->wx;
	} else if (abs((selmon->wx + selmon->ww) -
	               (nx + WIDTH(c))) <
	           currentconfig->appearance.snap) {
		nx = selmon->wx + selmon->ww - WIDTH(c);
	}
	if (abs(selmon->wy - ny) <
	    currentconfig->appearance.snap) {
		ny = selmon->wy;
	} else if (abs((selmon->wy + selmon->wh) -
	               (ny + HEIGHT(c))) <
	           currentconfig->appearance.snap) {
		ny = selmon->wy + selmon->wh - HEIGHT(c);
	}
	if (!c->isfloating &&
	    selmon->lt[selmon->sellt]->arrange &&
	    (abs(nx - c->x) > currentconfig->appearance.snap ||
	     abs(ny - c->y) > currentconfig->appearance.snap)) {
		togglefloating(NULL);
	}
	if (!selmon->lt[selmon->sellt]->arrange ||
	    c->isfloating) {
		resize(c, nx, ny, c->w, c->h, 1);
	}
}

void
button_release(XEvent *ev)
{
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
	handler = &idle_state;
}

void
key_press(XEvent *ev)
{
	(void)ev;
}
