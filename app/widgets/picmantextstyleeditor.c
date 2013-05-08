/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextStyleEditor
 * Copyright (C) 2010  Michael Natterer <mitch@picman.org>
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
#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"

#include "text/picmanfontlist.h"
#include "text/picmantext.h"

#include "picmancolorpanel.h"
#include "picmancontainerentry.h"
#include "picmancontainerview.h"
#include "picmantextbuffer.h"
#include "picmantextstyleeditor.h"
#include "picmantexttag.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_PICMAN,
  PROP_TEXT,
  PROP_BUFFER,
  PROP_FONTS,
  PROP_RESOLUTION_X,
  PROP_RESOLUTION_Y
};


static void      picman_text_style_editor_constructed       (GObject             *object);
static void      picman_text_style_editor_dispose           (GObject             *object);
static void      picman_text_style_editor_finalize          (GObject             *object);
static void      picman_text_style_editor_set_property      (GObject             *object,
                                                           guint                property_id,
                                                           const GValue        *value,
                                                           GParamSpec          *pspec);
static void      picman_text_style_editor_get_property      (GObject             *object,
                                                           guint                property_id,
                                                           GValue              *value,
                                                           GParamSpec          *pspec);

static GtkWidget * picman_text_style_editor_create_toggle   (PicmanTextStyleEditor *editor,
                                                           GtkTextTag          *tag,
                                                           const gchar         *stock_id,
                                                           const gchar         *tooltip);

static void      picman_text_style_editor_clear_tags        (GtkButton           *button,
                                                           PicmanTextStyleEditor *editor);

static void      picman_text_style_editor_font_changed      (PicmanContext         *context,
                                                           PicmanFont            *font,
                                                           PicmanTextStyleEditor *editor);
static void      picman_text_style_editor_set_font          (PicmanTextStyleEditor *editor,
                                                           GtkTextTag          *font_tag);
static void      picman_text_style_editor_set_default_font  (PicmanTextStyleEditor *editor);

static void      picman_text_style_editor_color_changed     (PicmanColorButton     *button,
                                                           PicmanTextStyleEditor *editor);
static void      picman_text_style_editor_set_color         (PicmanTextStyleEditor *editor,
                                                           GtkTextTag          *color_tag);
static void      picman_text_style_editor_set_default_color (PicmanTextStyleEditor *editor);

static void      picman_text_style_editor_tag_toggled       (GtkToggleButton     *toggle,
                                                           PicmanTextStyleEditor *editor);
static void      picman_text_style_editor_set_toggle        (PicmanTextStyleEditor *editor,
                                                           GtkToggleButton     *toggle,
                                                           gboolean             active);

static void      picman_text_style_editor_size_changed      (PicmanSizeEntry       *entry,
                                                           PicmanTextStyleEditor *editor);
static void      picman_text_style_editor_set_size          (PicmanTextStyleEditor *editor,
                                                           GtkTextTag          *size_tag);
static void      picman_text_style_editor_set_default_size  (PicmanTextStyleEditor *editor);

static void      picman_text_style_editor_baseline_changed  (GtkAdjustment       *adjustment,
                                                           PicmanTextStyleEditor *editor);
static void      picman_text_style_editor_set_baseline      (PicmanTextStyleEditor *editor,
                                                           GtkTextTag          *baseline_tag);

static void      picman_text_style_editor_kerning_changed   (GtkAdjustment       *adjustment,
                                                           PicmanTextStyleEditor *editor);
static void      picman_text_style_editor_set_kerning       (PicmanTextStyleEditor *editor,
                                                           GtkTextTag          *kerning_tag);

static void      picman_text_style_editor_update            (PicmanTextStyleEditor *editor);
static gboolean  picman_text_style_editor_update_idle       (PicmanTextStyleEditor *editor);


G_DEFINE_TYPE (PicmanTextStyleEditor, picman_text_style_editor,
               GTK_TYPE_BOX)

#define parent_class picman_text_style_editor_parent_class


static void
picman_text_style_editor_class_init (PicmanTextStyleEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_text_style_editor_constructed;
  object_class->dispose      = picman_text_style_editor_dispose;
  object_class->finalize     = picman_text_style_editor_finalize;
  object_class->set_property = picman_text_style_editor_set_property;
  object_class->get_property = picman_text_style_editor_get_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_TEXT,
                                   g_param_spec_object ("text",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_TEXT,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_BUFFER,
                                   g_param_spec_object ("buffer",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_TEXT_BUFFER,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_FONTS,
                                   g_param_spec_object ("fonts",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_FONT_LIST,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_RESOLUTION_X,
                                   g_param_spec_double ("resolution-x",
                                                        NULL, NULL,
                                                        PICMAN_MIN_RESOLUTION,
                                                        PICMAN_MAX_RESOLUTION,
                                                        1.0,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_RESOLUTION_Y,
                                   g_param_spec_double ("resolution-y",
                                                        NULL, NULL,
                                                        PICMAN_MIN_RESOLUTION,
                                                        PICMAN_MAX_RESOLUTION,
                                                        1.0,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
}

static void
picman_text_style_editor_init (PicmanTextStyleEditor *editor)
{
  GtkWidget *image;
  PicmanRGB    color;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (editor),
                                  GTK_ORIENTATION_VERTICAL);
  gtk_box_set_spacing (GTK_BOX (editor), 2);

  /*  upper row  */

  editor->upper_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (editor), editor->upper_hbox, FALSE, FALSE, 0);
  gtk_widget_show (editor->upper_hbox);

  editor->font_entry = picman_container_entry_new (NULL, NULL,
                                                 PICMAN_VIEW_SIZE_SMALL, 1);
  gtk_box_pack_start (GTK_BOX (editor->upper_hbox), editor->font_entry,
                      FALSE, FALSE, 0);
  gtk_widget_show (editor->font_entry);

  picman_help_set_help_data (editor->font_entry,
                           _("Change font of selected text"), NULL);

  editor->size_entry =
    picman_size_entry_new (1, 0, "%a", TRUE, FALSE, FALSE, 10,
                         PICMAN_SIZE_ENTRY_UPDATE_SIZE);
  gtk_table_set_col_spacing (GTK_TABLE (editor->size_entry), 1, 0);
  gtk_box_pack_start (GTK_BOX (editor->upper_hbox), editor->size_entry,
                      FALSE, FALSE, 0);
  gtk_widget_show (editor->size_entry);

  picman_help_set_help_data (editor->size_entry,
                           _("Change size of selected text"), NULL);

  g_signal_connect (editor->size_entry, "value-changed",
                    G_CALLBACK (picman_text_style_editor_size_changed),
                    editor);

  /*  lower row  */

  editor->lower_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (editor), editor->lower_hbox, FALSE, FALSE, 0);
  gtk_widget_show (editor->lower_hbox);

  editor->clear_button = gtk_button_new ();
  gtk_widget_set_can_focus (editor->clear_button, FALSE);
  gtk_box_pack_start (GTK_BOX (editor->lower_hbox), editor->clear_button,
                      FALSE, FALSE, 0);
  gtk_widget_show (editor->clear_button);

  picman_help_set_help_data (editor->clear_button,
                           _("Clear style of selected text"), NULL);

  g_signal_connect (editor->clear_button, "clicked",
                    G_CALLBACK (picman_text_style_editor_clear_tags),
                    editor);

  image = gtk_image_new_from_stock (GTK_STOCK_CLEAR, GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (editor->clear_button), image);
  gtk_widget_show (image);

  picman_rgba_set (&color, 0.0, 0.0, 0.0, 1.0);
  editor->color_button = picman_color_panel_new (_("Change color of selected text"),
                                               &color,
                                               PICMAN_COLOR_AREA_FLAT, 20, 20);

  gtk_box_pack_end (GTK_BOX (editor->lower_hbox), editor->color_button,
                    FALSE, FALSE, 0);
  gtk_widget_show (editor->color_button);

  picman_help_set_help_data (editor->color_button,
                           _("Change color of selected text"), NULL);

  g_signal_connect (editor->color_button, "color-changed",
                    G_CALLBACK (picman_text_style_editor_color_changed),
                    editor);

  editor->kerning_adjustment =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, -1000.0, 1000.0, 1.0, 10.0, 0.0));
  editor->kerning_spinbutton = gtk_spin_button_new (editor->kerning_adjustment,
                                                    1.0, 1);
  gtk_entry_set_width_chars (GTK_ENTRY (editor->kerning_spinbutton), 5);
  gtk_box_pack_end (GTK_BOX (editor->lower_hbox), editor->kerning_spinbutton,
                    FALSE, FALSE, 0);
  gtk_widget_show (editor->kerning_spinbutton);

  picman_help_set_help_data (editor->kerning_spinbutton,
                           _("Change kerning of selected text"), NULL);

  g_signal_connect (editor->kerning_adjustment, "value-changed",
                    G_CALLBACK (picman_text_style_editor_kerning_changed),
                    editor);

  editor->baseline_adjustment =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, -1000.0, 1000.0, 1.0, 10.0, 0.0));
  editor->baseline_spinbutton = gtk_spin_button_new (editor->baseline_adjustment,
                                                     1.0, 1);
  gtk_entry_set_width_chars (GTK_ENTRY (editor->baseline_spinbutton), 5);
  gtk_box_pack_end (GTK_BOX (editor->lower_hbox), editor->baseline_spinbutton,
                    FALSE, FALSE, 0);
  gtk_widget_show (editor->baseline_spinbutton);

  picman_help_set_help_data (editor->baseline_spinbutton,
                           _("Change baseline of selected text"), NULL);

  g_signal_connect (editor->baseline_adjustment, "value-changed",
                    G_CALLBACK (picman_text_style_editor_baseline_changed),
                    editor);
}

static void
picman_text_style_editor_constructed (GObject *object)
{
  PicmanTextStyleEditor *editor = PICMAN_TEXT_STYLE_EDITOR (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (editor->picman));
  g_assert (PICMAN_IS_FONT_LIST (editor->fonts));
  g_assert (PICMAN_IS_TEXT (editor->text));
  g_assert (PICMAN_IS_TEXT_BUFFER (editor->buffer));

  editor->context = picman_context_new (editor->picman, "text style editor", NULL);

  g_signal_connect (editor->context, "font-changed",
                    G_CALLBACK (picman_text_style_editor_font_changed),
                    editor);

  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (editor->size_entry), 0,
                                  editor->resolution_y, TRUE);

  /* use the global user context so we get the global FG/BG colors */
  picman_color_panel_set_context (PICMAN_COLOR_PANEL (editor->color_button),
                                picman_get_user_context (editor->picman));

  picman_container_view_set_container (PICMAN_CONTAINER_VIEW (editor->font_entry),
                                     editor->fonts);
  picman_container_view_set_context (PICMAN_CONTAINER_VIEW (editor->font_entry),
                                   editor->context);

  picman_text_style_editor_create_toggle (editor, editor->buffer->bold_tag,
                                        GTK_STOCK_BOLD,
                                        _("Bold"));
  picman_text_style_editor_create_toggle (editor, editor->buffer->italic_tag,
                                        GTK_STOCK_ITALIC,
                                        _("Italic"));
  picman_text_style_editor_create_toggle (editor, editor->buffer->underline_tag,
                                        GTK_STOCK_UNDERLINE,
                                        _("Underline"));
  picman_text_style_editor_create_toggle (editor, editor->buffer->strikethrough_tag,
                                        GTK_STOCK_STRIKETHROUGH,
                                        _("Strikethrough"));

  g_signal_connect_swapped (editor->text, "notify::font",
                            G_CALLBACK (picman_text_style_editor_update),
                            editor);
  g_signal_connect_swapped (editor->text, "notify::font-size",
                            G_CALLBACK (picman_text_style_editor_update),
                            editor);
  g_signal_connect_swapped (editor->text, "notify::font-size-unit",
                            G_CALLBACK (picman_text_style_editor_update),
                            editor);
  g_signal_connect_swapped (editor->text, "notify::color",
                            G_CALLBACK (picman_text_style_editor_update),
                            editor);

  g_signal_connect_data (editor->buffer, "changed",
                         G_CALLBACK (picman_text_style_editor_update),
                         editor, 0,
                         G_CONNECT_AFTER | G_CONNECT_SWAPPED);
  g_signal_connect_data (editor->buffer, "apply-tag",
                         G_CALLBACK (picman_text_style_editor_update),
                         editor, 0,
                         G_CONNECT_AFTER | G_CONNECT_SWAPPED);
  g_signal_connect_data (editor->buffer, "remove-tag",
                         G_CALLBACK (picman_text_style_editor_update),
                         editor, 0,
                         G_CONNECT_AFTER | G_CONNECT_SWAPPED);
  g_signal_connect_data (editor->buffer, "mark-set",
                         G_CALLBACK (picman_text_style_editor_update),
                         editor, 0,
                         G_CONNECT_AFTER | G_CONNECT_SWAPPED);

  picman_text_style_editor_update (editor);
}

static void
picman_text_style_editor_dispose (GObject *object)
{
  PicmanTextStyleEditor *editor = PICMAN_TEXT_STYLE_EDITOR (object);

  if (editor->text)
    {
      g_signal_handlers_disconnect_by_func (editor->text,
                                            picman_text_style_editor_update,
                                            editor);
    }

  if (editor->buffer)
    {
      g_signal_handlers_disconnect_by_func (editor->buffer,
                                            picman_text_style_editor_update,
                                            editor);
    }

  if (editor->update_idle_id)
    {
      g_source_remove (editor->update_idle_id);
      editor->update_idle_id = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_text_style_editor_finalize (GObject *object)
{
  PicmanTextStyleEditor *editor = PICMAN_TEXT_STYLE_EDITOR (object);

  if (editor->context)
    {
      g_object_unref (editor->context);
      editor->context = NULL;
    }

  if (editor->text)
    {
      g_object_unref (editor->text);
      editor->text = NULL;
    }

  if (editor->buffer)
    {
      g_object_unref (editor->buffer);
      editor->buffer = NULL;
    }

  if (editor->fonts)
    {
      g_object_unref (editor->fonts);
      editor->fonts = NULL;
    }

  if (editor->toggles)
    {
      g_list_free (editor->toggles);
      editor->toggles = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_text_style_editor_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PicmanTextStyleEditor *editor = PICMAN_TEXT_STYLE_EDITOR (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      editor->picman = g_value_get_object (value); /* don't ref */
      break;
    case PROP_TEXT:
      editor->text = g_value_dup_object (value);
      break;
    case PROP_BUFFER:
      editor->buffer = g_value_dup_object (value);
      break;
    case PROP_FONTS:
      editor->fonts = g_value_dup_object (value);
      break;
    case PROP_RESOLUTION_X:
      editor->resolution_x = g_value_get_double (value);
      break;
    case PROP_RESOLUTION_Y:
      editor->resolution_y = g_value_get_double (value);
      if (editor->size_entry)
        picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (editor->size_entry), 0,
                                        editor->resolution_y, TRUE);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_text_style_editor_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PicmanTextStyleEditor *editor = PICMAN_TEXT_STYLE_EDITOR (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, editor->picman);
      break;
    case PROP_TEXT:
      g_value_set_object (value, editor->text);
      break;
    case PROP_BUFFER:
      g_value_set_object (value, editor->buffer);
      break;
    case PROP_FONTS:
      g_value_set_object (value, editor->fonts);
      break;
    case PROP_RESOLUTION_X:
      g_value_set_double (value, editor->resolution_x);
      break;
    case PROP_RESOLUTION_Y:
      g_value_set_double (value, editor->resolution_y);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  public functions  */

GtkWidget *
picman_text_style_editor_new (Picman           *picman,
                            PicmanText       *text,
                            PicmanTextBuffer *buffer,
                            PicmanContainer  *fonts,
                            gdouble         resolution_x,
                            gdouble         resolution_y)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT (text), NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT_BUFFER (buffer), NULL);
  g_return_val_if_fail (resolution_x > 0.0, NULL);
  g_return_val_if_fail (resolution_y > 0.0, NULL);

  return g_object_new (PICMAN_TYPE_TEXT_STYLE_EDITOR,
                       "picman",         picman,
                       "text",         text,
                       "buffer",       buffer,
                       "fonts",        fonts,
                       "resolution-x", resolution_x,
                       "resolution-y", resolution_y,
                       NULL);
}

GList *
picman_text_style_editor_list_tags (PicmanTextStyleEditor  *editor,
                                  GList               **remove_tags)
{
  GList *toggles;
  GList *tags = NULL;

  g_return_val_if_fail (PICMAN_IS_TEXT_STYLE_EDITOR (editor), NULL);
  g_return_val_if_fail (remove_tags != NULL, NULL);

  *remove_tags = NULL;

  for (toggles = editor->toggles; toggles; toggles = g_list_next (toggles))
    {
      GtkTextTag *tag = g_object_get_data (toggles->data, "tag");

      if (gtk_toggle_button_get_active (toggles->data))
        {
          tags = g_list_prepend (tags, tag);
        }
      else
        {
          *remove_tags = g_list_prepend (*remove_tags, tag);
        }
    }

  {
    GList   *list;
    gdouble  pixels;

    for (list = editor->buffer->size_tags; list; list = g_list_next (list))
      *remove_tags = g_list_prepend (*remove_tags, list->data);

    pixels = picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (editor->size_entry), 0);

    if (pixels != 0.0)
      {
        GtkTextTag *tag;
        gdouble     points;

        points = picman_units_to_points (pixels,
                                       PICMAN_UNIT_PIXEL,
                                       editor->resolution_y);
        tag = picman_text_buffer_get_size_tag (editor->buffer,
                                             PANGO_SCALE * points);
        tags = g_list_prepend (tags, tag);
      }
  }

  {
    GList       *list;
    const gchar *font_name;

    for (list = editor->buffer->font_tags; list; list = g_list_next (list))
      *remove_tags = g_list_prepend (*remove_tags, list->data);

    font_name = picman_context_get_font_name (editor->context);

    if (font_name)
      {
        GtkTextTag  *tag;

        tag = picman_text_buffer_get_font_tag (editor->buffer, font_name);
        tags = g_list_prepend (tags, tag);
      }
  }

  {
    GList   *list;
    PicmanRGB  color;

    for (list = editor->buffer->color_tags; list; list = g_list_next (list))
      *remove_tags = g_list_prepend (*remove_tags, list->data);

    picman_color_button_get_color (PICMAN_COLOR_BUTTON (editor->color_button),
                                 &color);

    if (TRUE) /* FIXME should have "inconsistent" state as for font and size */
      {
        GtkTextTag *tag;

        tag = picman_text_buffer_get_color_tag (editor->buffer, &color);
        tags = g_list_prepend (tags, tag);
      }
  }

  *remove_tags = g_list_reverse (*remove_tags);

  return g_list_reverse (tags);
}


/*  private functions  */

static GtkWidget *
picman_text_style_editor_create_toggle (PicmanTextStyleEditor *editor,
                                      GtkTextTag          *tag,
                                      const gchar         *stock_id,
                                      const gchar         *tooltip)
{
  GtkWidget *toggle;
  GtkWidget *image;

  toggle = gtk_toggle_button_new ();
  gtk_widget_set_can_focus (toggle, FALSE);
  gtk_box_pack_start (GTK_BOX (editor->lower_hbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  picman_help_set_help_data (toggle, tooltip, NULL);

  editor->toggles = g_list_append (editor->toggles, toggle);
  g_object_set_data (G_OBJECT (toggle), "tag", tag);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_text_style_editor_tag_toggled),
                    editor);

  image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (toggle), image);
  gtk_widget_show (image);

  return toggle;
}

static void
picman_text_style_editor_clear_tags (GtkButton           *button,
                                   PicmanTextStyleEditor *editor)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (editor->buffer);

  if (gtk_text_buffer_get_has_selection (buffer))
    {
      GtkTextIter start, end;

      gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

      gtk_text_buffer_begin_user_action (buffer);

      gtk_text_buffer_remove_all_tags (buffer, &start, &end);

      gtk_text_buffer_end_user_action (buffer);
    }
}

static void
picman_text_style_editor_font_changed (PicmanContext         *context,
                                     PicmanFont            *font,
                                     PicmanTextStyleEditor *editor)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (editor->buffer);
  GList         *insert_tags;
  GList         *remove_tags;

  if (gtk_text_buffer_get_has_selection (buffer))
    {
      GtkTextIter start, end;

      gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

      picman_text_buffer_set_font (editor->buffer, &start, &end,
                                 picman_context_get_font_name (context));
    }

  insert_tags = picman_text_style_editor_list_tags (editor, &remove_tags);
  picman_text_buffer_set_insert_tags (editor->buffer, insert_tags, remove_tags);
}

static void
picman_text_style_editor_set_font (PicmanTextStyleEditor *editor,
                                 GtkTextTag          *font_tag)
{
  gchar *font = NULL;

  if (font_tag)
    font = picman_text_tag_get_font (font_tag);

  g_signal_handlers_block_by_func (editor->context,
                                   picman_text_style_editor_font_changed,
                                   editor);

  picman_context_set_font_name (editor->context, font);

  g_signal_handlers_unblock_by_func (editor->context,
                                     picman_text_style_editor_font_changed,
                                     editor);

  g_free (font);
}

static void
picman_text_style_editor_set_default_font (PicmanTextStyleEditor *editor)
{
  g_signal_handlers_block_by_func (editor->context,
                                   picman_text_style_editor_font_changed,
                                   editor);

  picman_context_set_font_name (editor->context, editor->text->font);

  g_signal_handlers_unblock_by_func (editor->context,
                                     picman_text_style_editor_font_changed,
                                     editor);
}

static void
picman_text_style_editor_color_changed (PicmanColorButton     *button,
                                      PicmanTextStyleEditor *editor)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (editor->buffer);
  GList         *insert_tags;
  GList         *remove_tags;

  if (gtk_text_buffer_get_has_selection (buffer))
    {
      GtkTextIter start, end;
      PicmanRGB     color;

      gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

      picman_color_button_get_color (button, &color);
      picman_text_buffer_set_color (editor->buffer, &start, &end, &color);
    }

  insert_tags = picman_text_style_editor_list_tags (editor, &remove_tags);
  picman_text_buffer_set_insert_tags (editor->buffer, insert_tags, remove_tags);
}

static void
picman_text_style_editor_set_color (PicmanTextStyleEditor *editor,
                                  GtkTextTag          *color_tag)
{
  PicmanRGB color;

  picman_rgba_set (&color, 0.0, 0.0, 0.0, 1.0);

  if (color_tag)
    picman_text_tag_get_color (color_tag, &color);

  g_signal_handlers_block_by_func (editor->color_button,
                                   picman_text_style_editor_color_changed,
                                   editor);

  picman_color_button_set_color (PICMAN_COLOR_BUTTON (editor->color_button),
                               &color);

  /* FIXME should have "inconsistent" state as for font and size */

  g_signal_handlers_unblock_by_func (editor->color_button,
                                     picman_text_style_editor_color_changed,
                                     editor);
}

static void
picman_text_style_editor_set_default_color (PicmanTextStyleEditor *editor)
{
  g_signal_handlers_block_by_func (editor->color_button,
                                   picman_text_style_editor_color_changed,
                                   editor);

  picman_color_button_set_color (PICMAN_COLOR_BUTTON (editor->color_button),
                               &editor->text->color);

  g_signal_handlers_unblock_by_func (editor->color_button,
                                     picman_text_style_editor_color_changed,
                                     editor);
}

static void
picman_text_style_editor_tag_toggled (GtkToggleButton     *toggle,
                                    PicmanTextStyleEditor *editor)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (editor->buffer);
  GtkTextTag    *tag    = g_object_get_data (G_OBJECT (toggle), "tag");
  GList         *insert_tags;
  GList         *remove_tags;

  if (gtk_text_buffer_get_has_selection (buffer))
    {
      GtkTextIter start, end;

      gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

      gtk_text_buffer_begin_user_action (buffer);

      if (gtk_toggle_button_get_active (toggle))
        {
          gtk_text_buffer_apply_tag (buffer, tag, &start, &end);
        }
      else
        {
          gtk_text_buffer_remove_tag (buffer, tag, &start, &end);
        }

      gtk_text_buffer_end_user_action (buffer);
    }

  insert_tags = picman_text_style_editor_list_tags (editor, &remove_tags);
  picman_text_buffer_set_insert_tags (editor->buffer, insert_tags, remove_tags);
}

static void
picman_text_style_editor_set_toggle (PicmanTextStyleEditor *editor,
                                   GtkToggleButton     *toggle,
                                   gboolean             active)
{
  g_signal_handlers_block_by_func (toggle,
                                   picman_text_style_editor_tag_toggled,
                                   editor);

  gtk_toggle_button_set_active (toggle, active);

  g_signal_handlers_unblock_by_func (toggle,
                                     picman_text_style_editor_tag_toggled,
                                     editor);
}

static void
picman_text_style_editor_size_changed (PicmanSizeEntry       *entry,
                                     PicmanTextStyleEditor *editor)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (editor->buffer);
  GList         *insert_tags;
  GList         *remove_tags;

  if (gtk_text_buffer_get_has_selection (buffer))
    {
      GtkTextIter start, end;
      gdouble     points;

      gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

      points = picman_units_to_points (picman_size_entry_get_refval (entry, 0),
                                     PICMAN_UNIT_PIXEL,
                                     editor->resolution_y);

      picman_text_buffer_set_size (editor->buffer, &start, &end,
                                 PANGO_SCALE * points);
    }

  insert_tags = picman_text_style_editor_list_tags (editor, &remove_tags);
  picman_text_buffer_set_insert_tags (editor->buffer, insert_tags, remove_tags);
}

static void
picman_text_style_editor_set_size (PicmanTextStyleEditor *editor,
                                 GtkTextTag          *size_tag)
{
  gint    size = 0;
  gdouble pixels;

  if (size_tag)
    size = picman_text_tag_get_size (size_tag);

  g_signal_handlers_block_by_func (editor->size_entry,
                                   picman_text_style_editor_size_changed,
                                   editor);

  pixels = picman_units_to_pixels ((gdouble) size / PANGO_SCALE,
                                 PICMAN_UNIT_POINT,
                                 editor->resolution_y);
  picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (editor->size_entry), 0, pixels);

  if (size == 0)
    {
      GtkWidget *spinbutton;

      spinbutton = picman_size_entry_get_help_widget (PICMAN_SIZE_ENTRY (editor->size_entry), 0);

      gtk_entry_set_text (GTK_ENTRY (spinbutton), "");
    }

  g_signal_handlers_unblock_by_func (editor->size_entry,
                                     picman_text_style_editor_size_changed,
                                     editor);
}

static void
picman_text_style_editor_set_default_size (PicmanTextStyleEditor *editor)
{
  gdouble pixels = picman_units_to_pixels (editor->text->font_size,
                                         editor->text->unit,
                                         editor->resolution_y);

  g_signal_handlers_block_by_func (editor->size_entry,
                                   picman_text_style_editor_size_changed,
                                   editor);

  picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (editor->size_entry), 0, pixels);

  g_signal_handlers_unblock_by_func (editor->size_entry,
                                     picman_text_style_editor_size_changed,
                                     editor);
}

static void
picman_text_style_editor_baseline_changed (GtkAdjustment       *adjustment,
                                         PicmanTextStyleEditor *editor)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (editor->buffer);
  GtkTextIter    start, end;

  if (! gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
      gtk_text_buffer_get_iter_at_mark (buffer, &start,
                                        gtk_text_buffer_get_insert (buffer));
      gtk_text_buffer_get_end_iter (buffer, &end);
    }

  picman_text_buffer_set_baseline (editor->buffer, &start, &end,
                                 gtk_adjustment_get_value (adjustment) *
                                 PANGO_SCALE);
}

static void
picman_text_style_editor_set_baseline (PicmanTextStyleEditor *editor,
                                     GtkTextTag          *baseline_tag)
{
  gint baseline = 0;

  if (baseline_tag)
    baseline = picman_text_tag_get_baseline (baseline_tag);

  g_signal_handlers_block_by_func (editor->baseline_adjustment,
                                   picman_text_style_editor_baseline_changed,
                                   editor);

  gtk_adjustment_set_value (editor->baseline_adjustment,
                            (gdouble) baseline / PANGO_SCALE);
  /* make sure the "" really gets replaced */
  gtk_adjustment_value_changed (editor->baseline_adjustment);

  g_signal_handlers_unblock_by_func (editor->baseline_adjustment,
                                     picman_text_style_editor_baseline_changed,
                                     editor);
}

static void
picman_text_style_editor_kerning_changed (GtkAdjustment       *adjustment,
                                        PicmanTextStyleEditor *editor)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (editor->buffer);
  GtkTextIter    start, end;

  if (! gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
      gtk_text_buffer_get_iter_at_mark (buffer, &start,
                                        gtk_text_buffer_get_insert (buffer));
      end = start;
      gtk_text_iter_forward_char (&end);
    }

  picman_text_buffer_set_kerning (editor->buffer, &start, &end,
                                gtk_adjustment_get_value (adjustment) *
                                PANGO_SCALE);
}

static void
picman_text_style_editor_set_kerning (PicmanTextStyleEditor *editor,
                                    GtkTextTag          *kerning_tag)
{
  gint kerning = 0;

  if (kerning_tag)
    kerning = picman_text_tag_get_kerning (kerning_tag);

  g_signal_handlers_block_by_func (editor->kerning_adjustment,
                                   picman_text_style_editor_kerning_changed,
                                   editor);

  gtk_adjustment_set_value (editor->kerning_adjustment,
                            (gdouble) kerning / PANGO_SCALE);
  /* make sure the "" really gets replaced */
  gtk_adjustment_value_changed (editor->kerning_adjustment);

  g_signal_handlers_unblock_by_func (editor->kerning_adjustment,
                                     picman_text_style_editor_kerning_changed,
                                     editor);
}

static void
picman_text_style_editor_update (PicmanTextStyleEditor *editor)
{
  if (editor->update_idle_id)
    g_source_remove (editor->update_idle_id);

  editor->update_idle_id =
    gdk_threads_add_idle ((GSourceFunc) picman_text_style_editor_update_idle,
                          editor);
}

static gboolean
picman_text_style_editor_update_idle (PicmanTextStyleEditor *editor)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (editor->buffer);

  if (editor->update_idle_id)
    {
      g_source_remove (editor->update_idle_id);
      editor->update_idle_id = 0;
    }

  if (gtk_text_buffer_get_has_selection (buffer))
    {
      GtkTextIter  start, end;
      GtkTextIter  iter;
      GList       *list;
      gboolean     any_toggle_active = TRUE;
      gboolean     font_differs      = FALSE;
      gboolean     color_differs     = FALSE;
      gboolean     size_differs      = FALSE;
      gboolean     baseline_differs  = FALSE;
      gboolean     kerning_differs   = FALSE;
      GtkTextTag  *font_tag          = NULL;
      GtkTextTag  *color_tag         = NULL;
      GtkTextTag  *size_tag          = NULL;
      GtkTextTag  *baseline_tag      = NULL;
      GtkTextTag  *kerning_tag       = NULL;

      gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
      gtk_text_iter_order (&start, &end);

      /*  first, switch all toggles on  */
      for (list = editor->toggles; list; list = g_list_next (list))
        {
          GtkToggleButton *toggle = list->data;

          picman_text_style_editor_set_toggle (editor, toggle, TRUE);
        }

      /*  and get some initial values  */
      font_tag     = picman_text_buffer_get_iter_font (editor->buffer,
                                                     &start, NULL);
      color_tag    = picman_text_buffer_get_iter_color (editor->buffer,
                                                      &start, NULL);
      size_tag     = picman_text_buffer_get_iter_size (editor->buffer,
                                                     &start, NULL);
      baseline_tag = picman_text_buffer_get_iter_baseline (editor->buffer,
                                                         &start, NULL);
      kerning_tag  = picman_text_buffer_get_iter_kerning (editor->buffer,
                                                        &start, NULL);

      for (iter = start;
           gtk_text_iter_in_range (&iter, &start, &end);
           gtk_text_iter_forward_cursor_position (&iter))
        {
          if (any_toggle_active)
            {
              any_toggle_active = FALSE;

              for (list = editor->toggles; list; list = g_list_next (list))
                {
                  GtkToggleButton *toggle = list->data;
                  GtkTextTag      *tag    = g_object_get_data (G_OBJECT (toggle),
                                                               "tag");

                  if (! gtk_text_iter_has_tag (&iter, tag))
                    {
                      picman_text_style_editor_set_toggle (editor, toggle, FALSE);
                    }
                  else
                    {
                      any_toggle_active = TRUE;
                    }
                }
            }

          if (! font_differs)
            {
              GtkTextTag *tag;

              tag = picman_text_buffer_get_iter_font (editor->buffer, &iter,
                                                    NULL);

              if (tag != font_tag)
                font_differs = TRUE;
            }

          if (! color_differs)
            {
              GtkTextTag *tag;

              tag = picman_text_buffer_get_iter_color (editor->buffer, &iter,
                                                     NULL);

              if (tag != color_tag)
                color_differs = TRUE;
            }

          if (! size_differs)
            {
              GtkTextTag *tag;

              tag = picman_text_buffer_get_iter_size (editor->buffer, &iter,
                                                    NULL);

              if (tag != size_tag)
                size_differs = TRUE;
            }

          if (! baseline_differs)
            {
              GtkTextTag *tag;

              tag = picman_text_buffer_get_iter_baseline (editor->buffer, &iter,
                                                        NULL);

              if (tag != baseline_tag)
                baseline_differs = TRUE;
            }

          if (! kerning_differs)
            {
              GtkTextTag *tag;

              tag = picman_text_buffer_get_iter_kerning (editor->buffer, &iter,
                                                       NULL);

              if (tag != kerning_tag)
                kerning_differs = TRUE;
            }

          if (! any_toggle_active &&
              color_differs       &&
              font_differs        &&
              size_differs        &&
              baseline_differs    &&
              kerning_differs)
            break;
       }

      if (font_differs)
        picman_text_style_editor_set_font (editor, NULL);
      else if (font_tag)
        picman_text_style_editor_set_font (editor, font_tag);
      else
        picman_text_style_editor_set_default_font (editor);

      if (color_differs)
        picman_text_style_editor_set_color (editor, NULL);
      else if (color_tag)
        picman_text_style_editor_set_color (editor, color_tag);
      else
        picman_text_style_editor_set_default_color (editor);

      if (size_differs)
        picman_text_style_editor_set_size (editor, NULL);
      else if (size_tag)
        picman_text_style_editor_set_size (editor, size_tag);
      else
        picman_text_style_editor_set_default_size (editor);

      if (baseline_differs)
        gtk_entry_set_text (GTK_ENTRY (editor->baseline_spinbutton), "");
      else
        picman_text_style_editor_set_baseline (editor, baseline_tag);

      if (kerning_differs)
        gtk_entry_set_text (GTK_ENTRY (editor->kerning_spinbutton), "");
      else
        picman_text_style_editor_set_kerning (editor, kerning_tag);
    }
  else /* no selection */
    {
      GtkTextIter  cursor;
      GSList      *tags;
      GSList      *tags_on;
      GSList      *tags_off;
      GList       *list;

      gtk_text_buffer_get_iter_at_mark (buffer, &cursor,
                                        gtk_text_buffer_get_insert (buffer));

      tags     = gtk_text_iter_get_tags (&cursor);
      tags_on  = gtk_text_iter_get_toggled_tags (&cursor, TRUE);
      tags_off = gtk_text_iter_get_toggled_tags (&cursor, FALSE);

      for (list = editor->buffer->font_tags; list; list = g_list_next (list))
        {
          GtkTextTag *tag = list->data;

          if ((g_slist_find (tags, tag) &&
               ! g_slist_find (tags_on, tag)) ||
              g_slist_find (tags_off, tag))
            {
              picman_text_style_editor_set_font (editor, tag);
              break;
            }
        }

      if (! list)
        picman_text_style_editor_set_default_font (editor);

      for (list = editor->buffer->color_tags; list; list = g_list_next (list))
        {
          GtkTextTag *tag = list->data;

          if ((g_slist_find (tags, tag) &&
               ! g_slist_find (tags_on, tag)) ||
              g_slist_find (tags_off, tag))
            {
              picman_text_style_editor_set_color (editor, tag);
              break;
            }
        }

      if (! list)
        picman_text_style_editor_set_default_color (editor);

      for (list = editor->buffer->size_tags; list; list = g_list_next (list))
        {
          GtkTextTag *tag = list->data;

          if ((g_slist_find (tags, tag) &&
               ! g_slist_find (tags_on, tag)) ||
              g_slist_find (tags_off, tag))
            {
              picman_text_style_editor_set_size (editor, tag);
              break;
            }
        }

      if (! list)
        picman_text_style_editor_set_default_size (editor);

      for (list = editor->buffer->baseline_tags; list; list = g_list_next (list))
        {
          GtkTextTag *tag = list->data;

          if ((g_slist_find (tags, tag) &&
               ! g_slist_find (tags_on, tag)) ||
              g_slist_find (tags_off, tag))
            {
              picman_text_style_editor_set_baseline (editor, tag);
              break;
            }
        }

      if (! list)
        picman_text_style_editor_set_baseline (editor, NULL);

      for (list = editor->toggles; list; list = g_list_next (list))
        {
          GtkToggleButton *toggle = list->data;
          GtkTextTag      *tag    = g_object_get_data (G_OBJECT (toggle),
                                                       "tag");

          picman_text_style_editor_set_toggle (editor, toggle,
                                             (g_slist_find (tags, tag) &&
                                              ! g_slist_find (tags_on, tag)) ||
                                             g_slist_find (tags_off, tag));
        }

      {
        GtkTextTag *tag;

        tag = picman_text_buffer_get_iter_kerning (editor->buffer, &cursor, NULL);
        picman_text_style_editor_set_kerning (editor, tag);
      }

      g_slist_free (tags);
      g_slist_free (tags_on);
      g_slist_free (tags_off);
    }

  return FALSE;
}
