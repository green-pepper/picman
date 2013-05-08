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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmantoolinfo.h"

#include "tools/picmanoperationtool.h"
#include "tools/tool_manager.h"

#include "actions.h"
#include "filters-commands.h"

#include "picman-intl.h"


/*  public functions  */

void
filters_filter_cmd_callback (GtkAction   *action,
                             const gchar *operation,
                             gpointer     data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  PicmanDisplay  *display;
  PicmanTool     *active_tool;
  return_if_no_drawable (image, drawable, data);
  return_if_no_display (display, data);

  active_tool = tool_manager_get_active (image->picman);

  if (G_TYPE_FROM_INSTANCE (active_tool) != PICMAN_TYPE_OPERATION_TOOL)
    {
      PicmanToolInfo *tool_info = picman_get_tool_info (image->picman,
                                                    "picman-operation-tool");

      if (PICMAN_IS_TOOL_INFO (tool_info))
        picman_context_set_tool (action_data_get_context (data), tool_info);
    }
  else
    {
      picman_context_tool_changed (action_data_get_context (data));
    }

  active_tool = tool_manager_get_active (image->picman);

  if (PICMAN_IS_OPERATION_TOOL (active_tool))
    {
      gchar       *label    = picman_strip_uline (gtk_action_get_label (action));
      const gchar *ellipsis = _("...");
      gint         label_len;
      gint         ellipsis_len;

      label_len    = strlen (label);
      ellipsis_len = strlen (ellipsis);

      if (label_len > ellipsis_len &&
          strcmp (label + label_len - ellipsis_len, ellipsis) == 0)
        {
          label[label_len - ellipsis_len] = '\0';
        }

      picman_operation_tool_set_operation (PICMAN_OPERATION_TOOL (active_tool),
                                         operation, label);
      tool_manager_initialize_active (image->picman, display);

      g_free (label);
    }
}
