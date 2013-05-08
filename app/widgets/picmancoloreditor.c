/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancoloreditor.c
 * Copyright (C) 2002 Michael Natterer <mitch@picman.org>
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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"

#include "picmancoloreditor.h"
#include "picmandocked.h"
#include "picmanfgbgeditor.h"
#include "picmanfgbgview.h"
#include "picmansessioninfo-aux.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_CONTEXT
};


static void   picman_color_editor_docked_iface_init (PicmanDockedInterface  *iface);

static void   picman_color_editor_dispose         (GObject           *object);
static void   picman_color_editor_set_property    (GObject           *object,
                                                 guint              property_id,
                                                 const GValue      *value,
                                                 GParamSpec        *pspec);
static void   picman_color_editor_get_property    (GObject           *object,
                                                 guint              property_id,
                                                 GValue            *value,
                                                 GParamSpec        *pspec);

static void   picman_color_editor_style_set       (GtkWidget         *widget,
                                                 GtkStyle          *prev_style);

static void   picman_color_editor_set_aux_info    (PicmanDocked        *docked,
                                                 GList             *aux_info);
static GList *picman_color_editor_get_aux_info     (PicmanDocked       *docked);
static GtkWidget *picman_color_editor_get_preview (PicmanDocked        *docked,
                                                 PicmanContext       *context,
                                                 GtkIconSize        size);
static void   picman_color_editor_set_context     (PicmanDocked        *docked,
                                                 PicmanContext       *context);

static void   picman_color_editor_fg_changed      (PicmanContext       *context,
                                                 const PicmanRGB     *rgb,
                                                 PicmanColorEditor   *editor);
static void   picman_color_editor_bg_changed      (PicmanContext       *context,
                                                 const PicmanRGB     *rgb,
                                                 PicmanColorEditor   *editor);
static void   picman_color_editor_color_changed   (PicmanColorSelector *selector,
                                                 const PicmanRGB     *rgb,
                                                 const PicmanHSV     *hsv,
                                                 PicmanColorEditor   *editor);
static void   picman_color_editor_tab_toggled     (GtkWidget         *widget,
                                                 PicmanColorEditor   *editor);
static void   picman_color_editor_fg_bg_notify    (GtkWidget         *widget,
                                                 GParamSpec        *pspec,
                                                 PicmanColorEditor   *editor);
static void   picman_color_editor_color_picked    (GtkWidget         *widget,
                                                 const PicmanRGB     *rgb,
                                                 PicmanColorEditor   *editor);
static void   picman_color_editor_entry_changed   (PicmanColorHexEntry *entry,
                                                 PicmanColorEditor   *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanColorEditor, picman_color_editor, PICMAN_TYPE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_color_editor_docked_iface_init))

#define parent_class picman_color_editor_parent_class

static PicmanDockedInterface *parent_docked_iface = NULL;


static void
picman_color_editor_class_init (PicmanColorEditorClass* klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose      = picman_color_editor_dispose;
  object_class->set_property = picman_color_editor_set_property;
  object_class->get_property = picman_color_editor_get_property;

  widget_class->style_set    = picman_color_editor_style_set;

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_READWRITE));
}

static void
picman_color_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  parent_docked_iface = g_type_interface_peek_parent (iface);

  if (! parent_docked_iface)
    parent_docked_iface = g_type_default_interface_peek (PICMAN_TYPE_DOCKED);

  iface->get_preview  = picman_color_editor_get_preview;
  iface->set_aux_info = picman_color_editor_set_aux_info;
  iface->get_aux_info = picman_color_editor_get_aux_info;
  iface->set_context  = picman_color_editor_set_context;
}

static void
picman_color_editor_init (PicmanColorEditor *editor)
{
  GtkWidget   *notebook;
  GtkWidget   *hbox;
  GtkWidget   *vbox;
  GtkWidget   *button;
  gint         content_spacing;
  gint         button_spacing;
  GtkIconSize  button_icon_size;
  PicmanRGB      rgb;
  PicmanHSV      hsv;
  GList       *list;
  GSList      *group;

  editor->context = NULL;
  editor->edit_bg = FALSE;

  picman_rgba_set (&rgb, 0.0, 0.0, 0.0, 1.0);
  picman_rgb_to_hsv (&rgb, &hsv);

  gtk_widget_style_get (GTK_WIDGET (editor),
                        "content-spacing",  &content_spacing,
                        "button-spacing",   &button_spacing,
                        "button-icon-size", &button_icon_size,
                        NULL);

  editor->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, button_spacing);
  gtk_box_set_homogeneous (GTK_BOX (editor->hbox), TRUE);
  gtk_box_pack_start (GTK_BOX (editor), editor->hbox, FALSE, FALSE, 0);
  gtk_widget_show (editor->hbox);

  editor->notebook = picman_color_selector_new (PICMAN_TYPE_COLOR_NOTEBOOK,
                                              &rgb, &hsv,
                                              PICMAN_COLOR_SELECTOR_HUE);
  picman_color_selector_set_show_alpha (PICMAN_COLOR_SELECTOR (editor->notebook),
                                      FALSE);
  gtk_box_pack_start (GTK_BOX (editor), editor->notebook,
                      TRUE, TRUE, content_spacing);
  gtk_widget_show (editor->notebook);

  g_signal_connect (editor->notebook, "color-changed",
                    G_CALLBACK (picman_color_editor_color_changed),
                    editor);

  notebook = PICMAN_COLOR_NOTEBOOK (editor->notebook)->notebook;

  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);

  picman_color_notebook_set_has_page (PICMAN_COLOR_NOTEBOOK (editor->notebook),
                                    PICMAN_TYPE_COLOR_SCALES, TRUE);

  group = NULL;

  for (list = PICMAN_COLOR_NOTEBOOK (editor->notebook)->selectors;
       list;
       list = g_list_next (list))
    {
      PicmanColorSelector      *selector;
      PicmanColorSelectorClass *selector_class;
      GtkWidget              *button;
      GtkWidget              *image;

      selector       = PICMAN_COLOR_SELECTOR (list->data);
      selector_class = PICMAN_COLOR_SELECTOR_GET_CLASS (selector);

      button = gtk_radio_button_new (group);
      group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
      gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), FALSE);
      gtk_box_pack_start (GTK_BOX (editor->hbox), button, TRUE, TRUE, 0);
      gtk_widget_show (button);

      image = gtk_image_new_from_stock (selector_class->stock_id,
                                        GTK_ICON_SIZE_BUTTON);
      gtk_container_add (GTK_CONTAINER (button), image);
      gtk_widget_show (image);

      picman_help_set_help_data (button,
                               selector_class->name, selector_class->help_id);

      g_object_set_data (G_OBJECT (button),   "selector", selector);
      g_object_set_data (G_OBJECT (selector), "button",   button);

      g_signal_connect (button, "toggled",
                        G_CALLBACK (picman_color_editor_tab_toggled),
                        editor);
    }

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);
  gtk_box_pack_start (GTK_BOX (editor), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /*  FG/BG editor  */
  editor->fg_bg = picman_fg_bg_editor_new (NULL);
  gtk_box_pack_start (GTK_BOX (hbox), editor->fg_bg, TRUE, TRUE, 0);
  gtk_widget_show (editor->fg_bg);

  g_signal_connect (editor->fg_bg, "notify::active-color",
                    G_CALLBACK (picman_color_editor_fg_bg_notify),
                    editor);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  /*  The color picker  */
  button = picman_pick_button_new ();
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "color-picked",
                    G_CALLBACK (picman_color_editor_color_picked),
                    editor);

  /*  The hex triplet entry  */
  editor->hex_entry = picman_color_hex_entry_new ();
  picman_help_set_help_data (editor->hex_entry,
                           _("Hexadecimal color notation as used in HTML and "
                             "CSS.  This entry also accepts CSS color names."),
                           NULL);
  gtk_box_pack_end (GTK_BOX (vbox), editor->hex_entry, FALSE, FALSE, 0);
  gtk_widget_show (editor->hex_entry);

  g_signal_connect (editor->hex_entry, "color-changed",
                    G_CALLBACK (picman_color_editor_entry_changed),
                    editor);
}

static void
picman_color_editor_dispose (GObject *object)
{
  PicmanColorEditor *editor = PICMAN_COLOR_EDITOR (object);

  if (editor->context)
    picman_docked_set_context (PICMAN_DOCKED (editor), NULL);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_color_editor_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_CONTEXT:
      picman_docked_set_context (PICMAN_DOCKED (object),
                               g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_editor_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanColorEditor *editor = PICMAN_COLOR_EDITOR (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (value, editor->context);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static GtkWidget *
picman_color_editor_get_preview (PicmanDocked  *docked,
                               PicmanContext *context,
                               GtkIconSize  size)
{
  GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (docked));
  GtkWidget   *preview;
  gint         width;
  gint         height;

  preview = picman_fg_bg_view_new (context);

  if (gtk_icon_size_lookup_for_settings (settings, size, &width, &height))
    gtk_widget_set_size_request (preview, width, height);

  return preview;
}

#define AUX_INFO_CURRENT_PAGE "current-page"

static void
picman_color_editor_set_aux_info (PicmanDocked *docked,
                                GList      *aux_info)
{
  PicmanColorEditor *editor   = PICMAN_COLOR_EDITOR (docked);
  GtkWidget       *notebook = PICMAN_COLOR_NOTEBOOK (editor->notebook)->notebook;
  GList           *list;

  parent_docked_iface->set_aux_info (docked, aux_info);

  for (list = aux_info; list; list = g_list_next (list))
    {
      PicmanSessionInfoAux *aux = list->data;

      if (! strcmp (aux->name, AUX_INFO_CURRENT_PAGE))
        {
          GList *children;
          GList *child;

          children = gtk_container_get_children (GTK_CONTAINER (notebook));

          for (child = children; child; child = g_list_next (child))
            {
              if (! strcmp (G_OBJECT_TYPE_NAME (child->data), aux->value))
                {
                  GtkWidget *button;

                  button = g_object_get_data (G_OBJECT (child->data), "button");

                  if (button)
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                                  TRUE);

                  break;
                }
            }

          g_list_free (children);
        }
    }
}

static GList *
picman_color_editor_get_aux_info (PicmanDocked *docked)
{
  PicmanColorEditor    *editor   = PICMAN_COLOR_EDITOR (docked);
  PicmanColorNotebook  *notebook = PICMAN_COLOR_NOTEBOOK (editor->notebook);
  GList              *aux_info;

  aux_info = parent_docked_iface->get_aux_info (docked);

  if (notebook->cur_page)
    {
      PicmanSessionInfoAux *aux;

      aux = picman_session_info_aux_new (AUX_INFO_CURRENT_PAGE,
                                       G_OBJECT_TYPE_NAME (notebook->cur_page));
      aux_info = g_list_append (aux_info, aux);
    }

  return aux_info;
}

static void
picman_color_editor_set_context (PicmanDocked  *docked,
                               PicmanContext *context)
{
  PicmanColorEditor *editor = PICMAN_COLOR_EDITOR (docked);

  if (context == editor->context)
    return;

  if (editor->context)
    {
      g_signal_handlers_disconnect_by_func (editor->context,
                                            picman_color_editor_fg_changed,
                                            editor);
      g_signal_handlers_disconnect_by_func (editor->context,
                                            picman_color_editor_bg_changed,
                                            editor);

      g_object_unref (editor->context);
    }

  editor->context = context;

  if (editor->context)
    {
      PicmanRGB rgb;

      g_object_ref (editor->context);

      g_signal_connect (editor->context, "foreground-changed",
                        G_CALLBACK (picman_color_editor_fg_changed),
                        editor);
      g_signal_connect (editor->context, "background-changed",
                        G_CALLBACK (picman_color_editor_bg_changed),
                        editor);

      if (editor->edit_bg)
        {
          picman_context_get_background (editor->context, &rgb);
          picman_color_editor_bg_changed (editor->context, &rgb, editor);
        }
      else
        {
          picman_context_get_foreground (editor->context, &rgb);
          picman_color_editor_fg_changed (editor->context, &rgb, editor);
        }

      g_object_set_data (G_OBJECT (context->picman->config->color_management),
                         "picman-context", editor->context);

      picman_color_selector_set_config (PICMAN_COLOR_SELECTOR (editor->notebook),
                                      context->picman->config->color_management);

      g_object_set_data (G_OBJECT (context->picman->config->color_management),
                         "picman-context", NULL);
    }

  picman_fg_bg_editor_set_context (PICMAN_FG_BG_EDITOR (editor->fg_bg), context);
}

GtkWidget *
picman_color_editor_new (PicmanContext *context)
{
  return g_object_new (PICMAN_TYPE_COLOR_EDITOR,
                       "context", context,
                       NULL);
}

static void
picman_color_editor_style_set (GtkWidget *widget,
                             GtkStyle  *prev_style)
{
  PicmanColorEditor *editor = PICMAN_COLOR_EDITOR (widget);

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  if (editor->hbox)
    picman_editor_set_box_style (PICMAN_EDITOR (editor), GTK_BOX (editor->hbox));
}


static void
picman_color_editor_set_color (PicmanColorEditor *editor,
                             const PicmanRGB   *rgb)
{
  PicmanHSV hsv;

  picman_rgb_to_hsv (rgb, &hsv);

  g_signal_handlers_block_by_func (editor->notebook,
                                   picman_color_editor_color_changed,
                                   editor);

  picman_color_selector_set_color (PICMAN_COLOR_SELECTOR (editor->notebook),
                                 rgb, &hsv);

  g_signal_handlers_unblock_by_func (editor->notebook,
                                     picman_color_editor_color_changed,
                                     editor);

  g_signal_handlers_block_by_func (editor->hex_entry,
                                   picman_color_editor_entry_changed,
                                   editor);

  picman_color_hex_entry_set_color (PICMAN_COLOR_HEX_ENTRY (editor->hex_entry),
                                  rgb);

  g_signal_handlers_unblock_by_func (editor->hex_entry,
                                     picman_color_editor_entry_changed,
                                     editor);
}

static void
picman_color_editor_fg_changed (PicmanContext     *context,
                              const PicmanRGB   *rgb,
                              PicmanColorEditor *editor)
{
  if (! editor->edit_bg)
    picman_color_editor_set_color (editor, rgb);
}

static void
picman_color_editor_bg_changed (PicmanContext     *context,
                              const PicmanRGB   *rgb,
                              PicmanColorEditor *editor)
{
  if (editor->edit_bg)
    picman_color_editor_set_color (editor, rgb);
}

static void
picman_color_editor_color_changed (PicmanColorSelector *selector,
                                 const PicmanRGB     *rgb,
                                 const PicmanHSV     *hsv,
                                 PicmanColorEditor   *editor)
{
  if (editor->context)
    {
      if (editor->edit_bg)
        {
          g_signal_handlers_block_by_func (editor->context,
                                           picman_color_editor_bg_changed,
                                           editor);

          picman_context_set_background (editor->context, rgb);

          g_signal_handlers_unblock_by_func (editor->context,
                                             picman_color_editor_bg_changed,
                                             editor);
        }
      else
        {
          g_signal_handlers_block_by_func (editor->context,
                                           picman_color_editor_fg_changed,
                                           editor);

          picman_context_set_foreground (editor->context, rgb);

          g_signal_handlers_unblock_by_func (editor->context,
                                             picman_color_editor_fg_changed,
                                             editor);
        }
    }

  g_signal_handlers_block_by_func (editor->hex_entry,
                                   picman_color_editor_entry_changed,
                                   editor);

  picman_color_hex_entry_set_color (PICMAN_COLOR_HEX_ENTRY (editor->hex_entry),
                                  rgb);

  g_signal_handlers_unblock_by_func (editor->hex_entry,
                                     picman_color_editor_entry_changed,
                                     editor);
}

static void
picman_color_editor_tab_toggled (GtkWidget       *widget,
                               PicmanColorEditor *editor)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
      GtkWidget *selector;

      selector = g_object_get_data (G_OBJECT (widget), "selector");

      if (selector)
        {
          GtkWidget *notebook;
          gint       page_num;

          notebook = PICMAN_COLOR_NOTEBOOK (editor->notebook)->notebook;

          page_num = gtk_notebook_page_num (GTK_NOTEBOOK (notebook), selector);

          if (page_num >= 0)
            gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page_num);
        }
    }
}

static void
picman_color_editor_fg_bg_notify (GtkWidget       *widget,
                                GParamSpec      *pspec,
                                PicmanColorEditor *editor)
{
  gboolean edit_bg;

  edit_bg = (PICMAN_FG_BG_EDITOR (widget)->active_color ==
             PICMAN_ACTIVE_COLOR_BACKGROUND);

  if (edit_bg != editor->edit_bg)
    {
      editor->edit_bg = edit_bg;

      if (editor->context)
        {
          PicmanRGB rgb;

          if (edit_bg)
            {
              picman_context_get_background (editor->context, &rgb);
              picman_color_editor_bg_changed (editor->context, &rgb, editor);
            }
          else
            {
              picman_context_get_foreground (editor->context, &rgb);
              picman_color_editor_fg_changed (editor->context, &rgb, editor);
            }
        }
    }
}

static void
picman_color_editor_color_picked (GtkWidget       *widget,
                                const PicmanRGB   *rgb,
                                PicmanColorEditor *editor)
{
  if (editor->context)
    {
      if (editor->edit_bg)
        picman_context_set_background (editor->context, rgb);
      else
        picman_context_set_foreground (editor->context, rgb);
    }
}

static void
picman_color_editor_entry_changed (PicmanColorHexEntry *entry,
                                 PicmanColorEditor   *editor)
{
  PicmanRGB  rgb;

  picman_color_hex_entry_get_color (entry, &rgb);

  if (editor->context)
    {
      if (editor->edit_bg)
        picman_context_set_background (editor->context, &rgb);
      else
        picman_context_set_foreground (editor->context, &rgb);
    }
}
