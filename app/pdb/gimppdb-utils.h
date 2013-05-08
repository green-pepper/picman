/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2003 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_PDB_UTILS_H__
#define __PICMAN_PDB_UTILS_H__


PicmanBrush     * picman_pdb_get_brush              (Picman               *picman,
                                                 const gchar        *name,
                                                 gboolean            writable,
                                                 GError            **error);
PicmanBrush     * picman_pdb_get_generated_brush    (Picman               *picman,
                                                 const gchar        *name,
                                                 gboolean            writable,
                                                 GError            **error);
PicmanDynamics  * picman_pdb_get_dynamics           (Picman               *picman,
                                                 const gchar        *name,
                                                 gboolean            writable,
                                                 GError            **error);
PicmanPattern   * picman_pdb_get_pattern            (Picman               *picman,
                                                 const gchar        *name,
                                                 GError            **error);
PicmanGradient  * picman_pdb_get_gradient           (Picman               *picman,
                                                 const gchar        *name,
                                                 gboolean            writable,
                                                 GError            **error);
PicmanPalette   * picman_pdb_get_palette            (Picman               *picman,
                                                 const gchar        *name,
                                                 gboolean            writable,
                                                 GError            **error);
PicmanFont      * picman_pdb_get_font               (Picman               *picman,
                                                 const gchar        *name,
                                                 GError            **error);
PicmanBuffer    * picman_pdb_get_buffer             (Picman               *picman,
                                                 const gchar        *name,
                                                 GError            **error);
PicmanPaintInfo * picman_pdb_get_paint_info         (Picman               *picman,
                                                 const gchar        *name,
                                                 GError            **error);

gboolean        picman_pdb_item_is_attached       (PicmanItem           *item,
                                                 PicmanImage          *image,
                                                 PicmanPDBItemModify   modify,
                                                 GError            **error);
gboolean        picman_pdb_item_is_in_tree        (PicmanItem           *item,
                                                 PicmanImage          *image,
                                                 PicmanPDBItemModify   modify,
                                                 GError            **error);
gboolean        picman_pdb_item_is_in_same_tree   (PicmanItem           *item,
                                                 PicmanItem           *item2,
                                                 PicmanImage          *image,
                                                 GError            **error);
gboolean        picman_pdb_item_is_not_ancestor   (PicmanItem           *item,
                                                 PicmanItem           *not_descendant,
                                                 GError            **error);
gboolean        picman_pdb_item_is_floating       (PicmanItem           *item,
                                                 PicmanImage          *dest_image,
                                                 GError            **error);
gboolean        picman_pdb_item_is_modifyable     (PicmanItem           *item,
                                                 PicmanPDBItemModify   modify,
                                                 GError            **error);
gboolean        picman_pdb_item_is_group          (PicmanItem           *item,
                                                 GError            **error);
gboolean        picman_pdb_item_is_not_group      (PicmanItem           *item,
                                                 GError            **error);

gboolean        picman_pdb_layer_is_text_layer    (PicmanLayer          *layer,
                                                 PicmanPDBItemModify   modify,
                                                 GError            **error);

gboolean        picman_pdb_image_is_base_type     (PicmanImage          *image,
                                                 PicmanImageBaseType   type,
                                                 GError            **error);
gboolean        picman_pdb_image_is_not_base_type (PicmanImage          *image,
                                                 PicmanImageBaseType   type,
                                                 GError            **error);

gboolean        picman_pdb_image_is_precision     (PicmanImage          *image,
                                                 PicmanPrecision       precision,
                                                 GError            **error);
gboolean        picman_pdb_image_is_not_precision (PicmanImage          *image,
                                                 PicmanPrecision       precision,
                                                 GError            **error);

PicmanStroke    * picman_pdb_get_vectors_stroke     (PicmanVectors        *vectors,
                                                 gint                stroke_ID,
                                                 PicmanPDBItemModify   modify,
                                                 GError            **error);


#endif /* __PICMAN_PDB_UTILS_H__ */
