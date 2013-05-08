/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-gegl-nodes.h
 * Copyright (C) 2012 Michael Natterer <mitch@picman.org>
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

#include "picman-gegl-types.h"

#include "picman-gegl-nodes.h"
#include "picman-gegl-utils.h"


GeglNode *
picman_gegl_create_flatten_node (const PicmanRGB *background)
{
  GeglNode  *node;
  GeglNode  *input;
  GeglNode  *output;
  GeglNode  *color;
  GeglNode  *over;
  GeglColor *c;

  g_return_val_if_fail (background != NULL, NULL);

  node = gegl_node_new ();

  input  = gegl_node_get_input_proxy  (node, "input");
  output = gegl_node_get_output_proxy (node, "output");

  c = picman_gegl_color_new (background);
  color = gegl_node_new_child (node,
                               "operation", "gegl:color",
                               "value",     c,
                               NULL);
  g_object_unref (c);

  over = gegl_node_new_child (node,
                              "operation", "gegl:over",
                              NULL);

  gegl_node_connect_to (input,  "output",
                        over,   "aux");
  gegl_node_connect_to (color,  "output",
                        over,   "input");
  gegl_node_connect_to (over,   "output",
                        output, "input");

  return node;
}

GeglNode *
picman_gegl_create_apply_opacity_node (GeglBuffer *mask,
                                     gint        mask_offset_x,
                                     gint        mask_offset_y,
                                     gdouble     opacity)
{
  GeglNode  *node;
  GeglNode  *input;
  GeglNode  *output;
  GeglNode  *opacity_node;
  GeglNode  *mask_source;

  g_return_val_if_fail (GEGL_IS_BUFFER (mask), NULL);

  node = gegl_node_new ();

  input  = gegl_node_get_input_proxy  (node, "input");
  output = gegl_node_get_output_proxy (node, "output");

  opacity_node = gegl_node_new_child (node,
                                      "operation", "gegl:opacity",
                                      "value",     opacity,
                                      NULL);

  mask_source = picman_gegl_add_buffer_source (node, mask,
                                             mask_offset_x,
                                             mask_offset_y);

  gegl_node_connect_to (input,        "output",
                        opacity_node, "input");
  gegl_node_connect_to (mask_source,  "output",
                        opacity_node, "aux");
  gegl_node_connect_to (opacity_node, "output",
                        output,       "input");

  return node;
}

GeglNode *
picman_gegl_add_buffer_source (GeglNode   *parent,
                             GeglBuffer *buffer,
                             gint        offset_x,
                             gint        offset_y)
{
  GeglNode *buffer_source;

  g_return_val_if_fail (GEGL_IS_NODE (parent), NULL);
  g_return_val_if_fail (GEGL_IS_BUFFER (buffer), NULL);

  buffer_source = gegl_node_new_child (parent,
                                       "operation", "gegl:buffer-source",
                                       "buffer",    buffer,
                                       NULL);

  if (offset_x != 0 || offset_y != 0)
    {
      GeglNode *translate =
        gegl_node_new_child (parent,
                             "operation", "gegl:translate",
                             "x",         (gdouble) offset_x,
                             "y",         (gdouble) offset_y,
                             NULL);

      gegl_node_connect_to (buffer_source, "output",
                            translate,     "input");

      buffer_source = translate;
    }

  return buffer_source;
}

void
picman_gegl_mode_node_set_mode (GeglNode             *node,
                              PicmanLayerModeEffects  mode,
                              gboolean              linear)
{
  const gchar *operation = "picman:normal-mode";

  g_return_if_fail (GEGL_IS_NODE (node));

  switch (mode)
    {
    case PICMAN_NORMAL_MODE:        operation = "picman:normal-mode"; break;
    case PICMAN_DISSOLVE_MODE:      operation = "picman:dissolve-mode"; break;
    case PICMAN_BEHIND_MODE:        operation = "picman:behind-mode"; break;
    case PICMAN_MULTIPLY_MODE:      operation = "picman:multiply-mode"; break;
    case PICMAN_SCREEN_MODE:        operation = "picman:screen-mode"; break;
    case PICMAN_OVERLAY_MODE:       operation = "picman:overlay-mode"; break;
    case PICMAN_DIFFERENCE_MODE:    operation = "picman:difference-mode"; break;
    case PICMAN_ADDITION_MODE:      operation = "picman:addition-mode"; break;
    case PICMAN_SUBTRACT_MODE:      operation = "picman:subtract-mode"; break;
    case PICMAN_DARKEN_ONLY_MODE:   operation = "picman:darken-only-mode"; break;
    case PICMAN_LIGHTEN_ONLY_MODE:  operation = "picman:lighten-only-mode"; break;
    case PICMAN_HUE_MODE:           operation = "picman:hue-mode"; break;
    case PICMAN_SATURATION_MODE:    operation = "picman:saturation-mode"; break;
    case PICMAN_COLOR_MODE:         operation = "picman:color-mode"; break;
    case PICMAN_VALUE_MODE:         operation = "picman:value-mode"; break;
    case PICMAN_DIVIDE_MODE:        operation = "picman:divide-mode"; break;
    case PICMAN_DODGE_MODE:         operation = "picman:dodge-mode"; break;
    case PICMAN_BURN_MODE:          operation = "picman:burn-mode"; break;
    case PICMAN_HARDLIGHT_MODE:     operation = "picman:hardlight-mode"; break;
    case PICMAN_SOFTLIGHT_MODE:     operation = "picman:softlight-mode"; break;
    case PICMAN_GRAIN_EXTRACT_MODE: operation = "picman:grain-extract-mode"; break;
    case PICMAN_GRAIN_MERGE_MODE:   operation = "picman:grain-merge-mode"; break;
    case PICMAN_COLOR_ERASE_MODE:   operation = "picman:color-erase-mode"; break;
    case PICMAN_ERASE_MODE:         operation = "picman:erase-mode"; break;
    case PICMAN_REPLACE_MODE:       operation = "picman:replace-mode"; break;
    case PICMAN_ANTI_ERASE_MODE:    operation = "picman:anti-erase-mode"; break;
    default:
      break;
    }

  gegl_node_set (node,
                 "operation", operation,
                 "linear",    linear,
                 NULL);
}

void
picman_gegl_mode_node_set_opacity (GeglNode *node,
                                 gdouble   opacity)
{
  g_return_if_fail (GEGL_IS_NODE (node));

  gegl_node_set (node,
                 "opacity", opacity,
                 NULL);
}

void
picman_gegl_node_set_matrix (GeglNode          *node,
                           const PicmanMatrix3 *matrix)
{
  gchar *matrix_string;

  g_return_if_fail (GEGL_IS_NODE (node));
  g_return_if_fail (matrix != NULL);

  matrix_string = gegl_matrix3_to_string ((GeglMatrix3 *) matrix);

  gegl_node_set (node,
                 "transform", matrix_string,
                 NULL);

  g_free (matrix_string);
}
