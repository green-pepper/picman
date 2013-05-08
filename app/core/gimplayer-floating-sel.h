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

#ifndef __PICMAN_LAYER_FLOATING_SEL_H__
#define __PICMAN_LAYER_FLOATING_SEL_H__


void                 floating_sel_attach            (PicmanLayer     *layer,
                                                     PicmanDrawable  *drawable);
void                 floating_sel_anchor            (PicmanLayer     *layer);
gboolean             floating_sel_to_layer          (PicmanLayer     *layer,
                                                     GError       **error);
void                 floating_sel_activate_drawable (PicmanLayer     *layer);
const PicmanBoundSeg * floating_sel_boundary          (PicmanLayer     *layer,
                                                     gint          *n_segs);
void                 floating_sel_invalidate        (PicmanLayer     *layer);


#endif /* __PICMAN_LAYER_FLOATING_SEL_H__ */
