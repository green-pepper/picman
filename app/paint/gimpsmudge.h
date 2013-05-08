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

#ifndef __PICMAN_SMUDGE_H__
#define __PICMAN_SMUDGE_H__


#include "picmanbrushcore.h"


#define PICMAN_TYPE_SMUDGE            (picman_smudge_get_type ())
#define PICMAN_SMUDGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SMUDGE, PicmanSmudge))
#define PICMAN_SMUDGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SMUDGE, PicmanSmudgeClass))
#define PICMAN_IS_SMUDGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SMUDGE))
#define PICMAN_IS_SMUDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SMUDGE))
#define PICMAN_SMUDGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SMUDGE, PicmanSmudgeClass))


typedef struct _PicmanSmudgeClass PicmanSmudgeClass;

struct _PicmanSmudge
{
  PicmanBrushCore  parent_instance;

  gboolean       initialized;
  GeglBuffer    *accum_buffer;
};

struct _PicmanSmudgeClass
{
  PicmanBrushCoreClass  parent_class;
};


void    picman_smudge_register (Picman                      *picman,
                              PicmanPaintRegisterCallback  callback);

GType   picman_smudge_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_SMUDGE_H__  */
