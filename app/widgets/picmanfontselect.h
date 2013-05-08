/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfontselect.h
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_FONT_SELECT_H__
#define __PICMAN_FONT_SELECT_H__

#include "picmanpdbdialog.h"

G_BEGIN_DECLS


#define PICMAN_TYPE_FONT_SELECT            (picman_font_select_get_type ())
#define PICMAN_FONT_SELECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FONT_SELECT, PicmanFontSelect))
#define PICMAN_FONT_SELECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FONT_SELECT, PicmanFontSelectClass))
#define PICMAN_IS_FONT_SELECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FONT_SELECT))
#define PICMAN_IS_FONT_SELECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FONT_SELECT))
#define PICMAN_FONT_SELECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FONT_SELECT, PicmanFontSelectClass))


typedef struct _PicmanFontSelectClass  PicmanFontSelectClass;

struct _PicmanFontSelect
{
  PicmanPdbDialog  parent_instance;
};

struct _PicmanFontSelectClass
{
  PicmanPdbDialogClass  parent_class;
};


GType  picman_font_select_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __PICMAN_FONT_SELECT_H__ */
