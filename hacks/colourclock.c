/* colourclock, Copyright (c) 2011 James McDonald <james@jamesmcdonald.com>
 *
 * Colour the screen with RGB values from the current time. i
 * red = hour, green = minute, blue = second
 * Display a clock and optionally an approximate HTML colour code.
 *
 * Based on greynetic and fps by Jamie Zawinski
 */

#include "screenhack.h"
#include <time.h>

struct state {
  Display *dpy;
  Window window;

  GC gc;
  int delay;
  unsigned long fg, bg;
  int xlim, ylim;
  Colormap cmap;
  XFontStruct *font;
};

static void *colourclock_init(Display *dpy, Window window)
{
  struct state *st = (struct state *)calloc(1, sizeof(*st));
  XGCValues gcv;
  XWindowAttributes xgwa;
  const char *font;
  XFontStruct *f;

  st->dpy = dpy;
  st->window = window;

  XGetWindowAttributes(st->dpy, st->window, &xgwa);
  st->xlim = xgwa.width;
  st->ylim = xgwa.height;
  st->cmap = xgwa.colormap;

  font = get_string_resource(dpy, "font", "Font");
  if(!font) font = "-*-courier-bold-r-normal-*-180-*";
  f = XLoadQueryFont (dpy, font);
  if (!f) f = XLoadQueryFont(dpy, "fixed");
  st->font = f;

  gcv.font = f->fid;
  gcv.foreground = st->fg = get_pixel_resource(st->dpy, st->cmap, "foreground","Foreground");
  st->gc = XCreateGC(st->dpy, st->window, GCFont|GCForeground, &gcv);

  st->delay = get_integer_resource(st->dpy, "delay", "Integer");
  if (st->delay < 0) st->delay = 0;
  
  return st;
}

static unsigned long colourclock_draw(Display *dpy, Window window,
				      void *closure)
{
  struct state *st = (struct state *)closure;
  XColor bgc;
  time_t now = time(0);
  struct tm *tm = localtime(&now);
  unsigned char cs[8], ts[9];
  unsigned short csw, tsw;

  bgc.flags = DoRed|DoGreen|DoBlue;
  bgc.red = (65535 * tm->tm_hour) / 23;
  bgc.green = (65535 * tm->tm_min) / 59;
  bgc.blue = (65535 * tm->tm_sec) / 59;

  XAllocColor(st->dpy, st->cmap, &bgc);
  XSetWindowBackground(dpy, window, bgc.pixel);
  XClearWindow(st->dpy, st->window);

  snprintf(cs, 8, "#%02x%02x%02x", bgc.red/256, bgc.green/256, bgc.blue/256);
  snprintf(ts, 9, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
  csw = XTextWidth(st->font, cs, 7);
  tsw = XTextWidth(st->font, ts, 8);

  XDrawString(st->dpy, st->window, st->gc, st->xlim/2-csw/2, 100, cs, 7);
  XDrawString(st->dpy, st->window, st->gc, st->xlim/2-tsw/2, 150, ts, 8);
  return st->delay;
}

static const char *colourclock_defaults[] = {
  ".foreground: white",
  "*delay:      1000000",
  0
};

static XrmOptionDescRec colourclock_options [] = {
  { "-delay", ".delay", XrmoptionSepArg, 0 },
  { 0, 0, 0, 0 }
};

static void colourclock_reshape(Display *dpy, Window window, void *closure,
				unsigned int w, unsigned int h)
{
  struct state *st = (struct state *)closure;
  st->xlim = w;
  st->ylim = h;
}

static Bool colourclock_event(Display *dpy, Window window, void *closure, XEvent *event)
{
  return False;
}

static void colourclock_free(Display *dpy, Window window, void *closure)
{
  struct state *st = (struct state *)closure;
  if(st->gc) XFreeGC(st->dpy, st->gc);
  if(st->font) XFreeFont(st->dpy, st->font);
  free(st);
}

XSCREENSAVER_MODULE("ColourClock", colourclock)
