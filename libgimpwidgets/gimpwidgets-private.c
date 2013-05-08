/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanwidgets-private.c
 * Copyright (C) 2003 Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"

#include "picmanwidgetstypes.h"

#include "picmanstock.h"
#include "picmanwidgets-private.h"

#include "libpicman/libpicman-intl.h"

#include "picman-wilber-pixbufs.h"


PicmanHelpFunc          _picman_standard_help_func  = NULL;
PicmanGetColorFunc      _picman_get_foreground_func = NULL;
PicmanGetColorFunc      _picman_get_background_func = NULL;
PicmanEnsureModulesFunc _picman_ensure_modules_func = NULL;


static void
picman_widgets_init_foreign_enums (void)
{
  static const PicmanEnumDesc input_mode_descs[] =
  {
    { GDK_MODE_DISABLED, NC_("input-mode", "Disabled"), NULL },
    { GDK_MODE_SCREEN,   NC_("input-mode", "Screen"),   NULL },
    { GDK_MODE_WINDOW,   NC_("input-mode", "Window"),   NULL },
    { 0, NULL, NULL }
  };

  picman_type_set_translation_domain (GDK_TYPE_INPUT_MODE,
                                    GETTEXT_PACKAGE "-libpicman");
  picman_type_set_translation_context (GDK_TYPE_INPUT_MODE, "input-mode");
  picman_enum_set_value_descriptions (GDK_TYPE_INPUT_MODE, input_mode_descs);
}

void
picman_widgets_init (PicmanHelpFunc          standard_help_func,
                   PicmanGetColorFunc      get_foreground_func,
                   PicmanGetColorFunc      get_background_func,
                   PicmanEnsureModulesFunc ensure_modules_func)
{
  static gboolean  picman_widgets_initialized = FALSE;

  GdkPixbuf *pixbuf;
  GList     *icon_list = NULL;
  gint       i;

  const guint8 *inline_pixbufs[] =
  {
    wilber_64,
    wilber_48,
    wilber_32,
    wilber_16
  };

  g_return_if_fail (standard_help_func != NULL);

  if (picman_widgets_initialized)
    g_error ("picman_widgets_init() must only be called once!");

  _picman_standard_help_func  = standard_help_func;
  _picman_get_foreground_func = get_foreground_func;
  _picman_get_background_func = get_background_func;
  _picman_ensure_modules_func = ensure_modules_func;

  picman_stock_init ();

  for (i = 0; i < G_N_ELEMENTS (inline_pixbufs); i++)
    {
      pixbuf = gdk_pixbuf_new_from_inline (-1, inline_pixbufs[i], FALSE, NULL);
      icon_list = g_list_prepend (icon_list, pixbuf);
    }

  gtk_window_set_default_icon_list (icon_list);

  g_list_free_full (icon_list, (GDestroyNotify) g_object_unref);

  picman_widgets_init_foreign_enums ();

  picman_widgets_initialized = TRUE;
}
