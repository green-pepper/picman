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

#include <stdlib.h>
#include <string.h> /* memcmp */

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picmancurve.h"
#include "picmancurve-load.h"
#include "picmancurve-save.h"
#include "picmanparamspecs.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_CURVE_TYPE,
  PROP_N_POINTS,
  PROP_POINTS,
  PROP_N_SAMPLES,
  PROP_SAMPLES
};


/*  local function prototypes  */

static void          picman_curve_config_iface_init (PicmanConfigInterface *iface);

static void          picman_curve_finalize          (GObject          *object);
static void          picman_curve_set_property      (GObject          *object,
                                                   guint             property_id,
                                                   const GValue     *value,
                                                   GParamSpec       *pspec);
static void          picman_curve_get_property      (GObject          *object,
                                                   guint             property_id,
                                                   GValue           *value,
                                                   GParamSpec       *pspec);

static gint64        picman_curve_get_memsize       (PicmanObject       *object,
                                                   gint64           *gui_size);

static void          picman_curve_get_preview_size  (PicmanViewable     *viewable,
                                                   gint              size,
                                                   gboolean          popup,
                                                   gboolean          dot_for_dot,
                                                   gint             *width,
                                                   gint             *height);
static gboolean      picman_curve_get_popup_size    (PicmanViewable     *viewable,
                                                   gint              width,
                                                   gint              height,
                                                   gboolean          dot_for_dot,
                                                   gint             *popup_width,
                                                   gint             *popup_height);
static PicmanTempBuf * picman_curve_get_new_preview   (PicmanViewable     *viewable,
                                                   PicmanContext      *context,
                                                   gint              width,
                                                   gint              height);
static gchar       * picman_curve_get_description   (PicmanViewable     *viewable,
                                                   gchar           **tooltip);

static void          picman_curve_dirty             (PicmanData         *data);
static const gchar * picman_curve_get_extension     (PicmanData         *data);
static PicmanData    * picman_curve_duplicate         (PicmanData         *data);

static gboolean      picman_curve_serialize         (PicmanConfig       *config,
                                                   PicmanConfigWriter *writer,
                                                   gpointer          data);
static gboolean      picman_curve_deserialize       (PicmanConfig       *config,
                                                   GScanner         *scanner,
                                                   gint              nest_level,
                                                   gpointer          data);
static gboolean      picman_curve_equal             (PicmanConfig       *a,
                                                   PicmanConfig       *b);
static void          _picman_curve_reset            (PicmanConfig       *config);
static gboolean      picman_curve_copy              (PicmanConfig       *src,
                                                   PicmanConfig       *dest,
                                                   GParamFlags       flags);

static void          picman_curve_set_n_points      (PicmanCurve        *curve,
                                                   gint              n_points);
static void          picman_curve_set_n_samples     (PicmanCurve        *curve,
                                                   gint              n_samples);

static void          picman_curve_calculate         (PicmanCurve        *curve);
static void          picman_curve_plot              (PicmanCurve        *curve,
                                                   gint              p1,
                                                   gint              p2,
                                                   gint              p3,
                                                   gint              p4);


G_DEFINE_TYPE_WITH_CODE (PicmanCurve, picman_curve, PICMAN_TYPE_DATA,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_curve_config_iface_init))

#define parent_class picman_curve_parent_class


static void
picman_curve_class_init (PicmanCurveClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanDataClass     *data_class        = PICMAN_DATA_CLASS (klass);
  GParamSpec        *array_spec;

  object_class->finalize           = picman_curve_finalize;
  object_class->set_property       = picman_curve_set_property;
  object_class->get_property       = picman_curve_get_property;

  picman_object_class->get_memsize   = picman_curve_get_memsize;

  viewable_class->default_stock_id = "FIXME";
  viewable_class->get_preview_size = picman_curve_get_preview_size;
  viewable_class->get_popup_size   = picman_curve_get_popup_size;
  viewable_class->get_new_preview  = picman_curve_get_new_preview;
  viewable_class->get_description  = picman_curve_get_description;

  data_class->dirty                = picman_curve_dirty;
  data_class->save                 = picman_curve_save;
  data_class->get_extension        = picman_curve_get_extension;
  data_class->duplicate            = picman_curve_duplicate;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_CURVE_TYPE,
                                 "curve-type",
                                 "The curve type",
                                 PICMAN_TYPE_CURVE_TYPE,
                                 PICMAN_CURVE_SMOOTH, 0);

  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_N_POINTS,
                                "n-points",
                                "The number of points",
                                17, 17, 17, 0);

  array_spec = g_param_spec_double ("point", NULL, NULL,
                                    -1.0, 1.0, 0.0, PICMAN_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_POINTS,
                                   picman_param_spec_value_array ("points",
                                                                NULL, NULL,
                                                                array_spec,
                                                                PICMAN_PARAM_STATIC_STRINGS |
                                                                PICMAN_CONFIG_PARAM_FLAGS));

  PICMAN_CONFIG_INSTALL_PROP_INT  (object_class, PROP_N_SAMPLES,
                                 "n-samples",
                                 "The number of samples",
                                 256, 256, 256, 0);

  array_spec = g_param_spec_double ("sample", NULL, NULL,
                                    0.0, 1.0, 0.0, PICMAN_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_SAMPLES,
                                   picman_param_spec_value_array ("samples",
                                                                NULL, NULL,
                                                                array_spec,
                                                                PICMAN_PARAM_STATIC_STRINGS |
                                                                PICMAN_CONFIG_PARAM_FLAGS));
}

static void
picman_curve_config_iface_init (PicmanConfigInterface *iface)
{
  iface->serialize   = picman_curve_serialize;
  iface->deserialize = picman_curve_deserialize;
  iface->equal       = picman_curve_equal;
  iface->reset       = _picman_curve_reset;
  iface->copy        = picman_curve_copy;
}

static void
picman_curve_init (PicmanCurve *curve)
{
  curve->n_points  = 0;
  curve->points    = NULL;
  curve->n_samples = 0;
  curve->samples   = NULL;
  curve->identity  = FALSE;
}

static void
picman_curve_finalize (GObject *object)
{
  PicmanCurve *curve = PICMAN_CURVE (object);

  if (curve->points)
    {
      g_free (curve->points);
      curve->points = NULL;
    }

  if (curve->samples)
    {
      g_free (curve->samples);
      curve->samples = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_curve_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  PicmanCurve *curve = PICMAN_CURVE (object);

  switch (property_id)
    {
    case PROP_CURVE_TYPE:
      picman_curve_set_curve_type (curve, g_value_get_enum (value));
      break;

    case PROP_N_POINTS:
      picman_curve_set_n_points (curve, g_value_get_int (value));
      break;

    case PROP_POINTS:
      {
        PicmanValueArray *array = g_value_get_boxed (value);
        gint            length;
        gint            i;

        if (! array)
          break;

        length = picman_value_array_length (array);

        for (i = 0; i < curve->n_points && i * 2 < length; i++)
          {
            GValue *x = picman_value_array_index (array, i * 2);
            GValue *y = picman_value_array_index (array, i * 2 + 1);

            curve->points[i].x = g_value_get_double (x);
            curve->points[i].y = g_value_get_double (y);
          }
      }
      break;

    case PROP_N_SAMPLES:
      picman_curve_set_n_samples (curve, g_value_get_int (value));
      break;

    case PROP_SAMPLES:
      {
        PicmanValueArray *array = g_value_get_boxed (value);
        gint            length;
        gint            i;

        if (! array)
          break;

        length = picman_value_array_length (array);

        for (i = 0; i < curve->n_samples && i < length; i++)
          {
            GValue *v = picman_value_array_index (array, i);

            curve->samples[i] = g_value_get_double (v);
          }
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_curve_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  PicmanCurve *curve = PICMAN_CURVE (object);

  switch (property_id)
    {
    case PROP_CURVE_TYPE:
      g_value_set_enum (value, curve->curve_type);
      break;

    case PROP_N_POINTS:
      g_value_set_int (value, curve->n_points);
      break;

    case PROP_POINTS:
      {
        PicmanValueArray *array = picman_value_array_new (curve->n_points * 2);
        GValue          v     = { 0, };
        gint            i;

        g_value_init (&v, G_TYPE_DOUBLE);

        for (i = 0; i < curve->n_points; i++)
          {
            g_value_set_double (&v, curve->points[i].x);
            picman_value_array_append (array, &v);

            g_value_set_double (&v, curve->points[i].y);
            picman_value_array_append (array, &v);
          }

        g_value_unset (&v);

        g_value_take_boxed (value, array);
      }
      break;

    case PROP_N_SAMPLES:
      g_value_set_int (value, curve->n_samples);
      break;

    case PROP_SAMPLES:
      {
        PicmanValueArray *array = picman_value_array_new (curve->n_samples);
        GValue          v     = { 0, };
        gint            i;

        g_value_init (&v, G_TYPE_DOUBLE);

        for (i = 0; i < curve->n_samples; i++)
          {
            g_value_set_double (&v, curve->samples[i]);
            picman_value_array_append (array, &v);
          }

        g_value_unset (&v);

        g_value_take_boxed (value, array);
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_curve_get_memsize (PicmanObject *object,
                        gint64     *gui_size)
{
  PicmanCurve *curve   = PICMAN_CURVE (object);
  gint64     memsize = 0;

  memsize += curve->n_points  * sizeof (PicmanVector2);
  memsize += curve->n_samples * sizeof (gdouble);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_curve_get_preview_size (PicmanViewable *viewable,
                             gint          size,
                             gboolean      popup,
                             gboolean      dot_for_dot,
                             gint         *width,
                             gint         *height)
{
  *width  = size;
  *height = size;
}

static gboolean
picman_curve_get_popup_size (PicmanViewable *viewable,
                           gint          width,
                           gint          height,
                           gboolean      dot_for_dot,
                           gint         *popup_width,
                           gint         *popup_height)
{
  *popup_width  = width  * 2;
  *popup_height = height * 2;

  return TRUE;
}

static PicmanTempBuf *
picman_curve_get_new_preview (PicmanViewable *viewable,
                            PicmanContext  *context,
                            gint          width,
                            gint          height)
{
  return NULL;
}

static gchar *
picman_curve_get_description (PicmanViewable  *viewable,
                            gchar        **tooltip)
{
  PicmanCurve *curve = PICMAN_CURVE (viewable);

  return g_strdup_printf ("%s", picman_object_get_name (curve));
}

static void
picman_curve_dirty (PicmanData *data)
{
  PicmanCurve *curve = PICMAN_CURVE (data);

  curve->identity = FALSE;

  picman_curve_calculate (curve);

  PICMAN_DATA_CLASS (parent_class)->dirty (data);
}

static const gchar *
picman_curve_get_extension (PicmanData *data)
{
  return PICMAN_CURVE_FILE_EXTENSION;
}

static PicmanData *
picman_curve_duplicate (PicmanData *data)
{
  PicmanCurve *new = g_object_new (PICMAN_TYPE_CURVE, NULL);

  picman_config_copy (PICMAN_CONFIG (data),
                    PICMAN_CONFIG (new), 0);

  return PICMAN_DATA (new);
}

static gboolean
picman_curve_serialize (PicmanConfig       *config,
                      PicmanConfigWriter *writer,
                      gpointer          data)
{
  return picman_config_serialize_properties (config, writer);
}

static gboolean
picman_curve_deserialize (PicmanConfig *config,
                        GScanner   *scanner,
                        gint        nest_level,
                        gpointer    data)
{
  gboolean success;

  success = picman_config_deserialize_properties (config, scanner, nest_level);

  PICMAN_CURVE (config)->identity = FALSE;

  return success;
}

static gboolean
picman_curve_equal (PicmanConfig *a,
                  PicmanConfig *b)
{
  PicmanCurve *a_curve = PICMAN_CURVE (a);
  PicmanCurve *b_curve = PICMAN_CURVE (b);

  if (a_curve->curve_type != b_curve->curve_type)
    return FALSE;

  if (memcmp (a_curve->points, b_curve->points,
              sizeof (PicmanVector2) * b_curve->n_points) ||
      memcmp (a_curve->samples, b_curve->samples,
              sizeof (gdouble) * b_curve->n_samples))
    return FALSE;

  return TRUE;
}

static void
_picman_curve_reset (PicmanConfig *config)
{
  picman_curve_reset (PICMAN_CURVE (config), TRUE);
}

static gboolean
picman_curve_copy (PicmanConfig  *src,
                 PicmanConfig  *dest,
                 GParamFlags  flags)
{
  PicmanCurve *src_curve  = PICMAN_CURVE (src);
  PicmanCurve *dest_curve = PICMAN_CURVE (dest);

  picman_config_sync (G_OBJECT (src), G_OBJECT (dest), flags);

  dest_curve->identity = src_curve->identity;

  picman_data_dirty (PICMAN_DATA (dest));

  return TRUE;
}


/*  public functions  */

PicmanData *
picman_curve_new (const gchar *name)
{
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (*name != '\0', NULL);

  return g_object_new (PICMAN_TYPE_CURVE,
                       "name", name,
                       NULL);
}

PicmanData *
picman_curve_get_standard (void)
{
  static PicmanData *standard_curve = NULL;

  if (! standard_curve)
    {
      standard_curve = picman_curve_new ("Standard");

      picman_data_clean (standard_curve);
      picman_data_make_internal (standard_curve,
                               "picman-curve-standard");

      g_object_ref (standard_curve);
    }

  return standard_curve;
}

void
picman_curve_reset (PicmanCurve *curve,
                  gboolean   reset_type)
{
  gint i;

  g_return_if_fail (PICMAN_IS_CURVE (curve));

  g_object_freeze_notify (G_OBJECT (curve));

  for (i = 0; i < curve->n_samples; i++)
    curve->samples[i] = (gdouble) i / (gdouble) (curve->n_samples - 1);

  g_object_notify (G_OBJECT (curve), "samples");

  curve->points[0].x = 0.0;
  curve->points[0].y = 0.0;

  for (i = 1; i < curve->n_points - 1; i++)
    {
      curve->points[i].x = -1.0;
      curve->points[i].y = -1.0;
    }

  curve->points[curve->n_points - 1].x = 1.0;
  curve->points[curve->n_points - 1].y = 1.0;

  g_object_notify (G_OBJECT (curve), "points");

  if (reset_type)
    {
      curve->curve_type = PICMAN_CURVE_SMOOTH;
      g_object_notify (G_OBJECT (curve), "curve-type");
    }

  curve->identity = TRUE;

  g_object_thaw_notify (G_OBJECT (curve));

  picman_data_dirty (PICMAN_DATA (curve));
}

void
picman_curve_set_curve_type (PicmanCurve     *curve,
                           PicmanCurveType  curve_type)
{
  g_return_if_fail (PICMAN_IS_CURVE (curve));

  if (curve->curve_type != curve_type)
    {
      g_object_freeze_notify (G_OBJECT (curve));

      curve->curve_type = curve_type;

      if (curve_type == PICMAN_CURVE_SMOOTH)
        {
          gint n_points;
          gint i;

          for (i = 0; i < curve->n_points; i++)
            {
              curve->points[i].x = -1;
              curve->points[i].y = -1;
            }

          /*  pick some points from the curve and make them control
           *  points
           */
          n_points = CLAMP (9, curve->n_points / 2, curve->n_points);

          for (i = 0; i < n_points; i++)
            {
              gint sample = i * (curve->n_samples - 1) / (n_points - 1);
              gint point  = i * (curve->n_points  - 1) / (n_points - 1);

              curve->points[point].x = ((gdouble) sample /
                                        (gdouble) (curve->n_samples - 1));
              curve->points[point].y = curve->samples[sample];
            }

          g_object_notify (G_OBJECT (curve), "points");
        }

      g_object_notify (G_OBJECT (curve), "curve-type");

      g_object_thaw_notify (G_OBJECT (curve));

      picman_data_dirty (PICMAN_DATA (curve));
    }
}

PicmanCurveType
picman_curve_get_curve_type (PicmanCurve *curve)
{
  g_return_val_if_fail (PICMAN_IS_CURVE (curve), PICMAN_CURVE_SMOOTH);

  return curve->curve_type;
}

static void
picman_curve_set_n_points (PicmanCurve *curve,
                         gint       n_points)
{
  g_return_if_fail (PICMAN_IS_CURVE (curve));

  if (n_points != curve->n_points)
    {
      gint i;

      g_object_freeze_notify (G_OBJECT (curve));

      curve->n_points = n_points;
      g_object_notify (G_OBJECT (curve), "n-points");

      curve->points = g_renew (PicmanVector2, curve->points, curve->n_points);

      curve->points[0].x = 0.0;
      curve->points[0].y = 0.0;

      for (i = 1; i < curve->n_points - 1; i++)
        {
          curve->points[i].x = -1.0;
          curve->points[i].y = -1.0;
        }

      curve->points[curve->n_points - 1].x = 1.0;
      curve->points[curve->n_points - 1].y = 1.0;

      g_object_notify (G_OBJECT (curve), "points");

      if (curve->curve_type == PICMAN_CURVE_SMOOTH)
        curve->identity = TRUE;

      g_object_thaw_notify (G_OBJECT (curve));
    }
}

gint
picman_curve_get_n_points (PicmanCurve *curve)
{
  g_return_val_if_fail (PICMAN_IS_CURVE (curve), 0);

  return curve->n_points;
}

static void
picman_curve_set_n_samples (PicmanCurve *curve,
                          gint       n_samples)
{
  g_return_if_fail (PICMAN_IS_CURVE (curve));

  if (n_samples != curve->n_samples)
    {
      gint i;

      g_object_freeze_notify (G_OBJECT (curve));

      curve->n_samples = n_samples;
      g_object_notify (G_OBJECT (curve), "n-samples");

      curve->samples = g_renew (gdouble, curve->samples, curve->n_samples);

      for (i = 0; i < curve->n_samples; i++)
        curve->samples[i] = (gdouble) i / (gdouble) (curve->n_samples - 1);

      g_object_notify (G_OBJECT (curve), "samples");

      if (curve->curve_type == PICMAN_CURVE_FREE)
        curve->identity = TRUE;

      g_object_thaw_notify (G_OBJECT (curve));
    }
}

gint
picman_curve_get_n_samples (PicmanCurve *curve)
{
  g_return_val_if_fail (PICMAN_IS_CURVE (curve), 0);

  return curve->n_samples;
}

gint
picman_curve_get_closest_point (PicmanCurve *curve,
                              gdouble    x)
{
  gint    closest_point = 0;
  gdouble distance      = G_MAXDOUBLE;
  gint    i;

  g_return_val_if_fail (PICMAN_IS_CURVE (curve), 0);

  for (i = 0; i < curve->n_points; i++)
    {
      if (curve->points[i].x >= 0.0 &&
          fabs (x - curve->points[i].x) < distance)
        {
          distance = fabs (x - curve->points[i].x);
          closest_point = i;
        }
    }

  if (distance > (1.0 / (curve->n_points * 2.0)))
    closest_point = ROUND (x * (gdouble) (curve->n_points - 1));

  return closest_point;
}

void
picman_curve_set_point (PicmanCurve *curve,
                      gint       point,
                      gdouble    x,
                      gdouble    y)
{
  g_return_if_fail (PICMAN_IS_CURVE (curve));
  g_return_if_fail (point >= 0 && point < curve->n_points);
  g_return_if_fail (x == -1.0 || (x >= 0 && x <= 1.0));
  g_return_if_fail (y == -1.0 || (y >= 0 && y <= 1.0));

  if (curve->curve_type == PICMAN_CURVE_FREE)
    return;

  g_object_freeze_notify (G_OBJECT (curve));

  curve->points[point].x = x;
  curve->points[point].y = y;

  g_object_notify (G_OBJECT (curve), "points");

  g_object_thaw_notify (G_OBJECT (curve));

  picman_data_dirty (PICMAN_DATA (curve));
}

void
picman_curve_move_point (PicmanCurve *curve,
                       gint       point,
                       gdouble    y)
{
  g_return_if_fail (PICMAN_IS_CURVE (curve));
  g_return_if_fail (point >= 0 && point < curve->n_points);
  g_return_if_fail (y >= 0 && y <= 1.0);

  if (curve->curve_type == PICMAN_CURVE_FREE)
    return;

  g_object_freeze_notify (G_OBJECT (curve));

  curve->points[point].y = y;

  g_object_notify (G_OBJECT (curve), "points");

  g_object_thaw_notify (G_OBJECT (curve));

  picman_data_dirty (PICMAN_DATA (curve));
}

void
picman_curve_delete_point (PicmanCurve *curve,
                         gint       point)
{
  g_return_if_fail (PICMAN_IS_CURVE (curve));
  g_return_if_fail (point >= 0 && point < curve->n_points);

  if (point == 0)
    {
      curve->points[0].x = 0.0;
      curve->points[0].y = 0.0;
    }
  else if (point == curve->n_points - 1)
    {
      curve->points[curve->n_points - 1].x = 1.0;
      curve->points[curve->n_points - 1].y = 1.0;
    }
  else
    {
      curve->points[point].x = -1.0;
      curve->points[point].y = -1.0;
    }

  g_object_notify (G_OBJECT (curve), "points");

  picman_data_dirty (PICMAN_DATA (curve));
}

void
picman_curve_get_point (PicmanCurve *curve,
                      gint       point,
                      gdouble   *x,
                      gdouble   *y)
{
  g_return_if_fail (PICMAN_IS_CURVE (curve));
  g_return_if_fail (point >= 0 && point < curve->n_points);

  if (curve->curve_type == PICMAN_CURVE_FREE)
    {
      if (x) *x = -1.0;
      if (y) *y = -1.0;

      return;
    }

  if (x) *x = curve->points[point].x;
  if (y) *y = curve->points[point].y;
}

void
picman_curve_set_curve (PicmanCurve *curve,
                      gdouble    x,
                      gdouble    y)
{
  g_return_if_fail (PICMAN_IS_CURVE (curve));
  g_return_if_fail (x >= 0 && x <= 1.0);
  g_return_if_fail (y >= 0 && y <= 1.0);

  if (curve->curve_type == PICMAN_CURVE_SMOOTH)
    return;

  g_object_freeze_notify (G_OBJECT (curve));

  curve->samples[ROUND (x * (gdouble) (curve->n_samples - 1))] = y;

  g_object_notify (G_OBJECT (curve), "samples");

  g_object_thaw_notify (G_OBJECT (curve));

  picman_data_dirty (PICMAN_DATA (curve));
}

/**
 * picman_curve_is_identity:
 * @curve: a #PicmanCurve object
 *
 * If this function returns %TRUE, then the curve maps each value to
 * itself. If it returns %FALSE, then this assumption can not be made.
 *
 * Return value: %TRUE if the curve is an identity mapping, %FALSE otherwise.
 **/
gboolean
picman_curve_is_identity (PicmanCurve *curve)
{
  g_return_val_if_fail (PICMAN_IS_CURVE (curve), FALSE);

  return curve->identity;
}

void
picman_curve_get_uchar (PicmanCurve *curve,
                      gint       n_samples,
                      guchar    *samples)
{
  gint i;

  g_return_if_fail (PICMAN_IS_CURVE (curve));
  /* FIXME: support n_samples != curve->n_samples */
  g_return_if_fail (n_samples == curve->n_samples);
  g_return_if_fail (samples != NULL);

  for (i = 0; i < curve->n_samples; i++)
    samples[i] = curve->samples[i] * 255.999;
}


/*  private functions  */

static void
picman_curve_calculate (PicmanCurve *curve)
{
  gint *points;
  gint  i;
  gint  num_pts;
  gint  p1, p2, p3, p4;

  if (picman_data_is_frozen (PICMAN_DATA (curve)))
    return;

  points = g_newa (gint, curve->n_points);

  switch (curve->curve_type)
    {
    case PICMAN_CURVE_SMOOTH:
      /*  cycle through the curves  */
      num_pts = 0;
      for (i = 0; i < curve->n_points; i++)
        if (curve->points[i].x >= 0.0)
          points[num_pts++] = i;

      /*  Initialize boundary curve points */
      if (num_pts != 0)
        {
          PicmanVector2 point;
          gint        boundary;

          point    = curve->points[points[0]];
          boundary = ROUND (point.x * (gdouble) (curve->n_samples - 1));

          for (i = 0; i < boundary; i++)
            curve->samples[i] = point.y;

          point    = curve->points[points[num_pts - 1]];
          boundary = ROUND (point.x * (gdouble) (curve->n_samples - 1));

          for (i = boundary; i < curve->n_samples; i++)
            curve->samples[i] = point.y;
        }

      for (i = 0; i < num_pts - 1; i++)
        {
          p1 = points[MAX (i - 1, 0)];
          p2 = points[i];
          p3 = points[i + 1];
          p4 = points[MIN (i + 2, num_pts - 1)];

          picman_curve_plot (curve, p1, p2, p3, p4);
        }

      /* ensure that the control points are used exactly */
      for (i = 0; i < num_pts; i++)
        {
          gdouble x = curve->points[points[i]].x;
          gdouble y = curve->points[points[i]].y;

          curve->samples[ROUND (x * (gdouble) (curve->n_samples - 1))] = y;
        }

      g_object_notify (G_OBJECT (curve), "samples");
      break;

    case PICMAN_CURVE_FREE:
      break;
    }
}

/*
 * This function calculates the curve values between the control points
 * p2 and p3, taking the potentially existing neighbors p1 and p4 into
 * account.
 *
 * This function uses a cubic bezier curve for the individual segments and
 * calculates the necessary intermediate control points depending on the
 * neighbor curve control points.
 */
static void
picman_curve_plot (PicmanCurve *curve,
                 gint       p1,
                 gint       p2,
                 gint       p3,
                 gint       p4)
{
  gint    i;
  gdouble x0, x3;
  gdouble y0, y1, y2, y3;
  gdouble dx, dy;
  gdouble slope;

  /* the outer control points for the bezier curve. */
  x0 = curve->points[p2].x;
  y0 = curve->points[p2].y;
  x3 = curve->points[p3].x;
  y3 = curve->points[p3].y;

  /*
   * the x values of the inner control points are fixed at
   * x1 = 2/3*x0 + 1/3*x3   and  x2 = 1/3*x0 + 2/3*x3
   * this ensures that the x values increase linearily with the
   * parameter t and enables us to skip the calculation of the x
   * values altogehter - just calculate y(t) evenly spaced.
   */

  dx = x3 - x0;
  dy = y3 - y0;

  g_return_if_fail (dx > 0);

  if (p1 == p2 && p3 == p4)
    {
      /* No information about the neighbors,
       * calculate y1 and y2 to get a straight line
       */
      y1 = y0 + dy / 3.0;
      y2 = y0 + dy * 2.0 / 3.0;
    }
  else if (p1 == p2 && p3 != p4)
    {
      /* only the right neighbor is available. Make the tangent at the
       * right endpoint parallel to the line between the left endpoint
       * and the right neighbor. Then point the tangent at the left towards
       * the control handle of the right tangent, to ensure that the curve
       * does not have an inflection point.
       */
      slope = (curve->points[p4].y - y0) / (curve->points[p4].x - x0);

      y2 = y3 - slope * dx / 3.0;
      y1 = y0 + (y2 - y0) / 2.0;
    }
  else if (p1 != p2 && p3 == p4)
    {
      /* see previous case */
      slope = (y3 - curve->points[p1].y) / (x3 - curve->points[p1].x);

      y1 = y0 + slope * dx / 3.0;
      y2 = y3 + (y1 - y3) / 2.0;
    }
  else /* (p1 != p2 && p3 != p4) */
    {
      /* Both neighbors are available. Make the tangents at the endpoints
       * parallel to the line between the opposite endpoint and the adjacent
       * neighbor.
       */
      slope = (y3 - curve->points[p1].y) / (x3 - curve->points[p1].x);

      y1 = y0 + slope * dx / 3.0;

      slope = (curve->points[p4].y - y0) / (curve->points[p4].x - x0);

      y2 = y3 - slope * dx / 3.0;
    }

  /*
   * finally calculate the y(t) values for the given bezier values. We can
   * use homogenously distributed values for t, since x(t) increases linearily.
   */
  for (i = 0; i <= ROUND (dx * (gdouble) (curve->n_samples - 1)); i++)
    {
      gdouble y, t;
      gint    index;

      t = i / dx / (gdouble) (curve->n_samples - 1);
      y =     y0 * (1-t) * (1-t) * (1-t) +
          3 * y1 * (1-t) * (1-t) * t     +
          3 * y2 * (1-t) * t     * t     +
              y3 * t     * t     * t;

      index = i + ROUND (x0 * (gdouble) (curve->n_samples - 1));

      if (index < curve->n_samples)
        curve->samples[index] = CLAMP (y, 0.0, 1.0);
    }
}
