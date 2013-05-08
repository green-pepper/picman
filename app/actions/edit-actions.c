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

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmandrawableundo.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanlayer.h"
#include "core/picmanlist.h"
#include "core/picmantoolinfo.h"
#include "core/picmanundostack.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "edit-actions.h"
#include "edit-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   edit_actions_foreground_changed (PicmanContext     *context,
                                               const PicmanRGB   *color,
                                               PicmanActionGroup *group);
static void   edit_actions_background_changed (PicmanContext     *context,
                                               const PicmanRGB   *color,
                                               PicmanActionGroup *group);
static void   edit_actions_pattern_changed    (PicmanContext     *context,
                                               PicmanPattern     *pattern,
                                               PicmanActionGroup *group);


static const PicmanActionEntry edit_actions[] =
{
  { "edit-menu",          NULL, NC_("edit-action", "_Edit")     },
  { "edit-paste-as-menu", NULL, NC_("edit-action", "Paste _as") },
  { "edit-buffer-menu",   NULL, NC_("edit-action", "_Buffer")   },

  { "undo-popup",
    GTK_STOCK_UNDO, NC_("edit-action", "Undo History Menu"), NULL, NULL, NULL,
    PICMAN_HELP_UNDO_DIALOG },

  { "edit-undo", GTK_STOCK_UNDO,
    NC_("edit-action", "_Undo"), "<primary>Z",
    NC_("edit-action", "Undo the last operation"),
    G_CALLBACK (edit_undo_cmd_callback),
    PICMAN_HELP_EDIT_UNDO },

  { "edit-redo", GTK_STOCK_REDO,
    NC_("edit-action", "_Redo"), "<primary>Y",
    NC_("edit-action", "Redo the last operation that was undone"),
    G_CALLBACK (edit_redo_cmd_callback),
    PICMAN_HELP_EDIT_REDO },

  { "edit-strong-undo", GTK_STOCK_UNDO,
    NC_("edit-action", "Strong Undo"), "<primary><shift>Z",
    NC_("edit-action", "Undo the last operation, skipping visibility changes"),
    G_CALLBACK (edit_strong_undo_cmd_callback),
    PICMAN_HELP_EDIT_STRONG_UNDO },

  { "edit-strong-redo", GTK_STOCK_REDO,
    NC_("edit-action", "Strong Redo"), "<primary><shift>Y",
    NC_("edit-action",
        "Redo the last operation that was undone, skipping visibility changes"),
    G_CALLBACK (edit_strong_redo_cmd_callback),
    PICMAN_HELP_EDIT_STRONG_REDO },

  { "edit-undo-clear", GTK_STOCK_CLEAR,
    NC_("edit-action", "_Clear Undo History"), "",
    NC_("edit-action", "Remove all operations from the undo history"),
    G_CALLBACK (edit_undo_clear_cmd_callback),
    PICMAN_HELP_EDIT_UNDO_CLEAR },

  { "edit-fade", GTK_STOCK_UNDO,
    NC_("edit-action", "_Fade..."), "",
    NC_("edit-action",
        "Modify paint mode and opacity of the last pixel manipulation"),
    G_CALLBACK (edit_fade_cmd_callback),
    PICMAN_HELP_EDIT_FADE },

  { "edit-cut", GTK_STOCK_CUT,
    NC_("edit-action", "Cu_t"), "<primary>X",
    NC_("edit-action", "Move the selected pixels to the clipboard"),
    G_CALLBACK (edit_cut_cmd_callback),
    PICMAN_HELP_EDIT_CUT },

  { "edit-copy", GTK_STOCK_COPY,
    NC_("edit-action", "_Copy"), "<primary>C",
    NC_("edit-action", "Copy the selected pixels to the clipboard"),
    G_CALLBACK (edit_copy_cmd_callback),
    PICMAN_HELP_EDIT_COPY },

  { "edit-copy-visible", NULL, /* PICMAN_STOCK_COPY_VISIBLE, */
    NC_("edit-action", "Copy _Visible"), "<primary><shift>C",
    NC_("edit-action", "Copy what is visible in the selected region"),
    G_CALLBACK (edit_copy_visible_cmd_callback),
    PICMAN_HELP_EDIT_COPY_VISIBLE },

  { "edit-paste", GTK_STOCK_PASTE,
    NC_("edit-action", "_Paste"), "<primary>V",
    NC_("edit-action", "Paste the content of the clipboard"),
    G_CALLBACK (edit_paste_cmd_callback),
    PICMAN_HELP_EDIT_PASTE },

  { "edit-paste-into", PICMAN_STOCK_PASTE_INTO,
    NC_("edit-action", "Paste _Into"), NULL,
    NC_("edit-action",
        "Paste the content of the clipboard into the current selection"),
    G_CALLBACK (edit_paste_into_cmd_callback),
    PICMAN_HELP_EDIT_PASTE_INTO },

  { "edit-paste-as-new", PICMAN_STOCK_PASTE_AS_NEW,
    NC_("edit-action", "From _Clipboard"), "<primary><shift>V",
    NC_("edit-action", "Create a new image from the content of the clipboard"),
    G_CALLBACK (edit_paste_as_new_cmd_callback),
    PICMAN_HELP_EDIT_PASTE_AS_NEW },

  { "edit-paste-as-new-short", PICMAN_STOCK_PASTE_AS_NEW,
    NC_("edit-action", "_New Image"), NULL,
    NC_("edit-action", "Create a new image from the content of the clipboard"),
    G_CALLBACK (edit_paste_as_new_cmd_callback),
    PICMAN_HELP_EDIT_PASTE_AS_NEW },

  { "edit-paste-as-new-layer", NULL,
    NC_("edit-action", "New _Layer"), NULL,
    NC_("edit-action", "Create a new layer from the content of the clipboard"),
    G_CALLBACK (edit_paste_as_new_layer_cmd_callback),
    PICMAN_HELP_EDIT_PASTE_AS_NEW_LAYER },

  { "edit-named-cut", GTK_STOCK_CUT,
    NC_("edit-action", "Cu_t Named..."), "",
    NC_("edit-action", "Move the selected pixels to a named buffer"),
    G_CALLBACK (edit_named_cut_cmd_callback),
    PICMAN_HELP_BUFFER_CUT },

  { "edit-named-copy", GTK_STOCK_COPY,
    NC_("edit-action", "_Copy Named..."), "",
    NC_("edit-action", "Copy the selected pixels to a named buffer"),
    G_CALLBACK (edit_named_copy_cmd_callback),
    PICMAN_HELP_BUFFER_COPY },

  { "edit-named-copy-visible", NULL, /* PICMAN_STOCK_COPY_VISIBLE, */
    NC_("edit-action", "Copy _Visible Named..."), "",
    NC_("edit-action",
        "Copy what is visible in the selected region to a named buffer"),
    G_CALLBACK (edit_named_copy_visible_cmd_callback),
    PICMAN_HELP_BUFFER_COPY },

  { "edit-named-paste", GTK_STOCK_PASTE,
    NC_("edit-action", "_Paste Named..."), "",
    NC_("edit-action", "Paste the content of a named buffer"),
    G_CALLBACK (edit_named_paste_cmd_callback),
    PICMAN_HELP_BUFFER_PASTE },

  { "edit-clear", GTK_STOCK_CLEAR,
    NC_("edit-action", "Cl_ear"), "Delete",
    NC_("edit-action", "Clear the selected pixels"),
    G_CALLBACK (edit_clear_cmd_callback),
    PICMAN_HELP_EDIT_CLEAR }
};

static const PicmanEnumActionEntry edit_fill_actions[] =
{
  { "edit-fill-fg", PICMAN_STOCK_TOOL_BUCKET_FILL,
    NC_("edit-action", "Fill with _FG Color"), "<primary>comma",
    NC_("edit-action", "Fill the selection using the foreground color"),
    PICMAN_FOREGROUND_FILL, FALSE,
    PICMAN_HELP_EDIT_FILL_FG },

  { "edit-fill-bg", PICMAN_STOCK_TOOL_BUCKET_FILL,
    NC_("edit-action", "Fill with B_G Color"), "<primary>period",
    NC_("edit-action", "Fill the selection using the background color"),
    PICMAN_BACKGROUND_FILL, FALSE,
    PICMAN_HELP_EDIT_FILL_BG },

  { "edit-fill-pattern", PICMAN_STOCK_TOOL_BUCKET_FILL,
    NC_("edit-action", "Fill _with Pattern"), "<primary>semicolon",
    NC_("edit-action", "Fill the selection using the active pattern"),
    PICMAN_PATTERN_FILL, FALSE,
    PICMAN_HELP_EDIT_FILL_PATTERN }
};


void
edit_actions_setup (PicmanActionGroup *group)
{
  PicmanContext *context = picman_get_user_context (group->picman);
  PicmanRGB      color;
  PicmanPattern *pattern;
  GtkAction   *action;

  picman_action_group_add_actions (group, "edit-action",
                                 edit_actions,
                                 G_N_ELEMENTS (edit_actions));

  picman_action_group_add_enum_actions (group, "edit-action",
                                      edit_fill_actions,
                                      G_N_ELEMENTS (edit_fill_actions),
                                      G_CALLBACK (edit_fill_cmd_callback));

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                        "edit-paste-as-new-short");
  gtk_action_set_accel_path (action, "<Actions>/edit/edit-paste-as-new");

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                        "edit-fill-pattern");
  g_object_set (action, "context", context, NULL);

  g_signal_connect_object (context, "foreground-changed",
                           G_CALLBACK (edit_actions_foreground_changed),
                           group, 0);
  g_signal_connect_object (context, "background-changed",
                           G_CALLBACK (edit_actions_background_changed),
                           group, 0);
  g_signal_connect_object (context, "pattern-changed",
                           G_CALLBACK (edit_actions_pattern_changed),
                           group, 0);

  picman_context_get_foreground (context, &color);
  edit_actions_foreground_changed (context, &color, group);

  picman_context_get_background (context, &color);
  edit_actions_background_changed (context, &color, group);

  pattern = picman_context_get_pattern (context);
  edit_actions_pattern_changed (context, pattern, group);

#define SET_ALWAYS_SHOW_IMAGE(action,show) \
        picman_action_group_set_action_always_show_image (group, action, show)

  SET_ALWAYS_SHOW_IMAGE ("edit-fill-fg",      TRUE);
  SET_ALWAYS_SHOW_IMAGE ("edit-fill-bg",      TRUE);
  SET_ALWAYS_SHOW_IMAGE ("edit-fill-pattern", TRUE);

#undef SET_ALWAYS_SHOW_IMAGE
}

void
edit_actions_update (PicmanActionGroup *group,
                     gpointer         data)
{
  PicmanImage    *image        = action_data_get_image (data);
  PicmanDrawable *drawable     = NULL;
  gchar        *undo_name    = NULL;
  gchar        *redo_name    = NULL;
  gchar        *fade_name    = NULL;
  gboolean      writable     = FALSE;
  gboolean      children     = FALSE;
  gboolean      undo_enabled = FALSE;
  gboolean      fade_enabled = FALSE;

  if (image)
    {
      drawable = picman_image_get_active_drawable (image);

      if (drawable)
        {
          writable = ! picman_item_is_content_locked (PICMAN_ITEM (drawable));

          if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)))
            children = TRUE;
        }

      undo_enabled = picman_image_undo_is_enabled (image);

      if (undo_enabled)
        {
          PicmanUndoStack *undo_stack = picman_image_get_undo_stack (image);
          PicmanUndoStack *redo_stack = picman_image_get_redo_stack (image);
          PicmanUndo      *undo       = picman_undo_stack_peek (undo_stack);
          PicmanUndo      *redo       = picman_undo_stack_peek (redo_stack);

          if (undo)
            {
              undo_name =
                g_strdup_printf (_("_Undo %s"),
                                 picman_object_get_name (undo));
            }

          if (redo)
            {
              redo_name =
                g_strdup_printf (_("_Redo %s"),
                                 picman_object_get_name (redo));
            }

          undo = picman_image_undo_get_fadeable (image);

          if (PICMAN_IS_DRAWABLE_UNDO (undo) &&
              PICMAN_DRAWABLE_UNDO (undo)->applied_buffer)
            {
              fade_enabled = TRUE;
            }

          if (fade_enabled)
            {
              fade_name =
                g_strdup_printf (_("_Fade %s..."),
                                 picman_object_get_name (undo));
            }
        }
    }


#define SET_LABEL(action,label) \
        picman_action_group_set_action_label (group, action, (label))
#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)

  SET_LABEL ("edit-undo", undo_name ? undo_name : _("_Undo"));
  SET_LABEL ("edit-redo", redo_name ? redo_name : _("_Redo"));
  SET_LABEL ("edit-fade", fade_name ? fade_name : _("_Fade..."));

  SET_SENSITIVE ("edit-undo",        undo_enabled && undo_name);
  SET_SENSITIVE ("edit-redo",        undo_enabled && redo_name);
  SET_SENSITIVE ("edit-strong-undo", undo_enabled && undo_name);
  SET_SENSITIVE ("edit-strong-redo", undo_enabled && redo_name);
  SET_SENSITIVE ("edit-undo-clear",  undo_enabled && (undo_name || redo_name));
  SET_SENSITIVE ("edit-fade",        fade_enabled && fade_name);

  g_free (undo_name);
  g_free (redo_name);
  g_free (fade_name);

  SET_SENSITIVE ("edit-cut",                writable && !children);
  SET_SENSITIVE ("edit-copy",               drawable);
  SET_SENSITIVE ("edit-copy-visible",       image);
  SET_SENSITIVE ("edit-paste",              !image || (!drawable ||
                                                       (writable && !children)));
  SET_SENSITIVE ("edit-paste-as-new-layer", image);
  SET_SENSITIVE ("edit-paste-into",         image && (!drawable ||
                                                      (writable  && !children)));

  SET_SENSITIVE ("edit-named-cut",          writable && !children);
  SET_SENSITIVE ("edit-named-copy",         drawable);
  SET_SENSITIVE ("edit-named-copy-visible", drawable);
  SET_SENSITIVE ("edit-named-paste",        TRUE);

  SET_SENSITIVE ("edit-clear",              writable && !children);
  SET_SENSITIVE ("edit-fill-fg",            writable && !children);
  SET_SENSITIVE ("edit-fill-bg",            writable && !children);
  SET_SENSITIVE ("edit-fill-pattern",       writable && !children);

#undef SET_LABEL
#undef SET_SENSITIVE
}


/*  private functions  */

static void
edit_actions_foreground_changed (PicmanContext     *context,
                                 const PicmanRGB   *color,
                                 PicmanActionGroup *group)
{
  picman_action_group_set_action_color (group, "edit-fill-fg", color, FALSE);
}

static void
edit_actions_background_changed (PicmanContext     *context,
                                 const PicmanRGB   *color,
                                 PicmanActionGroup *group)
{
  picman_action_group_set_action_color (group, "edit-fill-bg", color, FALSE);
}

static void
edit_actions_pattern_changed (PicmanContext     *context,
                              PicmanPattern     *pattern,
                              PicmanActionGroup *group)
{
  picman_action_group_set_action_viewable (group, "edit-fill-pattern",
                                         PICMAN_VIEWABLE (pattern));
}
