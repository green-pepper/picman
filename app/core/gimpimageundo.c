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

#include <string.h>

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman-utils.h"
#include "picmandrawable.h"
#include "picmangrid.h"
#include "picmanimage.h"
#include "picmanimage-colormap.h"
#include "picmanimage-grid.h"
#include "picmanimage-private.h"
#include "picmanimageundo.h"
#include "picmanparasitelist.h"


enum
{
  PROP_0,
  PROP_PREVIOUS_ORIGIN_X,
  PROP_PREVIOUS_ORIGIN_Y,
  PROP_PREVIOUS_WIDTH,
  PROP_PREVIOUS_HEIGHT,
  PROP_GRID,
  PROP_PARASITE_NAME
};


static void     picman_image_undo_constructed  (GObject             *object);
static void     picman_image_undo_set_property (GObject             *object,
                                              guint                property_id,
                                              const GValue        *value,
                                              GParamSpec          *pspec);
static void     picman_image_undo_get_property (GObject             *object,
                                              guint                property_id,
                                              GValue              *value,
                                              GParamSpec          *pspec);

static gint64   picman_image_undo_get_memsize  (PicmanObject          *object,
                                              gint64              *gui_size);

static void     picman_image_undo_pop          (PicmanUndo            *undo,
                                              PicmanUndoMode         undo_mode,
                                              PicmanUndoAccumulator *accum);
static void     picman_image_undo_free         (PicmanUndo            *undo,
                                              PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanImageUndo, picman_image_undo, PICMAN_TYPE_UNDO)

#define parent_class picman_image_undo_parent_class


static void
picman_image_undo_class_init (PicmanImageUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_image_undo_constructed;
  object_class->set_property     = picman_image_undo_set_property;
  object_class->get_property     = picman_image_undo_get_property;

  picman_object_class->get_memsize = picman_image_undo_get_memsize;

  undo_class->pop                = picman_image_undo_pop;
  undo_class->free               = picman_image_undo_free;

  g_object_class_install_property (object_class, PROP_PREVIOUS_ORIGIN_X,
                                   g_param_spec_int ("previous-origin-x",
                                                     NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE,
                                                     0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_PREVIOUS_ORIGIN_Y,
                                   g_param_spec_int ("previous-origin-y",
                                                     NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE,
                                                     0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_PREVIOUS_WIDTH,
                                   g_param_spec_int ("previous-width",
                                                     NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE,
                                                     0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_PREVIOUS_HEIGHT,
                                   g_param_spec_int ("previous-height",
                                                     NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE,
                                                     0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_GRID,
                                   g_param_spec_object ("grid", NULL, NULL,
                                                        PICMAN_TYPE_GRID,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_PARASITE_NAME,
                                   g_param_spec_string ("parasite-name",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_image_undo_init (PicmanImageUndo *undo)
{
}

static void
picman_image_undo_constructed (GObject *object)
{
  PicmanImageUndo *image_undo = PICMAN_IMAGE_UNDO (object);
  PicmanImage     *image;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  image = PICMAN_UNDO (object)->image;

  switch (PICMAN_UNDO (object)->undo_type)
    {
    case PICMAN_UNDO_IMAGE_TYPE:
      image_undo->base_type = picman_image_get_base_type (image);
      break;

    case PICMAN_UNDO_IMAGE_PRECISION:
      image_undo->precision = picman_image_get_precision (image);
      break;

    case PICMAN_UNDO_IMAGE_SIZE:
      image_undo->width  = picman_image_get_width  (image);
      image_undo->height = picman_image_get_height (image);
      break;

    case PICMAN_UNDO_IMAGE_RESOLUTION:
      picman_image_get_resolution (image,
                                 &image_undo->xresolution,
                                 &image_undo->yresolution);
      image_undo->resolution_unit = picman_image_get_unit (image);
      break;

    case PICMAN_UNDO_IMAGE_GRID:
      g_assert (PICMAN_IS_GRID (image_undo->grid));
      break;

    case PICMAN_UNDO_IMAGE_COLORMAP:
      image_undo->num_colors = picman_image_get_colormap_size (image);
      image_undo->colormap   = g_memdup (picman_image_get_colormap (image),
                                         PICMAN_IMAGE_COLORMAP_SIZE);
      break;

    case PICMAN_UNDO_PARASITE_ATTACH:
    case PICMAN_UNDO_PARASITE_REMOVE:
      g_assert (image_undo->parasite_name != NULL);

      image_undo->parasite = picman_parasite_copy
        (picman_image_parasite_find (image, image_undo->parasite_name));
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
picman_image_undo_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanImageUndo *image_undo = PICMAN_IMAGE_UNDO (object);

  switch (property_id)
    {
    case PROP_PREVIOUS_ORIGIN_X:
      image_undo->previous_origin_x = g_value_get_int (value);
      break;
    case PROP_PREVIOUS_ORIGIN_Y:
      image_undo->previous_origin_y = g_value_get_int (value);
      break;
    case PROP_PREVIOUS_WIDTH:
      image_undo->previous_width = g_value_get_int (value);
      break;
    case PROP_PREVIOUS_HEIGHT:
      image_undo->previous_height = g_value_get_int (value);
      break;
    case PROP_GRID:
      {
        PicmanGrid *grid = g_value_get_object (value);

        if (grid)
          image_undo->grid = picman_config_duplicate (PICMAN_CONFIG (grid));
      }
      break;
    case PROP_PARASITE_NAME:
      image_undo->parasite_name = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_image_undo_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PicmanImageUndo *image_undo = PICMAN_IMAGE_UNDO (object);

  switch (property_id)
    {
    case PROP_PREVIOUS_ORIGIN_X:
      g_value_set_int (value, image_undo->previous_origin_x);
      break;
    case PROP_PREVIOUS_ORIGIN_Y:
      g_value_set_int (value, image_undo->previous_origin_y);
      break;
    case PROP_PREVIOUS_WIDTH:
      g_value_set_int (value, image_undo->previous_width);
      break;
    case PROP_PREVIOUS_HEIGHT:
      g_value_set_int (value, image_undo->previous_height);
      break;
    case PROP_GRID:
      g_value_set_object (value, image_undo->grid);
       break;
    case PROP_PARASITE_NAME:
      g_value_set_string (value, image_undo->parasite_name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_image_undo_get_memsize (PicmanObject *object,
                             gint64     *gui_size)
{
  PicmanImageUndo *image_undo = PICMAN_IMAGE_UNDO (object);
  gint64         memsize    = 0;

  if (image_undo->colormap)
    memsize += PICMAN_IMAGE_COLORMAP_SIZE;

  memsize += picman_object_get_memsize (PICMAN_OBJECT (image_undo->grid),
                                      gui_size);
  memsize += picman_string_get_memsize (image_undo->parasite_name);
  memsize += picman_parasite_get_memsize (image_undo->parasite, gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_image_undo_pop (PicmanUndo            *undo,
                     PicmanUndoMode         undo_mode,
                     PicmanUndoAccumulator *accum)
{
  PicmanImageUndo    *image_undo = PICMAN_IMAGE_UNDO (undo);
  PicmanImage        *image      = undo->image;
  PicmanImagePrivate *private    = PICMAN_IMAGE_GET_PRIVATE (image);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  switch (undo->undo_type)
    {
    case PICMAN_UNDO_IMAGE_TYPE:
      {
        PicmanImageBaseType base_type;

        base_type = image_undo->base_type;
        image_undo->base_type = picman_image_get_base_type (image);
        g_object_set (image, "base-type", base_type, NULL);

        picman_image_colormap_changed (image, -1);

        if (image_undo->base_type != picman_image_get_base_type (image))
          accum->mode_changed = TRUE;
      }
      break;

    case PICMAN_UNDO_IMAGE_PRECISION:
      {
        PicmanPrecision precision;

        precision = image_undo->precision;
        image_undo->precision = picman_image_get_precision (image);
        g_object_set (image, "precision", precision, NULL);

        if (image_undo->precision != picman_image_get_precision (image))
          accum->precision_changed = TRUE;
      }
      break;

    case PICMAN_UNDO_IMAGE_SIZE:
      {
        gint width;
        gint height;
        gint previous_origin_x;
        gint previous_origin_y;
        gint previous_width;
        gint previous_height;

        width             = image_undo->width;
        height            = image_undo->height;
        previous_origin_x = image_undo->previous_origin_x;
        previous_origin_y = image_undo->previous_origin_y;
        previous_width    = image_undo->previous_width;
        previous_height   = image_undo->previous_height;

        /* Transform to a redo */
        image_undo->width             = picman_image_get_width  (image);
        image_undo->height            = picman_image_get_height (image);
        image_undo->previous_origin_x = -previous_origin_x;
        image_undo->previous_origin_y = -previous_origin_y;
        image_undo->previous_width    = width;
        image_undo->previous_height   = height;

        g_object_set (image,
                      "width",  width,
                      "height", height,
                      NULL);

        picman_drawable_invalidate_boundary
          (PICMAN_DRAWABLE (picman_image_get_mask (image)));

        if (picman_image_get_width  (image) != image_undo->width ||
            picman_image_get_height (image) != image_undo->height)
          {
            accum->size_changed      = TRUE;
            accum->previous_origin_x = previous_origin_x;
            accum->previous_origin_y = previous_origin_y;
            accum->previous_width    = previous_width;
            accum->previous_height   = previous_height;
          }
      }
      break;

    case PICMAN_UNDO_IMAGE_RESOLUTION:
      {
        gdouble xres;
        gdouble yres;

        picman_image_get_resolution (image, &xres, &yres);

        if (ABS (image_undo->xresolution - xres) >= 1e-5 ||
            ABS (image_undo->yresolution - yres) >= 1e-5)
          {
            private->xresolution = image_undo->xresolution;
            private->yresolution = image_undo->yresolution;

            image_undo->xresolution = xres;
            image_undo->yresolution = yres;

            accum->resolution_changed = TRUE;
          }
      }

      if (image_undo->resolution_unit != picman_image_get_unit (image))
        {
          PicmanUnit unit;

          unit = picman_image_get_unit (image);
          private->resolution_unit = image_undo->resolution_unit;
          image_undo->resolution_unit = unit;

          accum->unit_changed = TRUE;
        }
      break;

    case PICMAN_UNDO_IMAGE_GRID:
      {
        PicmanGrid *grid;

        grid = picman_config_duplicate (PICMAN_CONFIG (picman_image_get_grid (image)));

        picman_image_set_grid (image, image_undo->grid, FALSE);

        g_object_unref (image_undo->grid);
        image_undo->grid = grid;
      }
      break;

    case PICMAN_UNDO_IMAGE_COLORMAP:
      {
        guchar *colormap;
        gint    num_colors;

        num_colors = picman_image_get_colormap_size (image);
        colormap   = g_memdup (picman_image_get_colormap (image),
                               PICMAN_IMAGE_COLORMAP_SIZE);

        picman_image_set_colormap (image,
                                 image_undo->colormap, image_undo->num_colors,
                                 FALSE);

        if (image_undo->colormap)
          g_free (image_undo->colormap);

        image_undo->num_colors = num_colors;
        image_undo->colormap   = colormap;
      }
      break;

    case PICMAN_UNDO_PARASITE_ATTACH:
    case PICMAN_UNDO_PARASITE_REMOVE:
      {
        PicmanParasite *parasite = image_undo->parasite;
        const gchar  *name;

        image_undo->parasite = picman_parasite_copy
          (picman_image_parasite_find (image, image_undo->parasite_name));

        if (parasite)
          picman_parasite_list_add (private->parasites, parasite);
        else
          picman_parasite_list_remove (private->parasites,
                                     image_undo->parasite_name);

        name = parasite ? parasite->name : image_undo->parasite_name;

        if (strcmp (name, "icc-profile") == 0)
          picman_color_managed_profile_changed (PICMAN_COLOR_MANAGED (image));

        if (parasite)
          picman_parasite_free (parasite);
      }
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
picman_image_undo_free (PicmanUndo     *undo,
                      PicmanUndoMode  undo_mode)
{
  PicmanImageUndo *image_undo = PICMAN_IMAGE_UNDO (undo);

  if (image_undo->grid)
    {
      g_object_unref (image_undo->grid);
      image_undo->grid = NULL;
    }

  if (image_undo->colormap)
    {
      g_free (image_undo->colormap);
      image_undo->colormap = NULL;
    }

  if (image_undo->parasite_name)
    {
      g_free (image_undo->parasite_name);
      image_undo->parasite_name = NULL;
    }

  if (image_undo->parasite)
    {
      picman_parasite_free (image_undo->parasite);
      image_undo->parasite = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
