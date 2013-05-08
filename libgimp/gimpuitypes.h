/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanuitypes.h
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_UI_TYPES_H__
#define __PICMAN_UI_TYPES_H__

#include <libpicmanwidgets/picmanwidgetstypes.h>

G_BEGIN_DECLS

/* For information look into the html documentation */


typedef struct _PicmanAspectPreview        PicmanAspectPreview;
typedef struct _PicmanDrawablePreview      PicmanDrawablePreview;
typedef struct _PicmanProcBrowserDialog    PicmanProcBrowserDialog;
typedef struct _PicmanProgressBar          PicmanProgressBar;
typedef struct _PicmanZoomPreview          PicmanZoomPreview;

typedef struct _PicmanDrawableComboBox     PicmanDrawableComboBox;
typedef struct _PicmanChannelComboBox      PicmanChannelComboBox;
typedef struct _PicmanLayerComboBox        PicmanLayerComboBox;
typedef struct _PicmanVectorsComboBox      PicmanVectorsComboBox;
typedef struct _PicmanImageComboBox        PicmanImageComboBox;

typedef struct _PicmanSelectButton         PicmanSelectButton;
typedef struct _PicmanBrushSelectButton    PicmanBrushSelectButton;
typedef struct _PicmanFontSelectButton     PicmanFontSelectButton;
typedef struct _PicmanGradientSelectButton PicmanGradientSelectButton;
typedef struct _PicmanPaletteSelectButton  PicmanPaletteSelectButton;
typedef struct _PicmanPatternSelectButton  PicmanPatternSelectButton;


G_END_DECLS

#endif /* __PICMAN_UI_TYPES_H__ */
