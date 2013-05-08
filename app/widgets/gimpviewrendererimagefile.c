/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrendererimagefile.c
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

#include "libpicmanthumb/picmanthumb.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanimagefile.h"

#include "picmanviewrendererimagefile.h"
#include "picmanviewrenderer-frame.h"


static void        picman_view_renderer_imagefile_render   (PicmanViewRenderer *renderer,
                                                          GtkWidget        *widget);

static GdkPixbuf * picman_view_renderer_imagefile_get_icon (PicmanImagefile    *imagefile,
                                                          GtkWidget        *widget,
                                                          gint              size);


G_DEFINE_TYPE (PicmanViewRendererImagefile, picman_view_renderer_imagefile,
               PICMAN_TYPE_VIEW_RENDERER)

#define parent_class picman_view_renderer_imagefile_parent_class


static void
picman_view_renderer_imagefile_class_init (PicmanViewRendererImagefileClass *klass)
{
  PicmanViewRendererClass *renderer_class = PICMAN_VIEW_RENDERER_CLASS (klass);

  renderer_class->render = picman_view_renderer_imagefile_render;
}

static void
picman_view_renderer_imagefile_init (PicmanViewRendererImagefile *renderer)
{
}

static void
picman_view_renderer_imagefile_render (PicmanViewRenderer *renderer,
                                     GtkWidget        *widget)
{
  GdkPixbuf *pixbuf = picman_view_renderer_get_frame_pixbuf (renderer, widget,
                                                           renderer->width,
                                                           renderer->height);

  if (! pixbuf)
    {
      PicmanImagefile *imagefile = PICMAN_IMAGEFILE (renderer->viewable);

      pixbuf = picman_view_renderer_imagefile_get_icon (imagefile,
                                                      widget,
                                                      MIN (renderer->width,
                                                           renderer->height));
    }

  if (pixbuf)
    {
      picman_view_renderer_render_pixbuf (renderer, pixbuf);
      g_object_unref (pixbuf);
    }
  else
    {
      const gchar *stock_id = picman_viewable_get_stock_id (renderer->viewable);

      picman_view_renderer_render_stock (renderer, widget, stock_id);
    }
}


/* The code to get an icon for a mime-type is lifted from GtkRecentManager. */

static GdkPixbuf *
get_icon_for_mime_type (const char *mime_type,
			gint        pixel_size)
{
  GtkIconTheme *icon_theme;
  const gchar  *separator;
  GString      *icon_name;
  GdkPixbuf    *pixbuf;

  separator = strchr (mime_type, '/');
  if (! separator)
    return NULL;

  icon_theme = gtk_icon_theme_get_default ();

  /* try with the three icon name variants for MIME types */

  /* canonicalize MIME type: foo/x-bar -> foo-x-bar */
  icon_name = g_string_new (NULL);
  g_string_append_len (icon_name, mime_type, separator - mime_type);
  g_string_append_c (icon_name, '-');
  g_string_append (icon_name, separator + 1);
  pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                     pixel_size,
                                     0,
                                     NULL);
  g_string_free (icon_name, TRUE);
  if (pixbuf)
    return pixbuf;

  /* canonicalize MIME type, and prepend "gnome-mime-" */
  icon_name = g_string_new ("gnome-mime-");
  g_string_append_len (icon_name, mime_type, separator - mime_type);
  g_string_append_c (icon_name, '-');
  g_string_append (icon_name, separator + 1);
  pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                     pixel_size,
                                     0,
                                     NULL);
  g_string_free (icon_name, TRUE);
  if (pixbuf)
    return pixbuf;

  /* try the MIME family icon */
  icon_name = g_string_new ("gnome-mime-");
  g_string_append_len (icon_name, mime_type, separator - mime_type);
  pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                     pixel_size,
                                     0,
                                     NULL);
  g_string_free (icon_name, TRUE);

  return pixbuf;
}

static GdkPixbuf *
picman_view_renderer_imagefile_get_icon (PicmanImagefile *imagefile,
                                       GtkWidget     *widget,
                                       gint           size)
{
  GdkScreen     *screen     = gtk_widget_get_screen (widget);
  GtkIconTheme  *icon_theme = gtk_icon_theme_get_for_screen (screen);
  PicmanThumbnail *thumbnail  = picman_imagefile_get_thumbnail (imagefile);
  GdkPixbuf     *pixbuf     = NULL;

  if (! picman_object_get_name (imagefile))
    return NULL;

  if (! pixbuf)
    {
      GIcon *icon = picman_imagefile_get_gicon (imagefile);

      if (icon)
        {
          GtkIconInfo *info;

          info = gtk_icon_theme_lookup_by_gicon (icon_theme, icon, size, 0);

          if (info)
            {
              pixbuf = gtk_icon_info_load_icon (info, NULL);

              gtk_icon_info_free (info);
            }
        }
    }

  if (! pixbuf && thumbnail->image_mimetype)
    {
      pixbuf = get_icon_for_mime_type (thumbnail->image_mimetype,
                                       size);
    }

  if (! pixbuf)
    {
      const gchar *icon_name = GTK_STOCK_FILE;

      if (thumbnail->image_state == PICMAN_THUMB_STATE_FOLDER)
        icon_name = GTK_STOCK_DIRECTORY;

      pixbuf = gtk_icon_theme_load_icon (icon_theme,
                                         icon_name, size,
                                         GTK_ICON_LOOKUP_USE_BUILTIN,
                                         NULL);
    }

  return pixbuf;
}
