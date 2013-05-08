/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmanpdbcontext.c
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

#include "libpicmanconfig/picmanconfig.h"

#include "pdb-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmanlist.h"
#include "core/picmanpaintinfo.h"

#include "paint/picmanbrushcore.h"
#include "paint/picmanpaintoptions.h"

#include "picmanpdbcontext.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_ANTIALIAS,
  PROP_FEATHER,
  PROP_FEATHER_RADIUS_X,
  PROP_FEATHER_RADIUS_Y,
  PROP_SAMPLE_MERGED,
  PROP_SAMPLE_CRITERION,
  PROP_SAMPLE_THRESHOLD,
  PROP_SAMPLE_TRANSPARENT,
  PROP_INTERPOLATION,
  PROP_TRANSFORM_DIRECTION,
  PROP_TRANSFORM_RESIZE,
  PROP_TRANSFORM_RECURSION
};


static void   picman_pdb_context_constructed  (GObject      *object);
static void   picman_pdb_context_finalize     (GObject      *object);
static void   picman_pdb_context_set_property (GObject      *object,
                                             guint         property_id,
                                             const GValue *value,
                                             GParamSpec   *pspec);
static void   picman_pdb_context_get_property (GObject      *object,
                                             guint         property_id,
                                             GValue       *value,
                                             GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanPDBContext, picman_pdb_context, PICMAN_TYPE_CONTEXT)

#define parent_class picman_pdb_context_parent_class


static void
picman_pdb_context_class_init (PicmanPDBContextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_pdb_context_constructed;
  object_class->finalize     = picman_pdb_context_finalize;
  object_class->set_property = picman_pdb_context_set_property;
  object_class->get_property = picman_pdb_context_get_property;

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_ANTIALIAS,
                                    "antialias",
                                    N_("Smooth edges"),
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_FEATHER,
                                    "feather", NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_FEATHER_RADIUS_X,
                                   "feather-radius-x", NULL,
                                   0.0, 1000.0, 10.0,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_FEATHER_RADIUS_Y,
                                   "feather-radius-y", NULL,
                                   0.0, 1000.0, 10.0,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SAMPLE_MERGED,
                                    "sample-merged", NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_SAMPLE_CRITERION,
                                 "sample-criterion", NULL,
                                 PICMAN_TYPE_SELECT_CRITERION,
                                 PICMAN_SELECT_CRITERION_COMPOSITE,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_SAMPLE_THRESHOLD,
                                   "sample-threshold", NULL,
                                   0.0, 1.0, 0.0,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SAMPLE_TRANSPARENT,
                                    "sample-transparent", NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_INTERPOLATION,
                                 "interpolation", NULL,
                                 PICMAN_TYPE_INTERPOLATION_TYPE,
                                 PICMAN_INTERPOLATION_CUBIC,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_TRANSFORM_DIRECTION,
                                 "transform-direction", NULL,
                                 PICMAN_TYPE_TRANSFORM_DIRECTION,
                                 PICMAN_TRANSFORM_FORWARD,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_TRANSFORM_RESIZE,
                                 "transform-resize", NULL,
                                 PICMAN_TYPE_TRANSFORM_RESIZE,
                                 PICMAN_TRANSFORM_RESIZE_ADJUST,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_TRANSFORM_RECURSION,
                                "transform-recursion", NULL,
                                1, G_MAXINT32, 3,
                                PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_pdb_context_init (PicmanPDBContext *context)
{
  context->paint_options_list = picman_list_new (PICMAN_TYPE_PAINT_OPTIONS,
                                               FALSE);
}

static void
picman_pdb_context_constructed (GObject *object)
{
  PicmanInterpolationType  interpolation;
  gint                   threshold;
  GParamSpec            *pspec;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  /* get default interpolation from picmanrc */

  interpolation = PICMAN_CONTEXT (object)->picman->config->interpolation_type;

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (object),
                                        "interpolation");

  if (pspec)
    G_PARAM_SPEC_ENUM (pspec)->default_value = interpolation;

  g_object_set (object, "interpolation", interpolation, NULL);

  /* get default threshold from picmanrc */

  threshold = PICMAN_CONTEXT (object)->picman->config->default_threshold;

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (object),
                                        "sample-threshold");

  if (pspec)
    G_PARAM_SPEC_DOUBLE (pspec)->default_value = threshold / 255.0;

  g_object_set (object, "sample-threshold", threshold / 255.0, NULL);
}

static void
picman_pdb_context_finalize (GObject *object)
{
  PicmanPDBContext *context = PICMAN_PDB_CONTEXT (object);

  if (context->paint_options_list)
    {
      g_object_unref (context->paint_options_list);
      context->paint_options_list = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_pdb_context_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanPDBContext *options = PICMAN_PDB_CONTEXT (object);

  switch (property_id)
    {
    case PROP_ANTIALIAS:
      options->antialias = g_value_get_boolean (value);
      break;

    case PROP_FEATHER:
      options->feather = g_value_get_boolean (value);
      break;

    case PROP_FEATHER_RADIUS_X:
      options->feather_radius_x = g_value_get_double (value);
      break;

    case PROP_FEATHER_RADIUS_Y:
      options->feather_radius_y = g_value_get_double (value);
      break;

    case PROP_SAMPLE_MERGED:
      options->sample_merged = g_value_get_boolean (value);
      break;

    case PROP_SAMPLE_CRITERION:
      options->sample_criterion = g_value_get_enum (value);
      break;

    case PROP_SAMPLE_THRESHOLD:
      options->sample_threshold = g_value_get_double (value);
      break;

    case PROP_SAMPLE_TRANSPARENT:
      options->sample_transparent = g_value_get_boolean (value);
      break;

    case PROP_INTERPOLATION:
      options->interpolation = g_value_get_enum (value);
      break;

    case PROP_TRANSFORM_DIRECTION:
      options->transform_direction = g_value_get_enum (value);
      break;

    case PROP_TRANSFORM_RESIZE:
      options->transform_resize = g_value_get_enum (value);
      break;

    case PROP_TRANSFORM_RECURSION:
      options->transform_recursion = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_pdb_context_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanPDBContext *options = PICMAN_PDB_CONTEXT (object);

  switch (property_id)
    {
    case PROP_ANTIALIAS:
      g_value_set_boolean (value, options->antialias);
      break;

    case PROP_FEATHER:
      g_value_set_boolean (value, options->feather);
      break;

    case PROP_FEATHER_RADIUS_X:
      g_value_set_double (value, options->feather_radius_x);
      break;

    case PROP_FEATHER_RADIUS_Y:
      g_value_set_double (value, options->feather_radius_y);
      break;

    case PROP_SAMPLE_MERGED:
      g_value_set_boolean (value, options->sample_merged);
      break;

    case PROP_SAMPLE_CRITERION:
      g_value_set_enum (value, options->sample_criterion);
      break;

    case PROP_SAMPLE_THRESHOLD:
      g_value_set_double (value, options->sample_threshold);
      break;

    case PROP_SAMPLE_TRANSPARENT:
      g_value_set_boolean (value, options->sample_transparent);
      break;

    case PROP_INTERPOLATION:
      g_value_set_enum (value, options->interpolation);
      break;

    case PROP_TRANSFORM_DIRECTION:
      g_value_set_enum (value, options->transform_direction);
      break;

    case PROP_TRANSFORM_RESIZE:
      g_value_set_enum (value, options->transform_resize);
      break;

    case PROP_TRANSFORM_RECURSION:
      g_value_set_int (value, options->transform_recursion);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

PicmanContext *
picman_pdb_context_new (Picman        *picman,
                      PicmanContext *parent,
                      gboolean     set_parent)
{
  PicmanPDBContext *context;
  GList          *list;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (parent), NULL);

  context = g_object_new (PICMAN_TYPE_PDB_CONTEXT,
                          "picman", picman,
                          "name", "PDB Context",
                          NULL);

  picman_config_sync (G_OBJECT (parent), G_OBJECT (context), 0);

  if (set_parent)
    {
      picman_context_define_properties (PICMAN_CONTEXT (context),
                                      PICMAN_CONTEXT_ALL_PROPS_MASK, FALSE);
      picman_context_set_parent (PICMAN_CONTEXT (context), parent);

      for (list = picman_get_paint_info_iter (picman);
           list;
           list = g_list_next (list))
        {
          PicmanPaintInfo *info = list->data;

          picman_container_add (context->paint_options_list,
                              PICMAN_OBJECT (info->paint_options));
        }
    }
  else
    {
      for (list = PICMAN_LIST (PICMAN_PDB_CONTEXT (parent)->paint_options_list)->list;
           list;
           list = g_list_next (list))
        {
          PicmanPaintOptions *options = picman_config_duplicate (list->data);

          picman_container_add (context->paint_options_list,
                              PICMAN_OBJECT (options));
          g_object_unref (options);
        }
    }

  return PICMAN_CONTEXT (context);
}

PicmanPaintOptions *
picman_pdb_context_get_paint_options (PicmanPDBContext *context,
                                    const gchar    *name)
{
  g_return_val_if_fail (PICMAN_IS_PDB_CONTEXT (context), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  return (PicmanPaintOptions *)
    picman_container_get_child_by_name (context->paint_options_list, name);
}

GList *
picman_pdb_context_get_brush_options (PicmanPDBContext *context)
{
  GList *brush_options = NULL;
  GList *list;

  g_return_val_if_fail (PICMAN_IS_PDB_CONTEXT (context), NULL);

  for (list = PICMAN_LIST (context->paint_options_list)->list;
       list;
       list = g_list_next (list))
    {
      PicmanPaintOptions *options = list->data;

      if (g_type_is_a (options->paint_info->paint_type, PICMAN_TYPE_BRUSH_CORE))
        brush_options = g_list_prepend (brush_options, options);
    }

  return g_list_reverse (brush_options);
}
