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

#ifndef __PICMAN_GEGL_NODES_H__
#define __PICMAN_GEGL_NODES_H__


GeglNode * picman_gegl_create_flatten_node       (const PicmanRGB        *background);
GeglNode * picman_gegl_create_apply_opacity_node (GeglBuffer           *mask,
                                                gint                  mask_offset_x,
                                                gint                  mask_offset_y,
                                                gdouble               opacity);
GeglNode * picman_gegl_add_buffer_source         (GeglNode             *parent,
                                                GeglBuffer           *buffer,
                                                gint                  offset_x,
                                                gint                  offset_y);

void       picman_gegl_mode_node_set_mode        (GeglNode             *node,
                                                PicmanLayerModeEffects  mode,
                                                gboolean              linear);
void       picman_gegl_mode_node_set_opacity     (GeglNode             *node,
                                                gdouble               opacity);
void       picman_gegl_node_set_matrix           (GeglNode             *node,
                                                const PicmanMatrix3    *matrix);


#endif /* __PICMAN_GEGL_NODES_H__ */
