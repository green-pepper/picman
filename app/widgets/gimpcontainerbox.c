/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainerbox.c
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontainer.h"
#include "core/picmancontext.h"

#include "picmancontainerbox.h"
#include "picmancontainerview.h"
#include "picmandnd.h"
#include "picmandocked.h"
#include "picmanpropwidgets.h"
#include "picmanview.h"
#include "picmanviewrenderer.h"


static void   picman_container_box_view_iface_init   (PicmanContainerViewInterface *iface);
static void   picman_container_box_docked_iface_init (PicmanDockedInterface *iface);

static void   picman_container_box_constructed       (GObject      *object);

static GtkWidget * picman_container_box_get_preview  (PicmanDocked   *docked,
                                                    PicmanContext  *context,
                                                    GtkIconSize   size);
static void        picman_container_box_set_context  (PicmanDocked   *docked,
                                                    PicmanContext  *context);


G_DEFINE_TYPE_WITH_CODE (PicmanContainerBox, picman_container_box,
                         PICMAN_TYPE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONTAINER_VIEW,
                                                picman_container_box_view_iface_init)
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_container_box_docked_iface_init))

#define parent_class picman_container_box_parent_class


static void
picman_container_box_class_init (PicmanContainerBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_container_box_constructed;
  object_class->set_property = picman_container_view_set_property;
  object_class->get_property = picman_container_view_get_property;

  picman_container_view_install_properties (object_class);
}

static void
picman_container_box_init (PicmanContainerBox *box)
{
  GtkWidget *sb;

  box->scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (box), box->scrolled_win, TRUE, TRUE, 0);
  gtk_widget_show (box->scrolled_win);

  sb = gtk_scrolled_window_get_vscrollbar (GTK_SCROLLED_WINDOW (box->scrolled_win));

  gtk_widget_set_can_focus (sb, FALSE);
}

static void
picman_container_box_view_iface_init (PicmanContainerViewInterface *iface)
{
}

static void
picman_container_box_docked_iface_init (PicmanDockedInterface *iface)
{
  iface->get_preview = picman_container_box_get_preview;
  iface->set_context = picman_container_box_set_context;
}

static void
picman_container_box_constructed (GObject *object)
{
  PicmanContainerBox *box = PICMAN_CONTAINER_BOX (object);

  /* This is evil: the hash table of "insert_data" is created on
   * demand when PicmanContainerView API is used, using a
   * value_free_func that is set in the interface_init functions of
   * its implementors. Therefore, no PicmanContainerView API must be
   * called from any init() function, because the interface_init()
   * function of a subclass that sets the right value_free_func might
   * not have been called yet, leaving the insert_data hash table
   * without memory management.
   *
   * Call PicmanContainerView API from GObject::constructed() instead,
   * which runs after everything is set up correctly.
   */
  picman_container_view_set_dnd_widget (PICMAN_CONTAINER_VIEW (box),
                                      box->scrolled_win);

  G_OBJECT_CLASS (parent_class)->constructed (object);
}

void
picman_container_box_set_size_request (PicmanContainerBox *box,
                                     gint              width,
                                     gint              height)
{
  PicmanContainerView      *view;
  GtkScrolledWindowClass *sw_class;
  GtkStyle               *sw_style;
  GtkWidget              *sb;
  GtkRequisition          req;
  gint                    view_size;
  gint                    scrollbar_width;
  gint                    border_x;
  gint                    border_y;

  g_return_if_fail (PICMAN_IS_CONTAINER_BOX (box));

  view = PICMAN_CONTAINER_VIEW (box);

  view_size = picman_container_view_get_view_size (view, NULL);

  g_return_if_fail (width  <= 0 || width  >= view_size);
  g_return_if_fail (height <= 0 || height >= view_size);

  sw_class = GTK_SCROLLED_WINDOW_GET_CLASS (box->scrolled_win);

  if (sw_class->scrollbar_spacing >= 0)
    scrollbar_width = sw_class->scrollbar_spacing;
  else
    gtk_widget_style_get (GTK_WIDGET (box->scrolled_win),
                          "scrollbar-spacing", &scrollbar_width,
                          NULL);

  sb = gtk_scrolled_window_get_vscrollbar (GTK_SCROLLED_WINDOW (box->scrolled_win));

  gtk_widget_size_request (sb, &req);
  scrollbar_width += req.width;

  border_x = border_y = gtk_container_get_border_width (GTK_CONTAINER (box));

  sw_style = gtk_widget_get_style (box->scrolled_win);

  border_x += sw_style->xthickness * 2 + scrollbar_width;
  border_y += sw_style->ythickness * 2;

  gtk_widget_set_size_request (box->scrolled_win,
                               width  > 0 ? width  + border_x : -1,
                               height > 0 ? height + border_y : -1);
}

static void
picman_container_box_set_context (PicmanDocked  *docked,
                                PicmanContext *context)
{
  picman_container_view_set_context (PICMAN_CONTAINER_VIEW (docked), context);
}

static GtkWidget *
picman_container_box_get_preview (PicmanDocked   *docked,
                                PicmanContext  *context,
                                GtkIconSize   size)
{
  PicmanContainerBox  *box  = PICMAN_CONTAINER_BOX (docked);
  PicmanContainerView *view = PICMAN_CONTAINER_VIEW (docked);
  PicmanContainer     *container;
  GtkWidget         *preview;
  gint               width;
  gint               height;
  gint               border_width = 1;
  const gchar       *prop_name;

  container = picman_container_view_get_container (view);

  g_return_val_if_fail (container != NULL, NULL);

  gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (GTK_WIDGET (box)),
                                     size, &width, &height);

  prop_name = picman_context_type_to_prop_name (picman_container_get_children_type (container));

  preview = picman_prop_view_new (G_OBJECT (context), prop_name,
                                context, height);
  PICMAN_VIEW (preview)->renderer->size = -1;

  picman_container_view_get_view_size (view, &border_width);

  border_width = MIN (1, border_width);

  picman_view_renderer_set_size_full (PICMAN_VIEW (preview)->renderer,
                                    width, height, border_width);

  return preview;
}
