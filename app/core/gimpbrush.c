/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include <cairo.h>
#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "picmanbezierdesc.h"
#include "picmanbrush.h"
#include "picmanbrush-boundary.h"
#include "picmanbrush-load.h"
#include "picmanbrush-transform.h"
#include "picmanbrushcache.h"
#include "picmanbrushgenerated.h"
#include "picmanmarshal.h"
#include "picmantagged.h"
#include "picmantempbuf.h"

#include "picman-intl.h"


enum
{
  SPACING_CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SPACING
};


static void          picman_brush_tagged_iface_init     (PicmanTaggedInterface  *iface);

static void          picman_brush_finalize              (GObject              *object);
static void          picman_brush_set_property          (GObject              *object,
                                                       guint                 property_id,
                                                       const GValue         *value,
                                                       GParamSpec           *pspec);
static void          picman_brush_get_property          (GObject              *object,
                                                       guint                 property_id,
                                                       GValue               *value,
                                                       GParamSpec           *pspec);

static gint64        picman_brush_get_memsize           (PicmanObject           *object,
                                                       gint64               *gui_size);

static gboolean      picman_brush_get_size              (PicmanViewable         *viewable,
                                                       gint                 *width,
                                                       gint                 *height);
static PicmanTempBuf * picman_brush_get_new_preview       (PicmanViewable         *viewable,
                                                       PicmanContext          *context,
                                                       gint                  width,
                                                       gint                  height);
static gchar       * picman_brush_get_description       (PicmanViewable         *viewable,
                                                       gchar               **tooltip);

static void          picman_brush_dirty                 (PicmanData             *data);
static const gchar * picman_brush_get_extension         (PicmanData             *data);

static void          picman_brush_real_begin_use        (PicmanBrush            *brush);
static void          picman_brush_real_end_use          (PicmanBrush            *brush);
static PicmanBrush   * picman_brush_real_select_brush     (PicmanBrush            *brush,
                                                       const PicmanCoords     *last_coords,
                                                       const PicmanCoords     *current_coords);
static gboolean      picman_brush_real_want_null_motion (PicmanBrush            *brush,
                                                       const PicmanCoords     *last_coords,
                                                       const PicmanCoords     *current_coords);

static gchar       * picman_brush_get_checksum          (PicmanTagged           *tagged);


G_DEFINE_TYPE_WITH_CODE (PicmanBrush, picman_brush, PICMAN_TYPE_DATA,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_TAGGED,
                                                picman_brush_tagged_iface_init))

#define parent_class picman_brush_parent_class

static guint brush_signals[LAST_SIGNAL] = { 0 };


static void
picman_brush_class_init (PicmanBrushClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanDataClass     *data_class        = PICMAN_DATA_CLASS (klass);

  brush_signals[SPACING_CHANGED] =
    g_signal_new ("spacing-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanBrushClass, spacing_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->finalize           = picman_brush_finalize;
  object_class->get_property       = picman_brush_get_property;
  object_class->set_property       = picman_brush_set_property;

  picman_object_class->get_memsize   = picman_brush_get_memsize;

  viewable_class->default_stock_id = "picman-tool-paintbrush";
  viewable_class->get_size         = picman_brush_get_size;
  viewable_class->get_new_preview  = picman_brush_get_new_preview;
  viewable_class->get_description  = picman_brush_get_description;

  data_class->dirty                = picman_brush_dirty;
  data_class->get_extension        = picman_brush_get_extension;

  klass->begin_use                 = picman_brush_real_begin_use;
  klass->end_use                   = picman_brush_real_end_use;
  klass->select_brush              = picman_brush_real_select_brush;
  klass->want_null_motion          = picman_brush_real_want_null_motion;
  klass->transform_size            = picman_brush_real_transform_size;
  klass->transform_mask            = picman_brush_real_transform_mask;
  klass->transform_pixmap          = picman_brush_real_transform_pixmap;
  klass->transform_boundary        = picman_brush_real_transform_boundary;
  klass->spacing_changed           = NULL;

  g_object_class_install_property (object_class, PROP_SPACING,
                                   g_param_spec_double ("spacing", NULL,
                                                        _("Brush Spacing"),
                                                        1.0, 5000.0, 20.0,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
}

static void
picman_brush_tagged_iface_init (PicmanTaggedInterface *iface)
{
  iface->get_checksum = picman_brush_get_checksum;
}

static void
picman_brush_init (PicmanBrush *brush)
{
  brush->mask     = NULL;
  brush->pixmap   = NULL;

  brush->spacing  = 20;
  brush->x_axis.x = 15.0;
  brush->x_axis.y =  0.0;
  brush->y_axis.x =  0.0;
  brush->y_axis.y = 15.0;
}

static void
picman_brush_finalize (GObject *object)
{
  PicmanBrush *brush = PICMAN_BRUSH (object);

  if (brush->mask)
    {
      picman_temp_buf_unref (brush->mask);
      brush->mask = NULL;
    }

  if (brush->pixmap)
    {
      picman_temp_buf_unref (brush->pixmap);
      brush->pixmap = NULL;
    }

  if (brush->mask_cache)
    {
      g_object_unref (brush->mask_cache);
      brush->mask_cache = NULL;
    }

  if (brush->pixmap_cache)
    {
      g_object_unref (brush->pixmap_cache);
      brush->pixmap_cache = NULL;
    }

  if (brush->boundary_cache)
    {
      g_object_unref (brush->boundary_cache);
      brush->boundary_cache = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_brush_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  PicmanBrush *brush = PICMAN_BRUSH (object);

  switch (property_id)
    {
    case PROP_SPACING:
      picman_brush_set_spacing (brush, g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_brush_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  PicmanBrush *brush = PICMAN_BRUSH (object);

  switch (property_id)
    {
    case PROP_SPACING:
      g_value_set_double (value, brush->spacing);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_brush_get_memsize (PicmanObject *object,
                        gint64     *gui_size)
{
  PicmanBrush *brush   = PICMAN_BRUSH (object);
  gint64     memsize = 0;

  memsize += picman_temp_buf_get_memsize (brush->mask);
  memsize += picman_temp_buf_get_memsize (brush->pixmap);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_brush_get_size (PicmanViewable *viewable,
                     gint         *width,
                     gint         *height)
{
  PicmanBrush *brush = PICMAN_BRUSH (viewable);

  *width  = picman_temp_buf_get_width  (brush->mask);
  *height = picman_temp_buf_get_height (brush->mask);

  return TRUE;
}

static PicmanTempBuf *
picman_brush_get_new_preview (PicmanViewable *viewable,
                            PicmanContext  *context,
                            gint          width,
                            gint          height)
{
  PicmanBrush         *brush       = PICMAN_BRUSH (viewable);
  const PicmanTempBuf *mask_buf    = NULL;
  const PicmanTempBuf *pixmap_buf  = NULL;
  PicmanTempBuf       *return_buf  = NULL;
  gint               mask_width;
  gint               mask_height;
  guchar            *mask;
  guchar            *buf;
  gint               x, y;
  gboolean           scaled = FALSE;

  mask_buf   = brush->mask;
  pixmap_buf = brush->pixmap;

  mask_width  = picman_temp_buf_get_width  (mask_buf);
  mask_height = picman_temp_buf_get_height (mask_buf);

  if (mask_width > width || mask_height > height)
    {
      gdouble ratio_x = (gdouble) width  / (gdouble) mask_width;
      gdouble ratio_y = (gdouble) height / (gdouble) mask_height;
      gdouble scale   = MIN (ratio_x, ratio_y);

      if (scale != 1.0)
        {
          picman_brush_begin_use (brush);

          mask_buf = picman_brush_transform_mask (brush, scale,
                                                0.0, 0.0, 1.0);

          if (! mask_buf)
            {
              mask_buf = picman_temp_buf_new (1, 1, babl_format ("Y u8"));
              picman_temp_buf_data_clear ((PicmanTempBuf *) mask_buf);
            }
          else
            {
              picman_temp_buf_ref ((PicmanTempBuf *) mask_buf);
            }

          if (pixmap_buf)
            pixmap_buf = picman_brush_transform_pixmap (brush, scale,
                                                      0.0, 0.0, 1.0);

          mask_width  = picman_temp_buf_get_width  (mask_buf);
          mask_height = picman_temp_buf_get_height (mask_buf);

          scaled = TRUE;
        }
    }

  return_buf = picman_temp_buf_new (mask_width, mask_height,
                                  babl_format ("R'G'B'A u8"));
  picman_temp_buf_data_clear (return_buf);

  mask = picman_temp_buf_get_data (mask_buf);
  buf  = picman_temp_buf_get_data (return_buf);

  if (pixmap_buf)
    {
      guchar *pixmap = picman_temp_buf_get_data (pixmap_buf);

      for (y = 0; y < mask_height; y++)
        {
          for (x = 0; x < mask_width ; x++)
            {
              *buf++ = *pixmap++;
              *buf++ = *pixmap++;
              *buf++ = *pixmap++;
              *buf++ = *mask++;
            }
        }
    }
  else
    {
      for (y = 0; y < mask_height; y++)
        {
          for (x = 0; x < mask_width ; x++)
            {
              *buf++ = 0;
              *buf++ = 0;
              *buf++ = 0;
              *buf++ = *mask++;
            }
        }
    }

  if (scaled)
    {
      picman_temp_buf_unref ((PicmanTempBuf *) mask_buf);

      picman_brush_end_use (brush);
    }

  return return_buf;
}

static gchar *
picman_brush_get_description (PicmanViewable  *viewable,
                            gchar        **tooltip)
{
  PicmanBrush *brush = PICMAN_BRUSH (viewable);

  return g_strdup_printf ("%s (%d × %d)",
                          picman_object_get_name (brush),
                          picman_temp_buf_get_width  (brush->mask),
                          picman_temp_buf_get_height (brush->mask));
}

static void
picman_brush_dirty (PicmanData *data)
{
  PicmanBrush *brush = PICMAN_BRUSH (data);

  if (brush->mask_cache)
    picman_brush_cache_clear (brush->mask_cache);

  if (brush->pixmap_cache)
    picman_brush_cache_clear (brush->pixmap_cache);

  if (brush->boundary_cache)
    picman_brush_cache_clear (brush->boundary_cache);

  PICMAN_DATA_CLASS (parent_class)->dirty (data);
}

static const gchar *
picman_brush_get_extension (PicmanData *data)
{
  return PICMAN_BRUSH_FILE_EXTENSION;
}

static void
picman_brush_real_begin_use (PicmanBrush *brush)
{
  brush->mask_cache =
    picman_brush_cache_new ((GDestroyNotify) picman_temp_buf_unref, 'M', 'm');

  brush->pixmap_cache =
    picman_brush_cache_new ((GDestroyNotify) picman_temp_buf_unref, 'P', 'p');

  brush->boundary_cache =
    picman_brush_cache_new ((GDestroyNotify) picman_bezier_desc_free, 'B', 'b');
}

static void
picman_brush_real_end_use (PicmanBrush *brush)
{
  g_object_unref (brush->mask_cache);
  brush->mask_cache = NULL;

  g_object_unref (brush->pixmap_cache);
  brush->pixmap_cache = NULL;

  g_object_unref (brush->boundary_cache);
  brush->boundary_cache = NULL;
}

static PicmanBrush *
picman_brush_real_select_brush (PicmanBrush        *brush,
                              const PicmanCoords *last_coords,
                              const PicmanCoords *current_coords)
{
  return brush;
}

static gboolean
picman_brush_real_want_null_motion (PicmanBrush        *brush,
                                  const PicmanCoords *last_coords,
                                  const PicmanCoords *current_coords)
{
  return TRUE;
}

static gchar *
picman_brush_get_checksum (PicmanTagged *tagged)
{
  PicmanBrush *brush           = PICMAN_BRUSH (tagged);
  gchar     *checksum_string = NULL;

  if (brush->mask)
    {
      GChecksum *checksum = g_checksum_new (G_CHECKSUM_MD5);

      g_checksum_update (checksum, picman_temp_buf_get_data (brush->mask),
                         picman_temp_buf_get_data_size (brush->mask));
      if (brush->pixmap)
        g_checksum_update (checksum, picman_temp_buf_get_data (brush->pixmap),
                           picman_temp_buf_get_data_size (brush->pixmap));
      g_checksum_update (checksum, (const guchar *) &brush->spacing, sizeof (brush->spacing));
      g_checksum_update (checksum, (const guchar *) &brush->x_axis, sizeof (brush->x_axis));
      g_checksum_update (checksum, (const guchar *) &brush->y_axis, sizeof (brush->y_axis));

      checksum_string = g_strdup (g_checksum_get_string (checksum));

      g_checksum_free (checksum);
    }

  return checksum_string;
}

/*  public functions  */

PicmanData *
picman_brush_new (PicmanContext *context,
                const gchar *name)
{
  g_return_val_if_fail (name != NULL, NULL);

  return picman_brush_generated_new (name,
                                   PICMAN_BRUSH_GENERATED_CIRCLE,
                                   5.0, 2, 0.5, 1.0, 0.0);
}

PicmanData *
picman_brush_get_standard (PicmanContext *context)
{
  static PicmanData *standard_brush = NULL;

  if (! standard_brush)
    {
      standard_brush = picman_brush_new (context, "Standard");

      picman_data_clean (standard_brush);
      picman_data_make_internal (standard_brush, "picman-brush-standard");

      g_object_add_weak_pointer (G_OBJECT (standard_brush),
                                 (gpointer *) &standard_brush);
    }

  return standard_brush;
}

void
picman_brush_begin_use (PicmanBrush *brush)
{
  g_return_if_fail (PICMAN_IS_BRUSH (brush));

  brush->use_count++;

  if (brush->use_count == 1)
    PICMAN_BRUSH_GET_CLASS (brush)->begin_use (brush);
}

void
picman_brush_end_use (PicmanBrush *brush)
{
  g_return_if_fail (PICMAN_IS_BRUSH (brush));
  g_return_if_fail (brush->use_count > 0);

  brush->use_count--;

  if (brush->use_count == 0)
    PICMAN_BRUSH_GET_CLASS (brush)->end_use (brush);
}

PicmanBrush *
picman_brush_select_brush (PicmanBrush        *brush,
                         const PicmanCoords *last_coords,
                         const PicmanCoords *current_coords)
{
  g_return_val_if_fail (PICMAN_IS_BRUSH (brush), NULL);
  g_return_val_if_fail (last_coords != NULL, NULL);
  g_return_val_if_fail (current_coords != NULL, NULL);

  return PICMAN_BRUSH_GET_CLASS (brush)->select_brush (brush,
                                                     last_coords,
                                                     current_coords);
}

gboolean
picman_brush_want_null_motion (PicmanBrush        *brush,
                             const PicmanCoords *last_coords,
                             const PicmanCoords *current_coords)
{
  g_return_val_if_fail (PICMAN_IS_BRUSH (brush), FALSE);
  g_return_val_if_fail (last_coords != NULL, FALSE);
  g_return_val_if_fail (current_coords != NULL, FALSE);

  return PICMAN_BRUSH_GET_CLASS (brush)->want_null_motion (brush,
                                                         last_coords,
                                                         current_coords);
}

void
picman_brush_transform_size (PicmanBrush     *brush,
                           gdouble        scale,
                           gdouble        aspect_ratio,
                           gdouble        angle,
                           gint          *width,
                           gint          *height)
{
  g_return_if_fail (PICMAN_IS_BRUSH (brush));
  g_return_if_fail (scale > 0.0);
  g_return_if_fail (width != NULL);
  g_return_if_fail (height != NULL);

  if (scale        == 1.0 &&
      aspect_ratio == 0.0 &&
      ((angle == 0.0) || (angle == 0.5) || (angle == 1.0)))
    {
      *width  = picman_temp_buf_get_width  (brush->mask);
      *height = picman_temp_buf_get_height (brush->mask);;

      return;
    }

  PICMAN_BRUSH_GET_CLASS (brush)->transform_size (brush,
                                                scale, aspect_ratio, angle,
                                                width, height);
}

const PicmanTempBuf *
picman_brush_transform_mask (PicmanBrush *brush,
                           gdouble    scale,
                           gdouble    aspect_ratio,
                           gdouble    angle,
                           gdouble    hardness)
{
  const PicmanTempBuf *mask;
  gint               width;
  gint               height;

  g_return_val_if_fail (PICMAN_IS_BRUSH (brush), NULL);
  g_return_val_if_fail (scale > 0.0, NULL);

  picman_brush_transform_size (brush,
                             scale, aspect_ratio, angle,
                             &width, &height);

  mask = picman_brush_cache_get (brush->mask_cache,
                               width, height,
                               scale, aspect_ratio, angle, hardness);

  if (! mask)
    {
      if (scale        == 1.0 &&
          aspect_ratio == 0.0 &&
          angle        == 0.0 &&
          hardness     == 1.0)
        {
          mask = picman_temp_buf_copy (brush->mask);
        }
      else
        {
          mask = PICMAN_BRUSH_GET_CLASS (brush)->transform_mask (brush,
                                                               scale,
                                                               aspect_ratio,
                                                               angle,
                                                               hardness);
        }

      picman_brush_cache_add (brush->mask_cache,
                            (gpointer) mask,
                            width, height,
                            scale, aspect_ratio, angle, hardness);
    }

  return mask;
}

const PicmanTempBuf *
picman_brush_transform_pixmap (PicmanBrush *brush,
                             gdouble    scale,
                             gdouble    aspect_ratio,
                             gdouble    angle,
                             gdouble    hardness)
{
  const PicmanTempBuf *pixmap;
  gint               width;
  gint               height;

  g_return_val_if_fail (PICMAN_IS_BRUSH (brush), NULL);
  g_return_val_if_fail (brush->pixmap != NULL, NULL);
  g_return_val_if_fail (scale > 0.0, NULL);

  picman_brush_transform_size (brush,
                             scale, aspect_ratio, angle,
                             &width, &height);

  pixmap = picman_brush_cache_get (brush->pixmap_cache,
                                 width, height,
                                 scale, aspect_ratio, angle, hardness);

  if (! pixmap)
    {
      if (scale        == 1.0 &&
          aspect_ratio == 0.0 &&
          angle        == 0.0 &&
          hardness     == 1.0)
        {
          pixmap = picman_temp_buf_copy (brush->pixmap);
        }
      else
        {
          pixmap = PICMAN_BRUSH_GET_CLASS (brush)->transform_pixmap (brush,
                                                                   scale,
                                                                   aspect_ratio,
                                                                   angle,
                                                                   hardness);
        }

      picman_brush_cache_add (brush->pixmap_cache,
                            (gpointer) pixmap,
                            width, height,
                            scale, aspect_ratio, angle, hardness);
    }

  return pixmap;
}

const PicmanBezierDesc *
picman_brush_transform_boundary (PicmanBrush *brush,
                               gdouble    scale,
                               gdouble    aspect_ratio,
                               gdouble    angle,
                               gdouble    hardness,
                               gint      *width,
                               gint      *height)
{
  const PicmanBezierDesc *boundary;

  g_return_val_if_fail (PICMAN_IS_BRUSH (brush), NULL);
  g_return_val_if_fail (scale > 0.0, NULL);
  g_return_val_if_fail (width != NULL, NULL);
  g_return_val_if_fail (height != NULL, NULL);

  picman_brush_transform_size (brush,
                             scale, aspect_ratio, angle,
                             width, height);

  boundary = picman_brush_cache_get (brush->boundary_cache,
                                   *width, *height,
                                   scale, aspect_ratio, angle, hardness);

  if (! boundary)
    {
      boundary = PICMAN_BRUSH_GET_CLASS (brush)->transform_boundary (brush,
                                                                   scale,
                                                                   aspect_ratio,
                                                                   angle,
                                                                   hardness,
                                                                   width,
                                                                   height);

      /*  while the brush mask is always at least 1x1 pixels, its
       *  outline can correctly be NULL
       *
       *  FIXME: make the cache handle NULL things when it is
       *         properly implemented
       */
      if (boundary)
        picman_brush_cache_add (brush->boundary_cache,
                              (gpointer) boundary,
                              *width, *height,
                              scale, aspect_ratio, angle, hardness);
    }

  return boundary;
}

PicmanTempBuf *
picman_brush_get_mask (const PicmanBrush *brush)
{
  g_return_val_if_fail (brush != NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_BRUSH (brush), NULL);

  return brush->mask;
}

PicmanTempBuf *
picman_brush_get_pixmap (const PicmanBrush *brush)
{
  g_return_val_if_fail (brush != NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_BRUSH (brush), NULL);

  return brush->pixmap;
}

gint
picman_brush_get_spacing (const PicmanBrush *brush)
{
  g_return_val_if_fail (PICMAN_IS_BRUSH (brush), 0);

  return brush->spacing;
}

void
picman_brush_set_spacing (PicmanBrush *brush,
                        gint       spacing)
{
  g_return_if_fail (PICMAN_IS_BRUSH (brush));

  if (brush->spacing != spacing)
    {
      brush->spacing = spacing;

      g_signal_emit (brush, brush_signals[SPACING_CHANGED], 0);
      g_object_notify (G_OBJECT (brush), "spacing");
    }
}
