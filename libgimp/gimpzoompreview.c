/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanzoompreview.c
 * Copyright (C) 2005  David Odin <dindinx@picman.org>
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
#include "picmanzoompreview.h"


/**
 * SECTION: picmanzoompreview
 * @title: PicmanZoomPreview
 * @short_description: A drawable preview with zooming capabilities.
 *
 * A drawable preview with zooming capabilities.
 **/


enum
{
  PROP_0,
  PROP_DRAWABLE,
  PROP_MODEL
};


typedef struct PicmanZoomPreviewPrivate
{
  PicmanDrawable  *drawable;
  PicmanZoomModel *model;
  GdkRectangle   extents;
} PicmanZoomPreviewPrivate;

typedef struct
{
  gboolean  update;
} PreviewSettings;


#define PICMAN_ZOOM_PREVIEW_GET_PRIVATE(obj) \
  ((PicmanZoomPreviewPrivate *) ((PicmanZoomPreview *) (obj))->priv)

static void     picman_zoom_preview_constructed     (GObject         *object);
static void     picman_zoom_preview_finalize        (GObject         *object);
static void     picman_zoom_preview_dispose         (GObject         *object);
static void     picman_zoom_preview_get_property    (GObject         *object,
                                                   guint            property_id,
                                                   GValue          *value,
                                                   GParamSpec      *pspec);
static void     picman_zoom_preview_set_property    (GObject         *object,
                                                   guint            property_id,
                                                   const GValue    *value,
                                                   GParamSpec      *pspec);

static void     picman_zoom_preview_set_adjustments (PicmanZoomPreview *preview,
                                                   gdouble          old_factor,
                                                   gdouble          new_factor);
static void     picman_zoom_preview_size_allocate   (GtkWidget       *widget,
                                                   GtkAllocation   *allocation,
                                                   PicmanZoomPreview *preview);
static void     picman_zoom_preview_style_set       (GtkWidget       *widget,
                                                   GtkStyle        *prev_style);
static gboolean picman_zoom_preview_scroll_event    (GtkWidget       *widget,
                                                   GdkEventScroll  *event,
                                                   PicmanZoomPreview *preview);
static void     picman_zoom_preview_draw            (PicmanPreview     *preview);
static void     picman_zoom_preview_draw_buffer     (PicmanPreview     *preview,
                                                   const guchar    *buffer,
                                                   gint             rowstride);
static void     picman_zoom_preview_draw_thumb      (PicmanPreview     *preview,
                                                   PicmanPreviewArea *area,
                                                   gint             width,
                                                   gint             height);
static void     picman_zoom_preview_set_cursor      (PicmanPreview     *preview);
static void     picman_zoom_preview_transform       (PicmanPreview     *preview,
                                                   gint             src_x,
                                                   gint             src_y,
                                                   gint            *dest_x,
                                                   gint            *dest_y);
static void     picman_zoom_preview_untransform     (PicmanPreview     *preview,
                                                   gint             src_x,
                                                   gint             src_y,
                                                   gint            *dest_x,
                                                   gint            *dest_y);

static void     picman_zoom_preview_set_drawable    (PicmanZoomPreview *preview,
                                                   PicmanDrawable    *drawable);
static void     picman_zoom_preview_set_model       (PicmanZoomPreview *preview,
                                                   PicmanZoomModel   *model);

static void     picman_zoom_preview_get_source_area (PicmanPreview     *preview,
                                                   gint            *x,
                                                   gint            *y,
                                                   gint            *w,
                                                   gint            *h);


G_DEFINE_TYPE (PicmanZoomPreview, picman_zoom_preview, PICMAN_TYPE_SCROLLED_PREVIEW)

#define parent_class picman_zoom_preview_parent_class

static gint picman_zoom_preview_counter = 0;


static void
picman_zoom_preview_class_init (PicmanZoomPreviewClass *klass)
{
  GObjectClass     *object_class  = G_OBJECT_CLASS (klass);
  GtkWidgetClass   *widget_class  = GTK_WIDGET_CLASS (klass);
  PicmanPreviewClass *preview_class = PICMAN_PREVIEW_CLASS (klass);

  object_class->constructed  = picman_zoom_preview_constructed;
  object_class->finalize     = picman_zoom_preview_finalize;
  object_class->dispose      = picman_zoom_preview_dispose;
  object_class->get_property = picman_zoom_preview_get_property;
  object_class->set_property = picman_zoom_preview_set_property;

  widget_class->style_set    = picman_zoom_preview_style_set;

  preview_class->draw        = picman_zoom_preview_draw;
  preview_class->draw_buffer = picman_zoom_preview_draw_buffer;
  preview_class->draw_thumb  = picman_zoom_preview_draw_thumb;
  preview_class->set_cursor  = picman_zoom_preview_set_cursor;
  preview_class->transform   = picman_zoom_preview_transform;
  preview_class->untransform = picman_zoom_preview_untransform;

  g_type_class_add_private (object_class, sizeof (PicmanZoomPreviewPrivate));

  /**
   * PicmanZoomPreview:drawable:
   *
   * The drawable the #PicmanZoomPreview is attached to.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_DRAWABLE,
                                   g_param_spec_pointer ("drawable", NULL, NULL,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));

  /**
   * PicmanZoomPreview:model:
   *
   * The #PicmanZoomModel used by this #PicmanZoomPreview.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_MODEL,
                                   g_param_spec_object ("model", NULL, NULL,
                                                        PICMAN_TYPE_ZOOM_MODEL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_zoom_preview_init (PicmanZoomPreview *preview)
{
  preview->priv = G_TYPE_INSTANCE_GET_PRIVATE (preview,
                                               PICMAN_TYPE_ZOOM_PREVIEW,
                                               PicmanZoomPreviewPrivate);

  g_signal_connect (PICMAN_PREVIEW (preview)->area, "size-allocate",
                    G_CALLBACK (picman_zoom_preview_size_allocate),
                    preview);
  g_signal_connect (PICMAN_PREVIEW (preview)->area, "scroll-event",
                    G_CALLBACK (picman_zoom_preview_scroll_event),
                    preview);

  g_object_set (PICMAN_PREVIEW (preview)->area,
                "check-size", picman_check_size (),
                "check-type", picman_check_type (),
                NULL);

  picman_scrolled_preview_set_policy (PICMAN_SCROLLED_PREVIEW (preview),
                                    GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
}

static void
picman_zoom_preview_constructed (GObject *object)
{
  PicmanZoomPreviewPrivate *priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (object);
  gchar                  *data_name;
  PreviewSettings         settings;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  data_name = g_strdup_printf ("%s-zoom-preview-%d",
                               g_get_prgname (),
                               picman_zoom_preview_counter++);

  if (picman_get_data (data_name, &settings))
    {
      picman_preview_set_update (PICMAN_PREVIEW (object), settings.update);
    }

  g_object_set_data_full (object, "picman-zoom-preview-data-name",
                          data_name, (GDestroyNotify) g_free);

  if (! priv->model)
    {
      PicmanZoomModel *model = picman_zoom_model_new ();

      picman_zoom_model_set_range (model, 1.0, 256.0);
      picman_zoom_preview_set_model (PICMAN_ZOOM_PREVIEW (object), model);

      g_object_unref (model);
    }

  picman_zoom_preview_set_adjustments (PICMAN_ZOOM_PREVIEW (object), 1.0, 1.0);
}

static void
picman_zoom_preview_finalize (GObject *object)
{
  PicmanZoomPreviewPrivate *priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (object);

  if (priv->model)
    {
      g_object_unref (priv->model);
      priv->model = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_zoom_preview_dispose (GObject *object)
{
  const gchar *data_name = g_object_get_data (G_OBJECT (object),
                                               "picman-zoom-preview-data-name");

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
picman_zoom_preview_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanZoomPreview *preview = PICMAN_ZOOM_PREVIEW (object);

  switch (property_id)
    {
    case PROP_DRAWABLE:
      g_value_set_pointer (value, picman_zoom_preview_get_drawable (preview));
      break;

    case PROP_MODEL:
      g_value_set_object (value, picman_zoom_preview_get_model (preview));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_zoom_preview_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanZoomPreview *preview = PICMAN_ZOOM_PREVIEW (object);

  switch (property_id)
    {
    case PROP_DRAWABLE:
      picman_zoom_preview_set_drawable (preview, g_value_get_pointer (value));
      break;

    case PROP_MODEL:
      picman_zoom_preview_set_model (preview, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_zoom_preview_set_adjustments (PicmanZoomPreview *preview,
                                   gdouble          old_factor,
                                   gdouble          new_factor)
{
  PicmanScrolledPreview *scrolled_preview = PICMAN_SCROLLED_PREVIEW (preview);
  GtkAdjustment       *adj;
  gdouble              width;
  gdouble              height;
  gdouble              ratio;

  picman_scrolled_preview_freeze (scrolled_preview);

  width  = PICMAN_PREVIEW (preview)->width;
  height = PICMAN_PREVIEW (preview)->height;

  ratio = new_factor / old_factor;

  adj = gtk_range_get_adjustment (GTK_RANGE (scrolled_preview->hscr));
  gtk_adjustment_configure (adj,
                            (gtk_adjustment_get_value (adj) + width / 2.0) * ratio
                            - width / 2.0,
                            0,
                            width * new_factor,
                            new_factor,
                            MAX (width / 2.0, new_factor),
                            width);

  adj = gtk_range_get_adjustment (GTK_RANGE (scrolled_preview->vscr));
  gtk_adjustment_configure (adj,
                            (gtk_adjustment_get_value (adj) + height / 2.0) * ratio
                            - height / 2.0,
                            0,
                            height * new_factor,
                            new_factor,
                            MAX (height / 2.0, new_factor),
                            height);

  picman_scrolled_preview_thaw (scrolled_preview);
}

static void
picman_zoom_preview_size_allocate (GtkWidget       *widget,
                                 GtkAllocation   *allocation,
                                 PicmanZoomPreview *preview)
{
  PicmanZoomPreviewPrivate *priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);
  gdouble                 zoom;

  gint width  = PICMAN_PREVIEW (preview)->xmax - PICMAN_PREVIEW (preview)->xmin;
  gint height = PICMAN_PREVIEW (preview)->ymax - PICMAN_PREVIEW (preview)->ymin;

  PICMAN_PREVIEW (preview)->width  = MIN (width,  allocation->width);
  PICMAN_PREVIEW (preview)->height = MIN (height, allocation->height);

  zoom = picman_zoom_model_get_factor (priv->model);

  picman_zoom_preview_set_adjustments (preview, zoom, zoom);
}

static void
picman_zoom_preview_style_set (GtkWidget *widget,
                             GtkStyle  *prev_style)
{
  PicmanPreview            *preview  = PICMAN_PREVIEW (widget);
  PicmanZoomPreviewPrivate *priv     = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);
  PicmanDrawable           *drawable = priv->drawable;
  gint                    size;
  gint                    width, height;
  gint                    x1, y1;
  gint                    x2, y2;

  if (GTK_WIDGET_CLASS (parent_class)->style_set)
    GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (widget, "size", &size, NULL);

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
      preview->width  = MIN (width, size);
      preview->height = (height * preview->width) / width;
    }
  else
    {
      preview->height = MIN (height, size);
      preview->width  = (width * preview->height) / height;
    }

  gtk_widget_set_size_request (preview->area,
                               preview->width, preview->height);
}

static gboolean
picman_zoom_preview_scroll_event (GtkWidget       *widget,
                                GdkEventScroll  *event,
                                PicmanZoomPreview *preview)
{
  if (event->state & GDK_CONTROL_MASK)
    {
      PicmanZoomPreviewPrivate *priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);

      picman_scrolled_preview_freeze (PICMAN_SCROLLED_PREVIEW (preview));

      switch (event->direction)
        {
        case GDK_SCROLL_UP:
          picman_zoom_model_zoom (priv->model, PICMAN_ZOOM_IN, 0.0);
          break;

        case GDK_SCROLL_DOWN:
          picman_zoom_model_zoom (priv->model, PICMAN_ZOOM_OUT, 0.0);
          break;

        default:
          break;
        }

      picman_scrolled_preview_thaw (PICMAN_SCROLLED_PREVIEW (preview));
    }

  return FALSE;
}

static void
picman_zoom_preview_draw (PicmanPreview *preview)
{
  PicmanZoomPreviewPrivate *priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);
  PicmanDrawable           *drawable;
  guchar                 *data;
  gint                    width;
  gint                    height;
  gint                    bpp;

  if (! priv->model)
    return;

  drawable = priv->drawable;
  if (! drawable)
    return;

  data = picman_zoom_preview_get_source (PICMAN_ZOOM_PREVIEW (preview),
                                       &width, &height, &bpp);

  if (data)
    {
      picman_preview_area_draw (PICMAN_PREVIEW_AREA (preview->area),
                              0, 0, width, height,
                              picman_drawable_type (drawable->drawable_id),
                              data, width * bpp);
      g_free (data);
    }
}

static void
picman_zoom_preview_draw_buffer (PicmanPreview  *preview,
                               const guchar *buffer,
                               gint          rowstride)
{
  PicmanZoomPreviewPrivate *priv     = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);
  PicmanDrawable           *drawable = priv->drawable;
  gint32                  image_id;

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
      guchar  *sel;
      guchar  *src;
      gint     selection_id;
      gint     width, height;
      gint     bpp;
      gint     src_x;
      gint     src_y;
      gint     src_width;
      gint     src_height;
      gint     offsx = 0;
      gint     offsy = 0;

      selection_id = picman_image_get_selection (image_id);

      width  = preview->width;
      height = preview->height;

      picman_zoom_preview_get_source_area (preview,
                                         &src_x, &src_y,
                                         &src_width, &src_height);

      src = picman_drawable_get_sub_thumbnail_data (drawable->drawable_id,
                                                  src_x, src_y,
                                                  src_width, src_height,
                                                  &width, &height, &bpp);
      picman_drawable_offsets (drawable->drawable_id, &offsx, &offsy);
      sel = picman_drawable_get_sub_thumbnail_data (selection_id,
                                                  src_x + offsx, src_y + offsy,
                                                  src_width, src_height,
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
picman_zoom_preview_draw_thumb (PicmanPreview     *preview,
                              PicmanPreviewArea *area,
                              gint             width,
                              gint             height)
{
  PicmanZoomPreviewPrivate *priv     = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);
  PicmanDrawable           *drawable = priv->drawable;

  if (drawable)
    _picman_drawable_preview_area_draw_thumb (area, drawable, width, height);
}

static void
picman_zoom_preview_set_cursor (PicmanPreview *preview)
{
  if (! gtk_widget_get_realized (preview->area))
    return;

  if (picman_zoom_preview_get_factor (PICMAN_ZOOM_PREVIEW (preview)) > 1.0)
    {
      gdk_window_set_cursor (gtk_widget_get_window (preview->area),
                             PICMAN_SCROLLED_PREVIEW (preview)->cursor_move);
    }
  else
    {
      gdk_window_set_cursor (gtk_widget_get_window (preview->area),
                             preview->default_cursor);
    }
}

static void
picman_zoom_preview_transform (PicmanPreview *preview,
                             gint         src_x,
                             gint         src_y,
                             gint        *dest_x,
                             gint        *dest_y)
{
  PicmanZoomPreviewPrivate *priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);

  gdouble zoom = picman_zoom_preview_get_factor (PICMAN_ZOOM_PREVIEW (preview));

  *dest_x = ((gdouble) (src_x - priv->extents.x) *
             preview->width / priv->extents.width * zoom) - preview->xoff;

  *dest_y = ((gdouble) (src_y - priv->extents.y) *
             preview->height / priv->extents.height * zoom) - preview->yoff;
}

static void
picman_zoom_preview_untransform (PicmanPreview *preview,
                               gint         src_x,
                               gint         src_y,
                               gint        *dest_x,
                               gint        *dest_y)
{
  PicmanZoomPreviewPrivate *priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);

  gdouble zoom = picman_zoom_preview_get_factor (PICMAN_ZOOM_PREVIEW (preview));

  *dest_x = (priv->extents.x +
             ((gdouble) (src_x + preview->xoff) *
              priv->extents.width / preview->width / zoom));

  *dest_y = (priv->extents.y +
             ((gdouble) (src_y + preview->yoff) *
              priv->extents.height / preview->height / zoom));
}

static void
picman_zoom_preview_set_drawable (PicmanZoomPreview *preview,
                                PicmanDrawable    *drawable)
{
  PicmanZoomPreviewPrivate *priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);
  gint                    x, y;
  gint                    width, height;
  gint                    max_width, max_height;

  g_return_if_fail (priv->drawable == NULL);

  priv->drawable = drawable;

  if (picman_drawable_mask_intersect (drawable->drawable_id,
                                    &x, &y, &width, &height))
    {
      priv->extents.x = x;
      priv->extents.y = y;
    }
  else
    {
      width  = picman_drawable_width  (drawable->drawable_id);
      height = picman_drawable_height (drawable->drawable_id);

      priv->extents.x = 0;
      priv->extents.y = 0;
    }

  priv->extents.width  = width;
  priv->extents.height = height;

  if (width > height)
    {
      max_width  = MIN (width, 512);
      max_height = (height * max_width) / width;
    }
  else
    {
      max_height = MIN (height, 512);
      max_width  = (width * max_height) / height;
    }

  picman_preview_set_bounds (PICMAN_PREVIEW (preview),
                           0, 0, max_width, max_height);

  g_object_set (PICMAN_PREVIEW (preview)->frame,
                "ratio", (gdouble) width / (gdouble) height,
                NULL);
}

static void
picman_zoom_preview_set_model (PicmanZoomPreview *preview,
                             PicmanZoomModel   *model)
{
  PicmanZoomPreviewPrivate *priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);
  GtkWidget              *button_bar;
  GtkWidget              *button;
  GtkWidget              *box;

  g_return_if_fail (priv->model == NULL);

  if (! model)
    return;

  priv->model = g_object_ref (model);

  g_signal_connect_swapped (priv->model, "zoomed",
                            G_CALLBACK (picman_zoom_preview_set_adjustments),
                            preview);

  box = picman_preview_get_controls (PICMAN_PREVIEW (preview));
  g_return_if_fail (GTK_IS_BOX (box));

  button_bar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_end (GTK_BOX (box), button_bar, FALSE, FALSE, 0);
  gtk_widget_show (button_bar);

  /* zoom out */
  button = picman_zoom_button_new (priv->model,
                                 PICMAN_ZOOM_OUT, GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_box_pack_start (GTK_BOX (button_bar), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  /* zoom in */
  button = picman_zoom_button_new (priv->model,
                                 PICMAN_ZOOM_IN, GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_box_pack_start (GTK_BOX (button_bar), button, FALSE, FALSE, 0);
  gtk_widget_show (button);
}

static void
picman_zoom_preview_get_source_area (PicmanPreview *preview,
                                   gint        *x,
                                   gint        *y,
                                   gint        *w,
                                   gint        *h)
{
  PicmanZoomPreviewPrivate *priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);
  gdouble                 zoom = picman_zoom_model_get_factor (priv->model);

  picman_zoom_preview_untransform (preview, 0, 0, x, y);

  *w = priv->extents.width / zoom;
  *h = priv->extents.height / zoom;
}


/**
 * picman_zoom_preview_new:
 * @drawable: a #PicmanDrawable
 *
 * Creates a new #PicmanZoomPreview widget for @drawable.
 *
 * Since: PICMAN 2.4
 *
 * Returns: a new #PicmanZoomPreview.
 **/
GtkWidget *
picman_zoom_preview_new (PicmanDrawable *drawable)
{
  g_return_val_if_fail (drawable != NULL, NULL);

  return g_object_new (PICMAN_TYPE_ZOOM_PREVIEW,
                       "drawable", drawable,
                       NULL);
}

/**
 * picman_zoom_preview_new_with_model:
 * @drawable: a #PicmanDrawable
 * @model:    a #PicmanZoomModel
 *
 * Creates a new #PicmanZoomPreview widget for @drawable using the
 * given @model.
 *
 * This variant of picman_zoom_preview_new() allows you to create a
 * preview using an existing zoom model. This may be useful if for
 * example you want to have two zoom previews that keep their zoom
 * factor in sync.
 *
 * Since: PICMAN 2.4
 *
 * Returns: a new #PicmanZoomPreview.
 **/
GtkWidget *
picman_zoom_preview_new_with_model (PicmanDrawable  *drawable,
                                  PicmanZoomModel *model)

{
  g_return_val_if_fail (drawable != NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_ZOOM_MODEL (model), NULL);

  return g_object_new (PICMAN_TYPE_ZOOM_PREVIEW,
                       "drawable", drawable,
                       "model",    model,
                       NULL);
}


/**
 * picman_zoom_preview_get_drawable:
 * @preview: a #PicmanZoomPreview widget
 *
 * Returns the #PicmanDrawable the #PicmanZoomPreview is attached to.
 *
 * Return Value: the #PicmanDrawable that was passed to picman_zoom_preview_new().
 *
 * Since: PICMAN 2.4
 **/
PicmanDrawable *
picman_zoom_preview_get_drawable (PicmanZoomPreview *preview)
{
  g_return_val_if_fail (PICMAN_IS_ZOOM_PREVIEW (preview), NULL);

  return PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview)->drawable;
}

/**
 * picman_zoom_preview_get_model:
 * @preview: a #PicmanZoomPreview widget
 *
 * Returns the #PicmanZoomModel the preview is using.
 *
 * Return Value: a pointer to the #PicmanZoomModel owned by the @preview
 *
 * Since: PICMAN 2.4
 **/
PicmanZoomModel *
picman_zoom_preview_get_model (PicmanZoomPreview *preview)
{
  g_return_val_if_fail (PICMAN_IS_ZOOM_PREVIEW (preview), NULL);

  return PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview)->model;
}

/**
 * picman_zoom_preview_get_factor:
 * @preview: a #PicmanZoomPreview widget
 *
 * Returns the zoom factor the preview is currently using.
 *
 * Return Value: the current zoom factor
 *
 * Since: PICMAN 2.4
 **/
gdouble
picman_zoom_preview_get_factor (PicmanZoomPreview *preview)
{
  PicmanZoomPreviewPrivate *priv;

  g_return_val_if_fail (PICMAN_IS_ZOOM_PREVIEW (preview), 1.0);

  priv = PICMAN_ZOOM_PREVIEW_GET_PRIVATE (preview);

  return priv->model ? picman_zoom_model_get_factor (priv->model) : 1.0;
}

/**
 * picman_zoom_preview_get_source:
 * @preview: a #PicmanZoomPreview widget
 * @width: a pointer to an int where the current width of the zoom widget
 *         will be put.
 * @height: a pointer to an int where the current width of the zoom widget
 *          will be put.
 * @bpp: return location for the number of bytes per pixel
 *
 * Returns the scaled image data of the part of the drawable the
 * #PicmanZoomPreview is currently showing, as a newly allocated array of guchar.
 * This function also allow to get the current width, height and bpp of the
 * #PicmanZoomPreview.
 *
 * Return Value: newly allocated data that should be released using g_free()
 *               when it is not any longer needed
 *
 * Since: PICMAN 2.4
 */
guchar *
picman_zoom_preview_get_source (PicmanZoomPreview *preview,
                              gint            *width,
                              gint            *height,
                              gint            *bpp)
{
  PicmanDrawable *drawable;

  g_return_val_if_fail (PICMAN_IS_ZOOM_PREVIEW (preview), NULL);
  g_return_val_if_fail (width != NULL && height != NULL && bpp != NULL, NULL);

  drawable = picman_zoom_preview_get_drawable (preview);

  if (drawable)
    {
      PicmanPreview *picman_preview = PICMAN_PREVIEW (preview);
      gint         src_x;
      gint         src_y;
      gint         src_width;
      gint         src_height;

      *width  = picman_preview->width;
      *height = picman_preview->height;

      picman_zoom_preview_get_source_area (picman_preview,
                                         &src_x, &src_y,
                                         &src_width, &src_height);

      return picman_drawable_get_sub_thumbnail_data (drawable->drawable_id,
                                                   src_x, src_y,
                                                   src_width, src_height,
                                                   width, height, bpp);
    }
  else
    {
      *width  = 0;
      *height = 0;
      *bpp    = 0;

      return NULL;
    }
}
