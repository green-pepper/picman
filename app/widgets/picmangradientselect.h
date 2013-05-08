/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmangradientselect.h
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

#ifndef __PICMAN_GRADIENT_SELECT_H__
#define __PICMAN_GRADIENT_SELECT_H__

#include "picmanpdbdialog.h"

G_BEGIN_DECLS


#define PICMAN_TYPE_GRADIENT_SELECT            (picman_gradient_select_get_type ())
#define PICMAN_GRADIENT_SELECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_GRADIENT_SELECT, PicmanGradientSelect))
#define PICMAN_GRADIENT_SELECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_GRADIENT_SELECT, PicmanGradientSelectClass))
#define PICMAN_IS_GRADIENT_SELECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_GRADIENT_SELECT))
#define PICMAN_IS_GRADIENT_SELECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_GRADIENT_SELECT))
#define PICMAN_GRADIENT_SELECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_GRADIENT_SELECT, PicmanGradientSelectClass))


typedef struct _PicmanGradientSelectClass  PicmanGradientSelectClass;

struct _PicmanGradientSelect
{
  PicmanPdbDialog  parent_instance;

  gint           sample_size;
};

struct _PicmanGradientSelectClass
{
  PicmanPdbDialogClass  parent_class;
};


GType  picman_gradient_select_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __PICMAN_GRADIENT_SELECT_H__ */
