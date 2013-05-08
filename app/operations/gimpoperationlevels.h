/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationlevels.h
 * Copyright (C) 2007 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_OPERATION_LEVELS_H__
#define __PICMAN_OPERATION_LEVELS_H__


#include "picmanoperationpointfilter.h"


#define PICMAN_TYPE_OPERATION_LEVELS            (picman_operation_levels_get_type ())
#define PICMAN_OPERATION_LEVELS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_LEVELS, PicmanOperationLevels))
#define PICMAN_OPERATION_LEVELS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_LEVELS, PicmanOperationLevelsClass))
#define PICMAN_IS_OPERATION_LEVELS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_LEVELS))
#define PICMAN_IS_OPERATION_LEVELS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_LEVELS))
#define PICMAN_OPERATION_LEVELS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_LEVELS, PicmanOperationLevelsClass))


typedef struct _PicmanOperationLevels      PicmanOperationLevels;
typedef struct _PicmanOperationLevelsClass PicmanOperationLevelsClass;

struct _PicmanOperationLevels
{
  PicmanOperationPointFilter  parent_instance;
};

struct _PicmanOperationLevelsClass
{
  PicmanOperationPointFilterClass  parent_class;
};


GType     picman_operation_levels_get_type  (void) G_GNUC_CONST;

gdouble   picman_operation_levels_map_input (PicmanLevelsConfig     *config,
                                           PicmanHistogramChannel  channel,
                                           gdouble               value);


#endif /* __PICMAN_OPERATION_LEVELS_H__ */
