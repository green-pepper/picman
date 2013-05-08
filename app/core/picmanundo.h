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

#ifndef __PICMAN_UNDO_H__
#define __PICMAN_UNDO_H__


#include "picmanviewable.h"


struct _PicmanUndoAccumulator
{
  gboolean mode_changed;
  gboolean precision_changed;

  gboolean size_changed;
  gint     previous_origin_x;
  gint     previous_origin_y;
  gint     previous_width;
  gint     previous_height;

  gboolean resolution_changed;

  gboolean unit_changed;
};


#define PICMAN_TYPE_UNDO            (picman_undo_get_type ())
#define PICMAN_UNDO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_UNDO, PicmanUndo))
#define PICMAN_UNDO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_UNDO, PicmanUndoClass))
#define PICMAN_IS_UNDO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_UNDO))
#define PICMAN_IS_UNDO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_UNDO))
#define PICMAN_UNDO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_UNDO, PicmanUndoClass))


typedef struct _PicmanUndoClass PicmanUndoClass;

struct _PicmanUndo
{
  PicmanViewable      parent_instance;

  PicmanImage        *image;          /* the image this undo is part of     */
  guint             time;           /* time of undo step construction     */

  PicmanUndoType      undo_type;      /* undo type                          */
  PicmanDirtyMask     dirty_mask;     /* affected parts of the image        */

  PicmanTempBuf      *preview;
  guint             preview_idle_id;
};

struct _PicmanUndoClass
{
  PicmanViewableClass  parent_class;

  void (* pop)  (PicmanUndo            *undo,
                 PicmanUndoMode         undo_mode,
                 PicmanUndoAccumulator *accum);
  void (* free) (PicmanUndo            *undo,
                 PicmanUndoMode         undo_mode);
};


GType         picman_undo_get_type        (void) G_GNUC_CONST;

void          picman_undo_pop             (PicmanUndo            *undo,
                                         PicmanUndoMode         undo_mode,
                                         PicmanUndoAccumulator *accum);
void          picman_undo_free            (PicmanUndo            *undo,
                                         PicmanUndoMode         undo_mode);

void          picman_undo_create_preview  (PicmanUndo            *undo,
                                         PicmanContext         *context,
                                         gboolean             create_now);
void          picman_undo_refresh_preview (PicmanUndo            *undo,
                                         PicmanContext         *context);

const gchar * picman_undo_type_to_name    (PicmanUndoType         type);

gboolean      picman_undo_is_weak         (PicmanUndo            *undo);
gint          picman_undo_get_age         (PicmanUndo            *undo);
void          picman_undo_reset_age       (PicmanUndo            *undo);


#endif /* __PICMAN_UNDO_H__ */
