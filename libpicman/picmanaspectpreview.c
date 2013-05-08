/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanaspectpreview.c
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "picmanuitypes.h"

#include "picman.h"

#include "libpicman-intl.h"

#include "picmanaspectpreview.h"


/**
 * SECTION: picmanaspectpreview
 * @title: PicmanAspectPreview
 * @short_description: A widget providing a preview with fixed aspect ratio.
 *
 * A widget providing a preview with fixed aspect ratio.
 **/


enum
{
  PROP_0,
  PROP_DRAWABLE
};

typedef struct
{
  gboolean  update;
} PreviewSettings;


static void  picman_aspect_preview_constructed  (GObject         *object);
static void  picman_aspect_preview_dispose      (GObject         *object);
static void  picman_aspect_preview_get_property (GObject         *object,
                                               guint            property_id,
                                               GValue          *value,
                                               GParamSpec      *pspec);
static void  picman_aspect_preview_set_property (GObject         *object,
                                               guint            property_id,
                                               const GValue    *value,
                                               GParamSpec      *pspec);

static void  picman_aspect_preview_style_set    (GtkWidget       *widget,
                                               GtkStyle        *prev_style);
static void  picman_aspect_preview_draw         (PicmanPreview     *preview);
static void  picman_aspect_preview_draw_buffer  (PicmanPreview     *preview,
                                               const guchar    *buffer,
                                               gint             rowstride);
static void  picman_aspect_preview_transform    (PicmanPreview     *preview,
                                               gint             src_x,
                                               gint             src_y,
                                               gint            *dest_x,
                                               gint            *dest_y);
static void  picman_aspect_preview_untransform  (PicmanPreview     *preview,
                                               gint             src_x,
                                               gint             src_y,
                                               gint            *dest_x,
                                               gint            *dest_y);

static void  picman_aspect_preview_set_drawable (PicmanAspectPreview *preview,
                                               PicmanDrawable      *drawable);


G_DEFINE_TYPE (PicmanAspectPreview, picman_aspect_preview, PICMAN_TYPE_PREVIEW)

#define parent_class picman_aspect_preview_parent_class

static gint picman_aspect_preview_counter = 0;


static void
picman_aspect_preview_class_init (PicmanAspectPreviewClass *klass)
{
  GObjectClass     *object_class  = G_OBJECT_CLASS (klass);
  GtkWidgetClass   *widget_class  = GTK_WIDGET_CLASS (klass);
  PicmanPreviewClass *preview_class = PICMAN_PREVIEW_CLASS (klass);

  object_class->constructed  = picman_aspect_preview_constructed;
  object_class->dispose      = picman_aspect_preview_dispose;
  object_class->get_property = picman_aspect_preview_get_property;
  object_class->set_property = picman_aspect_preview_set_property;

  widget_class->style_set    = picman_aspect_preview_style_set;

  preview_class->draw        = picman_aspect_preview_draw;
  preview_class->draw_buffer = picman_aspect_preview_draw_buffer;
  preview_class->transform   = picman_aspect_preview_transform;
  preview_class->untransform = picman_aspect_preview_untransform;

  /**
   * PicmanAspectPreview:drawable:
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_DRAWABLE,
                                   g_param_spec_pointer ("drawable", NULL, NULL,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_aspect_preview_init (PicmanAspectPreview *preview)
{
  g_object_set (PICMAN_PREVIEW (preview)->area,
                "check-size", picman_check_size (),
                "check-type", picman_check_type (),
                NULL);
}

static void
picman_aspect_preview_constructed (GObject *object)
{
  gchar           *data_name;
  PreviewSettings  settings;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  data_name = g_strdup_printf ("%s-aspect-preview-%d",
                               g_get_prgname (),
                               picman_aspect_preview_counter++);

  if (picman_get_data (data_name, &settings))
    {
      picman_preview_set_update (PICMAN_PREVIEW (object), settings.update);
    }

  g_object_set_data_full (object, "picman-aspect-preview-data-name",
                          data_name, (GDestroyNotify) g_free);
}

static void
picman_aspect_preview_dispose (GObject *object)
{
  const gchar *data_name = g_object_get_data (G_OBJECT (object),
                                              "picman-aspect-preview-data-name");

  if (data_name)
    {
      PicmanPreview     *preview = PICMAN_PREVIEW (object);
      PreviewSettings  settings;

      settings.update = picman_preview_get_update (preview);

      picman_set_data (data_name, &settings, sizeof (PreviewSettings));
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_aspect_preview_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PicmanAspectPreview *preview = PICMAN_ASPECT_PREVIEW (object);

  switch (property_id)
    {
    case PROP_DRAWABLE:
      g_value_set_pointer (value, preview->drawable);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_aspect_preview_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  PicmanAspectPreview *preview = PICMAN_ASPECT_PREVIEW (object);

  switch (property_id)
    {
    case PROP_DRAWABLE:
      picman_aspect_preview_set_drawable (preview,
                                        g_value_get_pointer (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_aspect_preview_style_set (GtkWidget *widget,
                               GtkStyle  *prev_style)
{
  PicmanPreview  *preview  = PICMAN_PREVIEW (widget);
  PicmanDrawable *drawable = PICMAN_ASPECT_PREVIEW (preview)->drawable;
  gint          size;

  if (GTK_WIDGET_CLASS (parent_class)->style_set)
    GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (widget,
                        "size", &size,
                        NULL);

  if (drawable->width > drawable->height)
    {
      preview->width  = MIN (drawable->width, size);
      preview->height = (drawable->height * preview->width) / drawable->width;
    }
  else
    {
      preview->height = MIN (drawable->height, size);
      preview->width  = (drawable->width * preview->height) / drawable->height;
    }

  gtk_widget_set_size_request (preview->area,
                               preview->width, preview->height);
}


static void
picman_aspect_preview_draw (PicmanPreview *preview)
{
  g_return_if_fail (PICMAN_IS_ASPECT_PREVIEW (preview));

  picman_preview_area_fill (PICMAN_PREVIEW_AREA (preview->area),
                          0, 0,
                          preview->width,
                          preview->height,
                          0, 0, 0);
}

static void
picman_aspect_preview_draw_buffer (PicmanPreview  *preview,
                                 const guchar *buffer,
                                 gint          rowstride)
{
  PicmanDrawable *drawable = PICMAN_ASPECT_PREVIEW (preview)->drawable;
  gint32        image_id;

  image_id = picman_item_get_image (drawable->drawable_id);

  if (picman_selection_is_empty (image_id))
    {
      picman_preview_area_draw (PICMAN_PREVIEW_AREA (preview->area),
                              0, 0,
                              preview->width, preview->height,
                              picman_drawable_type (drawable->drawable_id),
                              buffer,
                              rowstride);
    }
  else
    {
      guchar *sel;
      guchar *src;
      gint    selection_id;
      gint    width, height;
      gint    bpp;

      selection_id = picman_image_get_selection (image_id);

      width  = preview->width;
      height = preview->height;

      src = picman_drawable_get_thumbnail_data (drawable->drawable_id,
                                              &width, &height, &bpp);
      sel = picman_drawable_get_thumbnail_data (selection_id,
                                              &width, &height, &bpp);

      picman_preview_area_mask (PICMAN_PREVIEW_AREA (preview->area),
                              0, 0, preview->width, preview->height,
                              picman_drawable_type (drawable->drawable_id),
                              src, width * drawable->bpp,
                              buffer, rowstride,
                              sel, width);

      g_free (sel);
      g_free (src);
    }
}

static void
picman_aspect_preview_transform (PicmanPreview *preview,
                               gint         src_x,
                               gint         src_y,
                               gint        *dest_x,
                               gint        *dest_y)
{
  PicmanDrawable *drawable = PICMAN_ASPECT_PREVIEW (preview)->drawable;

  *dest_x = (gdouble) src_x * preview->width / drawable->width;
  *dest_y = (gdouble) src_y * preview->height / drawable->height;
}

static void
picman_aspect_preview_untransform (PicmanPreview *preview,
                                 gint         src_x,
                                 gint         src_y,
                                 gint        *dest_x,
                                 gint        *dest_y)
{
  PicmanDrawable *drawable = PICMAN_ASPECT_PREVIEW (preview)->drawable;

  *dest_x = (gdouble) src_x * drawable->width / preview->width;
  *dest_y = (gdouble) src_y * drawable->height / preview->height;
}

static void
picman_aspect_preview_set_drawable (PicmanAspectPreview *preview,
                                  PicmanDrawable      *drawable)
{
  gint width;
  gint height;

  preview->drawable = drawable;

  if (drawable->width > drawable->height)
    {
      width  = MIN (drawable->width, 512);
      height = (drawable->height * width) / drawable->width;
    }
  else
    {
      height = MIN (drawable->height, 512);
      width  = (drawable->width * height) / drawable->height;
    }
  picman_preview_set_bounds (PICMAN_PREVIEW (preview), 0, 0, width, height);

  if (height > 0)
    g_object_set (PICMAN_PREVIEW (preview)->frame,
                  "ratio",
                  (gdouble) drawable->width / (gdouble) drawable->height,
                  NULL);
}

/**
 * picman_aspect_preview_new:
 * @drawable: a #PicmanDrawable
 * @toggle:   unused
 *
 * Creates a new #PicmanAspectPreview widget for @drawable. See also
 * picman_drawable_preview_new().
 *
 * In PICMAN 2.2 the @toggle parameter was provided to conviently access
 * the state of the "Preview" check-button. This is not any longer
 * necessary as the preview itself now stores this state, as well as
 * the scroll offset.
 *
 * Since: PICMAN 2.2
 *
 * Returns: a new #PicmanAspectPreview.
 **/
GtkWidget *
picman_aspect_preview_new (PicmanDrawable *drawable,
                         gboolean     *toggle)
{
  g_return_val_if_fail (drawable != NULL, NULL);

  return g_object_new (PICMAN_TYPE_ASPECT_PREVIEW,
                       "drawable", drawable,
                       NULL);
}
