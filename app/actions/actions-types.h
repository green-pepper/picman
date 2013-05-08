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

#ifndef __ACTIONS_TYPES_H__
#define __ACTIONS_TYPES_H__


#include "dialogs/dialogs-types.h"
#include "tools/tools-types.h"


typedef enum
{
  PICMAN_ACTION_SELECT_SET              =  0,
  PICMAN_ACTION_SELECT_SET_TO_DEFAULT   = -1,
  PICMAN_ACTION_SELECT_FIRST            = -2,
  PICMAN_ACTION_SELECT_LAST             = -3,
  PICMAN_ACTION_SELECT_SMALL_PREVIOUS   = -4,
  PICMAN_ACTION_SELECT_SMALL_NEXT       = -5,
  PICMAN_ACTION_SELECT_PREVIOUS         = -6,
  PICMAN_ACTION_SELECT_NEXT             = -7,
  PICMAN_ACTION_SELECT_SKIP_PREVIOUS    = -8,
  PICMAN_ACTION_SELECT_SKIP_NEXT        = -9,
  PICMAN_ACTION_SELECT_PERCENT_PREVIOUS = -10,
  PICMAN_ACTION_SELECT_PERCENT_NEXT     = -11
} PicmanActionSelectType;

typedef enum
{
  PICMAN_SAVE_MODE_SAVE,
  PICMAN_SAVE_MODE_SAVE_AS,
  PICMAN_SAVE_MODE_SAVE_A_COPY,
  PICMAN_SAVE_MODE_SAVE_AND_CLOSE,
  PICMAN_SAVE_MODE_EXPORT,
  PICMAN_SAVE_MODE_EXPORT_TO,
  PICMAN_SAVE_MODE_OVERWRITE
} PicmanSaveMode;


#endif /* __ACTIONS_TYPES_H__ */
