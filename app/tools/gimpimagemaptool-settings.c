/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanimagemaptool-settings.c
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

#include <errno.h>

#include <glib/gstdio.h>
#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimagemapconfig.h"
#include "core/picmanlist.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmansettingsbox.h"

#include "picmanimagemapoptions.h"
#include "picmanimagemaptool.h"
#include "picmanimagemaptool-settings.h"

#include "picman-intl.h"


/*  local function prototypes  */

static gboolean picman_image_map_tool_settings_import (PicmanSettingsBox  *box,
                                                     const gchar      *filename,
                                                     PicmanImageMapTool *tool);
static gboolean picman_image_map_tool_settings_export (PicmanSettingsBox  *box,
                                                     const gchar      *filename,
                                                     PicmanImageMapTool *tool);


/*  public functions  */

GtkWidget *
picman_image_map_tool_real_get_settings_ui (PicmanImageMapTool  *image_map_tool,
                                          PicmanContainer     *settings,
                                          const gchar       *settings_filename,
                                          const gchar       *import_dialog_title,
                                          const gchar       *export_dialog_title,
                                          const gchar       *file_dialog_help_id,
                                          const gchar       *default_folder,
                                          GtkWidget        **settings_box)
{
  PicmanToolInfo *tool_info;
  GtkSizeGroup *label_group;
  GtkWidget    *hbox;
  GtkWidget    *label;
  GtkWidget    *settings_combo;

  tool_info = PICMAN_TOOL (image_map_tool)->tool_info;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

  label_group = picman_image_map_tool_dialog_get_label_group (image_map_tool);

  label = gtk_label_new_with_mnemonic (_("Pre_sets:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_size_group_add_widget (label_group, label);
  gtk_widget_show (label);

  *settings_box = picman_settings_box_new (tool_info->picman,
                                         image_map_tool->config,
                                         settings,
                                         settings_filename,
                                         import_dialog_title,
                                         export_dialog_title,
                                         file_dialog_help_id,
                                         default_folder,
                                         NULL);
  gtk_box_pack_start (GTK_BOX (hbox), *settings_box, TRUE, TRUE, 0);
  gtk_widget_show (*settings_box);

  settings_combo = picman_settings_box_get_combo (PICMAN_SETTINGS_BOX (*settings_box));
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), settings_combo);

  g_signal_connect (image_map_tool->settings_box, "import",
                    G_CALLBACK (picman_image_map_tool_settings_import),
                    image_map_tool);

  g_signal_connect (image_map_tool->settings_box, "export",
                    G_CALLBACK (picman_image_map_tool_settings_export),
                    image_map_tool);

  return hbox;
}

gboolean
picman_image_map_tool_real_settings_import (PicmanImageMapTool  *tool,
                                          const gchar       *filename,
                                          GError           **error)
{
  gboolean success;

  if (PICMAN_TOOL (tool)->tool_info->picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  success = picman_config_deserialize_file (PICMAN_CONFIG (tool->config),
                                          filename,
                                          NULL, error);

  return success;
}

gboolean
picman_image_map_tool_real_settings_export (PicmanImageMapTool  *tool,
                                          const gchar       *filename,
                                          GError           **error)
{
  PicmanImageMapToolClass *klass = PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool);
  gchar                 *header;
  gchar                 *footer;
  gboolean               success;

  header = g_strdup_printf ("PICMAN %s tool settings",   klass->settings_name);
  footer = g_strdup_printf ("end of %s tool settings", klass->settings_name);

  if (PICMAN_TOOL (tool)->tool_info->picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  success = picman_config_serialize_to_file (PICMAN_CONFIG (tool->config),
                                           filename,
                                           header, footer,
                                           NULL, error);

  g_free (header);
  g_free (footer);

  return success;
}


/*  private functions  */

static gboolean
picman_image_map_tool_settings_import (PicmanSettingsBox  *box,
                                     const gchar      *filename,
                                     PicmanImageMapTool *tool)
{
  PicmanImageMapToolClass *tool_class = PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool);
  GError                *error      = NULL;

  g_return_val_if_fail (tool_class->settings_import != NULL, FALSE);

  if (! tool_class->settings_import (tool, filename, &error))
    {
      picman_message_literal (PICMAN_TOOL (tool)->tool_info->picman,
			    G_OBJECT (tool->dialog),
			    PICMAN_MESSAGE_ERROR, error->message);
      g_clear_error (&error);

      return FALSE;
    }

  picman_image_map_tool_preview (tool);

  g_object_set (PICMAN_TOOL_GET_OPTIONS (tool),
                "settings", filename,
                NULL);

  return TRUE;
}

static gboolean
picman_image_map_tool_settings_export (PicmanSettingsBox  *box,
                                     const gchar      *filename,
                                     PicmanImageMapTool *tool)
{
  PicmanImageMapToolClass *tool_class = PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool);
  GError                *error      = NULL;
  gchar                 *display_name;

  g_return_val_if_fail (tool_class->settings_export != NULL, FALSE);

  if (! tool_class->settings_export (tool, filename, &error))
    {
      picman_message_literal (PICMAN_TOOL (tool)->tool_info->picman,
			    G_OBJECT (tool->dialog),
			    PICMAN_MESSAGE_ERROR, error->message);
      g_clear_error (&error);

      return FALSE;
    }

  display_name = g_filename_display_name (filename);
  picman_message (PICMAN_TOOL (tool)->tool_info->picman,
                G_OBJECT (PICMAN_TOOL (tool)->display),
                PICMAN_MESSAGE_INFO,
                _("Settings saved to '%s'"),
                display_name);
  g_free (display_name);

  g_object_set (PICMAN_TOOL_GET_OPTIONS (tool),
                "settings", filename,
                NULL);

  return TRUE;
}
