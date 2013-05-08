/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmaniconpicker.c
 * Copyright (C) 2011 Michael Natterer <mitch@picman.org>
 *               2012 Daniel Sabo
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
#include "core/picmanlist.h"
#include "core/picmancontext.h"
#include "core/picmantemplate.h"
#include "core/picmanviewable.h"

#include "picmancontainerpopup.h"
#include "picmaniconpicker.h"
#include "picmanview.h"
#include "picmanviewablebutton.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"

enum
{
  PROP_0,
  PROP_PICMAN,
  PROP_STOCK_ID,
  PROP_ICON_PIXBUF
};


typedef struct _PicmanIconPickerPrivate PicmanIconPickerPrivate;

struct _PicmanIconPickerPrivate
{
  Picman          *picman;

  gchar         *stock_id;
  GdkPixbuf     *icon_pixbuf;

  PicmanViewable  *preview;

  PicmanContainer *stock_id_container;
  PicmanContext   *stock_id_context;
  PicmanObject    *null_template_object;

  GtkWidget     *right_click_menu;
  GtkWidget     *menu_item_file_icon;
  GtkWidget     *menu_item_stock_icon;
  GtkWidget     *menu_item_copy;
  GtkWidget     *menu_item_paste;
};

#define GET_PRIVATE(picker) \
        G_TYPE_INSTANCE_GET_PRIVATE (picker, \
                                     PICMAN_TYPE_ICON_PICKER, \
                                     PicmanIconPickerPrivate)


static void    picman_icon_picker_constructed     (GObject        *object);
static void    picman_icon_picker_finalize        (GObject        *object);
static void    picman_icon_picker_set_property    (GObject        *object,
                                                 guint           property_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void    picman_icon_picker_get_property    (GObject        *object,
                                                 guint           property_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);

static void    picman_icon_picker_icon_changed    (PicmanContext    *context,
                                                 PicmanTemplate   *template,
                                                 PicmanIconPicker *picker);
static void    picman_icon_picker_clicked         (GtkWidget      *widget,
                                                 GdkEventButton *event,
                                                 gpointer        data);

static void    picman_icon_picker_menu_from_file  (GtkWidget      *widget,
                                                 GdkEventButton *event,
                                                 gpointer        data);
static void    picman_icon_picker_menu_from_stock (GtkWidget      *widget,
                                                 GdkEventButton *event,
                                                 gpointer        data);
static void    picman_icon_picker_menu_paste      (GtkWidget      *widget,
                                                 GdkEventButton *event,
                                                 gpointer        data);
static void    picman_icon_picker_menu_copy       (GtkWidget      *widget,
                                                 GdkEventButton *event,
                                                 gpointer        data);


G_DEFINE_TYPE (PicmanIconPicker, picman_icon_picker, GTK_TYPE_BOX)

#define parent_class picman_icon_picker_parent_class


static void
picman_icon_picker_class_init (PicmanIconPickerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_icon_picker_constructed;
  object_class->finalize     = picman_icon_picker_finalize;
  object_class->set_property = picman_icon_picker_set_property;
  object_class->get_property = picman_icon_picker_get_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman", NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_STOCK_ID,
                                   g_param_spec_string ("stock-id", NULL, NULL,
                                                        "picman-toilet-paper",
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_ICON_PIXBUF,
                                   g_param_spec_object ("icon-pixbuf", NULL, NULL,
                                                        GDK_TYPE_PIXBUF,
                                                        PICMAN_PARAM_READWRITE));

  g_type_class_add_private (object_class, sizeof (PicmanIconPickerPrivate));
}

static void
picman_icon_picker_init (PicmanIconPicker *picker)
{
  PicmanIconPickerPrivate *private = GET_PRIVATE (picker);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (picker),
                                  GTK_ORIENTATION_HORIZONTAL);

  private->preview = g_object_new (PICMAN_TYPE_VIEWABLE,
                                   "stock-id", private->stock_id,
                                   "icon-pixbuf", private->icon_pixbuf,
                                   NULL);
}

static void
picman_icon_picker_constructed (GObject *object)
{
  PicmanIconPicker         *picker  = PICMAN_ICON_PICKER (object);
  PicmanIconPickerPrivate  *private = GET_PRIVATE (object);
  GtkWidget              *button;
  GtkWidget              *viewable_view;
  GSList                 *stock_list;
  GSList                 *list;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (private->picman));

  /* Set up the stock icon picker */
  private->stock_id_container = picman_list_new (PICMAN_TYPE_TEMPLATE, FALSE);
  private->stock_id_context = picman_context_new (private->picman, "foo", NULL);

  g_signal_connect (private->stock_id_context, "template-changed",
                    G_CALLBACK (picman_icon_picker_icon_changed),
                    picker);

  stock_list = gtk_stock_list_ids ();

  for (list = stock_list; list; list = g_slist_next (list))
    {
      PicmanObject *object = g_object_new (PICMAN_TYPE_TEMPLATE,
                                         "name",     list->data,
                                         "stock-id", list->data,
                                         NULL);

      picman_container_add (private->stock_id_container, object);
      g_object_unref (object);

      if (private->stock_id && strcmp (list->data, private->stock_id) == 0)
        picman_context_set_template (private->stock_id_context,
                                   PICMAN_TEMPLATE (object));
    }

  /* An extra template object, use to make all stock icons clickable
   * when a pixbuf icon is set.
   */
  private->null_template_object = g_object_new (PICMAN_TYPE_TEMPLATE,
                                                "name",     "",
                                                "stock-id", "",
                                                NULL);

  if (private->icon_pixbuf)
    {
      picman_context_set_template (private->stock_id_context,
                                 PICMAN_TEMPLATE (private->null_template_object));
    }

  g_slist_free_full (stock_list, (GDestroyNotify) g_free);


  /* Set up preview button */
  button = gtk_button_new ();
  gtk_box_pack_start (GTK_BOX (picker), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "button-press-event",
                    G_CALLBACK (picman_icon_picker_clicked),
                    object);


  viewable_view = picman_view_new (private->stock_id_context,
                                 private->preview,
                                 PICMAN_VIEW_SIZE_SMALL,
                                 0,
                                 FALSE);
  gtk_container_add (GTK_CONTAINER (button), GTK_WIDGET (viewable_view));
  gtk_widget_show (viewable_view);

  /* Set up button menu */
  private->right_click_menu = gtk_menu_new ();
  gtk_menu_attach_to_widget (GTK_MENU (private->right_click_menu), button, NULL);

  private->menu_item_file_icon =
    gtk_menu_item_new_with_label (_("From File..."));
  gtk_menu_shell_append (GTK_MENU_SHELL (private->right_click_menu),
                         GTK_WIDGET (private->menu_item_file_icon));

  g_signal_connect (private->menu_item_file_icon, "button-press-event",
                    G_CALLBACK (picman_icon_picker_menu_from_file),
                    object);

  private->menu_item_stock_icon =
    gtk_menu_item_new_with_label (_("From Stock Icons..."));
  gtk_menu_shell_append (GTK_MENU_SHELL (private->right_click_menu),
                         GTK_WIDGET (private->menu_item_stock_icon));

  g_signal_connect (private->menu_item_stock_icon, "button-press-event",
                    G_CALLBACK (picman_icon_picker_menu_from_stock),
                    object);

  private->menu_item_copy =
    gtk_menu_item_new_with_label (_("Copy Icon to Clipboard"));
  gtk_menu_shell_append (GTK_MENU_SHELL (private->right_click_menu),
                         GTK_WIDGET (private->menu_item_copy));

  g_signal_connect (private->menu_item_copy, "button-press-event",
                    G_CALLBACK (picman_icon_picker_menu_copy),
                    object);

  private->menu_item_paste =
    gtk_menu_item_new_with_label (_("Paste Icon from Clipboard"));
  gtk_menu_shell_append (GTK_MENU_SHELL (private->right_click_menu),
                         GTK_WIDGET (private->menu_item_paste));

  g_signal_connect (private->menu_item_paste, "button-press-event",
                    G_CALLBACK (picman_icon_picker_menu_paste),
                    object);

  gtk_widget_show_all (GTK_WIDGET (private->right_click_menu));
}

static void
picman_icon_picker_finalize (GObject *object)
{
  PicmanIconPickerPrivate *private = GET_PRIVATE (object);

  if (private->stock_id)
    {
      g_free (private->stock_id);
      private->stock_id = NULL;
    }

  if (private->stock_id_container)
    {
      g_object_unref (private->stock_id_container);
      private->stock_id_container = NULL;
    }

  if (private->stock_id_context)
    {
      g_object_unref (private->stock_id_context);
      private->stock_id_context = NULL;
    }

  if (private->icon_pixbuf)
    {
      g_object_unref (private->icon_pixbuf);
      private->icon_pixbuf = NULL;
    }

  if (private->preview)
    {
      g_object_unref (private->preview);
      private->preview = NULL;
    }

  if (private->null_template_object)
    {
      g_object_unref (private->null_template_object);
      private->null_template_object = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_icon_picker_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanIconPickerPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      private->picman = g_value_get_object (value); /* don't ref */
      break;

    case PROP_STOCK_ID:
      picman_icon_picker_set_stock_id (PICMAN_ICON_PICKER (object),
                                     g_value_get_string (value));
      break;

    case PROP_ICON_PIXBUF:
      picman_icon_picker_set_icon_pixbuf (PICMAN_ICON_PICKER (object),
                                        g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_icon_picker_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanIconPickerPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, private->picman);
      break;

    case PROP_STOCK_ID:
      g_value_set_string (value, private->stock_id);
      break;

    case PROP_ICON_PIXBUF:
      g_value_set_object (value, private->icon_pixbuf);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_icon_picker_new (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return g_object_new (PICMAN_TYPE_ICON_PICKER,
                       "picman", picman,
                       NULL);
}

const gchar *
picman_icon_picker_get_stock_id (PicmanIconPicker *picker)
{
  g_return_val_if_fail (PICMAN_IS_ICON_PICKER (picker), NULL);

  return GET_PRIVATE (picker)->stock_id;
}

void
picman_icon_picker_set_stock_id (PicmanIconPicker *picker,
                               const gchar    *stock_id)
{
  PicmanIconPickerPrivate *private;

  g_return_if_fail (PICMAN_IS_ICON_PICKER (picker));
  g_return_if_fail (stock_id != NULL);

  private = GET_PRIVATE (picker);

  g_free (private->stock_id);
  private->stock_id = g_strdup (stock_id);

  if (private->stock_id_container)
    {
      PicmanObject *object;

      object = picman_container_get_child_by_name (private->stock_id_container,
                                                 stock_id);

      if (object)
        picman_context_set_template (private->stock_id_context,
                                   PICMAN_TEMPLATE (object));
    }

  g_object_set (private->preview,
                "stock-id", private->stock_id,
                NULL);

  g_object_notify (G_OBJECT (picker), "stock-id");
}

GdkPixbuf  *
picman_icon_picker_get_icon_pixbuf (PicmanIconPicker *picker)
{
  g_return_val_if_fail (PICMAN_IS_ICON_PICKER (picker), NULL);

  return GET_PRIVATE (picker)->icon_pixbuf;
}

void
picman_icon_picker_set_icon_pixbuf (PicmanIconPicker *picker,
                                  GdkPixbuf      *value)
{
  PicmanIconPickerPrivate *private;

  g_return_if_fail (PICMAN_IS_ICON_PICKER (picker));
  g_return_if_fail (value == NULL || GDK_IS_PIXBUF (value));

  private = GET_PRIVATE (picker);

  if (private->icon_pixbuf)
    g_object_unref (private->icon_pixbuf);

  private->icon_pixbuf = value;

  if (private->icon_pixbuf)
    {
      g_object_ref (private->icon_pixbuf);

      picman_context_set_template (private->stock_id_context,
                                 PICMAN_TEMPLATE (private->null_template_object));
    }
  else
    {
      PicmanObject *object;

      object = picman_container_get_child_by_name (private->stock_id_container,
                                                 private->stock_id);

      if (object)
        picman_context_set_template (private->stock_id_context,
                                   PICMAN_TEMPLATE (object));
    }

  g_object_set (private->preview,
                "icon-pixbuf", private->icon_pixbuf,
                NULL);

  g_object_notify (G_OBJECT (picker), "icon-pixbuf");
}


/*  private functions  */

static void
picman_icon_picker_icon_changed (PicmanContext    *context,
                               PicmanTemplate   *template,
                               PicmanIconPicker *picker)
{
  PicmanIconPickerPrivate *private = GET_PRIVATE (picker);

  if (PICMAN_OBJECT (template) != private->null_template_object)
    {
      picman_icon_picker_set_icon_pixbuf (picker, NULL);
      picman_icon_picker_set_stock_id (picker, picman_object_get_name (template));
    }
}

static void
picman_icon_picker_menu_from_file (GtkWidget      *widget,
                                 GdkEventButton *event,
                                 gpointer        object)
{
  PicmanIconPicker *picker = PICMAN_ICON_PICKER (object);
  GtkWidget      *dialog;
  GtkFileFilter  *filter;

  dialog = gtk_file_chooser_dialog_new (_("Load Icon Image"),
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                        NULL);

  filter = gtk_file_filter_new ();
  gtk_file_filter_add_pixbuf_formats (filter);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      gchar     *filename;
      GdkPixbuf *icon_pixbuf = NULL;

      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

      icon_pixbuf = gdk_pixbuf_new_from_file (filename, NULL);

      if (icon_pixbuf)
        {
          picman_icon_picker_set_icon_pixbuf (picker, icon_pixbuf);
          g_object_unref (icon_pixbuf);
        }
      g_free (filename);
    }
  gtk_widget_destroy (dialog);
}

static void
picman_icon_picker_menu_copy (GtkWidget      *widget,
                            GdkEventButton *event,
                            gpointer        object)
{
  PicmanIconPicker        *picker    = PICMAN_ICON_PICKER (object);
  PicmanIconPickerPrivate *private   = GET_PRIVATE (picker);
  GtkClipboard          *clipboard = NULL;

  clipboard = gtk_clipboard_get_for_display (gtk_widget_get_display (widget),
                                             GDK_SELECTION_CLIPBOARD);

  if (private->icon_pixbuf)
    {
      gtk_clipboard_set_image (clipboard, private->icon_pixbuf);
    }
}

static void
picman_icon_picker_menu_paste (GtkWidget      *widget,
                             GdkEventButton *event,
                             gpointer        object)
{
  PicmanIconPicker *picker           = PICMAN_ICON_PICKER (object);
  GtkClipboard   *clipboard        = NULL;
  GdkPixbuf      *clipboard_pixbuf = NULL;

  clipboard = gtk_clipboard_get_for_display (gtk_widget_get_display (widget),
                                             GDK_SELECTION_CLIPBOARD);

  clipboard_pixbuf = gtk_clipboard_wait_for_image (clipboard);

  if (clipboard_pixbuf)
    {
      picman_icon_picker_set_icon_pixbuf (picker, clipboard_pixbuf);
      g_object_unref (clipboard_pixbuf);
    }
}

static void
picman_icon_picker_menu_position (GtkMenu  *menu,
                                gint     *x,
                                gint     *y,
                                gboolean *push_in,
                                gpointer  user_data)
{
  picman_button_menu_position (user_data, menu, GTK_POS_RIGHT, x, y);
}

static void
picman_icon_picker_clicked (GtkWidget      *widget,
                          GdkEventButton *event,
                          gpointer        object)
{
  PicmanIconPicker        *picker    = PICMAN_ICON_PICKER (object);
  PicmanIconPickerPrivate *private   = GET_PRIVATE (picker);
  GtkClipboard          *clipboard = NULL;

  clipboard = gtk_clipboard_get_for_display (gtk_widget_get_display (widget),
                                             GDK_SELECTION_CLIPBOARD);

  if (gtk_clipboard_wait_is_image_available (clipboard))
    gtk_widget_set_sensitive (private->menu_item_paste, TRUE);
  else
    gtk_widget_set_sensitive (private->menu_item_paste, FALSE);

  if (private->icon_pixbuf)
    gtk_widget_set_sensitive (private->menu_item_copy, TRUE);
  else
    gtk_widget_set_sensitive (private->menu_item_copy, FALSE);

  gtk_menu_popup (GTK_MENU (private->right_click_menu),
                  NULL, NULL,
                  picman_icon_picker_menu_position, widget,
                  event->button, event->time);
}

static void
picman_icon_picker_menu_from_stock (GtkWidget      *widget,
                                  GdkEventButton *event,
                                  gpointer        object)
{
  PicmanIconPicker        *picker  = PICMAN_ICON_PICKER (object);
  PicmanIconPickerPrivate *private = GET_PRIVATE (picker);
  GtkWidget             *popup;

  /* FIXME: Right clicking on this popup can cause a crash */
  popup = picman_container_popup_new (private->stock_id_container,
                                    private->stock_id_context,
                                    PICMAN_VIEW_TYPE_LIST,
                                    PICMAN_VIEW_SIZE_SMALL,
                                    PICMAN_VIEW_SIZE_SMALL,
                                    0,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);

  picman_container_popup_set_view_type (PICMAN_CONTAINER_POPUP (popup),
                                      PICMAN_VIEW_TYPE_GRID);

  picman_container_popup_show (PICMAN_CONTAINER_POPUP (popup),
                             GTK_WIDGET (picker));
}
