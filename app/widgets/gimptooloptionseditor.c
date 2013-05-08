/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantooloptionseditor.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanlist.h"
#include "core/picmantoolinfo.h"
#include "core/picmantooloptions.h"

#include "picmandnd.h"
#include "picmandocked.h"
#include "picmanhelp-ids.h"
#include "picmanmenufactory.h"
#include "picmanpropwidgets.h"
#include "picmanview.h"
#include "picmanviewrenderer.h"
#include "picmantooloptionseditor.h"
#include "picmanuimanager.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"

enum
{
  PROP_0,
  PROP_PICMAN,
};

struct _PicmanToolOptionsEditorPrivate
{
  Picman            *picman;

  GtkWidget       *scrolled_window;
  GtkWidget       *options_vbox;
  GtkWidget       *title_label;

  GtkWidget       *save_button;
  GtkWidget       *restore_button;
  GtkWidget       *delete_button;
  GtkWidget       *reset_button;

  PicmanToolOptions *visible_tool_options;
};


static void        picman_tool_options_editor_docked_iface_init (PicmanDockedInterface   *iface);
static void        picman_tool_options_editor_constructed       (GObject               *object);
static void        picman_tool_options_editor_dispose           (GObject               *object);
static void        picman_tool_options_editor_set_property      (GObject               *object,
                                                               guint                  property_id,
                                                               const GValue          *value,
                                                               GParamSpec            *pspec);
static void        picman_tool_options_editor_get_property      (GObject               *object,
                                                               guint                  property_id,
                                                               GValue                *value,
                                                               GParamSpec            *pspec);

static GtkWidget * picman_tool_options_editor_get_preview       (PicmanDocked            *docked,
                                                               PicmanContext           *context,
                                                               GtkIconSize            size);
static gchar     * picman_tool_options_editor_get_title         (PicmanDocked            *docked);
static gboolean    picman_tool_options_editor_get_prefer_icon   (PicmanDocked            *docked);
static void        picman_tool_options_editor_save_clicked      (GtkWidget             *widget,
                                                               PicmanToolOptionsEditor *editor);
static void        picman_tool_options_editor_restore_clicked   (GtkWidget             *widget,
                                                               PicmanToolOptionsEditor *editor);
static void        picman_tool_options_editor_delete_clicked    (GtkWidget             *widget,
                                                               PicmanToolOptionsEditor *editor);
static void        picman_tool_options_editor_drop_tool         (GtkWidget             *widget,
                                                               gint                   x,
                                                               gint                   y,
                                                               PicmanViewable          *viewable,
                                                               gpointer               data);
static void        picman_tool_options_editor_tool_changed      (PicmanContext           *context,
                                                               PicmanToolInfo          *tool_info,
                                                               PicmanToolOptionsEditor *editor);
static void        picman_tool_options_editor_presets_update    (PicmanToolOptionsEditor *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanToolOptionsEditor, picman_tool_options_editor,
                         PICMAN_TYPE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_tool_options_editor_docked_iface_init))

#define parent_class picman_tool_options_editor_parent_class


static void
picman_tool_options_editor_class_init (PicmanToolOptionsEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_tool_options_editor_constructed;
  object_class->dispose      = picman_tool_options_editor_dispose;
  object_class->set_property = picman_tool_options_editor_set_property;
  object_class->get_property = picman_tool_options_editor_get_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (PicmanToolOptionsEditorPrivate));
}

static void
picman_tool_options_editor_docked_iface_init (PicmanDockedInterface *docked_iface)
{
  docked_iface->get_preview     = picman_tool_options_editor_get_preview;
  docked_iface->get_title       = picman_tool_options_editor_get_title;
  docked_iface->get_prefer_icon = picman_tool_options_editor_get_prefer_icon;
}

static void
picman_tool_options_editor_init (PicmanToolOptionsEditor *editor)
{
  GtkScrolledWindow *scrolled_window;
  GtkWidget         *viewport;

  editor->p = G_TYPE_INSTANCE_GET_PRIVATE (editor,
                                           PICMAN_TYPE_TOOL_OPTIONS_EDITOR,
                                           PicmanToolOptionsEditorPrivate);

  gtk_widget_set_size_request (GTK_WIDGET (editor), -1, 200);

  picman_dnd_viewable_dest_add (GTK_WIDGET (editor),
                              PICMAN_TYPE_TOOL_INFO,
                              picman_tool_options_editor_drop_tool,
                              editor);

  /*  The label containing the tool options title */
  editor->p->title_label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (editor->p->title_label), 0.0, 0.0);
  picman_label_set_attributes (GTK_LABEL (editor->p->title_label),
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
  gtk_box_pack_start (GTK_BOX (editor),
                      editor->p->title_label, FALSE, FALSE, 0);
  gtk_widget_show (editor->p->title_label);

  editor->p->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  scrolled_window = GTK_SCROLLED_WINDOW (editor->p->scrolled_window);

  gtk_scrolled_window_set_policy (scrolled_window,
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  gtk_box_pack_start (GTK_BOX (editor), editor->p->scrolled_window,
                      TRUE, TRUE, 0);
  gtk_widget_show (editor->p->scrolled_window);

  viewport = gtk_viewport_new (gtk_scrolled_window_get_hadjustment (scrolled_window),
                               gtk_scrolled_window_get_vadjustment (scrolled_window));
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), GTK_SHADOW_NONE);
  gtk_container_add (GTK_CONTAINER (scrolled_window), viewport);
  gtk_widget_show (viewport);

  /*  The vbox containing the tool options  */
  editor->p->options_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (viewport), editor->p->options_vbox);
  gtk_widget_show (editor->p->options_vbox);
}

static void
picman_tool_options_editor_constructed (GObject *object)
{
  PicmanToolOptionsEditor *editor = PICMAN_TOOL_OPTIONS_EDITOR (object);
  PicmanContext           *user_context;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  editor->p->save_button =
    picman_editor_add_button (PICMAN_EDITOR (editor), GTK_STOCK_SAVE,
                            _("Save Tool Preset..."),
                            PICMAN_HELP_TOOL_OPTIONS_SAVE,
                            G_CALLBACK (picman_tool_options_editor_save_clicked),
                            NULL,
                            editor);

  editor->p->restore_button =
    picman_editor_add_button (PICMAN_EDITOR (editor), GTK_STOCK_REVERT_TO_SAVED,
                            _("Restore Tool Preset..."),
                            PICMAN_HELP_TOOL_OPTIONS_RESTORE,
                            G_CALLBACK (picman_tool_options_editor_restore_clicked),
                            NULL,
                            editor);

  editor->p->delete_button =
    picman_editor_add_button (PICMAN_EDITOR (editor), GTK_STOCK_DELETE,
                            _("Delete Tool Preset..."),
                            PICMAN_HELP_TOOL_OPTIONS_DELETE,
                            G_CALLBACK (picman_tool_options_editor_delete_clicked),
                            NULL,
                            editor);

  editor->p->reset_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor), "tool-options",
                                   "tool-options-reset",
                                   "tool-options-reset-all",
                                   GDK_SHIFT_MASK,
                                   NULL);

  user_context = picman_get_user_context (editor->p->picman);

  g_signal_connect_object (user_context, "tool-changed",
                           G_CALLBACK (picman_tool_options_editor_tool_changed),
                           editor,
                           0);

  picman_tool_options_editor_tool_changed (user_context,
                                         picman_context_get_tool (user_context),
                                         editor);
}

static void
picman_tool_options_editor_dispose (GObject *object)
{
  PicmanToolOptionsEditor *editor = PICMAN_TOOL_OPTIONS_EDITOR (object);

  if (editor->p->options_vbox)
    {
      GList *options;
      GList *list;

      options =
        gtk_container_get_children (GTK_CONTAINER (editor->p->options_vbox));

      for (list = options; list; list = g_list_next (list))
        {
          g_object_ref (list->data);
          gtk_container_remove (GTK_CONTAINER (editor->p->options_vbox),
                                GTK_WIDGET (list->data));
        }

      g_list_free (options);
      editor->p->options_vbox = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_tool_options_editor_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PicmanToolOptionsEditor *editor = PICMAN_TOOL_OPTIONS_EDITOR (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      editor->p->picman = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_tool_options_editor_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PicmanToolOptionsEditor *editor = PICMAN_TOOL_OPTIONS_EDITOR (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, editor->p->picman);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static GtkWidget *
picman_tool_options_editor_get_preview (PicmanDocked   *docked,
                                      PicmanContext  *context,
                                      GtkIconSize   size)
{
  GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (docked));
  GtkWidget   *view;
  gint         width;
  gint         height;

  gtk_icon_size_lookup_for_settings (settings, size, &width, &height);

  view = picman_prop_view_new (G_OBJECT (context), "tool", context, height);
  PICMAN_VIEW (view)->renderer->size = -1;
  picman_view_renderer_set_size_full (PICMAN_VIEW (view)->renderer,
                                    width, height, 0);

  return view;
}

static gchar *
picman_tool_options_editor_get_title (PicmanDocked *docked)
{
  PicmanToolOptionsEditor *editor = PICMAN_TOOL_OPTIONS_EDITOR (docked);
  PicmanContext           *context;
  PicmanToolInfo          *tool_info;

  context = picman_get_user_context (editor->p->picman);

  tool_info = picman_context_get_tool (context);

  return tool_info ? g_strdup (tool_info->blurb) : NULL;
}

static gboolean
picman_tool_options_editor_get_prefer_icon (PicmanDocked *docked)
{
  /* We support get_preview() for tab tyles, but we prefer to show our
   * icon
   */
  return TRUE;
}


/*  public functions  */

GtkWidget *
picman_tool_options_editor_new (Picman            *picman,
                              PicmanMenuFactory *menu_factory)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  return g_object_new (PICMAN_TYPE_TOOL_OPTIONS_EDITOR,
                       "picman",            picman,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<ToolOptions>",
                       "ui-path",         "/tool-options-popup",
                       NULL);
}

PicmanToolOptions *
picman_tool_options_editor_get_tool_options (PicmanToolOptionsEditor *editor)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_OPTIONS_EDITOR (editor), NULL);

  return editor->p->visible_tool_options;
}

/*  private functions  */

static void
picman_tool_options_editor_menu_pos (GtkMenu  *menu,
                                   gint     *x,
                                   gint     *y,
                                   gpointer  data)
{
  picman_button_menu_position (GTK_WIDGET (data), menu, GTK_POS_RIGHT, x, y);
}

static void
picman_tool_options_editor_menu_popup (PicmanToolOptionsEditor *editor,
                                     GtkWidget             *button,
                                     const gchar           *path)
{
  PicmanEditor *picman_editor = PICMAN_EDITOR (editor);

  gtk_ui_manager_get_widget (GTK_UI_MANAGER (picman_editor_get_ui_manager (picman_editor)),
                             picman_editor_get_ui_path (picman_editor));
  picman_ui_manager_update (picman_editor_get_ui_manager (picman_editor),
                          picman_editor_get_popup_data (picman_editor));

  picman_ui_manager_ui_popup (picman_editor_get_ui_manager (picman_editor), path,
                            button,
                            picman_tool_options_editor_menu_pos, button,
                            NULL, NULL);
}

static void
picman_tool_options_editor_save_clicked (GtkWidget             *widget,
                                       PicmanToolOptionsEditor *editor)
{
  if (gtk_widget_get_sensitive (editor->p->restore_button) /* evil but correct */)
    {
      picman_tool_options_editor_menu_popup (editor, widget,
                                           "/tool-options-popup/Save");
    }
  else
    {
      picman_ui_manager_activate_action (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                                       "tool-options",
                                       "tool-options-save-new-preset");
    }
}

static void
picman_tool_options_editor_restore_clicked (GtkWidget             *widget,
                                          PicmanToolOptionsEditor *editor)
{
  picman_tool_options_editor_menu_popup (editor, widget,
                                       "/tool-options-popup/Restore");
}

static void
picman_tool_options_editor_delete_clicked (GtkWidget             *widget,
                                         PicmanToolOptionsEditor *editor)
{
  picman_tool_options_editor_menu_popup (editor, widget,
                                       "/tool-options-popup/Delete");
}

static void
picman_tool_options_editor_drop_tool (GtkWidget    *widget,
                                    gint          x,
                                    gint          y,
                                    PicmanViewable *viewable,
                                    gpointer      data)
{
  PicmanToolOptionsEditor *editor = PICMAN_TOOL_OPTIONS_EDITOR (data);
  PicmanContext           *context;

  context = picman_get_user_context (editor->p->picman);

  picman_context_set_tool (context, PICMAN_TOOL_INFO (viewable));
}

static void
picman_tool_options_editor_tool_changed (PicmanContext           *context,
                                       PicmanToolInfo          *tool_info,
                                       PicmanToolOptionsEditor *editor)
{
  PicmanContainer *presets;
  GtkWidget     *options_gui;
  
  /* This will warn if tool info is changed to nothing.
   * This seems to happen if starting in SWM with tool editor visible
   * Maybe its normal, and the code should just be written to
   * handle this case, but someone smarter needs to take a look*/
  g_return_if_fail(PICMAN_IS_TOOL_INFO(tool_info));

  if (tool_info && tool_info->tool_options == editor->p->visible_tool_options)
    return;

  if (editor->p->visible_tool_options)
    {
      presets = editor->p->visible_tool_options->tool_info->presets;

      if (presets)
        g_signal_handlers_disconnect_by_func (presets,
                                              picman_tool_options_editor_presets_update,
                                              editor);

      options_gui = picman_tools_get_tool_options_gui (editor->p->visible_tool_options);

      if (options_gui)
        gtk_widget_hide (options_gui);

      editor->p->visible_tool_options = NULL;
    }

  if (tool_info && tool_info->tool_options)
    {
      presets = tool_info->presets;

      if (presets)
        {
          g_signal_connect_object (presets, "add",
                                   G_CALLBACK (picman_tool_options_editor_presets_update),
                                   G_OBJECT (editor), G_CONNECT_SWAPPED);
          g_signal_connect_object (presets, "remove",
                                   G_CALLBACK (picman_tool_options_editor_presets_update),
                                   G_OBJECT (editor), G_CONNECT_SWAPPED);
          g_signal_connect_object (presets, "thaw",
                                   G_CALLBACK (picman_tool_options_editor_presets_update),
                                   G_OBJECT (editor), G_CONNECT_SWAPPED);
        }

      options_gui = picman_tools_get_tool_options_gui (tool_info->tool_options);

      if (! gtk_widget_get_parent (options_gui))
        gtk_box_pack_start (GTK_BOX (editor->p->options_vbox), options_gui,
                            FALSE, FALSE, 0);

      gtk_widget_show (options_gui);

      editor->p->visible_tool_options = tool_info->tool_options;

      picman_help_set_help_data (editor->p->scrolled_window, NULL,
                               tool_info->help_id);

      picman_tool_options_editor_presets_update (editor);
    }

  if (editor->p->title_label != NULL)
    {
      gchar *title;

      title = picman_docked_get_title (PICMAN_DOCKED (editor));
      gtk_label_set_text (GTK_LABEL (editor->p->title_label), title);
      g_free (title);
    }

  picman_docked_title_changed (PICMAN_DOCKED (editor));
}

static void
picman_tool_options_editor_presets_update (PicmanToolOptionsEditor *editor)
{
  PicmanToolInfo *tool_info         = editor->p->visible_tool_options->tool_info;
  gboolean      save_sensitive    = FALSE;
  gboolean      restore_sensitive = FALSE;
  gboolean      delete_sensitive  = FALSE;
  gboolean      reset_sensitive   = FALSE;

  if (tool_info->presets)
    {
      save_sensitive  = TRUE;
      reset_sensitive = TRUE;

      if (! picman_container_is_empty (tool_info->presets))
        {
          restore_sensitive = TRUE;
          delete_sensitive  = TRUE;
        }
    }

  gtk_widget_set_sensitive (editor->p->save_button,    save_sensitive);
  gtk_widget_set_sensitive (editor->p->restore_button, restore_sensitive);
  gtk_widget_set_sensitive (editor->p->delete_button,  delete_sensitive);
  gtk_widget_set_sensitive (editor->p->reset_button,   reset_sensitive);
}
