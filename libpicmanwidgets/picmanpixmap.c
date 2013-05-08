/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpixmap.c
 * Copyright (C) 2000 Michael Natterer <mitch@picman.org>
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

#undef GSEAL_ENABLE
#undef GTK_DISABLE_DEPRECATED
#undef GDK_DISABLE_DEPRECATED

#include <stdio.h>

#include <gtk/gtk.h>

#include "picmanwidgetstypes.h"

#undef PICMAN_DISABLE_DEPRECATED
#include "picmanpixmap.h"


/**
 * SECTION: picmanpixmap
 * @title: PicmanPixmap
 * @short_description: Widget which creates a #GtkPixmap from XPM data.
 * @see_also: picman_pixmap_button_new(), #GtkPixmap
 *
 * Widget which creates a #GtkPixmap from XPM data.
 *
 * Use this widget instead of #GtkPixmap if you don't want to worry
 * about the parent container's "realized" state.
 *
 * Note that the drawback of the easy interface is that the actual
 * #GdkPixmap and it's mask have to be constructed every time you call
 * picman_pixmap_new() and cannot be cached in memory without doing bad
 * hacks.
 **/


static void   picman_pixmap_realize           (GtkWidget  *widget);
static void   picman_pixmap_create_from_xpm_d (PicmanPixmap *pixmap);


G_DEFINE_TYPE (PicmanPixmap, picman_pixmap, GTK_TYPE_IMAGE)

#define parent_class picman_pixmap_parent_class


static void
picman_pixmap_class_init (PicmanPixmapClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->realize = picman_pixmap_realize;
}

static void
picman_pixmap_init (PicmanPixmap *pixmap)
{
  pixmap->xpm_data = NULL;
}

/**
 * picman_pixmap_new:
 * @xpm_data: A pointer to a XPM data structure as found in XPM files.
 *
 * Creates a new #PicmanPixmap widget.
 *
 * Returns: A pointer to the new #PicmanPixmap widget.
 **/
GtkWidget *
picman_pixmap_new (gchar **xpm_data)
{
  PicmanPixmap *pixmap = g_object_new (PICMAN_TYPE_PIXMAP, NULL);

  picman_pixmap_set (pixmap, xpm_data);

  return GTK_WIDGET (pixmap);
}

/**
 * picman_pixmap_set:
 * @pixmap: The pixmap widget you want to set the new xpm_data for.
 * @xpm_data: A pointer to a XPM data structure as found in XPM files.
 *
 * Sets a new image for an existing #PicmanPixmap widget.
 **/
void
picman_pixmap_set (PicmanPixmap  *pixmap,
                 gchar      **xpm_data)
{
  g_return_if_fail (PICMAN_IS_PIXMAP (pixmap));

  pixmap->xpm_data = xpm_data;

  GTK_WIDGET (pixmap)->requisition.width  = 0;
  GTK_WIDGET (pixmap)->requisition.height = 0;

  if (! gtk_widget_get_realized (GTK_WIDGET (pixmap)))
    {
      if (xpm_data)
        {
          gint width, height;

          if (sscanf (xpm_data[0], "%d %d", &width, &height) != 2)
            {
              g_warning ("%s: passed pointer is no XPM data", G_STRFUNC);
            }
          else
            {
              gint xpad, ypad;

              gtk_misc_get_padding (GTK_MISC (pixmap), &xpad, &ypad);

              GTK_WIDGET (pixmap)->requisition.width  = width + xpad * 2;
              GTK_WIDGET (pixmap)->requisition.height = height + ypad * 2;
            }
        }
    }
  else
    {
      picman_pixmap_create_from_xpm_d (pixmap);
    }
}

static void
picman_pixmap_realize (GtkWidget *widget)
{
  if (GTK_WIDGET_CLASS (parent_class)->realize)
    GTK_WIDGET_CLASS (parent_class)->realize (widget);

  picman_pixmap_create_from_xpm_d (PICMAN_PIXMAP (widget));
}

static void
picman_pixmap_create_from_xpm_d (PicmanPixmap *pixmap)
{
  GtkStyle   *style;
  GdkPixmap  *gdk_pixmap = NULL;
  GdkBitmap  *mask       = NULL;

  if (pixmap->xpm_data)
    {
      GtkWidget  *widget;

      widget = GTK_WIDGET (pixmap);

      style = gtk_widget_get_style (widget);

      gdk_pixmap = gdk_pixmap_create_from_xpm_d (gtk_widget_get_window (widget),
                                                 &mask,
                                                 &style->bg[GTK_STATE_NORMAL],
                                                 pixmap->xpm_data);
    }

  gtk_image_set_from_pixmap (GTK_IMAGE (pixmap), gdk_pixmap, mask);

  if (gdk_pixmap)
    g_object_unref (gdk_pixmap);

  if (mask)
    g_object_unref (mask);
}
