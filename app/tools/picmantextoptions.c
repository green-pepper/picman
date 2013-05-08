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
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "config/picmanconfig-utils.h"

#include "core/picman.h"
#include "core/picmanviewable.h"

#include "text/picmantext.h"

#include "widgets/picmancolorpanel.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmandock.h"
#include "widgets/picmanmenufactory.h"
#include "widgets/picmanpropwidgets.h"
#include "widgets/picmantextbuffer.h"
#include "widgets/picmantexteditor.h"
#include "widgets/picmanviewablebox.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmantextoptions.h"
#include "picmantooloptions-gui.h"
#include "picmanrectangleoptions.h"

#include "picman-intl.h"


enum
{
  PROP_0 = PICMAN_RECTANGLE_OPTIONS_PROP_LAST + 1,
  PROP_FONT_SIZE,
  PROP_UNIT,
  PROP_ANTIALIAS,
  PROP_HINT_STYLE,
  PROP_LANGUAGE,
  PROP_BASE_DIR,
  PROP_JUSTIFICATION,
  PROP_INDENTATION,
  PROP_LINE_SPACING,
  PROP_LETTER_SPACING,
  PROP_BOX_MODE,

  PROP_USE_EDITOR,

  PROP_FONT_VIEW_TYPE,
  PROP_FONT_VIEW_SIZE
};


static void  picman_text_options_finalize           (GObject         *object);
static void  picman_text_options_set_property       (GObject         *object,
                                                   guint            property_id,
                                                   const GValue    *value,
                                                   GParamSpec      *pspec);
static void  picman_text_options_get_property       (GObject         *object,
                                                   guint            property_id,
                                                   GValue          *value,
                                                   GParamSpec      *pspec);

static void  picman_text_options_reset              (PicmanToolOptions *tool_options);

static void  picman_text_options_notify_font        (PicmanContext     *context,
                                                   GParamSpec      *pspec,
                                                   PicmanText        *text);
static void  picman_text_options_notify_text_font   (PicmanText        *text,
                                                   GParamSpec      *pspec,
                                                   PicmanContext     *context);
static void  picman_text_options_notify_color       (PicmanContext     *context,
                                                   GParamSpec      *pspec,
                                                   PicmanText        *text);
static void  picman_text_options_notify_text_color  (PicmanText        *text,
                                                   GParamSpec      *pspec,
                                                   PicmanContext     *context);


G_DEFINE_TYPE_WITH_CODE (PicmanTextOptions, picman_text_options,
                         PICMAN_TYPE_TOOL_OPTIONS,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_RECTANGLE_OPTIONS,
                                                NULL))

#define parent_class picman_text_options_parent_class


static void
picman_text_options_class_init (PicmanTextOptionsClass *klass)
{
  GObjectClass         *object_class  = G_OBJECT_CLASS (klass);
  PicmanToolOptionsClass *options_class = PICMAN_TOOL_OPTIONS_CLASS (klass);

  object_class->finalize     = picman_text_options_finalize;
  object_class->set_property = picman_text_options_set_property;
  object_class->get_property = picman_text_options_get_property;

  options_class->reset       = picman_text_options_reset;

  /* The 'highlight' property is defined here because we want different
   * default values for the Crop, Text and the Rectangle Select tools.
   */
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class,
                                    PICMAN_RECTANGLE_OPTIONS_PROP_HIGHLIGHT,
                                    "highlight", NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_UNIT (object_class, PROP_UNIT,
                                 "font-size-unit",
                                 N_("Font size unit"),
                                 TRUE, FALSE, PICMAN_UNIT_PIXEL,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_FONT_SIZE,
                                   "font-size",
                                   N_("Font size"),
                                   0.0, 8192.0, 18.0,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_ANTIALIAS,
                                    "antialias", NULL,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_HINT_STYLE,
                                 "hint-style",
                                 N_("Hinting alters the font outline to "
                                    "produce a crisp bitmap at small "
                                    "sizes"),
                                 PICMAN_TYPE_TEXT_HINT_STYLE,
                                 PICMAN_TEXT_HINT_STYLE_MEDIUM,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_STRING (object_class, PROP_LANGUAGE,
                                   "language",
                                   N_("The text language may have an effect "
                                      "on the way the text is rendered."),
                                   (const gchar *) gtk_get_default_language (),
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_BASE_DIR,
                                "base-direction", NULL,
                                 PICMAN_TYPE_TEXT_DIRECTION,
                                 PICMAN_TEXT_DIRECTION_LTR,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_JUSTIFICATION,
                                "justify",
                                N_("Text alignment"),
                                 PICMAN_TYPE_TEXT_JUSTIFICATION,
                                 PICMAN_TEXT_JUSTIFY_LEFT,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_INDENTATION,
                                   "indent",
                                   N_("Indentation of the first line"),
                                   -8192.0, 8192.0, 0.0,
                                   PICMAN_PARAM_STATIC_STRINGS |
                                   PICMAN_CONFIG_PARAM_DEFAULTS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_LINE_SPACING,
                                   "line-spacing",
                                   N_("Adjust line spacing"),
                                   -8192.0, 8192.0, 0.0,
                                   PICMAN_PARAM_STATIC_STRINGS |
                                   PICMAN_CONFIG_PARAM_DEFAULTS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_LETTER_SPACING,
                                   "letter-spacing",
                                   N_("Adjust letter spacing"),
                                   -8192.0, 8192.0, 0.0,
                                   PICMAN_PARAM_STATIC_STRINGS |
                                   PICMAN_CONFIG_PARAM_DEFAULTS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_BOX_MODE,
                                "box-mode",
                                 N_("Whether text flows into rectangular shape or "
                                    "moves into a new line when you press Enter"),
                                 PICMAN_TYPE_TEXT_BOX_MODE,
                                 PICMAN_TEXT_BOX_DYNAMIC,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_EDITOR,
                                    "use-editor",
                                    N_("Use an external editor window for text "
                                       "entry"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_FONT_VIEW_TYPE,
                                 "font-view-type", NULL,
                                 PICMAN_TYPE_VIEW_TYPE,
                                 PICMAN_VIEW_TYPE_LIST,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_FONT_VIEW_SIZE,
                                "font-view-size", NULL,
                                PICMAN_VIEW_SIZE_TINY,
                                PICMAN_VIEWABLE_MAX_BUTTON_SIZE,
                                PICMAN_VIEW_SIZE_SMALL,
                                PICMAN_PARAM_STATIC_STRINGS);

  picman_rectangle_options_install_properties (object_class);
}

static void
picman_text_options_init (PicmanTextOptions *options)
{
  options->size_entry = NULL;
}

static void
picman_text_options_finalize (GObject *object)
{
  PicmanTextOptions *options = PICMAN_TEXT_OPTIONS (object);

  if (options->language)
    {
      g_free (options->language);
      options->language = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_text_options_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanTextOptions *options = PICMAN_TEXT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_FONT_SIZE:
      g_value_set_double (value, options->font_size);
      break;
    case PROP_UNIT:
      g_value_set_int (value, options->unit);
      break;
    case PROP_ANTIALIAS:
      g_value_set_boolean (value, options->antialias);
      break;
    case PROP_HINT_STYLE:
      g_value_set_enum (value, options->hint_style);
      break;
    case PROP_LANGUAGE:
      g_value_set_string (value, options->language);
      break;
    case PROP_BASE_DIR:
      g_value_set_enum (value, options->base_dir);
      break;
    case PROP_JUSTIFICATION:
      g_value_set_enum (value, options->justify);
      break;
    case PROP_INDENTATION:
      g_value_set_double (value, options->indent);
      break;
    case PROP_LINE_SPACING:
      g_value_set_double (value, options->line_spacing);
      break;
    case PROP_LETTER_SPACING:
      g_value_set_double (value, options->letter_spacing);
      break;
    case PROP_BOX_MODE:
      g_value_set_enum (value, options->box_mode);
      break;

    case PROP_USE_EDITOR:
      g_value_set_boolean (value, options->use_editor);
      break;

    case PROP_FONT_VIEW_TYPE:
      g_value_set_enum (value, options->font_view_type);
      break;
    case PROP_FONT_VIEW_SIZE:
      g_value_set_int (value, options->font_view_size);
      break;

    default:
      picman_rectangle_options_get_property (object, property_id, value, pspec);
      break;
    }
}

static void
picman_text_options_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanTextOptions *options = PICMAN_TEXT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_FONT_SIZE:
      options->font_size = g_value_get_double (value);
      break;
    case PROP_UNIT:
      options->unit = g_value_get_int (value);
      break;
    case PROP_ANTIALIAS:
      options->antialias = g_value_get_boolean (value);
      break;
    case PROP_HINT_STYLE:
      options->hint_style = g_value_get_enum (value);
      break;
    case PROP_BASE_DIR:
      options->base_dir = g_value_get_enum (value);
      break;
    case PROP_LANGUAGE:
      g_free (options->language);
      options->language = g_value_dup_string (value);
      break;
    case PROP_JUSTIFICATION:
      options->justify = g_value_get_enum (value);
      break;
    case PROP_INDENTATION:
      options->indent = g_value_get_double (value);
      break;
    case PROP_LINE_SPACING:
      options->line_spacing = g_value_get_double (value);
      break;
    case PROP_LETTER_SPACING:
      options->letter_spacing = g_value_get_double (value);
      break;
    case PROP_BOX_MODE:
      options->box_mode = g_value_get_enum (value);
      break;

    case PROP_USE_EDITOR:
      options->use_editor = g_value_get_boolean (value);
      break;

    case PROP_FONT_VIEW_TYPE:
      options->font_view_type = g_value_get_enum (value);
      break;
    case PROP_FONT_VIEW_SIZE:
      options->font_view_size = g_value_get_int (value);
      break;

    default:
      picman_rectangle_options_set_property (object, property_id, value, pspec);
      break;
    }
}

static void
picman_text_options_reset (PicmanToolOptions *tool_options)
{
  GObject *object = G_OBJECT (tool_options);

  /*  implement reset() ourselves because the default impl would
   *  reset *all* properties, including all rectangle properties
   *  of the text box
   */

  /* context */
  picman_config_reset_property (object, "font");
  picman_config_reset_property (object, "foreground");

  /* text options */
  picman_config_reset_property (object, "font-size-unit");
  picman_config_reset_property (object, "font-size");
  picman_config_reset_property (object, "antialias");
  picman_config_reset_property (object, "hint-style");
  picman_config_reset_property (object, "language");
  picman_config_reset_property (object, "base-direction");
  picman_config_reset_property (object, "justify");
  picman_config_reset_property (object, "indent");
  picman_config_reset_property (object, "line-spacing");
  picman_config_reset_property (object, "box-mode");
  picman_config_reset_property (object, "use-editor");
}

static void
picman_text_options_notify_font (PicmanContext *context,
                               GParamSpec  *pspec,
                               PicmanText    *text)
{
  g_signal_handlers_block_by_func (text,
                                   picman_text_options_notify_text_font,
                                   context);

  g_object_set (text, "font", picman_context_get_font_name (context), NULL);

  g_signal_handlers_unblock_by_func (text,
                                     picman_text_options_notify_text_font,
                                     context);
}

static void
picman_text_options_notify_text_font (PicmanText    *text,
                                    GParamSpec  *pspec,
                                    PicmanContext *context)
{
  g_signal_handlers_block_by_func (context,
                                   picman_text_options_notify_font, text);

  picman_context_set_font_name (context, text->font);

  g_signal_handlers_unblock_by_func (context,
                                     picman_text_options_notify_font, text);
}

static void
picman_text_options_notify_color (PicmanContext *context,
                                GParamSpec  *pspec,
                                PicmanText    *text)
{
  PicmanRGB  color;

  picman_context_get_foreground (context, &color);

  g_signal_handlers_block_by_func (text,
                                   picman_text_options_notify_text_color,
                                   context);

  g_object_set (text, "color", &color, NULL);

  g_signal_handlers_unblock_by_func (text,
                                     picman_text_options_notify_text_color,
                                     context);
}

static void
picman_text_options_notify_text_color (PicmanText    *text,
                                     GParamSpec  *pspec,
                                     PicmanContext *context)
{
  g_signal_handlers_block_by_func (context,
                                   picman_text_options_notify_color, text);

  picman_context_set_foreground (context, &text->color);

  g_signal_handlers_unblock_by_func (context,
                                     picman_text_options_notify_color, text);
}

/*  This function could live in picmantexttool.c also.
 *  But it takes some bloat out of that file...
 */
void
picman_text_options_connect_text (PicmanTextOptions *options,
                                PicmanText        *text)
{
  PicmanContext *context;
  PicmanRGB      color;

  g_return_if_fail (PICMAN_IS_TEXT_OPTIONS (options));
  g_return_if_fail (PICMAN_IS_TEXT (text));

  context = PICMAN_CONTEXT (options);

  picman_context_get_foreground (context, &color);

  picman_config_sync (G_OBJECT (options), G_OBJECT (text), 0);

  g_object_set (text,
                "color", &color,
                "font",  picman_context_get_font_name (context),
                NULL);

  picman_config_connect (G_OBJECT (options), G_OBJECT (text), NULL);

  g_signal_connect_object (options, "notify::font",
                           G_CALLBACK (picman_text_options_notify_font),
                           text, 0);
  g_signal_connect_object (text, "notify::font",
                           G_CALLBACK (picman_text_options_notify_text_font),
                           options, 0);

  g_signal_connect_object (options, "notify::foreground",
                           G_CALLBACK (picman_text_options_notify_color),
                           text, 0);
  g_signal_connect_object (text, "notify::color",
                           G_CALLBACK (picman_text_options_notify_text_color),
                           options, 0);
}

GtkWidget *
picman_text_options_gui (PicmanToolOptions *tool_options)
{
  GObject         *config    = G_OBJECT (tool_options);
  PicmanTextOptions *options   = PICMAN_TEXT_OPTIONS (tool_options);
  GtkWidget       *main_vbox = picman_tool_options_gui (tool_options);
  GtkWidget       *table;
  GtkWidget       *vbox;
  GtkWidget       *hbox;
  GtkWidget       *button;
  GtkWidget       *entry;
  GtkWidget       *box;
  GtkWidget       *spinbutton;
  GtkWidget       *combo;
  GtkSizeGroup    *size_group;
  gint             row = 0;

  hbox = picman_prop_font_box_new (NULL, PICMAN_CONTEXT (tool_options),
                                 _("Font"), 2,
                                 "font-view-type", "font-view-size");
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  table = gtk_table_new (1, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  entry = picman_prop_size_entry_new (config,
                                    "font-size", FALSE, "font-size-unit", "%p",
                                    PICMAN_SIZE_ENTRY_UPDATE_SIZE, 72.0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("Size:"), 0.0, 0.5,
                             entry, 2, FALSE);

  options->size_entry = entry;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  button = picman_prop_check_button_new (config, "use-editor", _("Use editor"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = picman_prop_check_button_new (config, "antialias", _("Antialiasing"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  table = gtk_table_new (6, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  row = 0;

  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  button = picman_prop_enum_combo_box_new (config, "hint-style", -1, -1);
  picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("Hinting:"), 0.0, 0.5,
                             button, 1, TRUE);
  gtk_size_group_add_widget (size_group, button);

  button = picman_prop_color_button_new (config, "foreground", _("Text Color"),
                                       40, 24, PICMAN_COLOR_AREA_FLAT);
  picman_color_panel_set_context (PICMAN_COLOR_PANEL (button),
                                PICMAN_CONTEXT (options));
  picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("Color:"), 0.0, 0.5,
                             button, 1, TRUE);
  gtk_size_group_add_widget (size_group, button);

  box = picman_prop_enum_stock_box_new (config, "justify", "gtk-justify", 0, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("Justify:"), 0.0, 0.5,
                             box, 2, TRUE);
  gtk_size_group_add_widget (size_group, box);
  g_object_unref (size_group);

  spinbutton = picman_prop_spin_button_new (config, "indent", 1.0, 10.0, 1);
  gtk_entry_set_width_chars (GTK_ENTRY (spinbutton), 5);
  picman_table_attach_stock (GTK_TABLE (table), row++,
                           GTK_STOCK_INDENT, spinbutton, 1, TRUE);

  spinbutton = picman_prop_spin_button_new (config, "line-spacing", 1.0, 10.0, 1);
  gtk_entry_set_width_chars (GTK_ENTRY (spinbutton), 5);
  picman_table_attach_stock (GTK_TABLE (table), row++,
                           PICMAN_STOCK_LINE_SPACING, spinbutton, 1, TRUE);

  spinbutton = picman_prop_spin_button_new (config,
                                          "letter-spacing", 1.0, 10.0, 1);
  gtk_entry_set_width_chars (GTK_ENTRY (spinbutton), 5);
  picman_table_attach_stock (GTK_TABLE (table), row++,
                           PICMAN_STOCK_LETTER_SPACING, spinbutton, 1, TRUE);

  combo = picman_prop_enum_combo_box_new (config, "box-mode", 0, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("Box:"), 0.0, 0.5,
                             combo, 1, TRUE);

  /*  Only add the language entry if the iso-codes package is available.  */

#ifdef HAVE_ISO_CODES
  {
    GtkWidget *label;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_pack_start (GTK_BOX (main_vbox), vbox, FALSE, FALSE, 0);
    gtk_widget_show (vbox);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (hbox);

    label = gtk_label_new (_("Language:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    entry = picman_prop_language_entry_new (config, "language");
    gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
    gtk_widget_show (entry);
  }
#endif

  return main_vbox;
}

static void
picman_text_options_editor_dir_changed (PicmanTextEditor  *editor,
                                      PicmanTextOptions *options)
{
  g_object_set (options,
                "base-direction", editor->base_dir,
                NULL);
}

static void
picman_text_options_editor_notify_dir (PicmanTextOptions *options,
                                     GParamSpec      *pspec,
                                     PicmanTextEditor  *editor)
{
  PicmanTextDirection  dir;

  g_object_get (options,
                "base-direction", &dir,
                NULL);

  picman_text_editor_set_direction (editor, dir);
}

static void
picman_text_options_editor_notify_font (PicmanTextOptions *options,
                                      GParamSpec      *pspec,
                                      PicmanTextEditor  *editor)
{
  const gchar *font_name;

  font_name = picman_context_get_font_name (PICMAN_CONTEXT (options));

  picman_text_editor_set_font_name (editor, font_name);
}

GtkWidget *
picman_text_options_editor_new (GtkWindow       *parent,
                              Picman            *picman,
                              PicmanTextOptions *options,
                              PicmanMenuFactory *menu_factory,
                              const gchar     *title,
                              PicmanText        *text,
                              PicmanTextBuffer  *text_buffer,
                              gdouble          xres,
                              gdouble          yres)
{
  GtkWidget   *editor;
  const gchar *font_name;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT_OPTIONS (options), NULL);
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);
  g_return_val_if_fail (title != NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT (text), NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT_BUFFER (text_buffer), NULL);

  editor = picman_text_editor_new (title, parent, picman, menu_factory,
                                 text, text_buffer, xres, yres);

  font_name = picman_context_get_font_name (PICMAN_CONTEXT (options));

  picman_text_editor_set_direction (PICMAN_TEXT_EDITOR (editor),
                                  options->base_dir);
  picman_text_editor_set_font_name (PICMAN_TEXT_EDITOR (editor),
                                  font_name);

  g_signal_connect_object (editor, "dir-changed",
                           G_CALLBACK (picman_text_options_editor_dir_changed),
                           options, 0);
  g_signal_connect_object (options, "notify::base-direction",
                           G_CALLBACK (picman_text_options_editor_notify_dir),
                           editor, 0);
  g_signal_connect_object (options, "notify::font",
                           G_CALLBACK (picman_text_options_editor_notify_font),
                           editor, 0);

  return editor;
}
