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

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picmanimage.h"

#include "text/picmantextlayer.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmantexteditor.h"
#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "tools/picmantool.h"
#include "tools/picmantexttool.h"

#include "text-tool-actions.h"
#include "text-tool-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry text_tool_actions[] =
{
  { "text-tool-popup", NULL,
    NC_("text-tool-action", "Text Tool Menu"), NULL, NULL, NULL,
    NULL },

  { "text-tool-input-methods-menu", NULL,
    NC_("text-tool-action", "Input _Methods"), NULL, NULL, NULL,
    NULL },

  { "text-tool-cut", GTK_STOCK_CUT,
    NC_("text-tool-action", "Cu_t"), NULL, NULL,
    G_CALLBACK (text_tool_cut_cmd_callback),
    NULL },

  { "text-tool-copy", GTK_STOCK_COPY,
    NC_("text-tool-action", "_Copy"), NULL, NULL,
    G_CALLBACK (text_tool_copy_cmd_callback),
    NULL },

  { "text-tool-paste", GTK_STOCK_PASTE,
    NC_("text-tool-action", "_Paste"), NULL, NULL,
    G_CALLBACK (text_tool_paste_cmd_callback),
    NULL },

  { "text-tool-delete", GTK_STOCK_DELETE,
    NC_("text-tool-action", "_Delete"), NULL, NULL,
    G_CALLBACK (text_tool_delete_cmd_callback),
    NULL },

  { "text-tool-load", GTK_STOCK_OPEN,
    NC_("text-tool-action", "_Open text file..."), NULL, NULL,
    G_CALLBACK (text_tool_load_cmd_callback),
    NULL },

  { "text-tool-clear", GTK_STOCK_CLEAR,
    NC_("text-tool-action", "Cl_ear"), "",
    NC_("text-tool-action", "Clear all text"),
    G_CALLBACK (text_tool_clear_cmd_callback),
    NULL },

  { "text-tool-text-to-path", PICMAN_STOCK_PATH,
    NC_("text-tool-action", "_Path from Text"), "",
    NC_("text-tool-action",
        "Create a path from the outlines of the current text"),
    G_CALLBACK (text_tool_text_to_path_cmd_callback),
    NULL },

  { "text-tool-text-along-path", PICMAN_STOCK_PATH,
    NC_("text-tool-action", "Text _along Path"), "",
    NC_("text-tool-action",
        "Bend the text along the currently active path"),
    G_CALLBACK (text_tool_text_along_path_cmd_callback),
    NULL }
};

static const PicmanRadioActionEntry text_tool_direction_actions[] =
{
  { "text-tool-direction-ltr", PICMAN_STOCK_TEXT_DIR_LTR,
    NC_("text-tool-action", "From left to right"), NULL, NULL,
    PICMAN_TEXT_DIRECTION_LTR,
    NULL },

  { "text-tool-direction-rtl", PICMAN_STOCK_TEXT_DIR_RTL,
    NC_("text-tool-action", "From right to left"), NULL, NULL,
    PICMAN_TEXT_DIRECTION_RTL,
    NULL }
};


#define SET_HIDE_EMPTY(action,condition) \
        picman_action_group_set_action_hide_empty (group, action, (condition) != 0)

void
text_tool_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "text-tool-action",
                                 text_tool_actions,
                                 G_N_ELEMENTS (text_tool_actions));

  picman_action_group_add_radio_actions (group, "text-tool-action",
                                       text_tool_direction_actions,
                                       G_N_ELEMENTS (text_tool_direction_actions),
                                       NULL,
                                       PICMAN_TEXT_DIRECTION_LTR,
                                       G_CALLBACK (text_tool_direction_cmd_callback));

  SET_HIDE_EMPTY ("text-tool-input-methods-menu", FALSE);
}

/* The following code is written on the assumption that this is for a
 * context menu, activated by right-clicking in a text layer.
 * Therefore, the tool must have a display.  If for any reason the
 * code is adapted to a different situation, some existence testing
 * will need to be added.
 */
void
text_tool_actions_update (PicmanActionGroup *group,
                          gpointer         data)
{
  PicmanTextTool     *text_tool  = PICMAN_TEXT_TOOL (data);
  PicmanDisplay      *display    = PICMAN_TOOL (text_tool)->display;
  PicmanImage        *image      = picman_display_get_image (display);
  PicmanLayer        *layer;
  PicmanVectors      *vectors;
  PicmanDisplayShell *shell;
  GtkClipboard     *clipboard;
  gboolean          text_layer = FALSE;
  gboolean          text_sel   = FALSE;   /* some text is selected        */
  gboolean          clip       = FALSE;   /* clipboard has text available */
  gboolean          input_method_menu;
  gboolean          unicode_menu;

  layer = picman_image_get_active_layer (image);

  if (layer)
    text_layer = picman_item_is_text_layer (PICMAN_ITEM (layer));

  vectors = picman_image_get_active_vectors (image);

  text_sel = picman_text_tool_get_has_text_selection (text_tool);

  /* see whether there is text available for pasting */
  shell = picman_display_get_shell (display);
  clipboard = gtk_widget_get_clipboard (shell->canvas,
                                        GDK_SELECTION_CLIPBOARD);
  clip = gtk_clipboard_wait_is_text_available (clipboard);

  g_object_get (gtk_widget_get_settings (shell->canvas),
                "gtk-show-input-method-menu", &input_method_menu,
                "gtk-show-unicode-menu",      &unicode_menu,
                NULL);

#define SET_VISIBLE(action,condition) \
        picman_action_group_set_action_visible (group, action, (condition) != 0)
#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)
#define SET_ACTIVE(action,condition) \
        picman_action_group_set_action_active (group, action, (condition) != 0)

  SET_SENSITIVE ("text-tool-cut",             text_sel);
  SET_SENSITIVE ("text-tool-copy",            text_sel);
  SET_SENSITIVE ("text-tool-paste",           clip);
  SET_SENSITIVE ("text-tool-delete",          text_sel);
  SET_SENSITIVE ("text-tool-clear",           text_layer);
  SET_SENSITIVE ("text-tool-load",            image);
  SET_SENSITIVE ("text-tool-text-to-path",    text_layer);
  SET_SENSITIVE ("text-tool-text-along-path", text_layer && vectors);

  SET_VISIBLE ("text-tool-input-methods-menu", input_method_menu);
}
