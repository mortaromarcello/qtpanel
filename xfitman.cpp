/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Christopher "VdoP" Regali
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */



#include <QtGui/QX11Info>
#include <QtCore/QList>
#include <QtGui/QApplication>
#include <QtCore/QDebug>
#include <QtGui/QDesktopWidget>

#include <stdint.h>
#include "xfitman.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xutil.h>
#include <assert.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <QtGui/QWidget>
#include <QtGui/QIcon>
/**
 * @file xfitman.cpp
 * @brief implements class Xfitman
 * @author Christopher "VdoP" Regali
 */

/*
 Some requests from Clients include type of the Client, for example the _NET_ACTIVE_WINDOW
 message. Currently the types can be 1 for normal applications, and 2 for pagers.
 See http://standards.freedesktop.org/wm-spec/latest/ar01s09.html#sourceindication
 */
#define SOURCE_NORMAL   1
#define SOURCE_PAGER    2

/*
  _NET_WM_STATE actions
 */
#define _NET_WM_STATE_REMOVE    0    // remove/unset property
#define _NET_WM_STATE_ADD       1    // add/set property
#define _NET_WM_STATE_TOGGLE    2    // toggle property


const XfitMan&  xfitMan()
{
    static XfitMan instance;
    return instance;
}

/**
 * @brief constructor: gets Display vars and registers us
 */
XfitMan::XfitMan()
{
    root = QX11Info::appRootWindow();
    display = XOpenDisplay("");
    screen = DefaultScreen(display);
}

/**
 * @brief moves a window to a new position
 */

void XfitMan::moveWindow(Window _win, int _x, int _y) const
{
    XMoveWindow(QX11Info::display(), _win, _x, _y);
}

/************************************************

 ************************************************/
bool XfitMan::getWindowProperty(Window window,
                       Atom atom,               // property
                       Atom reqType,            // req_type
                       unsigned long* resultLen,// nitems_return
                       unsigned char** result   // prop_return
                      ) const
{
    int  format;
    unsigned long type, rest;
    return XGetWindowProperty(QX11Info::display(), window, atom, 0, 4096, false,
                              reqType, &type, &format, resultLen, &rest,
                              result)  == Success;
}


/************************************************

 ************************************************/
bool XfitMan::getRootWindowProperty(Atom atom,    // property
                           Atom reqType,            // req_type
                           unsigned long* resultLen,// nitems_return
                           unsigned char** result   // prop_return
                          ) const
{
    return getWindowProperty(root, atom, reqType, resultLen, result);
}


/**
 * @brief this one gets the active application window.
 */
Window XfitMan::getActiveAppWindow() const
{
    Window window = getActiveWindow();
    if (window == 0)
        return 0;

    if (acceptWindow(window))
        return window;

    Window transFor = None;
    if (XGetTransientForHint(QX11Info::display(), window, &transFor))
        return transFor;

    return 0;
}

/**
 * @brief returns the window that currently has inputfocus
 */
Window XfitMan::getActiveWindow() const
{
    unsigned long len;
    unsigned long *data;
    if (!getWindowProperty(root, atom("_NET_ACTIVE_WINDOW"), XA_WINDOW,
                          &len, (unsigned char**) &data)
       )
        return 0;

    Window result = 0;
    if (len)
        result = data[0];

    XFree(data);
    return result;
}


/**
 * @brief Get the number of desktops
 */

int XfitMan::getNumDesktop() const
{
    unsigned long length, *data;
    getRootWindowProperty(atom("_NET_NUMBER_OF_DESKTOPS"), XA_CARDINAL, &length, (unsigned char**) &data);
    if (data)
    {
        int res = data[0];
        XFree(data);
        return res;
    }
    return 0;
}

QStringList XfitMan::getDesktopNames() const
{  
    QStringList ret;
    unsigned long length;
    unsigned char *data = 0;

    if (getRootWindowProperty(atom("_NET_DESKTOP_NAMES"), atom("UTF8_STRING"), &length, &data))
    {
        if (data)
        {
            char* c = (char*)data;
            char* end = (char*)data + length;
            while (c < end)
            {
                ret << QString::fromUtf8(c);
                c += strlen(c) + 1; // for trailing \0
            }

            XFree(data);
        }
    }


    return ret;
}


QString XfitMan::getDesktopName(int desktopNum, const QString &defaultName) const
{
    QStringList names = getDesktopNames();
    if (desktopNum<0 || desktopNum>names.count()-1)
        return defaultName;

    return names.at(desktopNum);
}

/**
 * @brief resizes a window to the given dimensions
 */
void XfitMan::resizeWindow(Window _wid, int _width, int _height) const
{
    XResizeWindow(QX11Info::display(), _wid, _width, _height);
}



/**
 * @brief gets a windowpixmap from a window
 */

bool XfitMan::getClientIcon(Window _wid, QPixmap& _pixreturn) const
{
    int format;
    ulong type, nitems, extra;
    ulong* data = 0;

    XGetWindowProperty(QX11Info::display(), _wid, atom("_NET_WM_ICON"),
                       0, LONG_MAX, False, AnyPropertyType,
                       &type, &format, &nitems, &extra,
                       (uchar**)&data);
    if (!data)
    {
        return false;
    }

    QImage img (data[0], data[1], QImage::Format_ARGB32);
    for (int i=0; i<img.byteCount()/4; ++i)
        ((uint*)img.bits())[i] = data[i+2];

    _pixreturn = QPixmap::fromImage(img);
    XFree(data);

    return true;
}

bool XfitMan::getClientIcon(Window _wid, QIcon *icon) const
{
    int format;
    ulong type, nitems, extra;
    ulong* data = 0;

    XGetWindowProperty(QX11Info::display(), _wid, atom("_NET_WM_ICON"),
                       0, LONG_MAX, False, AnyPropertyType,
                       &type, &format, &nitems, &extra,
                       (uchar**)&data);
    if (!data)
    {
        return false;
    }

    ulong* d = data;
    while (d < data + nitems)
    {
        QImage img (d[0], d[1], QImage::Format_ARGB32);
        d+=2;
        for (int i=0; i<img.byteCount()/4; ++i, ++d)
            ((uint*)img.bits())[i] = *d;

        icon->addPixmap(QPixmap::fromImage(img));
    }

    XFree(data);
    return true;
}



/**
 * @brief destructor
 */
XfitMan::~XfitMan()
{
}
/**
 * @brief returns a windowname and sets _nameSource to the finally used Atom
 */

//i got the idea for this from taskbar-plugin of LXPanel - so credits fly out :)
QString XfitMan::getWindowTitle(Window _wid) const
{
    QString name = "";
    //first try the modern net-wm ones
    unsigned long length;
    unsigned char *data = NULL;
    Atom utf8Atom = atom("UTF8_STRING");

    if (getWindowProperty(_wid, atom("_NET_WM_VISIBLE_NAME"), utf8Atom, &length, &data))
    {
        name = QString::fromUtf8((char*) data);
        XFree(data);

    }

    if (name.isEmpty())
    {
        if (getWindowProperty(_wid, atom("_NET_WM_NAME"), utf8Atom, &length, &data))
        {
            name = QString::fromUtf8((char*) data);
            XFree(data);
        }
    }

    if (name.isEmpty())
    {
        if (getWindowProperty(_wid, atom("XA_WM_NAME"), XA_STRING, &length, &data))
        {
            name = (char*) data;
            XFree(data);
        }
    }

    if (name.isEmpty())
    {
        Status ok = XFetchName(QX11Info::display(), _wid, (char**) &data);
        name = QString((char*) data);
        if (0 != ok) XFree(data);
    }

    if (name.isEmpty())
    {
        XTextProperty prop;
        if (XGetWMName(QX11Info::display(), _wid, &prop))
        {
            name = QString::fromUtf8((char*) prop.value);
            XFree(prop.value);
        }
    }

    return name;
}

QString XfitMan::getApplicationName(Window _wid) const
{
    XClassHint hint;
    QString ret;

    if (XGetClassHint(QX11Info::display(), _wid, &hint))
    {
        if (hint.res_name)
        {
            ret = hint.res_name;
            XFree(hint.res_name);
        }
        if (hint.res_class)
        {
            XFree(hint.res_class);
        }
    }

    return ret;
}

/**
 * @brief sends a clientmessage to a window
 */
int XfitMan::clientMessage(Window _wid, Atom _msg,
                            unsigned long data0,
                            unsigned long data1,
                            unsigned long data2,
                            unsigned long data3,
                            unsigned long data4) const
{
    XClientMessageEvent msg;
    msg.window = _wid;
    msg.type = ClientMessage;
    msg.message_type = _msg;
    msg.send_event = true;
    msg.display = QX11Info::display();
    msg.format = 32;
    msg.data.l[0] = data0;
    msg.data.l[1] = data1;
    msg.data.l[2] = data2;
    msg.data.l[3] = data3;
    msg.data.l[4] = data4;
    if (XSendEvent(QX11Info::display(), root, FALSE, (SubstructureRedirectMask | SubstructureNotifyMask) , (XEvent *) &msg) == Success)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}


/***********************************************

 ***********************************************/
WindowAllowedActions XfitMan::getAllowedActions(Window window) const
{
    WindowAllowedActions actions = { };

    unsigned long len;
    unsigned long *data;
    if (getWindowProperty(window, atom("_NET_WM_ALLOWED_ACTIONS"), XA_ATOM, &len, (unsigned char**) &data))
    {
        for (unsigned long i=0; i<len; ++i)
        {
            if (data[i] == atom("_NET_WM_ACTION_MOVE"))             actions.Move = true;            else
            if (data[i] == atom("_NET_WM_ACTION_RESIZE"))           actions.Resize = true;          else
            if (data[i] == atom("_NET_WM_ACTION_MINIMIZE"))         actions.Minimize = true;        else
            if (data[i] == atom("_NET_WM_ACTION_SHADE"))            actions.Shade = true;           else
            if (data[i] == atom("_NET_WM_ACTION_STICK"))            actions.Stick = true;           else
            if (data[i] == atom("_NET_WM_ACTION_MAXIMIZE_HORZ"))    actions.MaximizeHoriz = true;   else
            if (data[i] == atom("_NET_WM_ACTION_MAXIMIZE_VERT"))    actions.MaximizeVert = true;    else
            if (data[i] == atom("_NET_WM_ACTION_FULLSCREEN"))       actions.FullScreen = true;      else
            if (data[i] == atom("_NET_WM_ACTION_CHANGE_DESKTOP"))   actions.ChangeDesktop = true;   else
            if (data[i] == atom("_NET_WM_ACTION_CLOSE"))            actions.Close = true;           else
            if (data[i] == atom("_NET_WM_ACTION_ABOVE"))            actions.AboveLayer = true;      else
            if (data[i] == atom("_NET_WM_ACTION_BELOW"))            actions.BelowLayer = true;
        }
        XFree(data);
    }

    return actions;
}


WindowState XfitMan::getWindowState(Window window) const
{
    WindowState state = { };

    unsigned long len;
    unsigned long *data;
    if (getWindowProperty(window, atom("_NET_WM_STATE"), XA_ATOM, &len, (unsigned char**) &data))
    {
        for (unsigned long i=0; i<len; ++i)
        {
            if (data[i] == atom("_NET_WM_STATE_MODAL"))             state.Modal = true;             else
            if (data[i] == atom("_NET_WM_STATE_STICKY"))            state.Sticky = true;            else
            if (data[i] == atom("_NET_WM_STATE_MAXIMIZED_VERT"))    state.MaximizedVert = true;     else
            if (data[i] == atom("_NET_WM_STATE_MAXIMIZED_HORZ"))    state.MaximizedHoriz = true;    else
            if (data[i] == atom("_NET_WM_STATE_SHADED"))            state.Shaded = true;            else
            if (data[i] == atom("_NET_WM_STATE_SKIP_TASKBAR"))      state.SkipTaskBar = true;       else
            if (data[i] == atom("_NET_WM_STATE_SKIP_PAGER"))        state.SkipPager = true;         else
            if (data[i] == atom("_NET_WM_STATE_HIDDEN"))            state.Hidden = true;            else
            if (data[i] == atom("_NET_WM_STATE_FULLSCREEN"))        state.FullScreen = true;        else
            if (data[i] == atom("_NET_WM_STATE_ABOVE"))             state.AboveLayer = true;        else
            if (data[i] == atom("_NET_WM_STATE_BELOW"))             state.BelowLayer = true;        else
            if (data[i] == atom("_NET_WM_STATE_DEMANDS_ATTENTION")) state.Attention = true;
        }
        XFree(data);
    }

    return state;

}



/**
 * @brief returns true if a window has its hidden_flag set
 */

bool XfitMan::isHidden(Window _wid) const
{
    return getWindowState(_wid).Hidden;
}

Atom XfitMan::atom(const char* atomName)
{
    static QHash<QString, Atom> hash;

    if (hash.contains(atomName))
        return hash.value(atomName);

    Atom atom = XInternAtom(QX11Info::display(), atomName, false);
    hash[atomName] = atom;
    return atom;
}

AtomList XfitMan::getWindowType(Window window) const
{
    AtomList result;

    unsigned long length, *data;
    length=0;
    if (!getWindowProperty(window, atom("_NET_WM_WINDOW_TYPE"), (Atom)AnyPropertyType, &length, (unsigned char**) &data))
        return result;

    for (unsigned int i = 0; i < length; i++)
        result.append(data[i]);

    XFree(data);
    return result;
}


/**
 * @brief rejects a window from beeing listed
 */
bool XfitMan::acceptWindow(Window window) const
{
    {
        AtomList types = getWindowType(window);
        AtomList ignoreList;
        ignoreList  << atom("_NET_WM_WINDOW_TYPE_DESKTOP")
                    << atom("_NET_WM_WINDOW_TYPE_DOCK")
                    << atom("_NET_WM_WINDOW_TYPE_SPLASH")
                    << atom("_NET_WM_WINDOW_TYPE_TOOLBAR")
                    << atom("_NET_WM_WINDOW_TYPE_MENU")
                    // for qlipper - using qpopup as a main window
                    << atom("_NET_WM_WINDOW_TYPE_POPUP_MENU");
        // issue #284: qmmp its not registered in window list panel
        // qmmp has _KDE_NET_WM_WINDOW_TYPE_OVERRIDE in its
        // _NET_WM_WINDOW_TYPE(ATOM) = _KDE_NET_WM_WINDOW_TYPE_OVERRIDE, _NET_WM_WINDOW_TYPE_NORMAL
        // Let's expect that _KDE_NET_WM_WINDOW_TYPE_OVERRIDE can be set for
        // regular windows too. If it should be hidden we should expect
        // one of atoms listed above.
//                    << atom("_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");

        foreach (Atom i, ignoreList)
        {
            if (types.contains(i))
                return false;
        }

        WindowState state = getWindowState(window);
        if (state.SkipTaskBar)  return false;
    }

    Window transFor = None;
    // WM_TRANSIENT_FOR hint not set - normal window
    if (!XGetTransientForHint(QX11Info::display(), window, &transFor))
        return true;

    if (transFor == 0)      return true;
    if (transFor == window) return true;
    if (transFor == root)   return true;

     AtomList transForTypes = getWindowType(transFor);
     return !transForTypes.contains(atom("_NET_WM_WINDOW_TYPE_NORMAL"));
}



/**
 * @brief gets a client list
 */
WindowList XfitMan::getClientList() const
{
    //initialize the parameters for the XGetWindowProperty call
    unsigned long length, *data;
    length=0;

    /**
     * @todo maybe support multiple desktops here!
     */
    QList<Window> output;

    if (getRootWindowProperty(atom("_NET_CLIENT_LIST"), (Atom)AnyPropertyType, &length,  (unsigned char**) &data))
    {
        for (unsigned int i = 0; i < length; i ++)
            output.append(data[i]);
        XFree(data);
    }

    return output;
}


/**
 * @brief returns the current desktop
 */
int XfitMan::getActiveDesktop() const
{
    int res = -2;
    unsigned long length, *data;
    if (getRootWindowProperty(atom("_NET_CURRENT_DESKTOP"), XA_CARDINAL, &length, (unsigned char**) &data))
    {
        if (data)
        {
            res = data[0];
            XFree(data);
        }
    }

    return res;
}


/**
 * @brief gets the desktop of the windows _wid
 */
int XfitMan::getWindowDesktop(Window _wid) const
{
    int  res = -1;
    unsigned long length, *data;
    // so we try to use net_wm_desktop first, but if the
    // system does not use net_wm standard we use win_workspace!
    if (getWindowProperty(_wid, atom("_NET_WM_DESKTOP"), XA_CARDINAL, &length, (unsigned char**) &data))
    {
        if (!data)
            return res;
        res = data[0];
        XFree(data);
    }
    else
    {
        if (getWindowProperty(_wid, atom("_WIN_WORKSPACE"), XA_CARDINAL, &length, (unsigned char**) &data))
        {
            if (!data)
                return res;
            res = data[0];
            XFree(data);
        }
    }

    return res;
}


/**
 * @brief moves a window to a specified desktop
 */

void XfitMan::moveWindowToDesktop(Window _wid, int _display) const
{
    WMState state = getWMState(_wid);
    if(state != WMStateWithdrawn)
        clientMessage(_wid, atom("_NET_WM_DESKTOP"), (unsigned long) _display,0,0,0,0);
    else
    {
        long desktop = _display;
        XChangeProperty(QX11Info::display(), _wid, XfitMan::atom("_NET_WM_DESKTOP"), XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char*)&desktop, 1);
    }
}


/**
 * @brief raises windows _wid
 */
void XfitMan::raiseWindow(Window _wid) const
{
    clientMessage(_wid, atom("_NET_ACTIVE_WINDOW"),
                  SOURCE_PAGER);
}


/************************************************

 ************************************************/
void XfitMan::minimizeWindow(Window _wid) const
{
    clientMessage(_wid, atom("WM_CHANGE_STATE"),
                  IconicState);
}


/************************************************

 ************************************************/
void XfitMan::maximizeWindow(Window _wid, MaximizeDirection direction) const
{
    Atom atom1 = 0, atom2= 0;
    switch (direction)
    {
        case MaximizeHoriz:
            atom1 = atom("_NET_WM_STATE_MAXIMIZED_HORZ");
            break;

        case MaximizeVert:
            atom1 = atom("_NET_WM_STATE_MAXIMIZED_VERT");
            break;

        case MaximizeBoth:
            atom1 = atom("_NET_WM_STATE_MAXIMIZED_VERT");
            atom2 = atom("_NET_WM_STATE_MAXIMIZED_HORZ");
            break;

    }

    clientMessage(_wid, atom("_NET_WM_STATE"),
                  _NET_WM_STATE_ADD,
                  atom1, atom2,
                  SOURCE_PAGER);
    raiseWindow(_wid);
}


/************************************************

 ************************************************/
void XfitMan::deMaximizeWindow(Window _wid) const
{
    clientMessage(_wid, atom("_NET_WM_STATE"),
                  _NET_WM_STATE_REMOVE,
                  atom("_NET_WM_STATE_MAXIMIZED_VERT"),
                  atom("_NET_WM_STATE_MAXIMIZED_HORZ"),
                  SOURCE_PAGER);
}

/************************************************

 ************************************************/
void XfitMan::shadeWindow(Window _wid, bool shade) const
{
    clientMessage(_wid, atom("_NET_WM_STATE"),
                  shade ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE,
                  atom("_NET_WM_STATE_SHADED"),
                  0,
                  SOURCE_PAGER);
}


/************************************************

 ************************************************/
void XfitMan::closeWindow(Window _wid) const
{
    clientMessage(_wid, atom("_NET_CLOSE_WINDOW"),
                  0, // Timestamp
                  SOURCE_PAGER);
}


/************************************************

 ************************************************/
void XfitMan::setWindowLayer(Window _wid, XfitMan::Layer layer) const
{
    ulong aboveAction = (layer == LayerAbove) ?
        _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;

    ulong belowAction = (layer == LayerBelow) ?
        _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;

    clientMessage(_wid, atom("_NET_WM_STATE"),
                  aboveAction,
                  atom("_NET_WM_STATE_ABOVE"),
                  0,
                  SOURCE_PAGER);

    clientMessage(_wid, atom("_NET_WM_STATE"),
                  belowAction,
                  atom("_NET_WM_STATE_BELOW"),
                  0,
                  SOURCE_PAGER);
}

/**
 * @brief changes active desktop to _desktop
 */
void XfitMan::setActiveDesktop(int _desktop) const
{
    clientMessage(root, atom("_NET_CURRENT_DESKTOP"), (unsigned long) _desktop,0,0,0,0);
}

/**
 * @brief sets net_wm_strut_partial = our reserved panelspace for the mainbar!
 */
void XfitMan::setStrut(Window _wid,
                       int left, int right,
                       int top,  int bottom,

                       int leftStartY,   int leftEndY,
                       int rightStartY,  int rightEndY,
                       int topStartX,    int topEndX,
                       int bottomStartX, int bottomEndX
                       ) const
{
    //qDebug() << "XfitMan: Trying to set STRUT_PARTIAL for panel!";
    unsigned long desstrut[12];
    memset(desstrut,0,sizeof(desstrut));
    //prepare the array
    //it has format:
    /*
     * left, right, top, bottom, left_start_y, left_end_y,
     * right_start_y, right_end_y, top_start_x, top_end_x, bottom_start_x,
     * bottom_end_x
     *
     */

    //so we take our panelsize from the bottom up
    desstrut[0] = left; desstrut[1] = right;
    desstrut[2] = top;  desstrut[3] = bottom;

    desstrut[4] = leftStartY;    desstrut[5] = leftEndY;
    desstrut[6] = rightStartY;   desstrut[7] = rightEndY;
    desstrut[8] = topStartX;     desstrut[9] = topEndX;
    desstrut[10] = bottomStartX; desstrut[11] = bottomEndX;

    //now we can change that property right
    XChangeProperty(QX11Info::display(), _wid , atom("_NET_WM_STRUT_PARTIAL"),
                    XA_CARDINAL, 32, PropModeReplace,  (unsigned char *) desstrut, 12  );

    //now some wm do not support net_wm_strut_partial but only net_wm_strut, so we also
    // send that one too xdg-std says: if you get a strut_partial ignore all following
    // struts! so if this msg is recognized its ok if not, we dont care either

    XChangeProperty(QX11Info::display(), _wid, atom("_NET_WM_STRUT"),
                    XA_CARDINAL, 32, PropModeReplace, (unsigned char*) desstrut, 4);
}


#ifdef DEBUG
/************************************************

 ************************************************/
QString XfitMan::debugWindow(Window wnd)
{
    if (!wnd)
        return QString("[%1]").arg(wnd,8, 16);

    QString typeStr;
    int  format;
    unsigned long type, length, rest, *data;
    length=0;
    if (XGetWindowProperty(QX11Info::display(), wnd, XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                           0, 4096, FALSE, AnyPropertyType, &type, &format,
                           &length, &rest,(unsigned char**) &data) == Success)
    {
        for (unsigned int i = 0; i < length; i++)
        {
            char* aname = XGetAtomName(QX11Info::display(), data[i]);
            typeStr = typeStr + " " + aname;
            XFree(aname);
        }
    }
    else
        typeStr ="ERROR";

    return QString("[%1] %2 %3").arg(wnd,8, 16).arg(xfitMan().getWindowTitle(wnd)).arg(typeStr);
}
#endif


/************************************************

 ************************************************/
const QRect XfitMan::availableGeometry(int screen) const
{
    QDesktopWidget *d = QApplication::desktop();

    if (screen < 0 || screen >= d->screenCount())
        screen = d->primaryScreen();

    QRect available = d->screenGeometry(screen);

    // Iterate over all the client windows and subtract from the available
    // area the space they reserved on the edges (struts).
    // Note: _NET_WORKAREA is not reliable as it exposes only one
    // rectangular area spanning all screens.
    Display *display = QX11Info::display();
    int x11Screen = d->isVirtualDesktop() ? DefaultScreen(display) : screen;

    Atom ret;
    int format, status;
    uchar* data = 0;
    ulong nitems, after;

    status = XGetWindowProperty(display, QX11Info::appRootWindow(x11Screen),
                                atom("_NET_CLIENT_LIST"), 0L, ~0L, False, XA_WINDOW,
                                &ret, &format, &nitems, &after, &data);

    if (status == Success && ret == XA_WINDOW && format == 32 && nitems)
    {
        const QRect desktopGeometry = d->rect();

        Window* xids = (Window*) data;
        for (quint32 i = 0; i < nitems; ++i)
        {
            ulong nitems2;
            uchar* data2 = 0;
            status = XGetWindowProperty(display, xids[i],
                                        atom("_NET_WM_STRUT_PARTIAL"), 0, 12, False, XA_CARDINAL,
                                        &ret, &format, &nitems2, &after, &data2);

            if (status == Success && ret == XA_CARDINAL && format == 32 && nitems2 == 12)
            {
                ulong* struts = (ulong*) data2;

                QRect left(desktopGeometry.x(),
                           desktopGeometry.y() + struts[4],
                           struts[0],
                           struts[5] - struts[4]);
                if (available.intersects(left))
                    available.setX(left.width());

                QRect right(desktopGeometry.x() + desktopGeometry.width() - struts[1],
                            desktopGeometry.y() + struts[6],
                            struts[1],
                            struts[7] - struts[6]);
                if (available.intersects(right))
                    available.setWidth(right.x() - available.x());

                QRect top(desktopGeometry.x() + struts[8],
                          desktopGeometry.y(),
                          struts[9] - struts[8],
                          struts[2]);
                if (available.intersects(top))
                    available.setY(top.height());

                QRect bottom(desktopGeometry.x() + struts[10],
                             desktopGeometry.y() + desktopGeometry.height() - struts[3],
                             struts[11] - struts[10],
                             struts[3]);
                if (available.intersects(bottom))
                    available.setHeight(bottom.y() - available.y());
            }
            if (data2)
                XFree(data2);
        }
    }
    if (data)
        XFree(data);

    return available;
}


/************************************************

 ************************************************/
const QRect XfitMan::availableGeometry(const QWidget *widget) const
{
    if (!widget)
    {
        qWarning("XfitMan::availableGeometry(): Attempt "
                 "to get the available geometry of a null widget");
        return QRect();
    }

    return availableGeometry(QApplication::desktop()->screenNumber(widget));
}


/************************************************

 ************************************************/
const QRect XfitMan::availableGeometry(const QPoint &point) const
{
    return availableGeometry(QApplication::desktop()->screenNumber(point));
}


/************************************************
 The Window Manager MUST set this property on the root window to be the ID of a child
 window created by himself, to indicate that a compliant window manager is active.

 http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2550836
 ************************************************/
bool XfitMan::isWindowManagerActive() const
{
    //Window *wins;

    //getRootWindowProperty(atom("_NET_SUPPORTING_WM_CHECK"), XA_WINDOW, &length, (unsigned char**) &wins);

    Atom type;
    unsigned long length;
    Window *wins;
    int format;
    unsigned long after;

    XGetWindowProperty(QX11Info::display(), root, atom("_NET_SUPPORTING_WM_CHECK"),
                       0, LONG_MAX,
                       false, XA_WINDOW, &type, &format, &length,
                       &after, (unsigned char **)&wins);

    if ( type == XA_WINDOW && length > 0 && wins[0] != None )
    {
        XFree(wins);
        return true;
    }
    return false;
}

bool XfitMan::getShowingDesktop() const
{
    bool show = false;
    unsigned long resultLen;
    unsigned char* result = NULL;
    if(getRootWindowProperty(atom("_NET_SHOWING_DESKTOP"), XA_CARDINAL, &resultLen, &result))
    {
        show = *reinterpret_cast<long*>(result) ? true : false;
        if(result)
            XFree(result);
    }
    return show;
}

void XfitMan::setShowingDesktop(bool show) const
{
    clientMessage(QX11Info::appRootWindow(), atom("_NET_SHOWING_DESKTOP"), show ? 1 : 0);
}

void XfitMan::setIconGeometry(Window _wid, QRect* rect) const
{
    Atom net_wm_icon_geometry = atom("_NET_WM_ICON_GEOMETRY");
    if(!rect)
        XDeleteProperty(QX11Info::display(), _wid, net_wm_icon_geometry);
    else
    {
        long data[4];
        data[0] = rect->x();
        data[1] = rect->y();
        data[2] = rect->width();
        data[3] = rect->height();
        XChangeProperty(QX11Info::display(), _wid, net_wm_icon_geometry,
                        XA_CARDINAL, 32, PropModeReplace, (unsigned char*)data, 4);
    }
}

XfitMan::WMState XfitMan::getWMState(Window _wid) const
{
  WMState state = WMStateWithdrawn;
    Atom wm_state = atom("WM_STATE");
    unsigned long resultLen;
    unsigned char* result = NULL;
    if(getWindowProperty(_wid, wm_state, wm_state, &resultLen, &result))
    {
        if(result)
        {
            state = WMState(*((long*)result));
            XFree(result);
        }
    }
    return state;
}

Atom XfitMan::getAtomByName(const QString& name) const
{
    QByteArray ba = name.toLocal8Bit();
    const char* str = ba.data();
    return XInternAtom(display, str, True);
}

Pixmap XfitMan::getRootPixmap() const
{
    Pixmap ret = None;
    Atom id = getAtomByName("_XROOTPMAP_ID");
    if (id) {
        int  act_format, c = 2 ;
        u_long  nitems ;
        u_long  bytes_after ;
        u_char *prop = NULL;
        Atom ret_type;
        do {
            if (XGetWindowProperty(display, root, id, 0, 1,
                      False, XA_PIXMAP, &ret_type, &act_format,
                      &nitems, &bytes_after, &prop) == Success) {
                if (ret_type == XA_PIXMAP) {
                    ret = *((Pixmap *)prop);
                    XFree(prop);
                    break;
                }
            }
        } while (--c > 0);
    }
    return ret;
}
