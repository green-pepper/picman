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

#ifndef __PICMAN_EDIT_H__
#define __PICMAN_EDIT_H__


const PicmanBuffer * picman_edit_cut                (PicmanImage     *image,
                                                 PicmanDrawable  *drawable,
                                                 PicmanContext   *context,
                                                 GError       **error);
const PicmanBuffer * picman_edit_copy               (PicmanImage     *image,
                                                 PicmanDrawable  *drawable,
                                                 PicmanContext   *context,
                                                 GError       **error);
const PicmanBuffer * picman_edit_copy_visible       (PicmanImage     *image,
                                                 PicmanContext   *context,
                                                 GError       **error);
PicmanLayer        * picman_edit_paste              (PicmanImage     *image,
                                                 PicmanDrawable  *drawable,
                                                 PicmanBuffer    *paste,
                                                 gboolean       paste_into,
                                                 gint           viewport_x,
                                                 gint           viewport_y,
                                                 gint           viewport_width,
                                                 gint           viewport_height);

const gchar      * picman_edit_named_cut          (PicmanImage     *image,
                                                 const gchar   *name,
                                                 PicmanDrawable  *drawable,
                                                 PicmanContext   *context,
                                                 GError       **error);
const gchar      * picman_edit_named_copy         (PicmanImage     *image,
                                                 const gchar   *name,
                                                 PicmanDrawable  *drawable,
                                                 PicmanContext   *context,
                                                 GError       **error);
const gchar      * picman_edit_named_copy_visible (PicmanImage     *image,
                                                 const gchar   *name,
                                                 PicmanContext   *context,
                                                 GError       **error);

gboolean           picman_edit_clear              (PicmanImage     *image,
                                                 PicmanDrawable  *drawable,
                                                 PicmanContext   *context);
gboolean           picman_edit_fill               (PicmanImage     *image,
                                                 PicmanDrawable  *drawable,
                                                 PicmanContext   *context,
                                                 PicmanFillType   fill_type,
                                                 gdouble        opacity,
                                                 PicmanLayerModeEffects  paint_mode);

gboolean           picman_edit_fill_full          (PicmanImage     *image,
                                                 PicmanDrawable  *drawable,
                                                 const PicmanRGB *color,
                                                 PicmanPattern   *pattern,
                                                 gdouble        opacity,
                                                 PicmanLayerModeEffects  paint_mode,
                                                 const gchar   *undo_desc);

gboolean           picman_edit_fade               (PicmanImage     *image,
                                                 PicmanContext   *context);


#endif  /*  __PICMAN_EDIT_H__  */
