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

#ifndef __PICMAN_HEAL_H__
#define __PICMAN_HEAL_H__


#include "picmansourcecore.h"


#define PICMAN_TYPE_HEAL            (picman_heal_get_type ())
#define PICMAN_HEAL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_HEAL, PicmanHeal))
#define PICMAN_HEAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_HEAL, PicmanHealClass))
#define PICMAN_IS_HEAL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_HEAL))
#define PICMAN_IS_HEAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_HEAL))
#define PICMAN_HEAL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_HEAL, PicmanHealClass))


typedef struct _PicmanHealClass PicmanHealClass;

struct _PicmanHeal
{
  PicmanSourceCore  parent_instance;
};

struct _PicmanHealClass
{
  PicmanSourceCoreClass  parent_class;
};


void    picman_heal_register (Picman                      *picman,
                            PicmanPaintRegisterCallback  callback);

GType   picman_heal_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_HEAL_H__  */
