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

#ifndef __PICMAN_IMAGE__UNDO_H__
#define __PICMAN_IMAGE__UNDO_H__


gboolean        picman_image_undo_is_enabled      (const PicmanImage  *image);
gboolean        picman_image_undo_enable          (PicmanImage        *image);
gboolean        picman_image_undo_disable         (PicmanImage        *image);
gboolean        picman_image_undo_freeze          (PicmanImage        *image);
gboolean        picman_image_undo_thaw            (PicmanImage        *image);

gboolean        picman_image_undo                 (PicmanImage        *image);
gboolean        picman_image_redo                 (PicmanImage        *image);

gboolean        picman_image_strong_undo          (PicmanImage        *image);
gboolean        picman_image_strong_redo          (PicmanImage        *image);

PicmanUndoStack * picman_image_get_undo_stack       (const PicmanImage  *image);
PicmanUndoStack * picman_image_get_redo_stack       (const PicmanImage  *image);

void            picman_image_undo_free            (PicmanImage        *image);

gint            picman_image_get_undo_group_count (const PicmanImage  *image);
gboolean        picman_image_undo_group_start     (PicmanImage        *image,
                                                 PicmanUndoType      undo_type,
                                                 const gchar      *name);
gboolean        picman_image_undo_group_end       (PicmanImage        *image);

PicmanUndo      * picman_image_undo_push            (PicmanImage        *image,
                                                 GType             object_type,
                                                 PicmanUndoType      undo_type,
                                                 const gchar      *name,
                                                 PicmanDirtyMask     dirty_mask,
                                                 ...) G_GNUC_NULL_TERMINATED;

PicmanUndo      * picman_image_undo_can_compress    (PicmanImage        *image,
                                                 GType             object_type,
                                                 PicmanUndoType      undo_type);

PicmanUndo      * picman_image_undo_get_fadeable    (PicmanImage        *image);


#endif /* __PICMAN_IMAGE__UNDO_H__ */
