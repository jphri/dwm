
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
updatetags(void)
{
	int moncount, i;
	unsigned int *tags;
	Monitor *m;

	moncount = 0;
	for(m = mons; m; m = m->next)
		moncount++;

	tags = malloc(sizeof(*tags) * moncount);
	if(!tags) 
		die("malloc");

	for(m = mons, i = 0; m; i++, m = m->next)
		tags[i] = m->tagset[m->seltags];

	XChangeProperty(dpy, root, 
			dwmatom[DwmTags], XA_CARDINAL, 32, PropModeReplace, (unsigned char*)tags, moncount);

	free(tags);
}
