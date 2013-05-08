/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmenudock.c
 * Copyright (C) 2001-2004 Michael Natterer <mitch@picman.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanlist.h"

#include "picmandialogfactory.h"
#include "picmandockable.h"
#include "picmandockbook.h"
#include "picmanmenudock.h"

#include "picman-intl.h"


#define DEFAULT_MINIMAL_WIDTH  200

struct _PicmanMenuDockPrivate
{
  gint make_sizeof_greater_than_zero;
};


static void   picman_menu_dock_style_set               (GtkWidget      *widget,
                                                      GtkStyle       *prev_style);


G_DEFINE_TYPE (PicmanMenuDock, picman_menu_dock, PICMAN_TYPE_DOCK)

#define parent_class picman_menu_dock_parent_class


static void
picman_menu_dock_class_init (PicmanMenuDockClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->style_set = picman_menu_dock_style_set;

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("minimal-width",
                                                             NULL, NULL,
                                                             0,
                                                             G_MAXINT,
                                                             DEFAULT_MINIMAL_WIDTH,
                                                             PICMAN_PARAM_READABLE));

  g_type_class_add_private (klass, sizeof (PicmanMenuDockPrivate));
}

static void
picman_menu_dock_init (PicmanMenuDock *dock)
{
}

static void
picman_menu_dock_style_set (GtkWidget *widget,
                          GtkStyle  *prev_style)
{
  gint minimal_width = -1;

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (widget,
                        "minimal-width", &minimal_width,
                        NULL);

  gtk_widget_set_size_request (widget, minimal_width, -1);
}

GtkWidget *
picman_menu_dock_new (void)
{
  return g_object_new (PICMAN_TYPE_MENU_DOCK, NULL);
}
