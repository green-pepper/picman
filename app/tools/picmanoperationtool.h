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

#ifndef __PICMAN_OPERATION_TOOL_H__
#define __PICMAN_OPERATION_TOOL_H__


#include "picmanimagemaptool.h"


#define PICMAN_TYPE_OPERATION_TOOL            (picman_operation_tool_get_type ())
#define PICMAN_OPERATION_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_TOOL, PicmanOperationTool))
#define PICMAN_OPERATION_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_OPERATION_TOOL, PicmanOperationToolClass))
#define PICMAN_IS_OPERATION_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_TOOL))
#define PICMAN_IS_OPERATION_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_OPERATION_TOOL))
#define PICMAN_OPERATION_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_OPERATION_TOOL, PicmanOperationToolClass))


typedef struct _PicmanOperationTool      PicmanOperationTool;
typedef struct _PicmanOperationToolClass PicmanOperationToolClass;

struct _PicmanOperationTool
{
  PicmanImageMapTool  parent_instance;

  gchar            *operation;
  gchar            *undo_desc;
  PicmanObject       *config;

  /* dialog */
  GtkWidget        *options_box;
  GtkWidget        *options_table;
};

struct _PicmanOperationToolClass
{
  PicmanImageMapToolClass  parent_class;
};


void    picman_operation_tool_register      (PicmanToolRegisterCallback  callback,
                                           gpointer                  data);

GType   picman_operation_tool_get_type      (void) G_GNUC_CONST;

void    picman_operation_tool_set_operation (PicmanOperationTool        *tool,
                                           const gchar              *operation,
                                           const gchar              *undo_desc);


#endif  /*  __PICMAN_OPERATION_TOOL_H__  */
