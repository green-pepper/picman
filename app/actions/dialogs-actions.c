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

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picman.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmantoolbox.h"

#include "display/picmanimagewindow.h"

#include "actions.h"
#include "dialogs-actions.h"
#include "dialogs-commands.h"

#include "picman-intl.h"


const PicmanStringActionEntry dialogs_dockable_actions[] =
{
  { "dialogs-toolbox", NULL,
    NC_("windows-action", "Tool_box"), "<primary>B",
    NULL /* set in dialogs_actions_update() */,
    "picman-toolbox",
    PICMAN_HELP_TOOLBOX },

  { "dialogs-tool-options", PICMAN_STOCK_TOOL_OPTIONS,
    NC_("dialogs-action", "Tool _Options"), NULL,
    NC_("dialogs-action", "Open the tool options dialog"),
    "picman-tool-options",
    PICMAN_HELP_TOOL_OPTIONS_DIALOG },

  { "dialogs-device-status", PICMAN_STOCK_DEVICE_STATUS,
    NC_("dialogs-action", "_Device Status"), NULL,
    NC_("dialogs-action", "Open the device status dialog"),
    "picman-device-status",
    PICMAN_HELP_DEVICE_STATUS_DIALOG },

  { "dialogs-layers", PICMAN_STOCK_LAYERS,
    NC_("dialogs-action", "_Layers"), "<primary>L",
    NC_("dialogs-action", "Open the layers dialog"),
    "picman-layer-list",
    PICMAN_HELP_LAYER_DIALOG },

  { "dialogs-channels", PICMAN_STOCK_CHANNELS,
    NC_("dialogs-action", "_Channels"), NULL,
    NC_("dialogs-action", "Open the channels dialog"),
    "picman-channel-list",
    PICMAN_HELP_CHANNEL_DIALOG },

  { "dialogs-vectors", PICMAN_STOCK_PATHS,
    NC_("dialogs-action", "_Paths"), NULL,
    NC_("dialogs-action", "Open the paths dialog"),
    "picman-vectors-list",
    PICMAN_HELP_PATH_DIALOG },

  { "dialogs-indexed-palette", PICMAN_STOCK_COLORMAP,
    NC_("dialogs-action", "Color_map"), NULL,
    NC_("dialogs-action", "Open the colormap dialog"),
    "picman-indexed-palette",
    PICMAN_HELP_INDEXED_PALETTE_DIALOG },

  { "dialogs-histogram", PICMAN_STOCK_HISTOGRAM,
    NC_("dialogs-action", "Histogra_m"), NULL,
    NC_("dialogs-action", "Open the histogram dialog"),
    "picman-histogram-editor",
    PICMAN_HELP_HISTOGRAM_DIALOG },

  { "dialogs-selection-editor", PICMAN_STOCK_TOOL_RECT_SELECT,
    NC_("dialogs-action", "_Selection Editor"), NULL,
    NC_("dialogs-action", "Open the selection editor"),
    "picman-selection-editor",
    PICMAN_HELP_SELECTION_DIALOG },

  { "dialogs-navigation", PICMAN_STOCK_NAVIGATION,
    NC_("dialogs-action", "Na_vigation"), NULL,
    NC_("dialogs-action", "Open the display navigation dialog"),
    "picman-navigation-view",
    PICMAN_HELP_NAVIGATION_DIALOG },

  { "dialogs-undo-history", PICMAN_STOCK_UNDO_HISTORY,
    NC_("dialogs-action", "Undo _History"), NULL,
    NC_("dialogs-action", "Open the undo history dialog"),
    "picman-undo-history",
    PICMAN_HELP_UNDO_DIALOG },

  { "dialogs-cursor", PICMAN_STOCK_CURSOR,
    NC_("dialogs-action", "Pointer"), NULL,
    NC_("dialogs-action", "Open the pointer information dialog"),
    "picman-cursor-view",
    PICMAN_HELP_POINTER_INFO_DIALOG },

  { "dialogs-sample-points", PICMAN_STOCK_SAMPLE_POINT,
    NC_("dialogs-action", "_Sample Points"), NULL,
    NC_("dialogs-action", "Open the sample points dialog"),
    "picman-sample-point-editor",
    PICMAN_HELP_SAMPLE_POINT_DIALOG },

  { "dialogs-colors", PICMAN_STOCK_DEFAULT_COLORS,
    NC_("dialogs-action", "Colo_rs"), NULL,
    NC_("dialogs-action", "Open the FG/BG color dialog"),
    "picman-color-editor",
    PICMAN_HELP_COLOR_DIALOG },

  { "dialogs-brushes", PICMAN_STOCK_BRUSH,
    NC_("dialogs-action", "_Brushes"), "<primary><shift>B",
    NC_("dialogs-action", "Open the brushes dialog"),
    "picman-brush-grid|picman-brush-list",
    PICMAN_HELP_BRUSH_DIALOG },

  { "dialogs-brush-editor", PICMAN_STOCK_BRUSH,
    NC_("dialogs-action", "Brush Editor"), NULL,
    NC_("dialogs-action", "Open the brush editor"),
    "picman-brush-editor",
    PICMAN_HELP_BRUSH_EDIT },

  { "dialogs-dynamics", PICMAN_STOCK_DYNAMICS,
    NC_("dialogs-action", "Paint Dynamics"), NULL,
    NC_("dialogs-action", "Open paint dynamics dialog"),
    "picman-dynamics-list",
    PICMAN_HELP_DYNAMICS_DIALOG },

  { "dialogs-dynamics-editor", PICMAN_STOCK_DYNAMICS,
    NC_("dialogs-action", "Paint Dynamics Editor"), NULL,
    NC_("dialogs-action", "Open the paint dynamics editor"),
    "picman-dynamics-editor",
    PICMAN_HELP_DYNAMICS_EDITOR_DIALOG },

  { "dialogs-patterns", PICMAN_STOCK_PATTERN,
    NC_("dialogs-action", "P_atterns"), "<primary><shift>P",
    NC_("dialogs-action", "Open the patterns dialog"),
    "picman-pattern-grid|picman-pattern-list",
    PICMAN_HELP_PATTERN_DIALOG },

  { "dialogs-gradients", PICMAN_STOCK_GRADIENT,
    NC_("dialogs-action", "_Gradients"), "<primary>G",
    NC_("dialogs-action", "Open the gradients dialog"),
    "picman-gradient-list|picman-gradient-grid",
    PICMAN_HELP_GRADIENT_DIALOG },

  { "dialogs-gradient-editor", PICMAN_STOCK_GRADIENT,
    NC_("dialogs-action", "Gradient Editor"), NULL,
    NC_("dialogs-action", "Open the gradient editor"),
    "picman-gradient-editor",
    PICMAN_HELP_GRADIENT_EDIT },

  { "dialogs-palettes", PICMAN_STOCK_PALETTE,
    NC_("dialogs-action", "Pal_ettes"), NULL,
    NC_("dialogs-action", "Open the palettes dialog"),
    "picman-palette-list|picman-palette-grid",
    PICMAN_HELP_PALETTE_DIALOG },

  { "dialogs-palette-editor", PICMAN_STOCK_PALETTE,
    NC_("dialogs-action", "Palette Editor"), NULL,
    NC_("dialogs-action", "Open the palette editor"),
    "picman-palette-editor",
    PICMAN_HELP_PALETTE_EDIT },

  { "dialogs-tool-presets", PICMAN_STOCK_TOOL_PRESET,
    NC_("dialogs-action", "Tool presets"), NULL,
    NC_("dialogs-action", "Open tool presets dialog"),
    "picman-tool-preset-list",
    PICMAN_HELP_TOOL_PRESET_DIALOG },

  { "dialogs-fonts", PICMAN_STOCK_FONT,
    NC_("dialogs-action", "_Fonts"), NULL,
    NC_("dialogs-action", "Open the fonts dialog"),
    "picman-font-list|picman-font-grid",
    PICMAN_HELP_FONT_DIALOG },

  { "dialogs-buffers", PICMAN_STOCK_BUFFER,
    NC_("dialogs-action", "B_uffers"), "",
    NC_("dialogs-action", "Open the named buffers dialog"),
    "picman-buffer-list|picman-buffer-grid",
    PICMAN_HELP_BUFFER_DIALOG },

  { "dialogs-images", PICMAN_STOCK_IMAGES,
    NC_("dialogs-action", "_Images"), NULL,
    NC_("dialogs-action", "Open the images dialog"),
    "picman-image-list|picman-image-grid",
    PICMAN_HELP_IMAGE_DIALOG },

  { "dialogs-document-history", "document-open-recent",
    NC_("dialogs-action", "Document Histor_y"), "",
    NC_("dialogs-action", "Open the document history dialog"),
    "picman-document-list|picman-document-grid",
    PICMAN_HELP_DOCUMENT_DIALOG },

  { "dialogs-templates", PICMAN_STOCK_TEMPLATE,
    NC_("dialogs-action", "_Templates"), "",
    NC_("dialogs-action", "Open the image templates dialog"),
    "picman-template-list|picman-template-grid",
    PICMAN_HELP_TEMPLATE_DIALOG },

  { "dialogs-error-console", PICMAN_STOCK_WARNING,
    NC_("dialogs-action", "Error Co_nsole"), NULL,
    NC_("dialogs-action", "Open the error console"),
    "picman-error-console",
    PICMAN_HELP_ERRORS_DIALOG }
};

gint n_dialogs_dockable_actions = G_N_ELEMENTS (dialogs_dockable_actions);

static const PicmanStringActionEntry dialogs_toplevel_actions[] =
{
  { "dialogs-preferences", GTK_STOCK_PREFERENCES,
    NC_("dialogs-action", "_Preferences"), NULL,
    NC_("dialogs-action", "Open the preferences dialog"),
    "picman-preferences-dialog",
    PICMAN_HELP_PREFS_DIALOG },

  { "dialogs-input-devices", PICMAN_STOCK_INPUT_DEVICE,
    NC_("dialogs-action", "_Input Devices"), NULL,
    NC_("dialogs-action", "Open the input devices editor"),
    "picman-input-devices-dialog",
    PICMAN_HELP_INPUT_DEVICES },

  { "dialogs-keyboard-shortcuts", PICMAN_STOCK_CHAR_PICKER,
    NC_("dialogs-action", "_Keyboard Shortcuts"), NULL,
    NC_("dialogs-action", "Open the keyboard shortcuts editor"),
    "picman-keyboard-shortcuts-dialog",
    PICMAN_HELP_KEYBOARD_SHORTCUTS },

  { "dialogs-module-dialog", GTK_STOCK_EXECUTE,
    NC_("dialogs-action", "_Modules"), NULL,
    NC_("dialogs-action", "Open the module manager dialog"),
    "picman-module-dialog",
    PICMAN_HELP_MODULE_DIALOG },

  { "dialogs-tips", PICMAN_STOCK_INFO,
    NC_("dialogs-action", "_Tip of the Day"), NULL,
    NC_("dialogs-action", "Show some helpful tips on using PICMAN"),
    "picman-tips-dialog",
    PICMAN_HELP_TIPS_DIALOG },

  { "dialogs-about", GTK_STOCK_ABOUT,
    NC_("dialogs-action", "_About"), NULL,
    NC_("dialogs-action", "About PICMAN"),
    "picman-about-dialog",
    PICMAN_HELP_ABOUT_DIALOG }
};


gboolean
dialogs_actions_toolbox_exists (Picman *picman)
{
  PicmanDialogFactory *factory       = picman_dialog_factory_get_singleton ();
  gboolean           toolbox_found = FALSE;
  GList             *iter;

  /* First look in session managed windows */
  toolbox_found =
    picman_dialog_factory_find_widget (factory, "picman-toolbox-window") != NULL;

  /* Then in image windows */
  if (! toolbox_found)
    {
      GList *windows = picman ? picman_get_image_windows (picman) : NULL;

      for (iter = windows; iter; iter = g_list_next (iter))
        {
          PicmanImageWindow *window = PICMAN_IMAGE_WINDOW (windows->data);

          if (picman_image_window_has_toolbox (window))
            {
              toolbox_found = TRUE;
              break;
            }
        }

      g_list_free (windows);
    }

  return toolbox_found;
}

void
dialogs_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_string_actions (group, "dialogs-action",
                                        dialogs_dockable_actions,
                                        G_N_ELEMENTS (dialogs_dockable_actions),
                                        G_CALLBACK (dialogs_create_dockable_cmd_callback));

  picman_action_group_add_string_actions (group, "dialogs-action",
                                        dialogs_toplevel_actions,
                                        G_N_ELEMENTS (dialogs_toplevel_actions),
                                        G_CALLBACK (dialogs_create_toplevel_cmd_callback));
}

void
dialogs_actions_update (PicmanActionGroup *group,
                        gpointer         data)
{
  Picman        *picman            = action_data_get_picman (data);
  const gchar *toolbox_label   = NULL;
  const gchar *toolbox_tooltip = NULL;

  if (dialogs_actions_toolbox_exists (picman))
    {
      toolbox_label   = _("Toolbox");
      toolbox_tooltip = _("Raise the toolbox");
    }
  else
    {
      toolbox_label   = _("New Toolbox");
      toolbox_tooltip = _("Create a new toolbox");
    }

  picman_action_group_set_action_label (group, "dialogs-toolbox", toolbox_label);
  picman_action_group_set_action_tooltip (group, "dialogs-toolbox", toolbox_tooltip);
}
