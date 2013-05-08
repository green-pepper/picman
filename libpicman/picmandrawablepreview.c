/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmandrawablepreview.c
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

#include "picmandrawablepreview.h"


/**
 * SECTION: picmandrawablepreview
 * @title: PicmanDrawablePreview
 * @short_description: A widget providing a preview of a #PicmanDrawable.
 *
 * A widget providing a preview of a #PicmanDrawable.
 **/


#define SELECTION_BORDER  8

enum
{
  PROP_0,
  PROP_DRAWABLE
};

typedef struct
{
  gint      x;
  gint      y;
  gboolean  update;
} PreviewSettings;


static void  picman_drawable_preview_constructed   (GObject         *object);
static void  picman_drawable_preview_dispose       (GObject         *object);
static void  picman_drawable_preview_get_property  (GObject         *object,
                                                  guint            property_id,
                                                  GValue          *value,
                                                  GParamSpec      *pspec);
static void  picman_drawable_preview_set_property  (GObject         *object,
                                                  guint            property_id,
                                                  const GValue    *value,
                                                  GParamSpec      *pspec);

static void  picman_drawable_preview_style_set     (GtkWidget       *widget,
                                                  GtkStyle        *prev_style);

static void  picman_drawable_preview_draw_original (PicmanPreview     *preview);
static void  picman_drawable_preview_draw_thumb    (PicmanPreview     *preview,
                                                  PicmanPreviewArea *area,
                                                  gint             width,
                                                  gint             height);
static void  picman_drawable_preview_draw_buffer   (PicmanPreview     *preview,
                                                  const guchar    *buffer,
                                                  gint             rowstride);

static void  picman_drawable_preview_set_drawable (PicmanDrawablePreview *preview,
                                                 PicmanDrawable        *drawable);


G_DEFINE_TYPE (PicmanDrawablePreview, picman_drawable_preview,
               PICMAN_TYPE_SCROLLED_PREVIEW)

#define parent_class picman_drawable_preview_parent_class

static gint picman_drawable_preview_counter = 0;


static void
picman_drawable_preview_class_init (PicmanDrawablePreviewClass *klass)
{
  GObjectClass     *object_class  = G_OBJECT_CLASS (klass);
  GtkWidgetClass   *widget_class  = GTK_WIDGET_CLASS (klass);
  PicmanPreviewClass *preview_class = PICMAN_PREVIEW_CLASS (klass);

  object_class->constructed  = picman_drawable_preview_constructed;
  object_class->dispose      = picman_drawable_preview_dispose;
  object_class->get_property = picman_drawable_preview_get_property;
  object_class->set_property = picman_drawable_preview_set_property;

  widget_class->style_set    = picman_drawable_preview_style_set;

  preview_class->draw        = picman_drawable_preview_draw_original;
  preview_class->draw_thumb  = picman_drawable_preview_draw_thumb;
  preview_class->draw_buffer = picman_drawable_preview_draw_buffer;

  /**
   * PicmanDrawablePreview:drawable:
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_DRAWABLE,
                                   g_param_spec_pointer ("drawable", NULL, NULL,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_drawable_preview_init (PicmanDrawablePreview *preview)
{
  g_object_set (PICMAN_PREVIEW (preview)->area,
                "check-size", picman_check_size (),
                "check-type", picman_check_type (),
                NULL);
}

static void
picman_drawable_preview_constructed (GObject *object)
{
  gchar           *data_name;
  PreviewSettings  settings;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  data_name = g_strdup_printf ("%s-drawable-preview-%d",
                               g_get_prgname (),
                               ++picman_drawable_preview_counter);

  if (picman_get_data (data_name, &settings))
    {
      picman_preview_set_update (PICMAN_PREVIEW (object), settings.update);
      picman_scrolled_preview_set_position (PICMAN_SCROLLED_PREVIEW (object),
                                          settings.x, settings.y);
    }

  g_object_set_data_full (object, "picman-drawable-preview-data-name",
                          data_name, (GDestroyNotify) g_free);
}

static void
picman_drawable_preview_dispose (GObject *object)
{
  const gchar *data_name = g_object_get_data (G_OBJECT (object),
                                              "picman-drawable-preview-data-name");

  if (data_name)
    {
      PicmanPreview     *preview = PICMAN_PREVIEW (object);
      PreviewSettings  settings;

      settings.x      = preview->xoff + preview->xmin;
      settings.y      = preview->yoff + preview->ymin;
      settings.update = picman_preview_get_update (preview);

      picman_set_data (data_name, &settings, sizeof (PreviewSettings));
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_drawable_preview_get_property (GObject    *object,
                                    guint       property_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  PicmanDrawablePreview *preview = PICMAN_DRAWABLE_PREVIEW (object);

  switch (property_id)
    {
    case PROP_DRAWABLE:
      g_value_set_pointer (value,
                           picman_drawable_preview_get_drawable (preview));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_drawable_preview_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  PicmanDrawablePreview *preview = PICMAN_DRAWABLE_PREVIEW (object);

  switch (property_id)
    {
    case PROP_DRAWABLE:
      picman_drawable_preview_set_drawable (preview,
                                          g_value_get_pointer (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_drawable_preview_style_set (GtkWidget *widget,
                                 GtkStyle  *prev_style)
{
  PicmanPreview *preview = PICMAN_PREVIEW (widget);
  gint         width   = preview->xmax - preview->xmin;
  gint         height  = preview->ymax - preview->ymin;
  gint         size;

  if (GTK_WIDGET_CLASS (parent_class)->style_set)
    GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (widget,
                        "size", &size,
                        NULL);

  gtk_widget_set_size_request (PICMAN_PREVIEW (preview)->area,
                               MIN (width, size), MIN (height, size));
}

static void
picman_drawable_preview_draw_original (PicmanPreview *preview)
{
  PicmanDrawablePreview *drawable_preview = PICMAN_DRAWABLE_PREVIEW (preview);
  PicmanDrawable        *drawable         = drawable_preview->drawable;
  guchar              *buffer;
  PicmanPixelRgn         srcPR;
  guint                rowstride;

  if (! drawable)
    return;

  rowstride = preview->width * drawable->bpp;
  buffer    = g_new (guchar, rowstride * preview->height);

  preview->xoff = CLAMP (preview->xoff,
                         0, preview->xmax - preview->xmin - preview->width);
  preview->yoff = CLAMP (preview->yoff,
                         0, preview->ymax - preview->ymin - preview->height);

  picman_pixel_rgn_init (&srcPR, drawable,
                       preview->xoff + preview->xmin,
                       preview->yoff + preview->ymin,
                       preview->width, preview->height,
                       FALSE, FALSE);

  picman_pixel_rgn_get_rect (&srcPR, buffer,
                           preview->xoff + preview->xmin,
                           preview->yoff + preview->ymin,
                           preview->width, preview->height);

  picman_preview_area_draw (PICMAN_PREVIEW_AREA (preview->area),
                          0, 0, preview->width, preview->height,
                          picman_drawable_type (drawable->drawable_id),
                          buffer,
                          rowstride);
  g_free (buffer);
}

static void
picman_drawable_preview_draw_thumb (PicmanPreview     *preview,
                                  PicmanPreviewArea *area,
                                  gint             width,
                                  gint             height)
{
  PicmanDrawablePreview *drawable_preview = PICMAN_DRAWABLE_PREVIEW (preview);
  PicmanDrawable        *drawable         = drawable_preview->drawable;

  if (drawable)
    _picman_drawable_preview_area_draw_thumb (area, drawable, width, height);
}

void
_picman_drawable_preview_area_draw_thumb (PicmanPreviewArea *area,
                                        PicmanDrawable    *drawable,
                                        gint             width,
                                        gint             height)
{
  guchar *buffer;
  gint    x1, y1, x2, y2;
  gint    bpp;
  gint    size = 100;
  gint    nav_width, nav_height;

  g_return_if_fail (PICMAN_IS_PREVIEW_AREA (area));
  g_return_if_fail (drawable != NULL);

  if (_picman_drawable_preview_get_bounds (drawable, &x1, &y1, &x2, &y2))
    {
      width  = x2 - x1;
      height = y2 - y1;
    }
  else
    {
      width  = picman_drawable_width  (drawable->drawable_id);
      height = picman_drawable_height (drawable->drawable_id);
    }

  if (width > height)
    {
      nav_width  = MIN (width, size);
      nav_height = (height * nav_width) / width;
    }
  else
    {
      nav_height = MIN (height, size);
      nav_width  = (width * nav_height) / height;
    }

  if (_picman_drawable_preview_get_bounds (drawable, &x1, &y1, &x2, &y2))
    {
      buffer = picman_drawable_get_sub_thumbnail_data (drawable->drawable_id,
                                                     x1, y1, x2 - x1, y2 - y1,
                                                     &nav_width, &nav_height,
                                                     &bpp);
    }
  else
    {
      buffer = picman_drawable_get_thumbnail_data (drawable->drawable_id,
                                                 &nav_width, &nav_height,
                                                 &bpp);
    }

  if (buffer)
    {
      PicmanImageType  type;

      gtk_widget_set_size_request (GTK_WIDGET (area), nav_width, nav_height);
      gtk_widget_show (GTK_WIDGET (area));
      gtk_widget_realize (GTK_WIDGET (area));

      switch (bpp)
        {
        case 1:  type = PICMAN_GRAY_IMAGE;   break;
        case 2:  type = PICMAN_GRAYA_IMAGE;  break;
        case 3:  type = PICMAN_RGB_IMAGE;    break;
        case 4:  type = PICMAN_RGBA_IMAGE;   break;
        default:
          g_free (buffer);
          return;
        }

      picman_preview_area_draw (area,
                              0, 0, nav_width, nav_height,
                              type, buffer, bpp * nav_width);

      g_free (buffer);
    }
}

static void
picman_drawable_preview_draw_area (PicmanDrawablePreview *preview,
                                 gint                 x,
                                 gint                 y,
                                 gint                 width,
                                 gint                 height,
                                 const guchar        *buf,
                                 gint                 rowstride)
{
  PicmanPreview  *picman_preview = PICMAN_PREVIEW (preview);
  PicmanDrawable *drawable     = preview->drawable;
  gint32        image_id;

  image_id = picman_item_get_image (drawable->drawable_id);

  if (picman_selection_is_empty (image_id))
    {
      picman_preview_area_draw (PICMAN_PREVIEW_AREA (picman_preview->area),
                              x - picman_preview->xoff - picman_preview->xmin,
                              y - picman_preview->yoff - picman_preview->ymin,
                              width,
                              height,
                              picman_drawable_type (drawable->drawable_id),
                              buf, rowstride);
    }
  else
    {
      gint offset_x, offset_y;
      gint mask_x, mask_y;
      gint mask_width, mask_height;
      gint draw_x, draw_y;
      gint draw_width, draw_height;

      picman_drawable_offsets (drawable->drawable_id, &offset_x, &offset_y);

      if (picman_drawable_mask_intersect (drawable->drawable_id,
                                        &mask_x, &mask_y,
                                        &mask_width, &mask_height) &&
          picman_rectangle_intersect (mask_x, mask_y,
                                    mask_width, mask_height,
                                    x, y, width, height,
                                    &draw_x, &draw_y,
                                    &draw_width, &draw_height))
        {
          PicmanDrawable *selection;
          PicmanPixelRgn  drawable_rgn;
          PicmanPixelRgn  selection_rgn;
          guchar       *src;
          guchar       *sel;

          selection = picman_drawable_get (picman_image_get_selection (image_id));

          picman_pixel_rgn_init (&drawable_rgn, drawable,
                               draw_x, draw_y, draw_width, draw_height,
                               FALSE, FALSE);
          picman_pixel_rgn_init (&selection_rgn, selection,
                               draw_x + offset_x, draw_y + offset_y,
                               draw_width, draw_height,
                               FALSE, FALSE);

          src = g_new (guchar, draw_width * draw_height * drawable->bpp);
          sel = g_new (guchar, draw_width * draw_height);

          picman_pixel_rgn_get_rect (&drawable_rgn, src,
                                   draw_x, draw_y,
                                   draw_width, draw_height);
          picman_pixel_rgn_get_rect (&selection_rgn, sel,
                                   draw_x + offset_x, draw_y + offset_y,
                                   draw_width, draw_height);

          picman_preview_area_mask (PICMAN_PREVIEW_AREA (picman_preview->area),
                                  draw_x - picman_preview->xoff - picman_preview->xmin,
                                  draw_y - picman_preview->yoff - picman_preview->ymin,
                                  draw_width,
                                  draw_height,
                                  picman_drawable_type (drawable->drawable_id),
                                  src, draw_width * drawable->bpp,
                                  (buf +
                                   (draw_x - x) * drawable->bpp +
                                   (draw_y - y) * rowstride),
                                  rowstride,
                                  sel, draw_width);

          g_free (sel);
          g_free (src);

          picman_drawable_detach (selection);
        }
    }
}

static void
picman_drawable_preview_draw_buffer (PicmanPreview  *preview,
                                   const guchar *buffer,
                                   gint          rowstride)
{
  picman_drawable_preview_draw_area (PICMAN_DRAWABLE_PREVIEW (preview),
                                   preview->xmin + preview->xoff,
                                   preview->ymin + preview->yoff,
                                   preview->width,
                                   preview->height,
                                   buffer, rowstride);
}

static void
picman_drawable_preview_set_drawable (PicmanDrawablePreview *drawable_preview,
                                    PicmanDrawable        *drawable)
{
  PicmanPreview *preview = PICMAN_PREVIEW (drawable_preview);
  gint         x1, y1, x2, y2;

  drawable_preview->drawable = drawable;

  _picman_drawable_preview_get_bounds (drawable, &x1, &y1, &x2, &y2);

  picman_preview_set_bounds (preview, x1, y1, x2, y2);

  if (picman_drawable_is_indexed (drawable->drawable_id))
    {
      guint32  image = picman_item_get_image (drawable->drawable_id);
      guchar  *cmap;
      gint     num_colors;

      cmap = picman_image_get_colormap (image, &num_colors);
      picman_preview_area_set_colormap (PICMAN_PREVIEW_AREA (preview->area),
                                      cmap, num_colors);
      g_free (cmap);
    }
}


#define MAX3(a, b, c)  (MAX (MAX ((a), (b)), (c)))
#define MIN3(a, b, c)  (MIN (MIN ((a), (b)), (c)))

gboolean
_picman_drawable_preview_get_bounds (PicmanDrawable *drawable,
                                   gint         *xmin,
                                   gint         *ymin,
                                   gint         *xmax,
                                   gint         *ymax)
{
  gint     width;
  gint     height;
  gint     offset_x;
  gint     offset_y;
  gint     x1, y1;
  gint     x2, y2;
  gboolean retval;

  g_return_val_if_fail (drawable != NULL, FALSE);

  width  = picman_drawable_width (drawable->drawable_id);
  height = picman_drawable_height (drawable->drawable_id);

  retval = picman_drawable_mask_bounds (drawable->drawable_id,
                                      &x1, &y1, &x2, &y2);

  picman_drawable_offsets (drawable->drawable_id, &offset_x, &offset_y);

  *xmin = MAX3 (x1 - SELECTION_BORDER, 0, - offset_x);
  *ymin = MAX3 (y1 - SELECTION_BORDER, 0, - offset_y);
  *xmax = MIN3 (x2 + SELECTION_BORDER, width,  width  + offset_x);
  *ymax = MIN3 (y2 + SELECTION_BORDER, height, height + offset_y);

  return retval;
}


/**
 * picman_drawable_preview_new:
 * @drawable: a #PicmanDrawable
 * @toggle:   unused
 *
 * Creates a new #PicmanDrawablePreview widget for @drawable.
 *
 * In PICMAN 2.2 the @toggle parameter was provided to conviently access
 * the state of the "Preview" check-button. This is not any longer
 * necessary as the preview itself now stores this state, as well as
 * the scroll offset.
 *
 * Returns: A pointer to the new #PicmanDrawablePreview widget.
 *
 * Since: PICMAN 2.2
 **/
GtkWidget *
picman_drawable_preview_new (PicmanDrawable *drawable,
                           gboolean     *toggle)
{
  g_return_val_if_fail (drawable != NULL, NULL);

  return g_object_new (PICMAN_TYPE_DRAWABLE_PREVIEW,
                       "drawable", drawable,
                       NULL);
}

/**
 * picman_drawable_preview_get_drawable:
 * @preview:   a #PicmanDrawablePreview widget
 *
 * Return value: the #PicmanDrawable that has been passed to
 *               picman_drawable_preview_new().
 *
 * Since: PICMAN 2.2
 **/
PicmanDrawable *
picman_drawable_preview_get_drawable (PicmanDrawablePreview *preview)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE_PREVIEW (preview), NULL);

  return preview->drawable;
}

/**
 * picman_drawable_preview_draw_region:
 * @preview: a #PicmanDrawablePreview widget
 * @region:  a #PicmanPixelRgn
 *
 * Since: PICMAN 2.2
 **/
void
picman_drawable_preview_draw_region (PicmanDrawablePreview *preview,
                                   const PicmanPixelRgn  *region)
{
  g_return_if_fail (PICMAN_IS_DRAWABLE_PREVIEW (preview));
  g_return_if_fail (preview->drawable != NULL);
  g_return_if_fail (region != NULL);

  /*  If the data field is initialized, this region is currently being
   *  processed and we can access it directly.
   */
  if (region->data)
    {
      picman_drawable_preview_draw_area (preview,
                                       region->x,
                                       region->y,
                                       region->w,
                                       region->h,
                                       region->data,
                                       region->rowstride);
    }
  else
    {
      PicmanPixelRgn  src = *region;
      gpointer      iter;

      src.dirty = FALSE; /* we don't dirty the tiles, just read them */

      for (iter = picman_pixel_rgns_register (1, &src);
           iter != NULL;
           iter = picman_pixel_rgns_process (iter))
        {
          picman_drawable_preview_draw_area (preview,
                                           src.x,
                                           src.y,
                                           src.w,
                                           src.h,
                                           src.data,
                                           src.rowstride);
        }
    }
}
