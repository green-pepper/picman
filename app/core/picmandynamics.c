/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#include <gegl.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picmancurve.h"
#include "picmandynamics.h"
#include "picmandynamics-load.h"
#include "picmandynamics-save.h"
#include "picmandynamicsoutput.h"

#include "picman-intl.h"


#define DEFAULT_NAME "Nameless dynamics"

enum
{
  PROP_0,

  PROP_NAME,

  PROP_OPACITY_OUTPUT,
  PROP_SIZE_OUTPUT,
  PROP_ANGLE_OUTPUT,
  PROP_COLOR_OUTPUT,
  PROP_FORCE_OUTPUT,
  PROP_HARDNESS_OUTPUT,
  PROP_ASPECT_RATIO_OUTPUT,
  PROP_SPACING_OUTPUT,
  PROP_RATE_OUTPUT,
  PROP_FLOW_OUTPUT,
  PROP_JITTER_OUTPUT
};


typedef struct _PicmanDynamicsPrivate PicmanDynamicsPrivate;

struct _PicmanDynamicsPrivate
{
  PicmanDynamicsOutput *opacity_output;
  PicmanDynamicsOutput *hardness_output;
  PicmanDynamicsOutput *force_output;
  PicmanDynamicsOutput *rate_output;
  PicmanDynamicsOutput *flow_output;
  PicmanDynamicsOutput *size_output;
  PicmanDynamicsOutput *aspect_ratio_output;
  PicmanDynamicsOutput *color_output;
  PicmanDynamicsOutput *angle_output;
  PicmanDynamicsOutput *jitter_output;
  PicmanDynamicsOutput *spacing_output;
};

#define GET_PRIVATE(output) \
        G_TYPE_INSTANCE_GET_PRIVATE (output, \
                                     PICMAN_TYPE_DYNAMICS, \
                                     PicmanDynamicsPrivate)


static void          picman_dynamics_finalize      (GObject      *object);
static void          picman_dynamics_set_property  (GObject      *object,
                                                  guint         property_id,
                                                  const GValue *value,
                                                  GParamSpec   *pspec);
static void          picman_dynamics_get_property  (GObject      *object,
                                                  guint         property_id,
                                                  GValue       *value,
                                                  GParamSpec   *pspec);
static void
       picman_dynamics_dispatch_properties_changed (GObject      *object,
                                                  guint         n_pspecs,
                                                  GParamSpec  **pspecs);

static const gchar * picman_dynamics_get_extension (PicmanData     *data);
static PicmanData *    picman_dynamics_duplicate     (PicmanData     *data);

static PicmanDynamicsOutput *
                     picman_dynamics_create_output (PicmanDynamics           *dynamics,
                                                  const gchar            *name,
                                                  PicmanDynamicsOutputType  type);
static void          picman_dynamics_output_notify (GObject          *output,
                                                  const GParamSpec *pspec,
                                                  PicmanDynamics     *dynamics);


G_DEFINE_TYPE (PicmanDynamics, picman_dynamics,
               PICMAN_TYPE_DATA)

#define parent_class picman_dynamics_parent_class


static void
picman_dynamics_class_init (PicmanDynamicsClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanDataClass     *data_class     = PICMAN_DATA_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);

  object_class->finalize                    = picman_dynamics_finalize;
  object_class->set_property                = picman_dynamics_set_property;
  object_class->get_property                = picman_dynamics_get_property;
  object_class->dispatch_properties_changed = picman_dynamics_dispatch_properties_changed;

  viewable_class->default_stock_id          = "picman-dynamics";

  data_class->save                          = picman_dynamics_save;
  data_class->get_extension                 = picman_dynamics_get_extension;
  data_class->duplicate                     = picman_dynamics_duplicate;

  PICMAN_CONFIG_INSTALL_PROP_STRING (object_class, PROP_NAME,
                                   "name", NULL,
                                   DEFAULT_NAME,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_OPACITY_OUTPUT,
                                   "opacity-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_FORCE_OUTPUT,
                                   "force-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_HARDNESS_OUTPUT,
                                   "hardness-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_RATE_OUTPUT,
                                   "rate-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_FLOW_OUTPUT,
                                   "flow-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_SIZE_OUTPUT,
                                   "size-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_ASPECT_RATIO_OUTPUT,
                                   "aspect-ratio-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_COLOR_OUTPUT,
                                   "color-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_ANGLE_OUTPUT,
                                   "angle-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_JITTER_OUTPUT,
                                   "jitter-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_SPACING_OUTPUT,
                                   "spacing-output", NULL,
                                   PICMAN_TYPE_DYNAMICS_OUTPUT,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  g_type_class_add_private (klass, sizeof (PicmanDynamicsPrivate));
}

static void
picman_dynamics_init (PicmanDynamics *dynamics)
{
  PicmanDynamicsPrivate *private = GET_PRIVATE (dynamics);

  private->opacity_output =
    picman_dynamics_create_output (dynamics,
                                 "opacity-output",
                                 PICMAN_DYNAMICS_OUTPUT_OPACITY);

  private->force_output =
    picman_dynamics_create_output (dynamics,
                                 "force-output",
                                 PICMAN_DYNAMICS_OUTPUT_FORCE);

  private->hardness_output =
    picman_dynamics_create_output (dynamics,
                                 "hardness-output",
                                 PICMAN_DYNAMICS_OUTPUT_HARDNESS);

  private->rate_output =
    picman_dynamics_create_output (dynamics,
                                 "rate-output",
                                 PICMAN_DYNAMICS_OUTPUT_RATE);

  private->flow_output =
    picman_dynamics_create_output (dynamics,
                                 "flow-output",
                                 PICMAN_DYNAMICS_OUTPUT_RATE);

  private->size_output =
    picman_dynamics_create_output (dynamics,
                                 "size-output",
                                 PICMAN_DYNAMICS_OUTPUT_SIZE);

  private->aspect_ratio_output =
    picman_dynamics_create_output (dynamics,
                                 "aspect-ratio-output",
                                 PICMAN_DYNAMICS_OUTPUT_ASPECT_RATIO);

  private->color_output =
    picman_dynamics_create_output (dynamics,
                                 "color-output",
                                 PICMAN_DYNAMICS_OUTPUT_COLOR);

  private->angle_output =
    picman_dynamics_create_output (dynamics,
                                 "angle-output",
                                 PICMAN_DYNAMICS_OUTPUT_ANGLE);

  private->jitter_output =
    picman_dynamics_create_output (dynamics,
                                 "jitter-output",
                                 PICMAN_DYNAMICS_OUTPUT_JITTER);

  private->spacing_output =
    picman_dynamics_create_output (dynamics,
                                 "spacing-output",
                                 PICMAN_DYNAMICS_OUTPUT_SPACING);
}

static void
picman_dynamics_finalize (GObject *object)
{
  PicmanDynamicsPrivate *private = GET_PRIVATE (object);

  g_object_unref (private->opacity_output);
  g_object_unref (private->force_output);
  g_object_unref (private->hardness_output);
  g_object_unref (private->rate_output);
  g_object_unref (private->flow_output);
  g_object_unref (private->size_output);
  g_object_unref (private->aspect_ratio_output);
  g_object_unref (private->color_output);
  g_object_unref (private->angle_output);
  g_object_unref (private->jitter_output);
  g_object_unref (private->spacing_output);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_dynamics_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  PicmanDynamicsPrivate *private     = GET_PRIVATE (object);
  PicmanDynamicsOutput  *src_output  = NULL;
  PicmanDynamicsOutput  *dest_output = NULL;

  switch (property_id)
    {
    case PROP_NAME:
      picman_object_set_name (PICMAN_OBJECT (object), g_value_get_string (value));
      break;

    case PROP_OPACITY_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->opacity_output;
      break;

    case PROP_FORCE_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->force_output;
      break;

    case PROP_HARDNESS_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->hardness_output;
      break;

    case PROP_RATE_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->rate_output;
      break;

    case PROP_FLOW_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->flow_output;
      break;

    case PROP_SIZE_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->size_output;
      break;

    case PROP_ASPECT_RATIO_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->aspect_ratio_output;
      break;

    case PROP_COLOR_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->color_output;
      break;

    case PROP_ANGLE_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->angle_output;
      break;

    case PROP_JITTER_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->jitter_output;
      break;

    case PROP_SPACING_OUTPUT:
      src_output  = g_value_get_object (value);
      dest_output = private->spacing_output;
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }

  if (src_output && dest_output)
    {
      picman_config_copy (PICMAN_CONFIG (src_output),
                        PICMAN_CONFIG (dest_output),
                        PICMAN_CONFIG_PARAM_SERIALIZE);
    }
}

static void
picman_dynamics_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  PicmanDynamicsPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_NAME:
      g_value_set_string (value, picman_object_get_name (object));
      break;

    case PROP_OPACITY_OUTPUT:
      g_value_set_object (value, private->opacity_output);
      break;

    case PROP_FORCE_OUTPUT:
      g_value_set_object (value, private->force_output);
      break;

    case PROP_HARDNESS_OUTPUT:
      g_value_set_object (value, private->hardness_output);
      break;

    case PROP_RATE_OUTPUT:
      g_value_set_object (value, private->rate_output);
      break;

    case PROP_FLOW_OUTPUT:
      g_value_set_object (value, private->flow_output);
      break;

    case PROP_SIZE_OUTPUT:
      g_value_set_object (value, private->size_output);
      break;

    case PROP_ASPECT_RATIO_OUTPUT:
      g_value_set_object (value, private->aspect_ratio_output);
      break;

    case PROP_COLOR_OUTPUT:
      g_value_set_object (value, private->color_output);
      break;

    case PROP_ANGLE_OUTPUT:
      g_value_set_object (value, private->angle_output);
      break;

    case PROP_JITTER_OUTPUT:
      g_value_set_object (value, private->jitter_output);
      break;

    case PROP_SPACING_OUTPUT:
      g_value_set_object (value, private->spacing_output);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_dynamics_dispatch_properties_changed (GObject     *object,
                                           guint        n_pspecs,
                                           GParamSpec **pspecs)
{
  gint i;

  G_OBJECT_CLASS (parent_class)->dispatch_properties_changed (object,
                                                              n_pspecs, pspecs);

  for (i = 0; i < n_pspecs; i++)
    {
      if (pspecs[i]->flags & PICMAN_CONFIG_PARAM_SERIALIZE)
        {
          picman_data_dirty (PICMAN_DATA (object));
          break;
        }
    }
}

static const gchar *
picman_dynamics_get_extension (PicmanData *data)
{
  return PICMAN_DYNAMICS_FILE_EXTENSION;
}

static PicmanData *
picman_dynamics_duplicate (PicmanData *data)
{
  PicmanData *dest = g_object_new (PICMAN_TYPE_DYNAMICS, NULL);

  picman_config_copy (PICMAN_CONFIG (data),
                    PICMAN_CONFIG (dest), 0);

  return PICMAN_DATA (dest);
}


/*  public functions  */

PicmanData *
picman_dynamics_new (PicmanContext *context,
                   const gchar *name)
{
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (name[0] != '\0', NULL);

  return g_object_new (PICMAN_TYPE_DYNAMICS,
                       "name", name,
                       NULL);
}

PicmanData *
picman_dynamics_get_standard (PicmanContext *context)
{
  static PicmanData *standard_dynamics = NULL;

  if (! standard_dynamics)
    {
      standard_dynamics = picman_dynamics_new (context, "Standard dynamics");

      picman_data_clean (standard_dynamics);
      picman_data_make_internal (standard_dynamics, "picman-dynamics-standard");

      g_object_add_weak_pointer (G_OBJECT (standard_dynamics),
                                 (gpointer *) &standard_dynamics);
    }

  return standard_dynamics;
}

PicmanDynamicsOutput *
picman_dynamics_get_output (PicmanDynamics           *dynamics,
                          PicmanDynamicsOutputType  type_id)
{
  PicmanDynamicsPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DYNAMICS (dynamics), NULL);

  private = GET_PRIVATE (dynamics);

  switch (type_id)
    {
    case PICMAN_DYNAMICS_OUTPUT_OPACITY:
      return private->opacity_output;
      break;

    case PICMAN_DYNAMICS_OUTPUT_FORCE:
      return private->force_output;
      break;

    case PICMAN_DYNAMICS_OUTPUT_HARDNESS:
      return private->hardness_output;
      break;

    case PICMAN_DYNAMICS_OUTPUT_RATE:
      return private->rate_output;
      break;

    case PICMAN_DYNAMICS_OUTPUT_FLOW:
      return private->flow_output;
      break;

    case PICMAN_DYNAMICS_OUTPUT_SIZE:
      return private->size_output;
      break;

    case PICMAN_DYNAMICS_OUTPUT_ASPECT_RATIO:
      return private->aspect_ratio_output;
      break;

    case PICMAN_DYNAMICS_OUTPUT_COLOR:
      return private->color_output;
      break;

    case PICMAN_DYNAMICS_OUTPUT_ANGLE:
      return private->angle_output;
      break;

    case PICMAN_DYNAMICS_OUTPUT_JITTER:
      return private->jitter_output;
      break;

    case PICMAN_DYNAMICS_OUTPUT_SPACING:
      return private->spacing_output;
      break;

    default:
      return NULL;
      break;
    }
}

gdouble
picman_dynamics_get_linear_value (PicmanDynamics           *dynamics,
                                PicmanDynamicsOutputType  type,
                                const PicmanCoords       *coords,
                                PicmanPaintOptions       *options,
                                gdouble                 fade_point)
{
  PicmanDynamicsOutput *output;

  g_return_val_if_fail (PICMAN_IS_DYNAMICS (dynamics), 0.0);

  output = picman_dynamics_get_output (dynamics, type);

  return picman_dynamics_output_get_linear_value (output, coords,
                                                options, fade_point);
}

gdouble
picman_dynamics_get_angular_value (PicmanDynamics           *dynamics,
                                 PicmanDynamicsOutputType  type,
                                 const PicmanCoords       *coords,
                                 PicmanPaintOptions       *options,
                                 gdouble                 fade_point)
{
  PicmanDynamicsOutput *output;

  g_return_val_if_fail (PICMAN_IS_DYNAMICS (dynamics), 0.0);

  output = picman_dynamics_get_output (dynamics, type);

  return picman_dynamics_output_get_angular_value (output, coords,
                                                 options, fade_point);
}

gdouble
picman_dynamics_get_aspect_value (PicmanDynamics           *dynamics,
                                PicmanDynamicsOutputType  type,
                                const PicmanCoords       *coords,
                                PicmanPaintOptions       *options,
                                gdouble                 fade_point)
{
  PicmanDynamicsOutput *output;

  g_return_val_if_fail (PICMAN_IS_DYNAMICS (dynamics), 0.0);

  output = picman_dynamics_get_output (dynamics, type);

  return picman_dynamics_output_get_aspect_value (output, coords,
                                                options, fade_point);
}


/*  private functions  */

static PicmanDynamicsOutput *
picman_dynamics_create_output (PicmanDynamics           *dynamics,
                             const gchar            *name,
                             PicmanDynamicsOutputType  type)
{
  PicmanDynamicsOutput *output = picman_dynamics_output_new (name, type);

  g_signal_connect (output, "notify",
                    G_CALLBACK (picman_dynamics_output_notify),
                    dynamics);

  return output;
}

static void
picman_dynamics_output_notify (GObject          *output,
                             const GParamSpec *pspec,
                             PicmanDynamics     *dynamics)
{
  g_object_notify (G_OBJECT (dynamics), picman_object_get_name (output));
}
