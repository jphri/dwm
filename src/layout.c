#include <X11/X.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

#include "dat.h"
#include "fns.h"

void
updatelayout(void)
{
	Atom utf8string = XInternAtom(dpy, "UTF8_STRING", False);

	char *buffer = NULL;
	int buffersize = 0;
	Monitor *m;

	for(m = mons; m; m = m->next) {
		int len = strlen(m->lt[m->sellt]->name);
		buffer = realloc(buffer, buffersize + len + 1);
		if(!buffer)
			die("realloc()");
		memcpy(buffer + buffersize, m->lt[m->sellt]->name, len);
		buffersize += len;
		buffer[buffersize] = '\0';
		buffersize += 1;
	}

	buffer = realloc(buffer, buffersize + 1);
	if(!buffer)
		die("realloc()");
	buffer[buffersize] = '\0';
	buffersize += 1;

	XChangeProperty(dpy, root, 
			dwmatom[DwmLayout], utf8string, 8, PropModeReplace, (unsigned char*)buffer, buffersize);

	free(buffer);
}
