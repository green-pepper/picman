/*
 *   gtk-doc can't build libpicman-scan.c without PLUG_IN_INFO being defined
 *   so we include this file as a workaround
 */

#include <glib.h>
#include <libpicman/picman.h>

PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,
  NULL,
  NULL,
  NULL,
};
