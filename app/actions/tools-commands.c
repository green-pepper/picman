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

#include "config.h"

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmantoolinfo.h"

#include "paint/picmaninkoptions.h"

#include "widgets/picmanenumaction.h"
#include "widgets/picmanuimanager.h"

#include "display/picmandisplay.h"

#include "tools/picman-tools.h"
#include "tools/picmancoloroptions.h"
#include "tools/picmanforegroundselectoptions.h"
#include "tools/picmanrectangleoptions.h"
#include "tools/picmanimagemaptool.h"
#include "tools/picmantoolcontrol.h"
#include "tools/picmantransformoptions.h"
#include "tools/tool_manager.h"

#include "actions.h"
#include "tools-commands.h"


/*  local function prototypes  */

static void   tools_activate_enum_action (const gchar *action_desc,
                                          gint         value);


/*  public functions  */

void
tools_select_cmd_callback (GtkAction   *action,
                           const gchar *value,
                           gpointer     data)
{
  Picman         *picman;
  PicmanToolInfo *tool_info;
  PicmanContext  *context;
  PicmanDisplay  *display;
  gboolean      rotate_layer = FALSE;
  return_if_no_picman (picman, data);

  /*  special case picman-rotate-tool being called from the Layer menu  */
  if (strcmp (value, "picman-rotate-layer") == 0)
    {
      rotate_layer = TRUE;
      value = "picman-rotate-tool";
    }

  tool_info = picman_get_tool_info (picman, value);

  context = picman_get_user_context (picman);

  /*  always allocate a new tool when selected from the image menu
   */
  if (picman_context_get_tool (context) != tool_info)
    {
      picman_context_set_tool (context, tool_info);

      if (rotate_layer)
        g_object_set (tool_info->tool_options,
                      "type", PICMAN_TRANSFORM_TYPE_LAYER,
                      NULL);
    }
  else
    {
      picman_context_tool_changed (context);
    }

  display = picman_context_get_display (context);

  if (display && picman_display_get_image (display))
    tool_manager_initialize_active (picman, display);
}

void
tools_color_average_radius_cmd_callback (GtkAction *action,
                                         gint       value,
                                         gpointer   data)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  return_if_no_context (context, data);

  tool_info = picman_context_get_tool (context);

  if (tool_info && PICMAN_IS_COLOR_OPTIONS (tool_info->tool_options))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (tool_info->tool_options),
                              "average-radius",
                              1.0, 1.0, 10.0, 0.1, FALSE);
    }
}

void
tools_paint_brush_size_cmd_callback (GtkAction *action,
                                     gint       value,
                                     gpointer   data)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  return_if_no_context (context, data);

  tool_info = picman_context_get_tool (context);

  if (tool_info && PICMAN_IS_PAINT_OPTIONS (tool_info->tool_options))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (tool_info->tool_options),
                              "brush-size",
                              0.1, 1.0, 10.0, 1.0, FALSE);
    }
}

void
tools_paint_brush_angle_cmd_callback (GtkAction *action,
                                      gint       value,
                                      gpointer   data)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  return_if_no_context (context, data);

  tool_info = picman_context_get_tool (context);

  if (tool_info && PICMAN_IS_PAINT_OPTIONS (tool_info->tool_options))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (tool_info->tool_options),
                              "brush-angle",
                              0.1, 1.0, 15.0, 0.1, TRUE);
    }
}

void
tools_paint_brush_aspect_ratio_cmd_callback (GtkAction *action,
                                             gint       value,
                                             gpointer   data)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  return_if_no_context (context, data);

  tool_info = picman_context_get_tool (context);

  if (tool_info && PICMAN_IS_PAINT_OPTIONS (tool_info->tool_options))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (tool_info->tool_options),
                              "brush-aspect-ratio",
                              0.01, 0.1, 1.0, 0.1, TRUE);
    }
}

void
tools_ink_blob_size_cmd_callback (GtkAction *action,
                                  gint       value,
                                  gpointer   data)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  return_if_no_context (context, data);

  tool_info = picman_context_get_tool (context);

  if (tool_info && PICMAN_IS_INK_OPTIONS (tool_info->tool_options))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (tool_info->tool_options),
                              "size",
                              1.0, 1.0, 10.0, 0.1, FALSE);
    }
}

void
tools_ink_blob_aspect_cmd_callback (GtkAction *action,
                                    gint       value,
                                    gpointer   data)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  return_if_no_context (context, data);

  tool_info = picman_context_get_tool (context);

  if (tool_info && PICMAN_IS_INK_OPTIONS (tool_info->tool_options))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (tool_info->tool_options),
                              "blob-aspect",
                              1.0, 0.1, 1.0, 0.1, FALSE);
    }
}

void
tools_ink_blob_angle_cmd_callback (GtkAction *action,
                                   gint       value,
                                   gpointer   data)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  return_if_no_context (context, data);

  tool_info = picman_context_get_tool (context);

  if (tool_info && PICMAN_IS_INK_OPTIONS (tool_info->tool_options))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (tool_info->tool_options),
                              "blob-angle",
                              1.0, 1.0, 15.0, 0.1, TRUE);
    }
}

void
tools_fg_select_brush_size_cmd_callback (GtkAction *action,
                                         gint       value,
                                         gpointer   data)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  return_if_no_context (context, data);

  tool_info = picman_context_get_tool (context);

#if 0
  if (tool_info && PICMAN_IS_FOREGROUND_SELECT_OPTIONS (tool_info->tool_options))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (tool_info->tool_options),
                              "stroke-width",
                              1.0, 4.0, 16.0, 0.1, FALSE);
    }
#endif
}

void
tools_transform_preview_opacity_cmd_callback (GtkAction *action,
                                              gint       value,
                                              gpointer   data)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  return_if_no_context (context, data);

  tool_info = picman_context_get_tool (context);

  if (tool_info && PICMAN_IS_TRANSFORM_OPTIONS (tool_info->tool_options))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (tool_info->tool_options),
                              "preview-opacity",
                              0.01, 0.1, 0.5, 0.1, FALSE);
    }
}

void
tools_value_1_cmd_callback (GtkAction *action,
                            gint       value,
                            gpointer   data)
{
  PicmanContext *context;
  PicmanTool    *tool;
  return_if_no_context (context, data);

  tool = tool_manager_get_active (context->picman);

  if (tool)
    {
      const gchar *action_desc;

      action_desc = picman_tool_control_get_action_value_1 (tool->control);

      if (action_desc)
        tools_activate_enum_action (action_desc, value);
    }
}

void
tools_value_2_cmd_callback (GtkAction *action,
                            gint       value,
                            gpointer   data)
{
  PicmanContext *context;
  PicmanTool    *tool;
  return_if_no_context (context, data);

  tool = tool_manager_get_active (context->picman);

  if (tool)
    {
      const gchar *action_desc;

      action_desc = picman_tool_control_get_action_value_2 (tool->control);

      if (action_desc)
        tools_activate_enum_action (action_desc, value);
    }
}

void
tools_value_3_cmd_callback (GtkAction *action,
                            gint       value,
                            gpointer   data)
{
  PicmanContext *context;
  PicmanTool    *tool;
  return_if_no_context (context, data);

  tool = tool_manager_get_active (context->picman);

  if (tool)
    {
      const gchar *action_desc;

      action_desc = picman_tool_control_get_action_value_3 (tool->control);

      if (action_desc)
        tools_activate_enum_action (action_desc, value);
    }
}

void
tools_value_4_cmd_callback (GtkAction *action,
                            gint       value,
                            gpointer   data)
{
  PicmanContext *context;
  PicmanTool    *tool;
  return_if_no_context (context, data);

  tool = tool_manager_get_active (context->picman);

  if (tool)
    {
      const gchar *action_desc;

      action_desc = picman_tool_control_get_action_value_4 (tool->control);

      if (action_desc)
        tools_activate_enum_action (action_desc, value);
    }
}

void
tools_object_1_cmd_callback (GtkAction *action,
                             gint       value,
                             gpointer   data)
{
  PicmanContext *context;
  PicmanTool    *tool;
  return_if_no_context (context, data);

  tool = tool_manager_get_active (context->picman);

  if (tool)
    {
      const gchar *action_desc;

      action_desc = picman_tool_control_get_action_object_1 (tool->control);

      if (action_desc)
        tools_activate_enum_action (action_desc, value);
    }
}

void
tools_object_2_cmd_callback (GtkAction *action,
                             gint       value,
                             gpointer   data)
{
  PicmanContext *context;
  PicmanTool    *tool;
  return_if_no_context (context, data);

  tool = tool_manager_get_active (context->picman);

  if (tool)
    {
      const gchar *action_desc;

      action_desc = picman_tool_control_get_action_object_2 (tool->control);

      if (action_desc)
        tools_activate_enum_action (action_desc, value);
    }
}


/*  private functions  */

static void
tools_activate_enum_action (const gchar *action_desc,
                            gint         value)
{
  gchar *group_name;
  gchar *action_name;

  group_name  = g_strdup (action_desc);
  action_name = strchr (group_name, '/');

  if (action_name)
    {
      GList     *managers;
      GtkAction *action;

      *action_name++ = '\0';

      managers = picman_ui_managers_from_name ("<Image>");

      action = picman_ui_manager_find_action (managers->data,
                                            group_name, action_name);

      if (PICMAN_IS_ENUM_ACTION (action) &&
          PICMAN_ENUM_ACTION (action)->value_variable)
        {
          picman_enum_action_selected (PICMAN_ENUM_ACTION (action), value);
        }
    }

  g_free (group_name);
}
