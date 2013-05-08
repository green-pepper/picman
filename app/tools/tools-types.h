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

#ifndef __TOOLS_TYPES_H__
#define __TOOLS_TYPES_H__

#include "paint/paint-types.h"
#include "display/display-types.h"

#include "tools/tools-enums.h"


G_BEGIN_DECLS


typedef struct _PicmanTool                     PicmanTool;
typedef struct _PicmanToolControl              PicmanToolControl;

typedef struct _PicmanBrushTool                PicmanBrushTool;
typedef struct _PicmanColorTool                PicmanColorTool;
typedef struct _PicmanDrawTool                 PicmanDrawTool;
typedef struct _PicmanForegroundSelectToolUndo PicmanForegroundSelectToolUndo;
typedef struct _PicmanImageMapTool             PicmanImageMapTool;
typedef struct _PicmanPaintTool                PicmanPaintTool;
typedef struct _PicmanTransformTool            PicmanTransformTool;
typedef struct _PicmanTransformToolUndo        PicmanTransformToolUndo;

typedef struct _PicmanColorOptions             PicmanColorOptions;
typedef struct _PicmanImageMapOptions          PicmanImageMapOptions;


/*  functions  */

typedef GtkWidget * (* PicmanToolOptionsGUIFunc) (PicmanToolOptions *tool_options);

typedef void (* PicmanToolRegisterCallback) (GType                     tool_type,
                                           GType                     tool_option_type,
                                           PicmanToolOptionsGUIFunc    options_gui_func,
                                           PicmanContextPropMask       context_props,
                                           const gchar              *identifier,
                                           const gchar              *blurb,
                                           const gchar              *help,
                                           const gchar              *menu_path,
                                           const gchar              *menu_accel,
                                           const gchar              *help_domain,
                                           const gchar              *help_data,
                                           const gchar              *stock_id,
                                           gpointer                  register_data);

typedef void (* PicmanToolRegisterFunc)     (PicmanToolRegisterCallback  callback,
                                           gpointer                  register_data);


G_END_DECLS

#endif /* __TOOLS_TYPES_H__ */
