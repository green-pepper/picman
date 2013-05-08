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

#ifndef __PICMAN_DODGE_BURN_H__
#define __PICMAN_DODGE_BURN_H__


#include "picmanbrushcore.h"


#define PICMAN_TYPE_DODGE_BURN            (picman_dodge_burn_get_type ())
#define PICMAN_DODGE_BURN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DODGE_BURN, PicmanDodgeBurn))
#define PICMAN_IS_DODGE_BURN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DODGE_BURN))
#define PICMAN_DODGE_BURN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DODGEBURN, PicmanDodgeBurnClass))
#define PICMAN_IS_DODGE_BURN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DODGE_BURN))


typedef struct _PicmanDodgeBurnClass PicmanDodgeBurnClass;

struct _PicmanDodgeBurn
{
  PicmanBrushCore  parent_instance;
};

struct _PicmanDodgeBurnClass
{
  PicmanBrushCoreClass  parent_class;
};


void    picman_dodge_burn_register (Picman                      *picman,
                                  PicmanPaintRegisterCallback  callback);

GType   picman_dodge_burn_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_DODGE_BURN_H__  */
