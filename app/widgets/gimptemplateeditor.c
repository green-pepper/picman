/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmantemplate.h"

#include "picmanpropwidgets.h"
#include "picmantemplateeditor.h"

#include "picman-intl.h"


#define SB_WIDTH            8
#define MAX_COMMENT_LENGTH  512  /* arbitrary */


enum
{
  PROP_0,
  PROP_TEMPLATE
};


typedef struct _PicmanTemplateEditorPrivate PicmanTemplateEditorPrivate;

struct _PicmanTemplateEditorPrivate
{
  PicmanTemplate  *template;

  GtkWidget     *aspect_button;
  gboolean       block_aspect;

  GtkWidget     *expander;
  GtkWidget     *size_se;
  GtkWidget     *memsize_label;
  GtkWidget     *pixel_label;
  GtkWidget     *more_label;
  GtkWidget     *resolution_se;
};

#define GET_PRIVATE(editor) \
        G_TYPE_INSTANCE_GET_PRIVATE (editor, \
                                     PICMAN_TYPE_TEMPLATE_EDITOR, \
                                     PicmanTemplateEditorPrivate)


static void    picman_template_editor_constructed  (GObject            *object);
static void    picman_template_editor_finalize     (GObject            *object);
static void    picman_template_editor_set_property (GObject            *object,
                                                  guint               property_id,
                                                  const GValue       *value,
                                                  GParamSpec         *pspec);
static void    picman_template_editor_get_property (GObject            *object,
                                                  guint               property_id,
                                                  GValue             *value,
                                                  GParamSpec         *pspec);

static void picman_template_editor_aspect_callback (GtkWidget          *widget,
                                                  PicmanTemplateEditor *editor);
static void picman_template_editor_template_notify (PicmanTemplate       *template,
                                                  GParamSpec         *param_spec,
                                                  PicmanTemplateEditor *editor);


G_DEFINE_TYPE (PicmanTemplateEditor, picman_template_editor, GTK_TYPE_BOX)

#define parent_class picman_template_editor_parent_class


static void
picman_template_editor_class_init (PicmanTemplateEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_template_editor_constructed;
  object_class->finalize     = picman_template_editor_finalize;
  object_class->set_property = picman_template_editor_set_property;
  object_class->get_property = picman_template_editor_get_property;

  g_object_class_install_property (object_class, PROP_TEMPLATE,
                                   g_param_spec_object ("template", NULL, NULL,
                                                        PICMAN_TYPE_TEMPLATE,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (object_class, sizeof (PicmanTemplateEditorPrivate));
}

static void
picman_template_editor_init (PicmanTemplateEditor *editor)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (editor),
                                  GTK_ORIENTATION_VERTICAL);

  gtk_box_set_spacing (GTK_BOX (editor), 12);
}

static void
picman_template_editor_constructed (GObject *object)
{
  PicmanTemplateEditor        *editor  = PICMAN_TEMPLATE_EDITOR (object);
  PicmanTemplateEditorPrivate *private = GET_PRIVATE (object);
  PicmanTemplate              *template;
  GtkWidget                 *aspect_box;
  GtkWidget                 *frame;
  GtkWidget                 *hbox;
  GtkWidget                 *vbox;
  GtkWidget                 *table;
  GtkWidget                 *label;
  GtkObject                 *adjustment;
  GtkWidget                 *width;
  GtkWidget                 *height;
  GtkWidget                 *xres;
  GtkWidget                 *yres;
  GtkWidget                 *chainbutton;
  GtkWidget                 *combo;
  GtkWidget                 *scrolled_window;
  GtkWidget                 *text_view;
  GtkTextBuffer             *text_buffer;
  GList                     *focus_chain = NULL;
  gchar                     *text;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (private->template != NULL);

  template = private->template;

  /*  Image size frame  */
  frame = picman_frame_new (_("Image Size"));
  gtk_box_pack_start (GTK_BOX (editor), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_col_spacing (GTK_TABLE (table), 0, 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacing (GTK_TABLE (table), 0, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  width = picman_spin_button_new (&adjustment,
                                1, 1, 1, 1, 10, 0,
                                1, 2);
  gtk_entry_set_width_chars (GTK_ENTRY (width), SB_WIDTH);

  height = picman_spin_button_new (&adjustment,
                                 1, 1, 1, 1, 10, 0,
                                 1, 2);
  gtk_entry_set_width_chars (GTK_ENTRY (height), SB_WIDTH);

  /*  the image size labels  */
  label = gtk_label_new_with_mnemonic (_("_Width:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), width);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (label);

  label = gtk_label_new_with_mnemonic (_("H_eight:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), height);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (label);

  /*  create the sizeentry which keeps it all together  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 2, 0, 2);
  gtk_widget_show (hbox);

  private->size_se = picman_size_entry_new (0,
                                          picman_template_get_unit (template),
                                          _("%p"),
                                          TRUE, FALSE, FALSE, SB_WIDTH,
                                          PICMAN_SIZE_ENTRY_UPDATE_SIZE);

  gtk_table_set_row_spacing (GTK_TABLE (private->size_se), 0, 2);
  gtk_table_set_col_spacing (GTK_TABLE (private->size_se), 1, 6);

  gtk_box_pack_start (GTK_BOX (hbox), private->size_se, FALSE, FALSE, 0);
  gtk_widget_show (private->size_se);

  picman_size_entry_add_field (PICMAN_SIZE_ENTRY (private->size_se),
                             GTK_SPIN_BUTTON (height), NULL);
  gtk_table_attach_defaults (GTK_TABLE (private->size_se), height, 0, 1, 1, 2);
  gtk_widget_show (height);

  picman_size_entry_add_field (PICMAN_SIZE_ENTRY (private->size_se),
                             GTK_SPIN_BUTTON (width), NULL);
  gtk_table_attach_defaults (GTK_TABLE (private->size_se), width, 0, 1, 0, 1);
  gtk_widget_show (width);

  picman_prop_coordinates_connect (G_OBJECT (template),
                                 "width", "height", "unit",
                                 private->size_se, NULL,
                                 picman_template_get_resolution_x (template),
                                 picman_template_get_resolution_y (template));

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 3, 2, 3);
  gtk_widget_show (hbox);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  aspect_box = picman_enum_stock_box_new (PICMAN_TYPE_ASPECT_TYPE,
                                        "picman", GTK_ICON_SIZE_MENU,
                                        G_CALLBACK (picman_template_editor_aspect_callback),
                                        editor,
                                        &private->aspect_button);
  gtk_widget_hide (private->aspect_button); /* hide "square" */

  gtk_box_pack_start (GTK_BOX (vbox), aspect_box, FALSE, FALSE, 0);
  gtk_widget_show (aspect_box);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  private->pixel_label = gtk_label_new (NULL);
  picman_label_set_attributes (GTK_LABEL (private->pixel_label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);
  gtk_misc_set_alignment (GTK_MISC (private->pixel_label), 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (vbox), private->pixel_label, FALSE, FALSE, 0);
  gtk_widget_show (private->pixel_label);

  private->more_label = gtk_label_new (NULL);
  picman_label_set_attributes (GTK_LABEL (private->more_label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);
  gtk_misc_set_alignment (GTK_MISC (private->more_label), 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (vbox), private->more_label, FALSE, FALSE, 0);
  gtk_widget_show (private->more_label);

#ifdef ENABLE_MEMSIZE_LABEL
  private->memsize_label = gtk_label_new (NULL);
  picman_label_set_attributes (GTK_LABEL (private->memsize_label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
  gtk_misc_set_alignment (GTK_MISC (private->memsize_label), 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (vbox), private->memsize_label, FALSE, FALSE, 0);
  gtk_widget_show (private->memsize_label);
#endif

  text = g_strdup_printf ("<b>%s</b>", _("_Advanced Options"));
  private->expander = g_object_new (GTK_TYPE_EXPANDER,
                                    "label",         text,
                                    "use-markup",    TRUE,
                                    "use-underline", TRUE,
                                    NULL);
  g_free (text);

  gtk_box_pack_start (GTK_BOX (editor), private->expander, TRUE, TRUE, 0);
  gtk_widget_show (private->expander);

  frame = picman_frame_new ("<expander>");
  gtk_container_add (GTK_CONTAINER (private->expander), frame);
  gtk_widget_show (frame);

  table = gtk_table_new (6, 2, FALSE);
  gtk_table_set_col_spacing (GTK_TABLE (table), 0, 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacing (GTK_TABLE (table), 0, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  xres = picman_spin_button_new (&adjustment,
                               1, 1, 1, 1, 10, 0,
                               1, 2);
  gtk_entry_set_width_chars (GTK_ENTRY (xres), SB_WIDTH);

  yres = picman_spin_button_new (&adjustment,
                               1, 1, 1, 1, 10, 0,
                               1, 2);
  gtk_entry_set_width_chars (GTK_ENTRY (yres), SB_WIDTH);

  /*  the resolution labels  */
  label = gtk_label_new_with_mnemonic (_("_X resolution:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), xres);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (label);

  label = gtk_label_new_with_mnemonic (_("_Y resolution:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), yres);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (label);

  /*  the resolution sizeentry  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 2, 0, 2);
  gtk_widget_show (hbox);

  private->resolution_se =
    picman_size_entry_new (0,
                         picman_template_get_resolution_unit (template),
                         _("pixels/%s"),
                         FALSE, FALSE, FALSE, SB_WIDTH,
                         PICMAN_SIZE_ENTRY_UPDATE_RESOLUTION);

  gtk_table_set_row_spacing (GTK_TABLE (private->resolution_se), 0, 2);
  gtk_table_set_col_spacing (GTK_TABLE (private->resolution_se), 1, 2);
  gtk_table_set_col_spacing (GTK_TABLE (private->resolution_se), 2, 2);

  gtk_box_pack_start (GTK_BOX (hbox), private->resolution_se, FALSE, FALSE, 0);
  gtk_widget_show (private->resolution_se);

  picman_size_entry_add_field (PICMAN_SIZE_ENTRY (private->resolution_se),
                             GTK_SPIN_BUTTON (yres), NULL);
  gtk_table_attach_defaults (GTK_TABLE (private->resolution_se), yres,
                             0, 1, 1, 2);
  gtk_widget_show (yres);

  picman_size_entry_add_field (PICMAN_SIZE_ENTRY (private->resolution_se),
                             GTK_SPIN_BUTTON (xres), NULL);
  gtk_table_attach_defaults (GTK_TABLE (private->resolution_se), xres,
                             0, 1, 0, 1);
  gtk_widget_show (xres);

  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (private->size_se), 0,
                                  picman_template_get_resolution_x (template),
                                  FALSE);
  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (private->size_se), 1,
                                  picman_template_get_resolution_y (template),
                                  FALSE);

  /*  the resolution chainbutton  */
  chainbutton = picman_chain_button_new (PICMAN_CHAIN_RIGHT);
  gtk_table_attach_defaults (GTK_TABLE (private->resolution_se), chainbutton,
                             1, 2, 0, 2);
  gtk_widget_show (chainbutton);

  picman_prop_coordinates_connect (G_OBJECT (template),
                                 "xresolution", "yresolution",
                                 "resolution-unit",
                                 private->resolution_se, chainbutton,
                                 1.0, 1.0);

  focus_chain = g_list_prepend (focus_chain,
                                PICMAN_SIZE_ENTRY (private->resolution_se)->unitmenu);
  focus_chain = g_list_prepend (focus_chain, chainbutton);
  focus_chain = g_list_prepend (focus_chain, yres);
  focus_chain = g_list_prepend (focus_chain, xres);

  gtk_container_set_focus_chain (GTK_CONTAINER (private->resolution_se),
                                 focus_chain);
  g_list_free (focus_chain);

  combo = picman_prop_enum_combo_box_new (G_OBJECT (template),
                                        "image-type",
                                        PICMAN_RGB, PICMAN_GRAY);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 2,
                             _("Color _space:"), 0.0, 0.5,
                             combo, 1, FALSE);

  combo = picman_prop_enum_combo_box_new (G_OBJECT (template),
                                        "precision", 0, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 3,
                             _("_Precision:"), 0.0, 0.5,
                             combo, 1, FALSE);

  combo = picman_prop_enum_combo_box_new (G_OBJECT (template),
                                        "fill-type",
                                        PICMAN_FOREGROUND_FILL,
                                        PICMAN_PATTERN_FILL);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 4,
                             _("_Fill with:"), 0.0, 0.5,
                             combo, 1, FALSE);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 5,
                             _("Comme_nt:"), 0.0, 0.0,
                             scrolled_window, 1, FALSE);

  text_buffer = picman_prop_text_buffer_new (G_OBJECT (template),
                                           "comment", MAX_COMMENT_LENGTH);

  text_view = gtk_text_view_new_with_buffer (text_buffer);
  g_object_unref (text_buffer);

  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);
  gtk_container_add (GTK_CONTAINER (scrolled_window), text_view);
  gtk_widget_show (text_view);

  g_signal_connect_object (template, "notify",
                           G_CALLBACK (picman_template_editor_template_notify),
                           editor, 0);

  /*  call the notify callback once to get the labels set initially  */
  picman_template_editor_template_notify (template, NULL, editor);
}

static void
picman_template_editor_finalize (GObject *object)
{
  PicmanTemplateEditorPrivate *private = GET_PRIVATE (object);

  if (private->template)
    {
      g_object_unref (private->template);
      private->template = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_template_editor_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanTemplateEditorPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_TEMPLATE:
      private->template = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_template_editor_get_property (GObject      *object,
                                   guint         property_id,
                                   GValue       *value,
                                   GParamSpec   *pspec)
{
  PicmanTemplateEditorPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_TEMPLATE:
      g_value_set_object (value, private->template);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_template_editor_new (PicmanTemplate *template,
                          Picman         *picman,
                          gboolean      edit_template)
{
  PicmanTemplateEditor        *editor;
  PicmanTemplateEditorPrivate *private;

  g_return_val_if_fail (!edit_template || PICMAN_IS_PICMAN (picman), NULL);

  editor = g_object_new (PICMAN_TYPE_TEMPLATE_EDITOR,
                         "template", template,
                         NULL);

  private = GET_PRIVATE (editor);

  if (edit_template)
    {
      GtkWidget   *table;
      GtkWidget   *entry;
      GtkWidget   *icon_picker;

      table = gtk_table_new (2, 2, FALSE);
      gtk_table_set_col_spacings (GTK_TABLE (table), 6);
      gtk_table_set_row_spacings (GTK_TABLE (table), 6);
      gtk_box_pack_start (GTK_BOX (editor), table, FALSE, FALSE, 0);
      gtk_box_reorder_child (GTK_BOX (editor), table, 0);
      gtk_widget_show (table);

      entry = picman_prop_entry_new (G_OBJECT (private->template), "name", 128);

      picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                                 _("_Name:"), 1.0, 0.5,
                                 entry, 1, FALSE);

      icon_picker = picman_prop_icon_picker_new (PICMAN_VIEWABLE (private->template),
                                               picman);
      picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                                 _("_Icon:"), 1.0, 0.5,
                                 icon_picker, 1, TRUE);
    }

  return GTK_WIDGET (editor);
}

PicmanTemplate *
picman_template_editor_get_template (PicmanTemplateEditor *editor)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE_EDITOR (editor), NULL);

  return GET_PRIVATE (editor)->template;
}

void
picman_template_editor_show_advanced (PicmanTemplateEditor *editor,
                                    gboolean            expanded)
{
  PicmanTemplateEditorPrivate *private;

  g_return_if_fail (PICMAN_IS_TEMPLATE_EDITOR (editor));

  private = GET_PRIVATE (editor);

  gtk_expander_set_expanded (GTK_EXPANDER (private->expander), expanded);
}

GtkWidget *
picman_template_editor_get_size_se (PicmanTemplateEditor *editor)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE_EDITOR (editor), NULL);

  return GET_PRIVATE (editor)->size_se;
}


/*  private functions  */

static void
picman_template_editor_set_pixels (PicmanTemplateEditor *editor,
                                 PicmanTemplate       *template)
{
  PicmanTemplateEditorPrivate *private = GET_PRIVATE (editor);
  gchar                     *text;

  text = g_strdup_printf (ngettext ("%d × %d pixel",
                                    "%d × %d pixels",
                                    picman_template_get_height (template)),
                          picman_template_get_width (template),
                          picman_template_get_height (template));
  gtk_label_set_text (GTK_LABEL (private->pixel_label), text);
  g_free (text);
}

static void
picman_template_editor_aspect_callback (GtkWidget          *widget,
                                      PicmanTemplateEditor *editor)
{
  PicmanTemplateEditorPrivate *private = GET_PRIVATE (editor);

  if (! private->block_aspect &&
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
      PicmanTemplate *template    = private->template;
      gint          width       = picman_template_get_width (template);
      gint          height      = picman_template_get_height (template);
      gdouble       xresolution = picman_template_get_resolution_x (template);
      gdouble       yresolution = picman_template_get_resolution_y (template);

      if (width == height)
        {
          private->block_aspect = TRUE;
          picman_int_radio_group_set_active (GTK_RADIO_BUTTON (private->aspect_button),
                                           PICMAN_ASPECT_SQUARE);
          private->block_aspect = FALSE;
          return;
       }

      g_signal_handlers_block_by_func (template,
                                       picman_template_editor_template_notify,
                                       editor);

      picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (private->size_se), 0,
                                      yresolution, FALSE);
      picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (private->size_se), 1,
                                      xresolution, FALSE);

      g_object_set (template,
                    "width",       height,
                    "height",      width,
                    "xresolution", yresolution,
                    "yresolution", xresolution,
                    NULL);

      g_signal_handlers_unblock_by_func (template,
                                         picman_template_editor_template_notify,
                                         editor);

      picman_template_editor_set_pixels (editor, template);
    }
}

static void
picman_template_editor_template_notify (PicmanTemplate       *template,
                                      GParamSpec         *param_spec,
                                      PicmanTemplateEditor *editor)
{
  PicmanTemplateEditorPrivate *private = GET_PRIVATE (editor);
  PicmanAspectType             aspect;
  const gchar               *desc;
  gchar                     *text;
  gint                       width;
  gint                       height;
  gint                       xres;
  gint                       yres;

  if (param_spec)
    {
      if (! strcmp (param_spec->name, "xresolution"))
        {
          picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (private->size_se), 0,
                                          picman_template_get_resolution_x (template),
                                          FALSE);
        }
      else if (! strcmp (param_spec->name, "yresolution"))
        {
          picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (private->size_se), 1,
                                          picman_template_get_resolution_y (template),
                                          FALSE);
        }
    }

#ifdef ENABLE_MEMSIZE_LABEL
  text = g_format_size (picman_template_get_initial_size (template));
  gtk_label_set_text (GTK_LABEL (private->memsize_label), text);
  g_free (text);
#endif

  picman_template_editor_set_pixels (editor, template);

  width  = picman_template_get_width (template);
  height = picman_template_get_height (template);

  if (width > height)
    aspect = PICMAN_ASPECT_LANDSCAPE;
  else if (height > width)
    aspect = PICMAN_ASPECT_PORTRAIT;
  else
    aspect = PICMAN_ASPECT_SQUARE;

  private->block_aspect = TRUE;
  picman_int_radio_group_set_active (GTK_RADIO_BUTTON (private->aspect_button),
                                   aspect);
  private->block_aspect = FALSE;

  picman_enum_get_value (PICMAN_TYPE_IMAGE_BASE_TYPE,
                       picman_template_get_base_type (template),
                       NULL, NULL, &desc, NULL);

  xres = ROUND (picman_template_get_resolution_x (template));
  yres = ROUND (picman_template_get_resolution_y (template));

  if (xres != yres)
    text = g_strdup_printf (_("%d × %d ppi, %s"), xres, yres, desc);
  else
    text = g_strdup_printf (_("%d ppi, %s"), yres, desc);

  gtk_label_set_text (GTK_LABEL (private->more_label), text);
  g_free (text);
}
