/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "config/picmanrc.h"

#include "core/picman.h"
#include "core/picmanlist.h"
#include "core/picmantemplate.h"

#include "widgets/picmancolorpanel.h"
#include "widgets/picmancontainercombobox.h"
#include "widgets/picmancontainerview.h"
#include "widgets/picmancontrollerlist.h"
#include "widgets/picmandevices.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmangrideditor.h"
#include "widgets/picmanhelp.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"
#include "widgets/picmanprefsbox.h"
#include "widgets/picmanprofilechooserdialog.h"
#include "widgets/picmanpropwidgets.h"
#include "widgets/picmantemplateeditor.h"
#include "widgets/picmantooleditor.h"
#include "widgets/picmanwidgets-constructors.h"
#include "widgets/picmanwidgets-utils.h"

#include "menus/menus.h"

#include "tools/picman-tools.h"

#include "gui/session.h"
#include "gui/themes.h"

#include "preferences-dialog.h"
#include "resolution-calibrate-dialog.h"

#include "picman-intl.h"


#define RESPONSE_RESET 1

#define COLOR_BUTTON_WIDTH  40
#define COLOR_BUTTON_HEIGHT 24


/*  preferences local functions  */

static GtkWidget * prefs_dialog_new               (Picman       *picman,
                                                   PicmanConfig *config);
static void        prefs_config_notify            (GObject    *config,
                                                   GParamSpec *param_spec,
                                                   GObject    *config_copy);
static void        prefs_config_copy_notify       (GObject    *config_copy,
                                                   GParamSpec *param_spec,
                                                   GObject    *config);
static void        prefs_response                 (GtkWidget  *widget,
                                                   gint        response_id,
                                                   GtkWidget  *dialog);

static void        prefs_message                  (GtkMessageType  type,
                                                   gboolean        destroy,
                                                   const gchar    *message);

static void   prefs_resolution_source_callback    (GtkWidget  *widget,
                                                   GObject    *config);
static void   prefs_resolution_calibrate_callback (GtkWidget  *widget,
                                                   GtkWidget  *entry);
static void   prefs_input_devices_dialog          (GtkWidget  *widget,
                                                   Picman       *picman);
static void   prefs_keyboard_shortcuts_dialog     (GtkWidget  *widget,
                                                   Picman       *picman);
static void   prefs_menus_save_callback           (GtkWidget  *widget,
                                                   Picman       *picman);
static void   prefs_menus_clear_callback          (GtkWidget  *widget,
                                                   Picman       *picman);
static void   prefs_menus_remove_callback         (GtkWidget  *widget,
                                                   Picman       *picman);
static void   prefs_session_save_callback         (GtkWidget  *widget,
                                                   Picman       *picman);
static void   prefs_session_clear_callback        (GtkWidget  *widget,
                                                   Picman       *picman);
static void   prefs_devices_save_callback         (GtkWidget  *widget,
                                                   Picman       *picman);
static void   prefs_devices_clear_callback        (GtkWidget  *widget,
                                                   Picman       *picman);
static void   prefs_tool_options_save_callback    (GtkWidget  *widget,
                                                   Picman       *picman);
static void   prefs_tool_options_clear_callback   (GtkWidget  *widget,
                                                   Picman       *picman);


/*  private variables  */

static GtkWidget *prefs_dialog = NULL;
static GtkWidget *tool_editor  = NULL;


/*  public function  */

GtkWidget *
preferences_dialog_create (Picman *picman)
{
  PicmanConfig *config;
  PicmanConfig *config_copy;
  PicmanConfig *config_orig;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (prefs_dialog)
    return prefs_dialog;

  /*  turn off autosaving while the prefs dialog is open  */
  picman_rc_set_autosave (PICMAN_RC (picman->edit_config), FALSE);

  config       = PICMAN_CONFIG (picman->edit_config);
  config_copy  = picman_config_duplicate (config);
  config_orig  = picman_config_duplicate (config);

  g_signal_connect_object (config, "notify",
                           G_CALLBACK (prefs_config_notify),
                           config_copy, 0);
  g_signal_connect_object (config_copy, "notify",
                           G_CALLBACK (prefs_config_copy_notify),
                           config, 0);

  prefs_dialog = prefs_dialog_new (picman, config_copy);

  g_object_add_weak_pointer (G_OBJECT (prefs_dialog),
                             (gpointer) &prefs_dialog);

  g_object_set_data (G_OBJECT (prefs_dialog), "picman", picman);

  g_object_set_data_full (G_OBJECT (prefs_dialog), "config-copy", config_copy,
                          (GDestroyNotify) g_object_unref);
  g_object_set_data_full (G_OBJECT (prefs_dialog), "config-orig", config_orig,
                          (GDestroyNotify) g_object_unref);

  return prefs_dialog;
}


/*  private functions  */

static void
prefs_config_notify (GObject    *config,
                     GParamSpec *param_spec,
                     GObject    *config_copy)
{
  GValue global_value = { 0, };
  GValue copy_value   = { 0, };

  g_value_init (&global_value, param_spec->value_type);
  g_value_init (&copy_value,   param_spec->value_type);

  g_object_get_property (config,      param_spec->name, &global_value);
  g_object_get_property (config_copy, param_spec->name, &copy_value);

  if (g_param_values_cmp (param_spec, &global_value, &copy_value))
    {
      g_signal_handlers_block_by_func (config_copy,
                                       prefs_config_copy_notify,
                                       config);

      g_object_set_property (config_copy, param_spec->name, &global_value);

      g_signal_handlers_unblock_by_func (config_copy,
                                         prefs_config_copy_notify,
                                         config);
    }

  g_value_unset (&global_value);
  g_value_unset (&copy_value);
}

static void
prefs_config_copy_notify (GObject    *config_copy,
                          GParamSpec *param_spec,
                          GObject    *config)
{
  GValue copy_value   = { 0, };
  GValue global_value = { 0, };

  g_value_init (&copy_value,   param_spec->value_type);
  g_value_init (&global_value, param_spec->value_type);

  g_object_get_property (config_copy, param_spec->name, &copy_value);
  g_object_get_property (config,      param_spec->name, &global_value);

  if (g_param_values_cmp (param_spec, &copy_value, &global_value))
    {
      if (param_spec->flags & PICMAN_CONFIG_PARAM_CONFIRM)
        {
#ifdef PICMAN_CONFIG_DEBUG
          g_print ("NOT Applying prefs change of '%s' to edit_config "
                   "because it needs confirmation\n",
                   param_spec->name);
#endif
        }
      else
        {
#ifdef PICMAN_CONFIG_DEBUG
          g_print ("Applying prefs change of '%s' to edit_config\n",
                   param_spec->name);
#endif
          g_signal_handlers_block_by_func (config,
                                           prefs_config_notify,
                                           config_copy);

          g_object_set_property (config, param_spec->name, &copy_value);

          g_signal_handlers_unblock_by_func (config,
                                             prefs_config_notify,
                                             config_copy);
        }
    }

  g_value_unset (&copy_value);
  g_value_unset (&global_value);
}

static void
prefs_response (GtkWidget *widget,
                gint       response_id,
                GtkWidget *dialog)
{
  Picman *picman = g_object_get_data (G_OBJECT (dialog), "picman");

  switch (response_id)
    {
    case RESPONSE_RESET:
      {
        GtkWidget *confirm;

        confirm = picman_message_dialog_new (_("Reset All Preferences"),
                                           PICMAN_STOCK_QUESTION,
                                           dialog,
                                           GTK_DIALOG_MODAL |
                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                           picman_standard_help_func, NULL,

                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           PICMAN_STOCK_RESET, GTK_RESPONSE_OK,

                                           NULL);

        gtk_dialog_set_alternative_button_order (GTK_DIALOG (confirm),
                                                 GTK_RESPONSE_OK,
                                                 GTK_RESPONSE_CANCEL,
                                                 -1);

        picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (confirm)->box,
                                           _("Do you really want to reset all "
                                             "preferences to default values?"));

        if (picman_dialog_run (PICMAN_DIALOG (confirm)) == GTK_RESPONSE_OK)
          {
            PicmanConfig *config_copy;

            config_copy = g_object_get_data (G_OBJECT (dialog), "config-copy");

            picman_config_reset (config_copy);
          }

        gtk_widget_destroy (confirm);

        return;
      }
      break;

    case GTK_RESPONSE_OK:
      {
        GObject *config_copy;
        GList   *restart_diff;
        GList   *confirm_diff;
        GList   *list;

        config_copy = g_object_get_data (G_OBJECT (dialog), "config-copy");

        /*  destroy config_orig  */
        g_object_set_data (G_OBJECT (dialog), "config-orig", NULL);

        gtk_widget_set_sensitive (GTK_WIDGET (dialog), FALSE);

        confirm_diff = picman_config_diff (G_OBJECT (picman->edit_config),
                                         config_copy,
                                         PICMAN_CONFIG_PARAM_CONFIRM);

        g_object_freeze_notify (G_OBJECT (picman->edit_config));

        for (list = confirm_diff; list; list = g_list_next (list))
          {
            GParamSpec *param_spec = list->data;
            GValue      value      = { 0, };

            g_value_init (&value, param_spec->value_type);

            g_object_get_property (config_copy,
                                   param_spec->name, &value);
            g_object_set_property (G_OBJECT (picman->edit_config),
                                   param_spec->name, &value);

            g_value_unset (&value);
          }

        g_object_thaw_notify (G_OBJECT (picman->edit_config));

        g_list_free (confirm_diff);

        picman_rc_save (PICMAN_RC (picman->edit_config));

        /*  spit out a solely informational warning about changed values
         *  which need restart
         */
        restart_diff = picman_config_diff (G_OBJECT (picman->edit_config),
                                         G_OBJECT (picman->config),
                                         PICMAN_CONFIG_PARAM_RESTART);

        if (restart_diff)
          {
            GString *string;

            string = g_string_new (_("You will have to restart PICMAN for "
                                     "the following changes to take effect:"));
            g_string_append (string, "\n\n");

            for (list = restart_diff; list; list = g_list_next (list))
              {
                GParamSpec *param_spec = list->data;

                g_string_append_printf (string, "%s\n", param_spec->name);
              }

            prefs_message (GTK_MESSAGE_INFO, FALSE, string->str);

            g_string_free (string, TRUE);
          }

        g_list_free (restart_diff);
      }
      break;

    default:
      {
        GObject *config_orig;
        GList   *diff;
        GList   *list;

        config_orig = g_object_get_data (G_OBJECT (dialog), "config-orig");

        /*  destroy config_copy  */
        g_object_set_data (G_OBJECT (dialog), "config-copy", NULL);

        gtk_widget_set_sensitive (GTK_WIDGET (dialog), FALSE);

        diff = picman_config_diff (G_OBJECT (picman->edit_config),
                                 config_orig,
                                 PICMAN_CONFIG_PARAM_SERIALIZE);

        g_object_freeze_notify (G_OBJECT (picman->edit_config));

        for (list = diff; list; list = g_list_next (list))
          {
            GParamSpec *param_spec = list->data;
            GValue      value      = { 0, };

            g_value_init (&value, param_spec->value_type);

            g_object_get_property (config_orig,
                                   param_spec->name, &value);
            g_object_set_property (G_OBJECT (picman->edit_config),
                                   param_spec->name, &value);

            g_value_unset (&value);
          }

        picman_tool_editor_revert_changes (PICMAN_TOOL_EDITOR (tool_editor));

        g_object_thaw_notify (G_OBJECT (picman->edit_config));

        g_list_free (diff);
      }

      tool_editor = NULL;
    }

  /*  enable autosaving again  */
  picman_rc_set_autosave (PICMAN_RC (picman->edit_config), TRUE);

  gtk_widget_destroy (dialog);
}

static void
prefs_template_select_callback (PicmanContainerView *view,
                                PicmanTemplate      *template,
                                gpointer           insert_data,
                                PicmanTemplate      *edit_template)
{
  if (template)
    {
      /*  make sure the resolution values are copied first (see bug #546924)  */
      picman_config_sync (G_OBJECT (template), G_OBJECT (edit_template),
                        PICMAN_TEMPLATE_PARAM_COPY_FIRST);
      picman_config_sync (G_OBJECT (template), G_OBJECT (edit_template),
                        0);
    }
}

static void
prefs_resolution_source_callback (GtkWidget *widget,
                                  GObject   *config)
{
  gdouble  xres;
  gdouble  yres;
  gboolean from_gdk;

  from_gdk = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

  if (from_gdk)
    {
      picman_get_screen_resolution (NULL, &xres, &yres);
    }
  else
    {
      PicmanSizeEntry *entry = g_object_get_data (G_OBJECT (widget),
                                                "monitor_resolution_sizeentry");

      g_return_if_fail (PICMAN_IS_SIZE_ENTRY (entry));

      xres = picman_size_entry_get_refval (entry, 0);
      yres = picman_size_entry_get_refval (entry, 1);
    }

  g_object_set (config,
                "monitor-xresolution",                      xres,
                "monitor-yresolution",                      yres,
                "monitor-resolution-from-windowing-system", from_gdk,
                NULL);
}

static void
prefs_resolution_calibrate_callback (GtkWidget *widget,
                                     GtkWidget *entry)
{
  GtkWidget *dialog;
  GtkWidget *prefs_box;
  GtkWidget *image;

  dialog = gtk_widget_get_toplevel (entry);

  prefs_box = g_object_get_data (G_OBJECT (dialog), "prefs-box");
  image     = picman_prefs_box_get_image (PICMAN_PREFS_BOX (prefs_box));

  resolution_calibrate_dialog (entry, gtk_image_get_pixbuf (GTK_IMAGE (image)));
}

static void
prefs_input_devices_dialog (GtkWidget *widget,
                            Picman      *picman)
{
  picman_dialog_factory_dialog_raise (picman_dialog_factory_get_singleton (),
                                    gtk_widget_get_screen (widget),
                                    "picman-input-devices-dialog", 0);
}

static void
prefs_keyboard_shortcuts_dialog (GtkWidget *widget,
                                 Picman      *picman)
{
  picman_dialog_factory_dialog_raise (picman_dialog_factory_get_singleton (),
                                    gtk_widget_get_screen (widget),
                                    "picman-keyboard-shortcuts-dialog", 0);
}

static void
prefs_menus_save_callback (GtkWidget *widget,
                           Picman      *picman)
{
  GtkWidget *clear_button;

  menus_save (picman, TRUE);

  clear_button = g_object_get_data (G_OBJECT (widget), "clear-button");

  if (clear_button)
    gtk_widget_set_sensitive (clear_button, TRUE);
}

static void
prefs_menus_clear_callback (GtkWidget *widget,
                            Picman      *picman)
{
  GError *error = NULL;

  if (! menus_clear (picman, &error))
    {
      prefs_message (GTK_MESSAGE_ERROR, TRUE, error->message);
      g_clear_error (&error);
    }
  else
    {
      gtk_widget_set_sensitive (widget, FALSE);

      prefs_message (GTK_MESSAGE_INFO, TRUE,
                     _("Your keyboard shortcuts will be reset to "
                       "default values the next time you start PICMAN."));
    }
}

static void
prefs_menus_remove_callback (GtkWidget *widget,
                             Picman      *picman)
{
  GtkWidget *dialog;

  dialog = picman_message_dialog_new (_("Remove all Keyboard Shortcuts"),
                                    PICMAN_STOCK_QUESTION,
                                    gtk_widget_get_toplevel (widget),
                                    GTK_DIALOG_MODAL |
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    picman_standard_help_func, NULL,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_CLEAR,  GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect_object (gtk_widget_get_toplevel (widget), "unmap",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog, G_CONNECT_SWAPPED);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("Do you really want to remove all "
                                       "keyboard shortcuts from all menus?"));

  if (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      menus_remove (picman);
    }

  gtk_widget_destroy (dialog);
}

static void
prefs_session_save_callback (GtkWidget *widget,
                             Picman      *picman)
{
  GtkWidget *clear_button;

  session_save (picman, TRUE);

  clear_button = g_object_get_data (G_OBJECT (widget), "clear-button");

  if (clear_button)
    gtk_widget_set_sensitive (clear_button, TRUE);
}

static void
prefs_session_clear_callback (GtkWidget *widget,
                              Picman      *picman)
{
  GError *error = NULL;

  if (! session_clear (picman, &error))
    {
      prefs_message (GTK_MESSAGE_ERROR, TRUE, error->message);
      g_clear_error (&error);
    }
  else
    {
      gtk_widget_set_sensitive (widget, FALSE);

      prefs_message (GTK_MESSAGE_INFO, TRUE,
                     _("Your window setup will be reset to "
                       "default values the next time you start PICMAN."));
    }
}

static void
prefs_devices_save_callback (GtkWidget *widget,
                             Picman      *picman)
{
  GtkWidget *clear_button;

  picman_devices_save (picman, TRUE);

  clear_button = g_object_get_data (G_OBJECT (widget), "clear-button");

  if (clear_button)
    gtk_widget_set_sensitive (clear_button, TRUE);
}

static void
prefs_devices_clear_callback (GtkWidget *widget,
                              Picman      *picman)
{
  GError *error = NULL;

  if (! picman_devices_clear (picman, &error))
    {
      prefs_message (GTK_MESSAGE_ERROR, TRUE, error->message);
      g_clear_error (&error);
    }
  else
    {
      gtk_widget_set_sensitive (widget, FALSE);

      prefs_message (GTK_MESSAGE_INFO, TRUE,
                     _("Your input device settings will be reset to "
                       "default values the next time you start PICMAN."));
    }
}

static void
prefs_tool_options_save_callback (GtkWidget *widget,
                                  Picman      *picman)
{
  GtkWidget *clear_button;

  picman_tools_save (picman, TRUE, TRUE);

  clear_button = g_object_get_data (G_OBJECT (widget), "clear-button");

  if (clear_button)
    gtk_widget_set_sensitive (clear_button, TRUE);
}

static void
prefs_tool_options_clear_callback (GtkWidget *widget,
                                   Picman      *picman)
{
  GError *error = NULL;

  if (! picman_tools_clear (picman, &error))
    {
      prefs_message (GTK_MESSAGE_ERROR, TRUE, error->message);
      g_clear_error (&error);
    }
  else
    {
      gtk_widget_set_sensitive (widget, FALSE);

      prefs_message (GTK_MESSAGE_INFO, TRUE,
                     _("Your tool options will be reset to "
                       "default values the next time you start PICMAN."));
    }
}

static void
prefs_format_string_select_callback (GtkTreeSelection *sel,
                                     GtkEntry         *entry)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;

  if (gtk_tree_selection_get_selected (sel, &model, &iter))
    {
      GValue val = { 0, };

      gtk_tree_model_get_value (model, &iter, 1, &val);
      gtk_entry_set_text (entry, g_value_get_string (&val));
      g_value_unset (&val);
    }
}

static void
prefs_theme_select_callback (GtkTreeSelection *sel,
                             Picman             *picman)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;

  if (gtk_tree_selection_get_selected (sel, &model, &iter))
    {
      GValue val = { 0, };

      gtk_tree_model_get_value (model, &iter, 0, &val);
      g_object_set (picman->config, "theme", g_value_get_string (&val), NULL);
      g_value_unset (&val);
    }
}

static void
prefs_theme_reload_callback (GtkWidget *button,
                             Picman      *picman)
{
  g_object_notify (G_OBJECT (picman->config), "theme");
}

static GtkWidget *
prefs_frame_new (const gchar  *label,
                 GtkContainer *parent,
                 gboolean      expand)
{
  GtkWidget *frame;
  GtkWidget *vbox;

  frame = picman_frame_new (label);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  if (GTK_IS_BOX (parent))
    gtk_box_pack_start (GTK_BOX (parent), frame, expand, expand, 0);
  else
    gtk_container_add (parent, frame);

  gtk_widget_show (frame);

  return vbox;
}

static GtkWidget *
prefs_table_new (gint          rows,
                 GtkContainer *parent)
{
  GtkWidget *table;

  table = gtk_table_new (rows, 2, FALSE);

  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);

  if (GTK_IS_BOX (parent))
    gtk_box_pack_start (GTK_BOX (parent), table, FALSE, FALSE, 0);
  else
    gtk_container_add (parent, table);

  gtk_widget_show (table);

  return table;
}

static void
prefs_profile_combo_dialog_response (PicmanProfileChooserDialog *dialog,
                                     gint                      response,
                                     PicmanColorProfileComboBox *combo)
{
  if (response == GTK_RESPONSE_ACCEPT)
    {
      gchar *filename;

      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

      if (filename)
        {
          gchar *label = picman_profile_chooser_dialog_get_desc (dialog,
                                                               filename);

          picman_color_profile_combo_box_set_active (combo, filename, label);

          g_free (label);
        }
    }

  gtk_widget_hide (GTK_WIDGET (dialog));
}

static void
prefs_profile_combo_changed (PicmanColorProfileComboBox *combo,
                             GObject                  *config)
{
  gchar *filename = picman_color_profile_combo_box_get_active (combo);

  g_object_set (config,
                g_object_get_data (G_OBJECT (combo), "property-name"), filename,
                NULL);

  g_free (filename);
}

static GtkWidget *
prefs_profile_combo_add_tooltip (GtkWidget   *combo,
                                 GObject     *config,
                                 const gchar *property_name)
{
  GParamSpec  *param_spec;
  const gchar *blurb;

  param_spec = g_object_class_find_property (G_OBJECT_GET_CLASS (config),
                                             property_name);

  blurb = g_param_spec_get_blurb (param_spec);

  if (blurb)
    picman_help_set_help_data (combo,
                             dgettext (GETTEXT_PACKAGE "-libpicman", blurb),
                             NULL);

  return combo;
}

static GtkWidget *
prefs_profile_combo_box_new (Picman         *picman,
                             GObject      *config,
                             GtkListStore *store,
                             const gchar  *label,
                             const gchar  *property_name)
{
  GtkWidget *dialog = picman_profile_chooser_dialog_new (picman, label);
  GtkWidget *combo;
  gchar     *filename;

  g_object_get (config, property_name, &filename, NULL);

  combo = picman_color_profile_combo_box_new_with_model (dialog,
                                                       GTK_TREE_MODEL (store));

  g_object_set_data (G_OBJECT (combo),
                     "property-name", (gpointer) property_name);

  picman_color_profile_combo_box_set_active (PICMAN_COLOR_PROFILE_COMBO_BOX (combo),
                                           filename, NULL);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (prefs_profile_combo_dialog_response),
                    combo);

  g_signal_connect (combo, "changed",
                    G_CALLBACK (prefs_profile_combo_changed),
                    config);

  return prefs_profile_combo_add_tooltip (combo, config, property_name);
}

static GtkWidget *
prefs_button_add (const gchar *stock_id,
                  const gchar *label,
                  GtkBox      *box)
{
  GtkWidget *button;

  button = picman_stock_button_new (stock_id, label);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  return button;
}

static GtkWidget *
prefs_check_button_add (GObject     *config,
                        const gchar *property_name,
                        const gchar *label,
                        GtkBox      *vbox)
{
  GtkWidget *button;

  button = picman_prop_check_button_new (config, property_name, label);

  if (button)
    {
      gtk_box_pack_start (vbox, button, FALSE, FALSE, 0);
      gtk_widget_show (button);
    }

  return button;
}

static GtkWidget *
prefs_check_button_add_with_icon (GObject      *config,
                                  const gchar  *property_name,
                                  const gchar  *label,
                                  const gchar  *stock_id,
                                  GtkBox       *vbox,
                                  GtkSizeGroup *group)
{
  GtkWidget *button;
  GtkWidget *hbox;
  GtkWidget *image;

  button = picman_prop_check_button_new (config, property_name, label);
  if (!button)
    return NULL;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (vbox, hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
  gtk_misc_set_padding (GTK_MISC (image), 2, 2);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  gtk_widget_show (image);

  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);

  if (group)
    gtk_size_group_add_widget (group, image);

  return button;
}

static GtkWidget *
prefs_widget_add_aligned (GtkWidget    *widget,
                          const gchar  *text,
                          GtkTable     *table,
                          gint          table_row,
                          gboolean      left_align,
                          GtkSizeGroup *group)
{
  GtkWidget *label = picman_table_attach_aligned (table, 0, table_row,
                                                text, 0.0, 0.5,
                                                widget, 1, left_align);
  if (group)
    gtk_size_group_add_widget (group, label);

  return label;
}

static GtkWidget *
prefs_color_button_add (GObject      *config,
                        const gchar  *property_name,
                        const gchar  *label,
                        const gchar  *title,
                        GtkTable     *table,
                        gint          table_row,
                        GtkSizeGroup *group)
{
  GtkWidget *button = picman_prop_color_button_new (config, property_name,
                                                  title,
                                                  COLOR_BUTTON_WIDTH,
                                                  COLOR_BUTTON_HEIGHT,
                                                  PICMAN_COLOR_AREA_FLAT);

  if (button)
    prefs_widget_add_aligned (button, label, table, table_row, TRUE, group);

  return button;
}

static GtkWidget *
prefs_enum_combo_box_add (GObject      *config,
                          const gchar  *property_name,
                          gint          minimum,
                          gint          maximum,
                          const gchar  *label,
                          GtkTable     *table,
                          gint          table_row,
                          GtkSizeGroup *group)
{
  GtkWidget *combo = picman_prop_enum_combo_box_new (config, property_name,
                                                   minimum, maximum);

  if (combo)
    prefs_widget_add_aligned (combo, label, table, table_row, FALSE, group);

  return combo;
}

static GtkWidget *
prefs_boolean_combo_box_add (GObject      *config,
                             const gchar  *property_name,
                             const gchar  *true_text,
                             const gchar  *false_text,
                             const gchar  *label,
                             GtkTable     *table,
                             gint          table_row,
                             GtkSizeGroup *group)
{
  GtkWidget *combo = picman_prop_boolean_combo_box_new (config, property_name,
                                                      true_text, false_text);

  if (combo)
    prefs_widget_add_aligned (combo, label, table, table_row, FALSE, group);

  return combo;
}

#ifdef HAVE_ISO_CODES
static GtkWidget *
prefs_language_combo_box_add (GObject      *config,
                              const gchar  *property_name,
                              GtkBox       *vbox)
{
  GtkWidget *combo = picman_prop_language_combo_box_new (config, property_name);

  if (combo)
    {
      gtk_box_pack_start (vbox, combo, FALSE, FALSE, 0);
      gtk_widget_show (combo);
    }

  return combo;
}
#endif

static GtkWidget *
prefs_spin_button_add (GObject      *config,
                       const gchar  *property_name,
                       gdouble       step_increment,
                       gdouble       page_increment,
                       gint          digits,
                       const gchar  *label,
                       GtkTable     *table,
                       gint          table_row,
                       GtkSizeGroup *group)
{
  GtkWidget *button = picman_prop_spin_button_new (config, property_name,
                                                 step_increment,
                                                 page_increment,
                                                 digits);

  if (button)
    prefs_widget_add_aligned (button, label, table, table_row, TRUE, group);

  return button;
}

static GtkWidget *
prefs_memsize_entry_add (GObject      *config,
                         const gchar  *property_name,
                         const gchar  *label,
                         GtkTable     *table,
                         gint          table_row,
                         GtkSizeGroup *group)
{
  GtkWidget *entry = picman_prop_memsize_entry_new (config, property_name);

  if (entry)
    prefs_widget_add_aligned (entry, label, table, table_row, TRUE, group);

  return entry;
}

static void
prefs_canvas_padding_color_changed (GtkWidget *button,
                                    GtkWidget *combo)
{
  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (combo),
                                 PICMAN_CANVAS_PADDING_MODE_CUSTOM);
}

static void
prefs_display_options_frame_add (Picman         *picman,
                                 GObject      *object,
                                 const gchar  *label,
                                 GtkContainer *parent)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *checks_vbox;
  GtkWidget *table;
  GtkWidget *combo;
  GtkWidget *button;

  vbox = prefs_frame_new (label, parent, FALSE);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  checks_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_box_pack_start (GTK_BOX (hbox), checks_vbox, TRUE, TRUE, 0);
  gtk_widget_show (checks_vbox);

#ifndef GDK_WINDOWING_QUARTZ
  prefs_check_button_add (object, "show-menubar",
                          _("Show _menubar"),
                          GTK_BOX (checks_vbox));
#endif /* !GDK_WINDOWING_QUARTZ */
  prefs_check_button_add (object, "show-rulers",
                          _("Show _rulers"),
                          GTK_BOX (checks_vbox));
  prefs_check_button_add (object, "show-scrollbars",
                          _("Show scroll_bars"),
                          GTK_BOX (checks_vbox));
  prefs_check_button_add (object, "show-statusbar",
                          _("Show s_tatusbar"),
                          GTK_BOX (checks_vbox));

  checks_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_box_pack_start (GTK_BOX (hbox), checks_vbox, TRUE, TRUE, 0);
  gtk_widget_show (checks_vbox);

  prefs_check_button_add (object, "show-selection",
                          _("Show s_election"),
                          GTK_BOX (checks_vbox));
  prefs_check_button_add (object, "show-layer-boundary",
                          _("Show _layer boundary"),
                          GTK_BOX (checks_vbox));
  prefs_check_button_add (object, "show-guides",
                          _("Show _guides"),
                          GTK_BOX (checks_vbox));
  prefs_check_button_add (object, "show-grid",
                          _("Show gri_d"),
                          GTK_BOX (checks_vbox));

  table = prefs_table_new (2, GTK_CONTAINER (vbox));

  combo = prefs_enum_combo_box_add (object, "padding-mode", 0, 0,
                                    _("Canvas _padding mode:"),
                                    GTK_TABLE (table), 0,
                                    NULL);

  button = prefs_color_button_add (object, "padding-color",
                                   _("Custom p_adding color:"),
                                   _("Select Custom Canvas Padding Color"),
                                   GTK_TABLE (table), 1, NULL);
  picman_color_panel_set_context (PICMAN_COLOR_PANEL (button),
                                picman_get_user_context (picman));

  g_signal_connect (button, "color-changed",
                    G_CALLBACK (prefs_canvas_padding_color_changed),
                    combo);
}

static void
prefs_help_func (const gchar *help_id,
                 gpointer     help_data)
{
  GtkWidget *prefs_box;
  GtkWidget *notebook;
  GtkWidget *event_box;
  gint       page_num;

  prefs_box = g_object_get_data (G_OBJECT (help_data), "prefs-box");
  notebook  = picman_prefs_box_get_notebook (PICMAN_PREFS_BOX (prefs_box));
  page_num  = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  event_box = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);

  help_id = g_object_get_data (G_OBJECT (event_box), "picman-help-id");
  picman_standard_help_func (help_id, NULL);
}

static void
prefs_message (GtkMessageType  type,
               gboolean        destroy_with_parent,
               const gchar    *message)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (GTK_WINDOW (prefs_dialog),
                                   destroy_with_parent ?
                                   GTK_DIALOG_DESTROY_WITH_PARENT : 0,
                                   type, GTK_BUTTONS_OK,
                                   "%s", message);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);
  gtk_widget_show (dialog);
}

static gboolean
prefs_idle_unref (gpointer data)
{
  g_object_unref (data);

  return FALSE;
}

static GdkPixbuf *
prefs_get_pixbufs (Picman         *picman,
                   const gchar  *name,
                   GdkPixbuf   **small_pixbuf)
{
  GdkPixbuf *pixbuf = NULL;
  gchar     *basename;
  gchar     *filename;

  *small_pixbuf = NULL;

  basename = g_strconcat (name, ".png", NULL);
  filename = themes_get_theme_file (picman, "images", "preferences",
                                    basename, NULL);

  if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
    pixbuf = gdk_pixbuf_new_from_file (filename, NULL);

  g_free (filename);
  g_free (basename);

  basename = g_strconcat (name, "-22.png", NULL);
  filename = themes_get_theme_file (picman, "images", "preferences",
                                    basename, NULL);

  if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
    *small_pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
  else if (pixbuf)
    *small_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
                                             22, 22, GDK_INTERP_BILINEAR);

  g_free (filename);
  g_free (basename);

  if (pixbuf)
    g_idle_add (prefs_idle_unref, pixbuf);

  if (*small_pixbuf)
    g_idle_add (prefs_idle_unref, *small_pixbuf);

  return pixbuf;
}

static GtkWidget *
prefs_dialog_new (Picman       *picman,
                  PicmanConfig *config)
{
  GtkWidget         *dialog;
  GtkTreeIter        top_iter;
  GtkTreeIter        child_iter;

  GtkSizeGroup      *size_group = NULL;
  GdkPixbuf         *pixbuf;
  GdkPixbuf         *small_pixbuf = NULL;
  GtkWidget         *prefs_box;
  GtkWidget         *vbox;
  GtkWidget         *hbox;
  GtkWidget         *vbox2;
  GtkWidget         *button;
  GtkWidget         *button2;
  GtkWidget         *table;
  GtkWidget         *label;
  GtkWidget         *entry;
  GtkWidget         *calibrate_button;
  GSList            *group;
  GtkWidget         *editor;
  gint               i;

  GObject           *object;
  PicmanCoreConfig    *core_config;
  PicmanDisplayConfig *display_config;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_CONFIG (config), NULL);

  object         = G_OBJECT (config);
  core_config    = PICMAN_CORE_CONFIG (config);
  display_config = PICMAN_DISPLAY_CONFIG (config);

  dialog = picman_dialog_new (_("Preferences"), "picman-preferences",
                            NULL, 0,
                            prefs_help_func,
                            PICMAN_HELP_PREFS_DIALOG,

                            PICMAN_STOCK_RESET, RESPONSE_RESET,
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           RESPONSE_RESET,
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (prefs_response),
                    dialog);

  /* The prefs box */
  prefs_box = picman_prefs_box_new ();
  gtk_container_set_border_width (GTK_CONTAINER (prefs_box), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      prefs_box, TRUE, TRUE, 0);
  gtk_widget_show (prefs_box);

  g_object_set_data (G_OBJECT (dialog), "prefs-box", prefs_box);


  /*****************/
  /*  Environment  */
  /*****************/
  pixbuf = prefs_get_pixbufs (picman, "environment", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Environment"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_ENVIRONMENT,
                                  NULL,
                                  &top_iter);

  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  vbox2 = prefs_frame_new (_("Resource Consumption"),
                           GTK_CONTAINER (vbox), FALSE);

#ifdef ENABLE_MP
  table = prefs_table_new (5, GTK_CONTAINER (vbox2));
#else
  table = prefs_table_new (4, GTK_CONTAINER (vbox2));
#endif /* ENABLE_MP */

  prefs_spin_button_add (object, "undo-levels", 1.0, 5.0, 0,
                         _("Minimal number of _undo levels:"),
                         GTK_TABLE (table), 0, size_group);
  prefs_memsize_entry_add (object, "undo-size",
                           _("Maximum undo _memory:"),
                           GTK_TABLE (table), 1, size_group);
  prefs_memsize_entry_add (object, "tile-cache-size",
                           _("Tile cache _size:"),
                           GTK_TABLE (table), 2, size_group);
  prefs_memsize_entry_add (object, "max-new-image-size",
                           _("Maximum _new image size:"),
                           GTK_TABLE (table), 3, size_group);

#ifdef ENABLE_MP
  prefs_spin_button_add (object, "num-processors", 1.0, 4.0, 0,
                         _("Number of _processors to use:"),
                         GTK_TABLE (table), 4, size_group);
#endif /* ENABLE_MP */

  /*  Image Thumbnails  */
  vbox2 = prefs_frame_new (_("Image Thumbnails"), GTK_CONTAINER (vbox), FALSE);

  table = prefs_table_new (2, GTK_CONTAINER (vbox2));

  prefs_enum_combo_box_add (object, "thumbnail-size", 0, 0,
                            _("Size of _thumbnails:"),
                            GTK_TABLE (table), 0, size_group);

  prefs_memsize_entry_add (object, "thumbnail-filesize-limit",
                           _("Maximum _filesize for thumbnailing:"),
                           GTK_TABLE (table), 1, size_group);

  g_object_unref (size_group);
  size_group = NULL;

  /*  Document History  */
  vbox2 = prefs_frame_new (_("Document History"), GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add (object, "save-document-history",
                          _("Keep record of used files in the Recent Documents list"),
                          GTK_BOX (vbox2));


  /***************/
  /*  Interface  */
  /***************/
  pixbuf = prefs_get_pixbufs (picman, "interface", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("User Interface"),
                                  pixbuf,
                                  _("Interface"),
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_INTERFACE,
                                  NULL,
                                  &top_iter);

  /*  Language  */

  /*  Only add the language entry if the iso-codes package is available.  */
#ifdef HAVE_ISO_CODES
  vbox2 = prefs_frame_new (_("Language"), GTK_CONTAINER (vbox), FALSE);

  prefs_language_combo_box_add (object, "language", GTK_BOX (vbox2));
#endif

  /*  Previews  */
  vbox2 = prefs_frame_new (_("Previews"), GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add (object, "layer-previews",
                          _("_Enable layer & channel previews"),
                          GTK_BOX (vbox2));

  table = prefs_table_new (3, GTK_CONTAINER (vbox2));

  prefs_enum_combo_box_add (object, "layer-preview-size", 0, 0,
                            _("_Default layer & channel preview size:"),
                            GTK_TABLE (table), 0, size_group);
  prefs_enum_combo_box_add (object, "navigation-preview-size", 0, 0,
                            _("Na_vigation preview size:"),
                            GTK_TABLE (table), 1, size_group);

  /* Keyboard Shortcuts */
  vbox2 = prefs_frame_new (_("Keyboard Shortcuts"),
                           GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add (object, "can-change-accels",
                          _("_Use dynamic keyboard shortcuts"),
                          GTK_BOX (vbox2));

  button = prefs_button_add (GTK_STOCK_PREFERENCES,
                             _("Configure _Keyboard Shortcuts..."),
                             GTK_BOX (vbox2));
  g_signal_connect (button, "clicked",
                    G_CALLBACK (prefs_keyboard_shortcuts_dialog),
                    picman);

  prefs_check_button_add (object, "save-accels",
                          _("_Save keyboard shortcuts on exit"),
                          GTK_BOX (vbox2));

  button = prefs_button_add (GTK_STOCK_SAVE,
                             _("Save Keyboard Shortcuts _Now"),
                             GTK_BOX (vbox2));
  g_signal_connect (button, "clicked",
                    G_CALLBACK (prefs_menus_save_callback),
                    picman);

  button2 = prefs_button_add (PICMAN_STOCK_RESET,
                              _("_Reset Keyboard Shortcuts to Default Values"),
                              GTK_BOX (vbox2));
  g_signal_connect (button2, "clicked",
                    G_CALLBACK (prefs_menus_clear_callback),
                    picman);

  g_object_set_data (G_OBJECT (button), "clear-button", button2);

  button = prefs_button_add (GTK_STOCK_CLEAR,
                             _("Remove _All Keyboard Shortcuts"),
                             GTK_BOX (vbox2));
  g_signal_connect (button, "clicked",
                    G_CALLBACK (prefs_menus_remove_callback),
                    picman);


  /***********/
  /*  Theme  */
  /***********/
  pixbuf = prefs_get_pixbufs (picman, "theme", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Theme"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_THEME,
                                  NULL,
                                  &top_iter);

  vbox2 = prefs_frame_new (_("Select Theme"), GTK_CONTAINER (vbox), TRUE);

  {
    GtkWidget         *scrolled_win;
    GtkListStore      *list_store;
    GtkWidget         *view;
    GtkTreeSelection  *sel;
    gchar            **themes;
    gint               n_themes;
    gint               i;

    scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request (scrolled_win, -1, 80);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_win),
                                         GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox2), scrolled_win, TRUE, TRUE, 0);
    gtk_widget_show (scrolled_win);

    list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);

    view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
    gtk_container_add (GTK_CONTAINER (scrolled_win), view);
    gtk_widget_show (view);

    g_object_unref (list_store);

    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view), 0,
                                                 _("Theme"),
                                                 gtk_cell_renderer_text_new (),
                                                 "text", 0,
                                                 NULL);
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view), 1,
                                                 _("Folder"),
                                                 gtk_cell_renderer_text_new (),
                                                 "text", 1,
                                                 NULL);

    sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

    themes = themes_list_themes (picman, &n_themes);

    for (i = 0; i < n_themes; i++)
      {
        GtkTreeIter iter;

        gtk_list_store_append (list_store, &iter);
        gtk_list_store_set (list_store, &iter,
                            0, themes[i],
                            1, themes_get_theme_dir (picman, themes[i]),
                            -1);

        if (PICMAN_GUI_CONFIG (object)->theme &&
            ! strcmp (PICMAN_GUI_CONFIG (object)->theme, themes[i]))
          {
            GtkTreePath *path;

            path = gtk_tree_model_get_path (GTK_TREE_MODEL (list_store), &iter);

            gtk_tree_view_set_cursor (GTK_TREE_VIEW (view), path, NULL, FALSE);
            gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (view), path,
                                          NULL, FALSE, 0.0, 0.0);

            gtk_tree_path_free (path);
          }
      }

    if (themes)
      g_strfreev (themes);

    g_signal_connect (sel, "changed",
                      G_CALLBACK (prefs_theme_select_callback),
                      picman);
  }

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  button = prefs_button_add (GTK_STOCK_REFRESH,
                             _("Reload C_urrent Theme"),
                             GTK_BOX (hbox));
  g_signal_connect (button, "clicked",
                    G_CALLBACK (prefs_theme_reload_callback),
                    picman);


  /*****************/
  /*  Help System  */
  /*****************/
  pixbuf = prefs_get_pixbufs (picman, "help-system", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Help System"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_HELP,
                                  NULL,
                                  &top_iter);

  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  /*  General  */
  vbox2 = prefs_frame_new (_("General"), GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add (object, "show-tooltips",
                          _("Show _tooltips"),
                          GTK_BOX (vbox2));
  prefs_check_button_add (object, "show-help-button",
                          _("Show help _buttons"),
                          GTK_BOX (vbox2));

  {
    GtkWidget   *combo;
    GtkWidget   *hbox;
    GtkWidget   *image;
    GtkWidget   *label;
    const gchar *icon;
    const gchar *text;

    table = prefs_table_new (2, GTK_CONTAINER (vbox2));
    combo = prefs_boolean_combo_box_add (object, "user-manual-online",
                                         _("Use the online version"),
                                         _("Use a locally installed copy"),
                                         _("User manual:"),
                                         GTK_TABLE (table), 0, size_group);
    picman_help_set_help_data (combo, NULL, NULL);

    if (picman_help_user_manual_is_installed (picman))
      {
        icon = PICMAN_STOCK_INFO;
        text = _("There's a local installation of the user manual.");
      }
    else
      {
        icon = PICMAN_STOCK_WARNING;
        text = _("The user manual is not installed locally.");
      }

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 2, 1, 2);
    gtk_widget_show (hbox);

    image = gtk_image_new_from_stock (icon, GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
    gtk_widget_show (image);

    label = gtk_label_new (text);
    picman_label_set_attributes (GTK_LABEL (label),
                               PANGO_ATTR_STYLE, PANGO_STYLE_ITALIC,
                               -1);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
    gtk_widget_show (label);
  }

  /*  Help Browser  */
  vbox2 = prefs_frame_new (_("Help Browser"), GTK_CONTAINER (vbox), FALSE);
  table = prefs_table_new (1, GTK_CONTAINER (vbox2));

  prefs_enum_combo_box_add (object, "help-browser", 0, 0,
                            _("H_elp browser to use:"),
                            GTK_TABLE (table), 0, size_group);

  g_object_unref (size_group);
  size_group = NULL;


  /******************/
  /*  Tool Options  */
  /******************/
  pixbuf = prefs_get_pixbufs (picman, "tool-options", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  C_("preferences", "Tool Options"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_TOOL_OPTIONS,
                                  NULL,
                                  &top_iter);

  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  /*  General  */
  vbox2 = prefs_frame_new (_("General"), GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add (object, "save-tool-options",
                          _("_Save tool options on exit"),
                          GTK_BOX (vbox2));

  button = prefs_button_add (GTK_STOCK_SAVE,
                             _("Save Tool Options _Now"),
                             GTK_BOX (vbox2));
  g_signal_connect (button, "clicked",
                    G_CALLBACK (prefs_tool_options_save_callback),
                    picman);

  button2 = prefs_button_add (PICMAN_STOCK_RESET,
                              _("_Reset Saved Tool Options to "
                                "Default Values"),
                              GTK_BOX (vbox2));
  g_signal_connect (button2, "clicked",
                    G_CALLBACK (prefs_tool_options_clear_callback),
                    picman);

  g_object_set_data (G_OBJECT (button), "clear-button", button2);

  /*  Snapping Distance  */
  vbox2 = prefs_frame_new (_("Guide & Grid Snapping"),
                           GTK_CONTAINER (vbox), FALSE);
  table = prefs_table_new (1, GTK_CONTAINER (vbox2));

  prefs_spin_button_add (object, "snap-distance", 1.0, 5.0, 0,
                         _("_Snap distance:"),
                         GTK_TABLE (table), 0, size_group);

  /*  Scaling  */
  vbox2 = prefs_frame_new (_("Scaling"), GTK_CONTAINER (vbox), FALSE);
  table = prefs_table_new (1, GTK_CONTAINER (vbox2));

  prefs_enum_combo_box_add (object, "interpolation-type", 0, 0,
                            _("Default _interpolation:"),
                            GTK_TABLE (table), 0, size_group);

  g_object_unref (size_group);
  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  /*  Global Brush, Pattern, ...  */
  vbox2 = prefs_frame_new (_("Paint Options Shared Between Tools"),
                           GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add_with_icon (object, "global-brush",
                                    _("_Brush"),    PICMAN_STOCK_BRUSH,
                                    GTK_BOX (vbox2), size_group);
  prefs_check_button_add_with_icon (object, "global-dynamics",
                                    _("_Dynamics"), PICMAN_STOCK_DYNAMICS,
                                    GTK_BOX (vbox2), size_group);
  prefs_check_button_add_with_icon (object, "global-pattern",
                                    _("_Pattern"),  PICMAN_STOCK_PATTERN,
                                    GTK_BOX (vbox2), size_group);
  prefs_check_button_add_with_icon (object, "global-gradient",
                                    _("_Gradient"), PICMAN_STOCK_GRADIENT,
                                    GTK_BOX (vbox2), size_group);

  /*  Move Tool */
  vbox2 = prefs_frame_new (_("Move Tool"),
                           GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add_with_icon (object, "move-tool-changes-active",
                                    _("Set layer or path as active"),
                                    PICMAN_STOCK_TOOL_MOVE,
                                    GTK_BOX (vbox2), size_group);

  g_object_unref (size_group);
  size_group = NULL;


  /*************/
  /*  Toolbox  */
  /*************/
  pixbuf = prefs_get_pixbufs (picman, "toolbox", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Toolbox"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_TOOLBOX,
                                  NULL,
                                  &top_iter);

  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  /*  Appearance  */
  vbox2 = prefs_frame_new (_("Appearance"),
                           GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add_with_icon (object, "toolbox-color-area",
                                    _("Show _foreground & background color"),
                                    PICMAN_STOCK_DEFAULT_COLORS,
                                    GTK_BOX (vbox2), size_group);
  prefs_check_button_add_with_icon (object, "toolbox-foo-area",
                                    _("Show active _brush, pattern & gradient"),
                                    PICMAN_STOCK_BRUSH,
                                    GTK_BOX (vbox2), size_group);
  prefs_check_button_add_with_icon (object, "toolbox-image-area",
                                    _("Show active _image"),
                                    PICMAN_STOCK_IMAGE,
                                    GTK_BOX (vbox2), size_group);

  g_object_unref (size_group);
  size_group = NULL;

  /* Tool Editor */
  vbox2 = prefs_frame_new (_("Tools configuration"),
                           GTK_CONTAINER (vbox), TRUE);
  tool_editor = picman_tool_editor_new (picman->tool_info_list, picman->user_context,
                                      picman_tools_get_default_order (picman),
                                      PICMAN_VIEW_SIZE_SMALL, 1);

  gtk_box_pack_start (GTK_BOX (vbox2), tool_editor, TRUE, TRUE, 0);
  gtk_widget_show (tool_editor);


  /***********************/
  /*  Default New Image  */
  /***********************/
  pixbuf = prefs_get_pixbufs (picman, "new-image", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Default New Image"),
                                  pixbuf,
                                  _("Default Image"),
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_NEW_IMAGE,
                                  NULL,
                                  &top_iter);

  table = prefs_table_new (1, GTK_CONTAINER (vbox));

  {
    GtkWidget *combo;

    combo = picman_container_combo_box_new (picman->templates,
                                          picman_get_user_context (picman),
                                          16, 0);
    picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                               _("_Template:"),  0.0, 0.5,
                               combo, 1, FALSE);

    picman_container_view_select_item (PICMAN_CONTAINER_VIEW (combo), NULL);

    g_signal_connect (combo, "select-item",
                      G_CALLBACK (prefs_template_select_callback),
                      core_config->default_image);
  }

  editor = picman_template_editor_new (core_config->default_image, picman, FALSE);
  picman_template_editor_show_advanced (PICMAN_TEMPLATE_EDITOR (editor), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), editor, FALSE, FALSE, 0);
  gtk_widget_show (editor);

  /*  Quick Mask Color */
  vbox2 = prefs_frame_new (_("Quick Mask"), GTK_CONTAINER (vbox), FALSE);
  table = prefs_table_new (1, GTK_CONTAINER (vbox2));
  button = picman_prop_color_button_new (object, "quick-mask-color",
                                       _("Set the default Quick Mask color"),
                                       COLOR_BUTTON_WIDTH,
                                       COLOR_BUTTON_HEIGHT,
                                       PICMAN_COLOR_AREA_SMALL_CHECKS);
  picman_color_panel_set_context (PICMAN_COLOR_PANEL (button),
                                picman_get_user_context (picman));
  prefs_widget_add_aligned (button, _("Quick Mask color:"),
                            GTK_TABLE (table), 0, TRUE, NULL);



  /******************/
  /*  Default Grid  */
  /******************/
  pixbuf = prefs_get_pixbufs (picman, "default-grid", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Default Image Grid"),
                                  pixbuf,
                                  _("Default Grid"),
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_DEFAULT_GRID,
                                  NULL,
                                  &top_iter);

  /*  Grid  */
  editor = picman_grid_editor_new (core_config->default_grid,
                                 picman_get_user_context (picman),
                                 picman_template_get_resolution_x (core_config->default_image),
                                 picman_template_get_resolution_y (core_config->default_image));
  gtk_box_pack_start (GTK_BOX (vbox), editor, TRUE, TRUE, 0);
  gtk_widget_show (editor);


  /*******************/
  /*  Image Windows  */
  /*******************/
  pixbuf = prefs_get_pixbufs (picman, "image-windows", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Image Windows"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_IMAGE_WINDOW,
                                  NULL,
                                  &top_iter);

  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  /*  General  */
  vbox2 = prefs_frame_new (_("General"), GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add (object, "default-dot-for-dot",
                          _("Use \"_Dot for dot\" by default"),
                          GTK_BOX (vbox2));

  table = prefs_table_new (1, GTK_CONTAINER (vbox2));

  prefs_spin_button_add (object, "marching-ants-speed", 1.0, 10.0, 0,
                         _("Marching _ants speed:"),
                         GTK_TABLE (table), 0, size_group);

  /*  Zoom & Resize Behavior  */
  vbox2 = prefs_frame_new (_("Zoom & Resize Behavior"),
                           GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add (object, "resize-windows-on-zoom",
                          _("Resize window on _zoom"),
                          GTK_BOX (vbox2));
  prefs_check_button_add (object, "resize-windows-on-resize",
                          _("Resize window on image _size change"),
                          GTK_BOX (vbox2));

  table = prefs_table_new (1, GTK_CONTAINER (vbox2));

  prefs_boolean_combo_box_add (object, "initial-zoom-to-fit",
                               _("Fit to window"),
                               "1:1",
                               _("Initial zoom _ratio:"),
                               GTK_TABLE (table), 0, size_group);

  /*  Space Bar  */
  vbox2 = prefs_frame_new (_("Space Bar"),
                           GTK_CONTAINER (vbox), FALSE);

  table = prefs_table_new (1, GTK_CONTAINER (vbox2));

  prefs_enum_combo_box_add (object, "space-bar-action", 0, 0,
                            _("_While space bar is pressed:"),
                            GTK_TABLE (table), 0, size_group);

  /*  Mouse Pointers  */
  vbox2 = prefs_frame_new (_("Mouse Pointers"),
                           GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add (object, "show-brush-outline",
                          _("Show _brush outline"),
                          GTK_BOX (vbox2));
  prefs_check_button_add (object, "show-paint-tool-cursor",
                          _("Show pointer for brush _tools"),
                          GTK_BOX (vbox2));

  table = prefs_table_new (2, GTK_CONTAINER (vbox2));

  prefs_enum_combo_box_add (object, "cursor-mode", 0, 0,
                            _("Pointer _mode:"),
                            GTK_TABLE (table), 0, size_group);
  prefs_enum_combo_box_add (object, "cursor-handedness", 0, 0,
                            _("Pointer _handedness:"),
                            GTK_TABLE (table), 1, NULL);

  g_object_unref (size_group);
  size_group = NULL;


  /********************************/
  /*  Image Windows / Appearance  */
  /********************************/
  pixbuf = prefs_get_pixbufs (picman, "image-windows", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Image Window Appearance"),
                                  pixbuf,
                                  _("Appearance"),
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_IMAGE_WINDOW_APPEARANCE,
                                  &top_iter,
                                  &child_iter);

  prefs_display_options_frame_add (picman,
                                   G_OBJECT (display_config->default_view),
                                   _("Default Appearance in Normal Mode"),
                                   GTK_CONTAINER (vbox));

  prefs_display_options_frame_add (picman,
                                   G_OBJECT (display_config->default_fullscreen_view),
                                   _("Default Appearance in Fullscreen Mode"),
                                   GTK_CONTAINER (vbox));


  /****************************************************/
  /*  Image Windows / Image Title & Statusbar Format  */
  /****************************************************/
  pixbuf = prefs_get_pixbufs (picman, "image-title", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Image Title & Statusbar Format"),
                                  pixbuf,
                                  _("Title & Status"),
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_IMAGE_WINDOW_TITLE,
                                  &top_iter,
                                  &child_iter);

  {
    const gchar *format_strings[] =
    {
      NULL,
      NULL,
      "%f-%p.%i (%t) %z%%",
      "%f-%p.%i (%t) %d:%s",
      "%f-%p.%i (%t) %wx%h"
    };

    const gchar *format_names[] =
    {
      N_("Current format"),
      N_("Default format"),
      N_("Show zoom percentage"),
      N_("Show zoom ratio"),
      N_("Show image size")
    };

    struct
    {
      gchar       *current_setting;
      const gchar *default_setting;
      const gchar *title;
      const gchar *property_name;
    }
    formats[] =
    {
      { NULL, PICMAN_CONFIG_DEFAULT_IMAGE_TITLE_FORMAT,
        N_("Image Title Format"),     "image-title-format"  },
      { NULL, PICMAN_CONFIG_DEFAULT_IMAGE_STATUS_FORMAT,
        N_("Image Statusbar Format"), "image-status-format" }
    };

    gint format;

    g_assert (G_N_ELEMENTS (format_strings) == G_N_ELEMENTS (format_names));

    formats[0].current_setting = display_config->image_title_format;
    formats[1].current_setting = display_config->image_status_format;

    for (format = 0; format < G_N_ELEMENTS (formats); format++)
      {
        GtkWidget        *scrolled_win;
        GtkListStore     *list_store;
        GtkWidget        *view;
        GtkTreeSelection *sel;
        gint              i;

        format_strings[0] = formats[format].current_setting;
        format_strings[1] = formats[format].default_setting;

        vbox2 = prefs_frame_new (gettext (formats[format].title),
                                 GTK_CONTAINER (vbox), TRUE);

        entry = picman_prop_entry_new (object, formats[format].property_name, 0);
        gtk_box_pack_start (GTK_BOX (vbox2), entry, FALSE, FALSE, 0);
        gtk_widget_show (entry);

        scrolled_win = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_win),
                                             GTK_SHADOW_IN);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                        GTK_POLICY_NEVER,
                                        GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start (GTK_BOX (vbox2), scrolled_win, TRUE, TRUE, 0);
        gtk_widget_show (scrolled_win);

        list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);

        view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
        gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), FALSE);
        gtk_container_add (GTK_CONTAINER (scrolled_win), view);
        gtk_widget_show (view);

        g_object_unref (list_store);

        gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view), 0,
                                                     NULL,
                                                     gtk_cell_renderer_text_new (),
                                                     "text", 0,
                                                     NULL);
        gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view), 1,
                                                     NULL,
                                                     gtk_cell_renderer_text_new (),
                                                     "text", 1,
                                                     NULL);

        sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

        for (i = 0; i < G_N_ELEMENTS (format_strings); i++)
          {
            GtkTreeIter iter;

            gtk_list_store_append (list_store, &iter);
            gtk_list_store_set (list_store, &iter,
                                0, gettext (format_names[i]),
                                1, format_strings[i],
                                -1);

            if (i == 0)
              gtk_tree_selection_select_iter (sel, &iter);
          }

        g_signal_connect (sel, "changed",
                          G_CALLBACK (prefs_format_string_select_callback),
                          entry);
      }
  }


  /*************/
  /*  Display  */
  /*************/
  pixbuf = prefs_get_pixbufs (picman, "display", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Display"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_DISPLAY,
                                  NULL,
                                  &top_iter);

  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  /*  Transparency  */
  vbox2 = prefs_frame_new (_("Transparency"), GTK_CONTAINER (vbox), FALSE);
  table = prefs_table_new (2, GTK_CONTAINER (vbox2));

  prefs_enum_combo_box_add (object, "transparency-type", 0, 0,
                            _("_Check style:"),
                            GTK_TABLE (table), 0, size_group);
  prefs_enum_combo_box_add (object, "transparency-size", 0, 0,
                            _("Check _size:"),
                            GTK_TABLE (table), 1, size_group);

  vbox2 = prefs_frame_new (_("Monitor Resolution"),
                           GTK_CONTAINER (vbox), FALSE);

  {
    gchar *pixels_per_unit = g_strconcat (_("Pixels"), "/%s", NULL);

    entry = picman_prop_coordinates_new (object,
                                       "monitor-xresolution",
                                       "monitor-yresolution",
                                       NULL,
                                       pixels_per_unit,
                                       PICMAN_SIZE_ENTRY_UPDATE_RESOLUTION,
                                       0.0, 0.0,
                                       TRUE);

    g_free (pixels_per_unit);
  }

  gtk_table_set_col_spacings (GTK_TABLE (entry), 2);
  gtk_table_set_row_spacings (GTK_TABLE (entry), 2);

  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (entry),
                                _("Horizontal"), 0, 1, 0.0);
  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (entry),
                                _("Vertical"), 0, 2, 0.0);
  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (entry),
                                _("ppi"), 1, 4, 0.0);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 24);
  gtk_widget_show (entry);
  gtk_widget_set_sensitive (entry, ! display_config->monitor_res_from_gdk);

  group = NULL;

  {
    gdouble  xres, yres;
    gchar   *str;

    picman_get_screen_resolution (NULL, &xres, &yres);

    str = g_strdup_printf (_("_Detect automatically (currently %d × %d ppi)"),
                           ROUND (xres), ROUND (yres));

    button = gtk_radio_button_new_with_mnemonic (group, str);

    g_free (str);
  }

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_object_set_data (G_OBJECT (button), "monitor_resolution_sizeentry", entry);

  g_signal_connect (button, "toggled",
                    G_CALLBACK (prefs_resolution_source_callback),
                    config);

  button = gtk_radio_button_new_with_mnemonic (group, _("_Enter manually"));
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  if (! display_config->monitor_res_from_gdk)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  calibrate_button = gtk_button_new_with_mnemonic (_("C_alibrate..."));
  label = gtk_bin_get_child (GTK_BIN (calibrate_button));
  gtk_misc_set_padding (GTK_MISC (label), 4, 0);
  gtk_box_pack_start (GTK_BOX (hbox), calibrate_button, FALSE, FALSE, 0);
  gtk_widget_show (calibrate_button);
  gtk_widget_set_sensitive (calibrate_button,
                            ! display_config->monitor_res_from_gdk);

  g_object_bind_property (button, "active",
                          entry,  "sensitive",
                          G_BINDING_SYNC_CREATE);
  g_object_bind_property (button,           "active",
                          calibrate_button, "sensitive",
                          G_BINDING_SYNC_CREATE);

  g_signal_connect (calibrate_button, "clicked",
                    G_CALLBACK (prefs_resolution_calibrate_callback),
                    entry);

  g_object_unref (size_group);
  size_group = NULL;


  /**********************/
  /*  Color Management  */
  /**********************/
  pixbuf = prefs_get_pixbufs (picman, "color-management", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Color Management"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_COLOR_MANAGEMENT,
                                  NULL,
                                  &top_iter);

  table = prefs_table_new (10, GTK_CONTAINER (vbox));

  {
    static const struct
    {
      const gchar *label;
      const gchar *fs_label;
      const gchar *property_name;
    }
    profiles[] =
    {
      { N_("_RGB profile:"),
        N_("Select RGB Color Profile"),     "rgb-profile"     },
      { N_("_CMYK profile:"),
        N_("Select CMYK Color Profile"),    "cmyk-profile"    },
      { N_("_Monitor profile:"),
        N_("Select Monitor Color Profile"), "display-profile" },
      { N_("_Print simulation profile:"),
        N_("Select Printer Color Profile"), "printer-profile" }
    };

    GObject      *color_config;
    GtkListStore *store;
    gchar        *filename;
    gint          row = 0;

    g_object_get (object, "color-management", &color_config, NULL);

    prefs_enum_combo_box_add (color_config, "mode", 0, 0,
                              _("_Mode of operation:"),
                              GTK_TABLE (table), row++, NULL);
    gtk_table_set_row_spacing (GTK_TABLE (table), row - 1, 12);

    filename = picman_personal_rc_file ("profilerc");
    store = picman_color_profile_store_new (filename);
    g_free (filename);

    picman_color_profile_store_add (PICMAN_COLOR_PROFILE_STORE (store), NULL, NULL);

    for (i = 0; i < G_N_ELEMENTS (profiles); i++)
      {
        button = prefs_profile_combo_box_new (picman,
                                              color_config,
                                              store,
                                              gettext (profiles[i].fs_label),
                                              profiles[i].property_name);

        picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                                   gettext (profiles[i].label), 0.0, 0.5,
                                   button, 1, FALSE);


        if (i == 2) /* display profile */
          {
            gtk_table_set_row_spacing (GTK_TABLE (table), row - 2, 12);

            button =
              picman_prop_check_button_new (color_config,
                                          "display-profile-from-gdk",
                                          _("_Try to use the system monitor "
                                            "profile"));

            gtk_table_attach_defaults (GTK_TABLE (table),
                                       button, 1, 2, row, row + 1);
            gtk_widget_show (button);
            row++;

            prefs_enum_combo_box_add (color_config,
                                      "display-rendering-intent", 0, 0,
                                      _("_Display rendering intent:"),
                                      GTK_TABLE (table), row++, NULL);
            gtk_table_set_row_spacing (GTK_TABLE (table), row - 1, 12);
          }

        if (i == 3) /* printer profile */
          {
            prefs_enum_combo_box_add (color_config,
                                      "simulation-rendering-intent", 0, 0,
                                      _("_Softproof rendering intent:"),
                                      GTK_TABLE (table), row++, NULL);
          }
      }

    g_object_unref (store);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 2, row, row + 1);
    gtk_widget_show (hbox);
    row++;

    button = picman_prop_check_button_new (color_config, "simulation-gamut-check",
                                         _("Mark out of gamut colors"));
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    button = picman_prop_color_button_new (color_config, "out-of-gamut-color",
                                         _("Select Warning Color"),
                                         COLOR_BUTTON_WIDTH,
                                         COLOR_BUTTON_HEIGHT,
                                         PICMAN_COLOR_AREA_FLAT);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    gtk_widget_show (button);

    picman_color_panel_set_context (PICMAN_COLOR_PANEL (button),
                                  picman_get_user_context (picman));

    gtk_table_set_row_spacing (GTK_TABLE (table), row - 1, 12);

    button = prefs_enum_combo_box_add (object, "color-profile-policy", 0, 0,
                                       _("File Open behaviour:"),
                                       GTK_TABLE (table), row++, NULL);

    g_object_unref (color_config);
  }


  /*******************/
  /*  Input Devices  */
  /*******************/
  pixbuf = prefs_get_pixbufs (picman, "input-devices", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Input Devices"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_INPUT_DEVICES,
                                  NULL,
                                  &top_iter);

  /*  Extended Input Devices  */
  vbox2 = prefs_frame_new (_("Extended Input Devices"),
                           GTK_CONTAINER (vbox), FALSE);

  button = prefs_button_add (GTK_STOCK_PREFERENCES,
                             _("Configure E_xtended Input Devices..."),
                             GTK_BOX (vbox2));
  g_signal_connect (button, "clicked",
                    G_CALLBACK (prefs_input_devices_dialog),
                    picman);

  prefs_check_button_add (object, "save-device-status",
                          _("_Save input device settings on exit"),
                          GTK_BOX (vbox2));

  button = prefs_button_add (GTK_STOCK_SAVE,
                             _("Save Input Device Settings _Now"),
                             GTK_BOX (vbox2));
  g_signal_connect (button, "clicked",
                    G_CALLBACK (prefs_devices_save_callback),
                    picman);

  button2 = prefs_button_add (PICMAN_STOCK_RESET,
                              _("_Reset Saved Input Device Settings to "
                                "Default Values"),
                              GTK_BOX (vbox2));
  g_signal_connect (button2, "clicked",
                    G_CALLBACK (prefs_devices_clear_callback),
                    picman);

  g_object_set_data (G_OBJECT (button), "clear-button", button2);


  /****************************/
  /*  Additional Controllers  */
  /****************************/
  pixbuf = prefs_get_pixbufs (picman, "controllers", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Additional Input Controllers"),
                                  pixbuf,
                                  _("Input Controllers"),
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_INPUT_CONTROLLERS,
                                  &top_iter,
                                  &child_iter);

  vbox2 = picman_controller_list_new (picman);
  gtk_box_pack_start (GTK_BOX (vbox), vbox2, TRUE, TRUE, 0);
  gtk_widget_show (vbox2);


  /***********************/
  /*  Window Management  */
  /***********************/
  pixbuf = prefs_get_pixbufs (picman, "window-management", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Window Management"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_WINDOW_MANAGEMENT,
                                  NULL,
                                  &top_iter);

  vbox2 = prefs_frame_new (_("Window Manager Hints"),
                           GTK_CONTAINER (vbox), FALSE);

  table = prefs_table_new (1, GTK_CONTAINER (vbox2));

  prefs_enum_combo_box_add (object, "dock-window-hint", 0, 0,
                            _("Hint for _docks and toolbox:"),
                            GTK_TABLE (table), 1, size_group);

  vbox2 = prefs_frame_new (_("Focus"),
                           GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add (object, "activate-on-focus",
                          _("Activate the _focused image"),
                          GTK_BOX (vbox2));

  /* Window Positions */
  vbox2 = prefs_frame_new (_("Window Positions"), GTK_CONTAINER (vbox), FALSE);

  prefs_check_button_add (object, "save-session-info",
                          _("_Save window positions on exit"),
                          GTK_BOX (vbox2));

  button = prefs_button_add (GTK_STOCK_SAVE,
                             _("Save Window Positions _Now"),
                             GTK_BOX (vbox2));
  g_signal_connect (button, "clicked",
                    G_CALLBACK (prefs_session_save_callback),
                    picman);

  button2 = prefs_button_add (PICMAN_STOCK_RESET,
                              _("_Reset Saved Window Positions to "
                                "Default Values"),
                              GTK_BOX (vbox2));
  g_signal_connect (button2, "clicked",
                    G_CALLBACK (prefs_session_clear_callback),
                    picman);

  g_object_set_data (G_OBJECT (button), "clear-button", button2);


  /*************/
  /*  Folders  */
  /*************/
  pixbuf = prefs_get_pixbufs (picman, "folders", &small_pixbuf);
  vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                  _("Folders"),
                                  pixbuf,
                                  NULL,
                                  small_pixbuf,
                                  PICMAN_HELP_PREFS_FOLDERS,
                                  NULL,
                                  &top_iter);

  {
    static const struct
    {
      const gchar *property_name;
      const gchar *label;
      const gchar *fs_label;
    }
    dirs[] =
    {
      {
        "temp-path",
        N_("Temporary folder:"),
        N_("Select Folder for Temporary Files")
      },
      {
        "swap-path",
        N_("Swap folder:"),
        N_("Select Swap Folder")
      }
    };

    table = prefs_table_new (G_N_ELEMENTS (dirs) + 1, GTK_CONTAINER (vbox));

    for (i = 0; i < G_N_ELEMENTS (dirs); i++)
      {
        button = picman_prop_file_chooser_button_new (object,
                                                    dirs[i].property_name,
                                                    gettext (dirs[i].fs_label),
                                                    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
        picman_table_attach_aligned (GTK_TABLE (table), 0, i,
                                   gettext (dirs[i].label), 0.0, 0.5,
                                   button, 1, FALSE);
      }
  }


  /*********************/
  /* Folders / <paths> */
  /*********************/
  {
    static const struct
    {
      const gchar *tree_label;
      const gchar *label;
      const gchar *icon;
      const gchar *help_data;
      const gchar *fs_label;
      const gchar *path_property_name;
      const gchar *writable_property_name;
    }
    paths[] =
    {
      { N_("Brushes"), N_("Brush Folders"), "folders-brushes",
        PICMAN_HELP_PREFS_FOLDERS_BRUSHES,
        N_("Select Brush Folders"),
        "brush-path", "brush-path-writable" },
      { N_("Dynamics"), N_("Dynamics Folders"), "folders-dynamics",
        PICMAN_HELP_PREFS_FOLDERS_DYNAMICS,
        N_("Select Dynamics Folders"),
        "dynamics-path", "dynamics-path-writable" },
      { N_("Patterns"), N_("Pattern Folders"), "folders-patterns",
        PICMAN_HELP_PREFS_FOLDERS_PATTERNS,
        N_("Select Pattern Folders"),
        "pattern-path", "pattern-path-writable" },
      { N_("Palettes"), N_("Palette Folders"), "folders-palettes",
        PICMAN_HELP_PREFS_FOLDERS_PALETTES,
        N_("Select Palette Folders"),
        "palette-path", "palette-path-writable" },
      { N_("Gradients"), N_("Gradient Folders"), "folders-gradients",
        PICMAN_HELP_PREFS_FOLDERS_GRADIENTS,
        N_("Select Gradient Folders"),
        "gradient-path", "gradient-path-writable" },
      { N_("Fonts"), N_("Font Folders"), "folders-fonts",
        PICMAN_HELP_PREFS_FOLDERS_FONTS,
        N_("Select Font Folders"),
        "font-path", NULL },
      { N_("Tool Presets"), N_("Tool Preset Folders"), "folders-tool-presets",
        PICMAN_HELP_PREFS_FOLDERS_TOOL_PRESETS,
        N_("Select Tool Preset Folders"),
        "tool-preset-path", "tool-preset-path-writable" },
      { N_("Plug-Ins"), N_("Plug-In Folders"), "folders-plug-ins",
        PICMAN_HELP_PREFS_FOLDERS_PLUG_INS,
        N_("Select Plug-In Folders"),
        "plug-in-path", NULL },
      { N_("Scripts"), N_("Script-Fu Folders"), "folders-scripts",
        PICMAN_HELP_PREFS_FOLDERS_SCRIPTS,
        N_("Select Script-Fu Folders"),
        "script-fu-path", NULL },
      { N_("Modules"), N_("Module Folders"), "folders-modules",
        PICMAN_HELP_PREFS_FOLDERS_MODULES,
        N_("Select Module Folders"),
        "module-path", NULL },
      { N_("Interpreters"), N_("Interpreter Folders"), "folders-interp",
        PICMAN_HELP_PREFS_FOLDERS_INTERPRETERS,
        N_("Select Interpreter Folders"),
        "interpreter-path", NULL },
      { N_("Environment"), N_("Environment Folders"), "folders-environ",
        PICMAN_HELP_PREFS_FOLDERS_ENVIRONMENT,
        N_("Select Environment Folders"),
        "environ-path", NULL },
      { N_("Themes"), N_("Theme Folders"), "folders-themes",
        PICMAN_HELP_PREFS_FOLDERS_THEMES,
        N_("Select Theme Folders"),
        "theme-path", NULL }
    };

    for (i = 0; i < G_N_ELEMENTS (paths); i++)
      {
        GtkWidget *editor;

        pixbuf = prefs_get_pixbufs (picman, paths[i].icon, &small_pixbuf);
        vbox = picman_prefs_box_add_page (PICMAN_PREFS_BOX (prefs_box),
                                        gettext (paths[i].label),
                                        pixbuf,
                                        gettext (paths[i].tree_label),
                                        small_pixbuf,
                                        paths[i].help_data,
                                        &top_iter,
                                        &child_iter);

        editor = picman_prop_path_editor_new (object,
                                            paths[i].path_property_name,
                                            paths[i].writable_property_name,
                                            gettext (paths[i].fs_label));
        gtk_box_pack_start (GTK_BOX (vbox), editor, TRUE, TRUE, 0);
        gtk_widget_show (editor);
      }
  }

  {
    GtkWidget    *tv;
    GtkTreeModel *model;
    GtkTreePath  *path;

    tv = picman_prefs_box_get_tree_view (PICMAN_PREFS_BOX (prefs_box));
    gtk_tree_view_expand_all (GTK_TREE_VIEW (tv));

    /*  collapse the Folders subtree */
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tv));
    path = gtk_tree_model_get_path (model, &top_iter);
    gtk_tree_view_collapse_row (GTK_TREE_VIEW (tv), path);
    gtk_tree_path_free (path);
  }

  return dialog;
}
