/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationdissolvemode.h
 * Copyright (C) 2012 Ville Sokk <ville.sokk@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __PICMAN_OPERATION_DISSOLVE_MODE_H__
#define __PICMAN_OPERATION_DISSOLVE_MODE_H__


#include "picmanoperationpointlayermode.h"


#define PICMAN_TYPE_OPERATION_DISSOLVE_MODE            (picman_operation_dissolve_mode_get_type ())
#define PICMAN_OPERATION_DISSOLVE_MODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_DISSOLVE_MODE, PicmanOperationDissolveMode))
#define PICMAN_OPERATION_DISSOLVE_MODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_DISSOLVE_MODE, PicmanOperationDissolveModeClass))
#define PICMAN_IS_OPERATION_DISSOLVE_MODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_DISSOLVE_MODE))
#define PICMAN_IS_OPERATION_DISSOLVE_MODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_DISSOLVE_MODE))
#define PICMAN_OPERATION_DISSOLVE_MODE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_DISSOLVE_MODE, PicmanOperationDissolveModeClass))


typedef struct _PicmanOperationDissolveMode      PicmanOperationDissolveMode;
typedef struct _PicmanOperationDissolveModeClass PicmanOperationDissolveModeClass;

struct _PicmanOperationDissolveModeClass
{
  PicmanOperationPointLayerModeClass parent_class;
};

struct _PicmanOperationDissolveMode
{
  PicmanOperationPointLayerMode parent_instance;
};


GType   picman_operation_dissolve_mode_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_OPERATION_DISSOLVE_MODE_H__ */
