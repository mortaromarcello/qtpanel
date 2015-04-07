#include "compmgr.h"
#include "workspace.h"
#include "atoms.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>
#include <stdio.h>

typedef struct _ignore {
	struct _ignore* next;
	unsigned long sequence;
} ignore;

Display *dpy;
static int composite_opcode;
static ignore* ignore_head, **ignore_tail = &ignore_head;
static int render_event, render_error;
static int xfixes_event, xfixes_error;
static int damage_event, damage_error;

static void discard_ignore (Display *dpy, unsigned long sequence)
{
	while (ignore_head)
	{
		if ((long) (sequence - ignore_head->sequence) > 0)
	{
		ignore  *next = ignore_head->next;
		free (ignore_head);
		ignore_head = next;
		if (!ignore_head)
			ignore_tail = &ignore_head;
	}
	else
		break;
	}
}

static int should_ignore (Display *dpy, unsigned long sequence)
{
	discard_ignore (dpy, sequence);
	return ignore_head && ignore_head->sequence == sequence;
}

static int error (Display *dpy, XErrorEvent *ev)
{
	int o;
	const char* name = NULL;
	static char buffer[256];
	if (should_ignore (dpy, ev->serial))
		return 0;
	if (ev->request_code == composite_opcode && ev->minor_code == X_CompositeRedirectSubwindows)
	{
		fprintf (stderr, "Another composite manager is already running\n");
		exit (1);
	}

	o = ev->error_code - xfixes_error;
	switch (o) {
		case BadRegion: name = "BadRegion";	break;
		default: break;
	}
	o = ev->error_code - damage_error;
	switch (o) {
		case BadDamage: name = "BadDamage";	break;
		default: break;
	}
	o = ev->error_code - render_error;
	switch (o) {
		case BadPictFormat: name ="BadPictFormat"; break;
		case BadPicture: name ="BadPicture"; break;
		case BadPictOp: name ="BadPictOp"; break;
		case BadGlyphSet: name ="BadGlyphSet"; break;
		case BadGlyph: name ="BadGlyph"; break;
		default: break;
	}
	if (name == NULL)
	{
		buffer[0] = '\0';
		XGetErrorText (dpy, ev->error_code, buffer, sizeof (buffer));
		name = buffer;
	}
	fprintf (stderr, "error %d: %s request %d minor %d serial %lu\n",
		ev->error_code, (strlen (name) > 0) ? name : "unknown",
		ev->request_code, ev->minor_code, ev->serial);
/*		abort ();	    this is just annoying to most people */
	return 0;
}

static bool register_cm(Display* dpy)
{
	Window w;
	Atom a;
	a = XInternAtom (dpy, "_NET_WM_CM_S0", False);
	w = XGetSelectionOwner (dpy, a);
	if (w != None)
	{
		XTextProperty tp;
		char **strs;
		int count;
		Atom winNameAtom = XInternAtom (dpy, "_NET_WM_NAME", False);
		if (!XGetTextProperty (dpy, w, &tp, winNameAtom) && !XGetTextProperty (dpy, w, &tp, XA_WM_NAME))
		{
			fprintf (stderr, "Another composite manager is already running (0x%lx)\n", (unsigned long) w);
			return false;
		}
		if (XmbTextPropertyToTextList (dpy, &tp, &strs, &count) == Success)
		{
			fprintf (stderr, "Another composite manager is already running (%s)\n", strs[0]);
			XFreeStringList (strs);
		}
		XFree (tp.value);
		return false;
	}
	w = XCreateSimpleWindow (dpy, RootWindow (dpy, 0), 0, 0, 1, 1, 0, None, None);
	Xutf8SetWMProperties (dpy, w, "xcompmgr", "xcompmgr", NULL, 0, NULL, NULL, NULL);
	XSetSelectionOwner (dpy, a, w, 0);
	return true;
}

void CompMgr::run()
{
	dpy = XOpenDisplay(0);
	::createAtomList();
	XSetErrorHandler (error);
	if (!XRenderQueryExtension (dpy, &render_event, &render_error))
	{
		fprintf (stderr, "No render extension\n");
		return;
    }
	if (!XDamageQueryExtension (dpy, &damage_event, &damage_error))
	{
		fprintf (stderr, "No damage extension\n");
		return;
	}
	if (!XFixesQueryExtension (dpy, &xfixes_event, &xfixes_error))
	{
		fprintf (stderr, "No XFixes extension\n");
		return;
	}
	if (!register_cm(dpy))
		return;
	Workspace* workspace = Workspace::instance();
	qDebug("Composite Manager started.");
	while (!m_stop)
	{
		XEvent event;
		XNextEvent( dpy, &event );
		workspace->x11Event( &event );
	}
	qDebug("Composite Manager stopped.");
	delete workspace;
	XCloseDisplay (dpy);
}
