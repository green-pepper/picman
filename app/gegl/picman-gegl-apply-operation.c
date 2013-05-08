/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-apply-operation.c
 * Copyright (C) 2012 Øyvind Kolås <pippin@picman.org>
 *                    Sven Neumann <sven@picman.org>
 *                    Michael Natterer <mitch@picman.org>
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

#include "core/picman-utils.h"
#include "core/picmanprogress.h"

#include "picman-gegl-apply-operation.h"
#include "picman-gegl-nodes.h"
#include "gegl/picman-gegl-utils.h"


void
picman_gegl_apply_operation (GeglBuffer          *src_buffer,
                           PicmanProgress        *progress,
                           const gchar         *undo_desc,
                           GeglNode            *operation,
                           GeglBuffer          *dest_buffer,
                           const GeglRectangle *dest_rect)
{
  GeglNode      *gegl;
  GeglNode      *dest_node;
  GeglRectangle  rect = { 0, };
  gdouble        value;
  gboolean       progress_active = FALSE;

  g_return_if_fail (src_buffer == NULL || GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_NODE (operation));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));

  if (dest_rect)
    {
      rect = *dest_rect;
    }
  else
    {
      rect = *GEGL_RECTANGLE (0, 0, gegl_buffer_get_width  (dest_buffer),
                                    gegl_buffer_get_height (dest_buffer));
    }

  gegl = gegl_node_new ();

  if (! gegl_node_get_parent (operation))
    gegl_node_add_child (gegl, operation);

  if (src_buffer)
    {
      GeglNode *src_node;

      src_node = gegl_node_new_child (gegl,
                                      "operation", "gegl:buffer-source",
                                      "buffer",    src_buffer,
                                      NULL);

      gegl_node_connect_to (src_node,  "output",
                            operation, "input");
    }

  dest_node = gegl_node_new_child (gegl,
                                   "operation", "gegl:write-buffer",
                                   "buffer",    dest_buffer,
                                   NULL);


  gegl_node_connect_to (operation, "output",
                        dest_node, "input");

  if (progress)
    {
      GeglProcessor *processor;

      processor = gegl_node_new_processor (dest_node, &rect);

      progress_active = picman_progress_is_active (progress);

      if (progress_active)
        {
          if (undo_desc)
            picman_progress_set_text (progress, undo_desc);
        }
      else
        {
          picman_progress_start (progress, undo_desc, FALSE);
        }

      while (gegl_processor_work (processor, &value))
        picman_progress_set_value (progress, value);

      g_object_unref (processor);
    }
  else
    {
      gegl_node_blit (dest_node, 1.0, &rect,
                      NULL, NULL, 0, GEGL_BLIT_DEFAULT);
    }

  g_object_unref (gegl);

  if (progress && ! progress_active)
    picman_progress_end (progress);
}

void
picman_gegl_apply_color_reduction (GeglBuffer   *src_buffer,
                                 PicmanProgress *progress,
                                 const gchar  *undo_desc,
                                 GeglBuffer   *dest_buffer,
                                 gint          bits,
                                 gint          dither_type)
{
  GeglNode *node;

  g_return_if_fail (GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));

  node = gegl_node_new_child (NULL,
                              "operation",       "gegl:color-reduction",
                              "red-bits",        bits,
                              "green-bits",      bits,
                              "blue-bits",       bits,
                              "alpha-bits",      bits,
                              "dither-strategy", dither_type,
                              NULL);

  picman_gegl_apply_operation (src_buffer, progress, undo_desc,
                             node, dest_buffer, NULL);
  g_object_unref (node);
}

void
picman_gegl_apply_flatten (GeglBuffer    *src_buffer,
                         PicmanProgress  *progress,
                         const gchar   *undo_desc,
                         GeglBuffer    *dest_buffer,
                         const PicmanRGB *background)
{
  GeglNode *node;

  g_return_if_fail (GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));
  g_return_if_fail (background != NULL);

  node = picman_gegl_create_flatten_node (background);

  picman_gegl_apply_operation (src_buffer, progress, undo_desc,
                             node, dest_buffer, NULL);
  g_object_unref (node);
}

void
picman_gegl_apply_feather (GeglBuffer   *src_buffer,
                         PicmanProgress *progress,
                         const gchar  *undo_desc,
                         GeglBuffer   *dest_buffer,
                         gdouble       radius_x,
                         gdouble       radius_y)
{
  g_return_if_fail (GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));

  /* 3.5 is completely magic and picked to visually match the old
   * gaussian_blur_region() on a crappy laptop display
   */
  picman_gegl_apply_gaussian_blur (src_buffer,
                                 progress, undo_desc,
                                 dest_buffer,
                                 radius_x / 3.5,
                                 radius_y / 3.5);
}

void
picman_gegl_apply_gaussian_blur (GeglBuffer   *src_buffer,
                               PicmanProgress *progress,
                               const gchar  *undo_desc,
                               GeglBuffer   *dest_buffer,
                               gdouble       std_dev_x,
                               gdouble       std_dev_y)
{
  GeglNode *node;

  g_return_if_fail (GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));

  node = gegl_node_new_child (NULL,
                              "operation", "gegl:gaussian-blur",
                              "std-dev-x", std_dev_x,
                              "std-dev-y", std_dev_y,
                              NULL);

  picman_gegl_apply_operation (src_buffer, progress, undo_desc,
                             node, dest_buffer, NULL);
  g_object_unref (node);
}

void
picman_gegl_apply_invert (GeglBuffer    *src_buffer,
                        PicmanProgress  *progress,
                        const gchar   *undo_desc,
                        GeglBuffer    *dest_buffer)
{
  GeglNode *node;

  g_return_if_fail (GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));

  node = gegl_node_new_child (NULL,
                              "operation", "gegl:invert",
                              NULL);

  picman_gegl_apply_operation (src_buffer, progress, undo_desc,
                             node, dest_buffer, NULL);
  g_object_unref (node);
}


void
picman_gegl_apply_opacity (GeglBuffer    *src_buffer,
                         PicmanProgress  *progress,
                         const gchar   *undo_desc,
                         GeglBuffer    *dest_buffer,
                         GeglBuffer    *mask,
                         gint           mask_offset_x,
                         gint           mask_offset_y,
                         gdouble        opacity)
{
  GeglNode *node;

  g_return_if_fail (GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));
  g_return_if_fail (mask == NULL || GEGL_IS_BUFFER (mask));

  node = picman_gegl_create_apply_opacity_node (mask,
                                              mask_offset_x,
                                              mask_offset_y,
                                              opacity);

  picman_gegl_apply_operation (src_buffer, progress, undo_desc,
                             node, dest_buffer, NULL);
  g_object_unref (node);
}

void
picman_gegl_apply_scale (GeglBuffer            *src_buffer,
                       PicmanProgress          *progress,
                       const gchar           *undo_desc,
                       GeglBuffer            *dest_buffer,
                       PicmanInterpolationType  interpolation_type,
                       gdouble                x,
                       gdouble                y)
{
  GeglNode *node;

  g_return_if_fail (GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));

  node = gegl_node_new_child (NULL,
                              "operation", "gegl:scale",
                              "origin-x",   0.0,
                              "origin-y",   0.0,
                              "filter",     picman_interpolation_to_gegl_filter (interpolation_type),
                              "hard-edges", TRUE,
                              "x",          x,
                              "y",          y,
                              NULL);

  picman_gegl_apply_operation (src_buffer, progress, undo_desc,
                             node, dest_buffer, NULL);
  g_object_unref (node);
}

void
picman_gegl_apply_set_alpha (GeglBuffer    *src_buffer,
                           PicmanProgress  *progress,
                           const gchar   *undo_desc,
                           GeglBuffer    *dest_buffer,
                           gdouble        value)
{
  GeglNode *node;

  g_return_if_fail (GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));

  node = gegl_node_new_child (NULL,
                              "operation", "picman:set-alpha",
                              "value",     value,
                              NULL);

  picman_gegl_apply_operation (src_buffer, progress, undo_desc,
                             node, dest_buffer, NULL);
  g_object_unref (node);
}

void
picman_gegl_apply_threshold (GeglBuffer    *src_buffer,
                           PicmanProgress  *progress,
                           const gchar   *undo_desc,
                           GeglBuffer    *dest_buffer,
                           gdouble        value)
{
  GeglNode *node;

  g_return_if_fail (GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));

  node = gegl_node_new_child (NULL,
                              "operation", "gegl:threshold",
                              "value",     value,
                              NULL);

  picman_gegl_apply_operation (src_buffer, progress, undo_desc,
                             node, dest_buffer, NULL);
  g_object_unref (node);
}

void
picman_gegl_apply_transform (GeglBuffer            *src_buffer,
                           PicmanProgress          *progress,
                           const gchar           *undo_desc,
                           GeglBuffer            *dest_buffer,
                           PicmanInterpolationType  interpolation_type,
                           PicmanMatrix3           *transform)
{
  GeglNode *node;

  g_return_if_fail (GEGL_IS_BUFFER (src_buffer));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (GEGL_IS_BUFFER (dest_buffer));

  node = gegl_node_new_child (NULL,
                              "operation", "gegl:transform",
                              "filter",     picman_interpolation_to_gegl_filter (interpolation_type),
                              "hard-edges", TRUE,
                              NULL);

  picman_gegl_node_set_matrix (node, transform);

  picman_gegl_apply_operation (src_buffer, progress, undo_desc,
                             node, dest_buffer, NULL);
  g_object_unref (node);
}
