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

#ifndef __DISPLAY_TYPES_H__
#define __DISPLAY_TYPES_H__


#include "widgets/widgets-types.h"

#include "display/display-enums.h"


typedef struct _PicmanCanvas               PicmanCanvas;
typedef struct _PicmanCanvasGroup          PicmanCanvasGroup;
typedef struct _PicmanCanvasItem           PicmanCanvasItem;

typedef struct _PicmanDisplay              PicmanDisplay;
typedef struct _PicmanDisplayShell         PicmanDisplayShell;
typedef struct _PicmanMotionBuffer         PicmanMotionBuffer;

typedef struct _PicmanImageWindow          PicmanImageWindow;
typedef struct _PicmanMultiWindowStrategy  PicmanMultiWindowStrategy;
typedef struct _PicmanSingleWindowStrategy PicmanSingleWindowStrategy;

typedef struct _PicmanCursorView           PicmanCursorView;
typedef struct _PicmanNavigationEditor     PicmanNavigationEditor;
typedef struct _PicmanScaleComboBox        PicmanScaleComboBox;
typedef struct _PicmanStatusbar            PicmanStatusbar;

typedef struct _PicmanToolDialog           PicmanToolDialog;

typedef struct _PicmanDisplayXfer          PicmanDisplayXfer;
typedef struct _Selection                Selection;


#endif /* __DISPLAY_TYPES_H__ */
