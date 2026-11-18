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

static int ocx, ocy, nw, nh;
static Client *c;
static Monitor *m;
static Time lasttime = 0;
static int x, y;
	
static void motion_notify(XEvent *ev);
static void key_press(XEvent *ev);
static void button_release(XEvent *ev);

XEventHandlerList resize_mouse_state = {
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
resizemouse(const Arg *arg)
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
	handler = &resize_mouse_state;
}

void
motion_notify(XEvent *ev)
{
	if ((ev->xmotion.time - lasttime) <=
	    (1000 / currentconfig->lsettings.refreshrate)) {
		return;
	}
	lasttime = ev->xmotion.time;

	nw = MAX(ev->xmotion.x - ocx - 2 * c->bw + 1, 1);
	nh = MAX(ev->xmotion.y - ocy - 2 * c->bw + 1, 1);
	if (c->mon->wx + nw >= selmon->wx &&
	    c->mon->wx + nw <= selmon->wx + selmon->ww &&
	    c->mon->wy + nh >= selmon->wy &&
	    c->mon->wy + nh <= selmon->wy + selmon->wh) {
		if (!c->isfloating &&
		    selmon->lt[selmon->sellt]->arrange &&
		    (abs(nw - c->w) >
		             currentconfig->appearance.snap ||
		     abs(nh - c->h) >
		             currentconfig->appearance.snap)) {
			togglefloating(NULL);
		}
	}
	if (!selmon->lt[selmon->sellt]->arrange ||
	    c->isfloating) {
		resize(c, c->x, c->y, nw, nh, 1);
	}
}

void
button_release(XEvent *ev)
{
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1,
	             c->h + c->bw - 1);
	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, ev))
		;
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
