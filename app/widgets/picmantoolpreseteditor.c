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

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmantoolinfo.h"
#include "core/picmantooloptions.h"
#include "core/picmantoolpreset.h"

#include "picmandocked.h"
#include "picmantoolpreseteditor.h"
#include "picmanmenufactory.h"
#include "picmanpropwidgets.h"

#include "picman-intl.h"


struct _PicmanToolPresetEditorPrivate
{
  PicmanToolPreset *tool_preset_model;

  GtkWidget      *tool_icon;
  GtkWidget      *tool_label;

  GtkWidget      *fg_bg_toggle;
  GtkWidget      *brush_toggle;
  GtkWidget      *dynamics_toggle;
  GtkWidget      *gradient_toggle;
  GtkWidget      *pattern_toggle;
  GtkWidget      *palette_toggle;
  GtkWidget      *font_toggle;
};


/*  local function prototypes  */

static void   picman_tool_preset_editor_constructed  (GObject              *object);
static void   picman_tool_preset_editor_finalize     (GObject              *object);

static void   picman_tool_preset_editor_set_data     (PicmanDataEditor       *editor,
                                                    PicmanData             *data);

static void   picman_tool_preset_editor_sync_data    (PicmanToolPresetEditor *editor);
static void   picman_tool_preset_editor_notify_model (PicmanToolPreset       *options,
                                                    const GParamSpec     *pspec,
                                                    PicmanToolPresetEditor *editor);
static void   picman_tool_preset_editor_notify_data  (PicmanToolPreset       *options,
                                                    const GParamSpec     *pspec,
                                                    PicmanToolPresetEditor *editor);



G_DEFINE_TYPE_WITH_CODE (PicmanToolPresetEditor, picman_tool_preset_editor,
                         PICMAN_TYPE_DATA_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED, NULL))

#define parent_class picman_tool_preset_editor_parent_class


static void
picman_tool_preset_editor_class_init (PicmanToolPresetEditorClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanDataEditorClass *editor_class = PICMAN_DATA_EDITOR_CLASS (klass);

  object_class->constructed = picman_tool_preset_editor_constructed;
  object_class->finalize    = picman_tool_preset_editor_finalize;

  editor_class->set_data    = picman_tool_preset_editor_set_data;
  editor_class->title       = _("Tool Preset Editor");

  g_type_class_add_private (klass, sizeof (PicmanToolPresetEditorPrivate));
}

static void
picman_tool_preset_editor_init (PicmanToolPresetEditor *editor)
{
  editor->priv = G_TYPE_INSTANCE_GET_PRIVATE (editor,
                                              PICMAN_TYPE_TOOL_PRESET_EDITOR,
                                              PicmanToolPresetEditorPrivate);
}

static void
picman_tool_preset_editor_constructed (GObject *object)
{
  PicmanToolPresetEditor *editor      = PICMAN_TOOL_PRESET_EDITOR (object);
  PicmanDataEditor       *data_editor = PICMAN_DATA_EDITOR (editor);
  PicmanToolPreset       *preset;
  GtkWidget            *hbox;
  GtkWidget            *label;
  GtkWidget            *button;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  preset = editor->priv->tool_preset_model =
    g_object_new (PICMAN_TYPE_TOOL_PRESET,
                  "picman", data_editor->context->picman,
                  NULL);

  g_signal_connect (preset, "notify",
                    G_CALLBACK (picman_tool_preset_editor_notify_model),
                    editor);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (data_editor), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  editor->priv->tool_icon = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (hbox), editor->priv->tool_icon,
                      FALSE, FALSE, 0);
  gtk_widget_show (editor->priv->tool_icon);

  editor->priv->tool_label = gtk_label_new ("");
  picman_label_set_attributes (GTK_LABEL (editor->priv->tool_label),
                             PANGO_ATTR_STYLE, PANGO_STYLE_ITALIC,
                             -1);
  gtk_box_pack_start (GTK_BOX (hbox), editor->priv->tool_label,
                      FALSE, FALSE, 0);
  gtk_widget_show (editor->priv->tool_label);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (data_editor), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Icon:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  button = picman_prop_icon_picker_new (PICMAN_VIEWABLE (preset),
                                      data_editor->context->picman);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = editor->priv->fg_bg_toggle =
    picman_prop_check_button_new (G_OBJECT (preset), "use-fg-bg",
                                _("Apply stored FG/BG"));
  gtk_box_pack_start (GTK_BOX (data_editor), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = editor->priv->brush_toggle =
    picman_prop_check_button_new (G_OBJECT (preset), "use-brush",
                                _("Apply stored brush"));
  gtk_box_pack_start (GTK_BOX (data_editor), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = editor->priv->dynamics_toggle =
    picman_prop_check_button_new (G_OBJECT (preset), "use-dynamics",
                                _("Apply stored dynamics"));
  gtk_box_pack_start (GTK_BOX (data_editor), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = editor->priv->gradient_toggle =
    picman_prop_check_button_new (G_OBJECT (preset), "use-gradient",
                                _("Apply stored gradient"));
  gtk_box_pack_start (GTK_BOX (data_editor), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = editor->priv->pattern_toggle =
    picman_prop_check_button_new (G_OBJECT (preset), "use-pattern",
                                _("Apply stored pattern"));
  gtk_box_pack_start (GTK_BOX (data_editor), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = editor->priv->palette_toggle =
    picman_prop_check_button_new (G_OBJECT (preset), "use-palette",
                                _("Apply stored palette"));
  gtk_box_pack_start (GTK_BOX (data_editor), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = editor->priv->font_toggle =
    picman_prop_check_button_new (G_OBJECT (preset), "use-font",
                                _("Apply stored font"));
  gtk_box_pack_start (GTK_BOX (data_editor), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  if (data_editor->data)
    picman_tool_preset_editor_sync_data (editor);
}

static void
picman_tool_preset_editor_finalize (GObject *object)
{
  PicmanToolPresetEditor *editor = PICMAN_TOOL_PRESET_EDITOR (object);

  if (editor->priv->tool_preset_model)
    {
      g_object_unref (editor->priv->tool_preset_model);
      editor->priv->tool_preset_model = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_tool_preset_editor_set_data (PicmanDataEditor *editor,
                                  PicmanData       *data)
{
  PicmanToolPresetEditor *preset_editor = PICMAN_TOOL_PRESET_EDITOR (editor);

  if (editor->data)
    g_signal_handlers_disconnect_by_func (editor->data,
                                          picman_tool_preset_editor_notify_data,
                                          editor);

  PICMAN_DATA_EDITOR_CLASS (parent_class)->set_data (editor, data);

  if (editor->data)
    {
      g_signal_connect (editor->data, "notify",
                        G_CALLBACK (picman_tool_preset_editor_notify_data),
                        editor);

      if (preset_editor->priv->tool_preset_model)
        picman_tool_preset_editor_sync_data (preset_editor);
    }

  gtk_widget_set_sensitive (GTK_WIDGET (editor), editor->data_editable);
}


/*  public functions  */

GtkWidget *
picman_tool_preset_editor_new (PicmanContext     *context,
                             PicmanMenuFactory *menu_factory)
{
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return g_object_new (PICMAN_TYPE_TOOL_PRESET_EDITOR,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<ToolPresetEditor>",
                       "ui-path",         "/tool-preset-editor-popup",
                       "data-factory",    context->picman->tool_preset_factory,
                       "context",         context,
                       "data",            picman_context_get_tool_preset (context),
                       NULL);
}


/*  private functions  */

static void
picman_tool_preset_editor_sync_data (PicmanToolPresetEditor *editor)
{
  PicmanToolPresetEditorPrivate *priv        = editor->priv;
  PicmanDataEditor              *data_editor = PICMAN_DATA_EDITOR (editor);
  PicmanToolPreset              *preset;
  PicmanToolInfo                *tool_info;
  PicmanContextPropMask          serialize_props;
  const gchar                 *stock_id;
  gchar                       *label;

  g_signal_handlers_block_by_func (priv->tool_preset_model,
                                   picman_tool_preset_editor_notify_model,
                                   editor);

  picman_config_copy (PICMAN_CONFIG (data_editor->data),
                    PICMAN_CONFIG (priv->tool_preset_model),
                    PICMAN_CONFIG_PARAM_SERIALIZE);

  g_signal_handlers_unblock_by_func (priv->tool_preset_model,
                                     picman_tool_preset_editor_notify_model,
                                     editor);

  tool_info = priv->tool_preset_model->tool_options->tool_info;

  stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool_info));
  label    = g_strdup_printf (_("%s Preset"), tool_info->blurb);

  gtk_image_set_from_stock (GTK_IMAGE (priv->tool_icon),
                            stock_id, GTK_ICON_SIZE_MENU);
  gtk_label_set_text (GTK_LABEL (priv->tool_label), label);

  g_free (label);

  preset = PICMAN_TOOL_PRESET (data_editor->data);

  serialize_props =
    picman_context_get_serialize_properties (PICMAN_CONTEXT (preset->tool_options));

  gtk_widget_set_sensitive (priv->fg_bg_toggle,
                            (serialize_props & PICMAN_CONTEXT_FOREGROUND_MASK) != 0);
  gtk_widget_set_sensitive (priv->brush_toggle,
                            (serialize_props & PICMAN_CONTEXT_BRUSH_MASK) != 0);
  gtk_widget_set_sensitive (priv->dynamics_toggle,
                            (serialize_props & PICMAN_CONTEXT_DYNAMICS_MASK) != 0);
  gtk_widget_set_sensitive (priv->gradient_toggle,
                            (serialize_props & PICMAN_CONTEXT_GRADIENT_MASK) != 0);
  gtk_widget_set_sensitive (priv->pattern_toggle,
                            (serialize_props & PICMAN_CONTEXT_PATTERN_MASK) != 0);
  gtk_widget_set_sensitive (priv->palette_toggle,
                            (serialize_props & PICMAN_CONTEXT_PALETTE_MASK) != 0);
  gtk_widget_set_sensitive (priv->font_toggle,
                            (serialize_props & PICMAN_CONTEXT_FONT_MASK) != 0);
 }

static void
picman_tool_preset_editor_notify_model (PicmanToolPreset       *options,
                                      const GParamSpec     *pspec,
                                      PicmanToolPresetEditor *editor)
{
  PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (editor);

  if (data_editor->data)
    {
      g_signal_handlers_block_by_func (data_editor->data,
                                       picman_tool_preset_editor_notify_data,
                                       editor);

      picman_config_copy (PICMAN_CONFIG (editor->priv->tool_preset_model),
                        PICMAN_CONFIG (data_editor->data),
                        PICMAN_CONFIG_PARAM_SERIALIZE);

      g_signal_handlers_unblock_by_func (data_editor->data,
                                         picman_tool_preset_editor_notify_data,
                                         editor);
    }
}

static void
picman_tool_preset_editor_notify_data (PicmanToolPreset       *options,
                                     const GParamSpec     *pspec,
                                     PicmanToolPresetEditor *editor)
{
  PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (editor);

  g_signal_handlers_block_by_func (editor->priv->tool_preset_model,
                                   picman_tool_preset_editor_notify_model,
                                   editor);

  picman_config_copy (PICMAN_CONFIG (data_editor->data),
                    PICMAN_CONFIG (editor->priv->tool_preset_model),
                    PICMAN_CONFIG_PARAM_SERIALIZE);

  g_signal_handlers_unblock_by_func (editor->priv->tool_preset_model,
                                     picman_tool_preset_editor_notify_model,
                                     editor);
}
