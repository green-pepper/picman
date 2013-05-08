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

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmandatafactory.h"
#include "core/picmantoolinfo.h"
#include "core/picmantooloptions.h"
#include "core/picmantoolpreset.h"

#include "widgets/picmandataeditor.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmaneditor.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"
#include "widgets/picmantooloptionseditor.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanwindowstrategy.h"

#include "dialogs/data-delete-dialog.h"

#include "tool-options-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   tool_options_show_preset_editor (Picman           *picman,
                                               PicmanEditor     *editor,
                                               PicmanToolPreset *preset);


/*  public functions  */

void
tool_options_save_new_preset_cmd_callback (GtkAction *action,
                                           gpointer   user_data)
{
  PicmanEditor  *editor  = PICMAN_EDITOR (user_data);
  Picman        *picman    = picman_editor_get_ui_manager (editor)->picman;
  PicmanContext *context = picman_get_user_context (picman);
  PicmanData    *data;

  data = picman_data_factory_data_new (context->picman->tool_preset_factory,
                                     context, _("Untitled"));

  tool_options_show_preset_editor (picman, editor, PICMAN_TOOL_PRESET (data));
}

void
tool_options_save_preset_cmd_callback (GtkAction *action,
                                       gint       value,
                                       gpointer   data)
{
  PicmanEditor     *editor    = PICMAN_EDITOR (data);
  Picman           *picman      = picman_editor_get_ui_manager (editor)->picman;
  PicmanContext    *context   = picman_get_user_context (picman);
  PicmanToolInfo   *tool_info = picman_context_get_tool (context);
  PicmanToolPreset *preset;

  preset = (PicmanToolPreset *)
    picman_container_get_child_by_index (tool_info->presets, value);

  if (preset)
    {
      picman_config_sync (G_OBJECT (tool_info->tool_options),
                        G_OBJECT (preset->tool_options), 0);

      tool_options_show_preset_editor (picman, editor, preset);
    }
}

void
tool_options_restore_preset_cmd_callback (GtkAction *action,
                                          gint       value,
                                          gpointer   data)
{
  PicmanEditor     *editor    = PICMAN_EDITOR (data);
  Picman           *picman      = picman_editor_get_ui_manager (editor)->picman;
  PicmanContext    *context   = picman_get_user_context (picman);
  PicmanToolInfo   *tool_info = picman_context_get_tool (context);
  PicmanToolPreset *preset;

  preset = (PicmanToolPreset *)
    picman_container_get_child_by_index (tool_info->presets, value);

  if (preset)
    {
      if (picman_context_get_tool_preset (context) != preset)
        picman_context_set_tool_preset (context, preset);
      else
        picman_context_tool_preset_changed (context);
    }
}

void
tool_options_edit_preset_cmd_callback (GtkAction *action,
                                       gint       value,
                                       gpointer   data)
{
  PicmanEditor     *editor    = PICMAN_EDITOR (data);
  Picman           *picman      = picman_editor_get_ui_manager (editor)->picman;
  PicmanContext    *context   = picman_get_user_context (picman);
  PicmanToolInfo   *tool_info = picman_context_get_tool (context);
  PicmanToolPreset *preset;

  preset = (PicmanToolPreset *)
    picman_container_get_child_by_index (tool_info->presets, value);

  if (preset)
    {
      tool_options_show_preset_editor (picman, editor, preset);
    }
}

void
tool_options_delete_preset_cmd_callback (GtkAction *action,
                                         gint       value,
                                         gpointer   data)
{
  PicmanEditor     *editor    = PICMAN_EDITOR (data);
  PicmanContext    *context   = picman_get_user_context (picman_editor_get_ui_manager (editor)->picman);
  PicmanToolInfo   *tool_info = picman_context_get_tool (context);
  PicmanToolPreset *preset;

  preset = (PicmanToolPreset *)
    picman_container_get_child_by_index (tool_info->presets, value);

  if (preset &&
      picman_data_is_deletable (PICMAN_DATA (preset)))
    {
      PicmanDataFactory *factory = context->picman->tool_preset_factory;
      GtkWidget       *dialog;

      dialog = data_delete_dialog_new (factory, PICMAN_DATA (preset), NULL,
                                       GTK_WIDGET (editor));
      gtk_widget_show (dialog);
    }
}

void
tool_options_reset_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  PicmanEditor   *editor    = PICMAN_EDITOR (data);
  PicmanContext  *context   = picman_get_user_context (picman_editor_get_ui_manager (editor)->picman);
  PicmanToolInfo *tool_info = picman_context_get_tool (context);

  picman_tool_options_reset (tool_info->tool_options);
}

void
tool_options_reset_all_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  PicmanEditor *editor = PICMAN_EDITOR (data);
  GtkWidget  *dialog;

  dialog = picman_message_dialog_new (_("Reset All Tool Options"),
                                    PICMAN_STOCK_QUESTION,
                                    GTK_WIDGET (editor),
                                    GTK_DIALOG_MODAL |
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    picman_standard_help_func, NULL,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    PICMAN_STOCK_RESET, GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect_object (gtk_widget_get_toplevel (GTK_WIDGET (editor)),
                           "unmap",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog, G_CONNECT_SWAPPED);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("Do you really want to reset all "
                                       "tool options to default values?"));

  if (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      Picman  *picman = picman_editor_get_ui_manager (editor)->picman;
      GList *list;

      for (list = picman_get_tool_info_iter (picman);
           list;
           list = g_list_next (list))
        {
          PicmanToolInfo *tool_info = list->data;

          picman_tool_options_reset (tool_info->tool_options);
        }
    }

  gtk_widget_destroy (dialog);
}


/*  private functions  */

static void
tool_options_show_preset_editor (Picman           *picman,
                                 PicmanEditor     *editor,
                                 PicmanToolPreset *preset)
{
  GtkWidget *dockable;

  dockable =
    picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (picman)),
                                               picman,
                                               picman_dialog_factory_get_singleton (),
                                               gtk_widget_get_screen (GTK_WIDGET (editor)),
                                               "picman-tool-preset-editor");

  picman_data_editor_set_data (PICMAN_DATA_EDITOR (gtk_bin_get_child (GTK_BIN (dockable))),
                             PICMAN_DATA (preset));
}
