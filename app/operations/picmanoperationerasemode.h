/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationerasemode.h
 * Copyright (C) 2008 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_OPERATION_ERASE_MODE_H__
#define __PICMAN_OPERATION_ERASE_MODE_H__


#include "picmanoperationpointlayermode.h"


#define PICMAN_TYPE_OPERATION_ERASE_MODE            (picman_operation_erase_mode_get_type ())
#define PICMAN_OPERATION_ERASE_MODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_ERASE_MODE, PicmanOperationEraseMode))
#define PICMAN_OPERATION_ERASE_MODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_ERASE_MODE, PicmanOperationEraseModeClass))
#define PICMAN_IS_OPERATION_ERASE_MODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_ERASE_MODE))
#define PICMAN_IS_OPERATION_ERASE_MODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_ERASE_MODE))
#define PICMAN_OPERATION_ERASE_MODE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_ERASE_MODE, PicmanOperationEraseModeClass))


typedef struct _PicmanOperationEraseMode      PicmanOperationEraseMode;
typedef struct _PicmanOperationEraseModeClass PicmanOperationEraseModeClass;

struct _PicmanOperationEraseMode
{
  PicmanOperationPointLayerMode  parent_instance;
};

struct _PicmanOperationEraseModeClass
{
  PicmanOperationPointLayerModeClass  parent_class;
};


GType   picman_operation_erase_mode_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_OPERATION_ERASE_MODE_H__ */
