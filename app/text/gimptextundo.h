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

#ifndef __PICMAN_TEXT_UNDO_H__
#define __PICMAN_TEXT_UNDO_H__


#include "core/picmanitemundo.h"


#define PICMAN_TYPE_TEXT_UNDO            (picman_text_undo_get_type ())
#define PICMAN_TEXT_UNDO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEXT_UNDO, PicmanTextUndo))
#define PICMAN_TEXT_UNDO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEXT_UNDO, PicmanTextUndoClass))
#define PICMAN_IS_TEXT_UNDO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEXT_UNDO))
#define PICMAN_IS_TEXT_UNDO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEXT_UNDO))
#define PICMAN_TEXT_UNDO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TEXT_UNDO, PicmanTextUndoClass))


typedef struct _PicmanTextUndoClass PicmanTextUndoClass;

struct _PicmanTextUndo
{
  PicmanItemUndo      parent_instance;

  PicmanText         *text;
  const GParamSpec *pspec;
  GValue           *value;
  gboolean          modified;
  const Babl       *format;
};

struct _PicmanTextUndoClass
{
  PicmanItemClass  parent_class;
};


GType      picman_text_undo_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_TEXT_UNDO_H__ */
