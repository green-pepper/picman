/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandeviceeditor.c
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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
#include "core/picmanmarshal.h"

#include "picmancontainerview.h"
#include "picmancontainertreestore.h"
#include "picmancontainertreeview.h"
#include "picmandeviceeditor.h"
#include "picmandeviceinfo.h"
#include "picmandeviceinfoeditor.h"
#include "picmandevicemanager.h"
#include "picmandevices.h"
#include "picmanmessagebox.h"
#include "picmanmessagedialog.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_PICMAN
};


typedef struct _PicmanDeviceEditorPrivate PicmanDeviceEditorPrivate;

struct _PicmanDeviceEditorPrivate
{
  Picman      *picman;

  GQuark     name_changed_handler;

  GtkWidget *treeview;
  GtkWidget *delete_button;

  GtkWidget *label;
  GtkWidget *image;

  GtkWidget *notebook;
};


#define PICMAN_DEVICE_EDITOR_GET_PRIVATE(editor) \
        G_TYPE_INSTANCE_GET_PRIVATE (editor, \
                                     PICMAN_TYPE_DEVICE_EDITOR, \
                                     PicmanDeviceEditorPrivate)


static void   picman_device_editor_constructed    (GObject           *object);
static void   picman_device_editor_dispose        (GObject           *object);
static void   picman_device_editor_set_property   (GObject           *object,
                                                 guint              property_id,
                                                 const GValue      *value,
                                                 GParamSpec        *pspec);
static void   picman_device_editor_get_property   (GObject           *object,
                                                 guint              property_id,
                                                 GValue            *value,
                                                 GParamSpec        *pspec);

static void   picman_device_editor_add_device     (PicmanContainer     *container,
                                                 PicmanDeviceInfo    *info,
                                                 PicmanDeviceEditor  *editor);
static void   picman_device_editor_remove_device  (PicmanContainer     *container,
                                                 PicmanDeviceInfo    *info,
                                                 PicmanDeviceEditor  *editor);
static void   picman_device_editor_device_changed (PicmanDeviceInfo    *info,
                                                 PicmanDeviceEditor  *editor);

static void   picman_device_editor_select_device  (PicmanContainerView *view,
                                                 PicmanViewable      *viewable,
                                                 gpointer           insert_data,
                                                 PicmanDeviceEditor  *editor);

static void   picman_device_editor_switch_page    (GtkNotebook       *notebook,
                                                 gpointer           page,
                                                 guint              page_num,
                                                 PicmanDeviceEditor  *editor);
static void   picman_device_editor_delete_clicked (GtkWidget         *button,
                                                 PicmanDeviceEditor  *editor);


G_DEFINE_TYPE (PicmanDeviceEditor, picman_device_editor, GTK_TYPE_PANED)

#define parent_class picman_device_editor_parent_class


static void
picman_device_editor_class_init (PicmanDeviceEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_device_editor_constructed;
  object_class->dispose      = picman_device_editor_dispose;
  object_class->set_property = picman_device_editor_set_property;
  object_class->get_property = picman_device_editor_get_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (object_class, sizeof (PicmanDeviceEditorPrivate));
}

static void
picman_device_editor_init (PicmanDeviceEditor *editor)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (editor);
  GtkWidget               *vbox;
  GtkWidget               *ebox;
  GtkWidget               *hbox;
  gint                     icon_width;
  gint                     icon_height;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (editor),
                                  GTK_ORIENTATION_HORIZONTAL);

  gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (GTK_WIDGET (editor)),
                                     GTK_ICON_SIZE_BUTTON,
                                     &icon_width, &icon_height);

  private->treeview = picman_container_tree_view_new (NULL, NULL, icon_height, 0);
  gtk_widget_set_size_request (private->treeview, 200, -1);
  gtk_paned_pack1 (GTK_PANED (editor), private->treeview, TRUE, FALSE);
  gtk_widget_show (private->treeview);

  g_signal_connect_object (private->treeview, "select-item",
                           G_CALLBACK (picman_device_editor_select_device),
                           G_OBJECT (editor), 0);

  private->delete_button =
    picman_editor_add_button (PICMAN_EDITOR (private->treeview),
                            GTK_STOCK_DELETE,
                            _("Delete the selected device"),
                            NULL,
                            G_CALLBACK (picman_device_editor_delete_clicked),
                            NULL,
                            editor);

  gtk_widget_set_sensitive (private->delete_button, FALSE);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_paned_pack2 (GTK_PANED (editor), vbox, TRUE, FALSE);
  gtk_widget_show (vbox);

  ebox = gtk_event_box_new ();
  gtk_widget_set_state (ebox, GTK_STATE_SELECTED);
  gtk_box_pack_start (GTK_BOX (vbox), ebox, FALSE, FALSE, 0);
  gtk_widget_show (ebox);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
  gtk_container_add (GTK_CONTAINER (ebox), hbox);
  gtk_widget_show (hbox);

  private->label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (private->label), 0.0, 0.5);
  gtk_label_set_ellipsize (GTK_LABEL (private->label), PANGO_ELLIPSIZE_END);
  picman_label_set_attributes (GTK_LABEL (private->label),
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
  gtk_box_pack_start (GTK_BOX (hbox), private->label, TRUE, TRUE, 0);
  gtk_widget_show (private->label);

  private->image = gtk_image_new ();
  gtk_widget_set_size_request (private->image, -1, 24);
  gtk_box_pack_end (GTK_BOX (hbox), private->image, FALSE, FALSE, 0);
  gtk_widget_show (private->image);

  private->notebook = gtk_notebook_new ();
  gtk_notebook_set_show_border (GTK_NOTEBOOK (private->notebook), FALSE);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (private->notebook), FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), private->notebook, TRUE, TRUE, 0);
  gtk_widget_show (private->notebook);

  g_signal_connect (private->notebook, "switch-page",
                    G_CALLBACK (picman_device_editor_switch_page),
                    editor);
}

static void
picman_device_editor_constructed (GObject *object)
{
  PicmanDeviceEditor        *editor  = PICMAN_DEVICE_EDITOR (object);
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (editor);
  PicmanContainer           *devices;
  GList                   *list;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (private->picman));

  devices = PICMAN_CONTAINER (picman_devices_get_manager (private->picman));

  /*  connect to "remove" before the container view does so we can get
   *  the notebook child stored in its model
   */
  g_signal_connect (devices, "remove",
                    G_CALLBACK (picman_device_editor_remove_device),
                    editor);

  picman_container_view_set_container (PICMAN_CONTAINER_VIEW (private->treeview),
                                     devices);

  picman_container_view_set_context (PICMAN_CONTAINER_VIEW (private->treeview),
                                   picman_get_user_context (private->picman));

  g_signal_connect (devices, "add",
                    G_CALLBACK (picman_device_editor_add_device),
                    editor);

  private->name_changed_handler =
    picman_container_add_handler (devices, "name-changed",
                                G_CALLBACK (picman_device_editor_device_changed),
                                editor);

  for (list = PICMAN_LIST (devices)->list;
       list;
       list = g_list_next (list))
    {
      picman_device_editor_add_device (devices, list->data, editor);
    }
}

static void
picman_device_editor_dispose (GObject *object)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (object);
  PicmanContainer           *devices;

  devices = PICMAN_CONTAINER (picman_devices_get_manager (private->picman));

  g_signal_handlers_disconnect_by_func (devices,
                                        picman_device_editor_add_device,
                                        object);

  g_signal_handlers_disconnect_by_func (devices,
                                        picman_device_editor_remove_device,
                                        object);

  if (private->name_changed_handler)
    {
      picman_container_remove_handler (devices, private->name_changed_handler);
      private->name_changed_handler = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_device_editor_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      private->picman = g_value_get_object (value); /* don't ref */
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_device_editor_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, private->picman);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_device_editor_add_device (PicmanContainer    *container,
                               PicmanDeviceInfo   *info,
                               PicmanDeviceEditor *editor)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (editor);
  GtkWidget               *widget;
  GtkTreeIter             *iter;

  widget = picman_device_info_editor_new (info);
  gtk_notebook_append_page (GTK_NOTEBOOK (private->notebook), widget, NULL);
  gtk_widget_show (widget);

  iter = picman_container_view_lookup (PICMAN_CONTAINER_VIEW (private->treeview),
                                     PICMAN_VIEWABLE (info));

  if (iter)
    {
      PicmanContainerTreeView *treeview;

      treeview = PICMAN_CONTAINER_TREE_VIEW (private->treeview);

      gtk_tree_store_set (GTK_TREE_STORE (treeview->model), iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_USER_DATA, widget,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_SENSITIVE,
                          picman_device_info_get_device (info, NULL) != NULL,
                          -1);
    }
}

static void
picman_device_editor_remove_device (PicmanContainer    *container,
                                  PicmanDeviceInfo   *info,
                                  PicmanDeviceEditor *editor)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (editor);
  GtkTreeIter             *iter;

  iter = picman_container_view_lookup (PICMAN_CONTAINER_VIEW (private->treeview),
                                     PICMAN_VIEWABLE (info));

  if (iter)
    {
      PicmanContainerTreeView *treeview;
      GtkWidget             *widget;

      treeview = PICMAN_CONTAINER_TREE_VIEW (private->treeview);

      gtk_tree_model_get (treeview->model, iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_USER_DATA, &widget,
                          -1);

      if (widget)
        gtk_widget_destroy (widget);
    }
}

static void
picman_device_editor_device_changed (PicmanDeviceInfo   *info,
                                   PicmanDeviceEditor *editor)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (editor);
  GtkTreeIter             *iter;

  iter = picman_container_view_lookup (PICMAN_CONTAINER_VIEW (private->treeview),
                                     PICMAN_VIEWABLE (info));

  if (iter)
    {
      PicmanContainerTreeView *treeview;

      treeview = PICMAN_CONTAINER_TREE_VIEW (private->treeview);

      gtk_tree_store_set (GTK_TREE_STORE (treeview->model), iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_SENSITIVE,
                          picman_device_info_get_device (info, NULL) != NULL,
                          -1);
    }
}

static void
picman_device_editor_select_device (PicmanContainerView *view,
                                  PicmanViewable      *viewable,
                                  gpointer           insert_data,
                                  PicmanDeviceEditor  *editor)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (editor);

  if (viewable && insert_data)
    {
      PicmanContainerTreeView *treeview;
      GtkWidget             *widget;

      treeview = PICMAN_CONTAINER_TREE_VIEW (private->treeview);

      gtk_tree_model_get (treeview->model, insert_data,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_USER_DATA, &widget,
                          -1);

      if (widget)
        {
          gint page_num = gtk_notebook_page_num (GTK_NOTEBOOK (private->notebook),
                                                 widget);

          gtk_notebook_set_current_page (GTK_NOTEBOOK (private->notebook),
                                         page_num);
        }
    }
}

static void
picman_device_editor_switch_page (GtkNotebook      *notebook,
                                gpointer          page,
                                guint             page_num,
                                PicmanDeviceEditor *editor)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (editor);
  GtkWidget               *widget;
  PicmanDeviceInfo          *info;
  gboolean                 delete_sensitive = FALSE;

  widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);

  g_object_get (widget ,"info", &info, NULL);

  gtk_label_set_text (GTK_LABEL (private->label),
                      picman_object_get_name (info));
  gtk_image_set_from_stock (GTK_IMAGE (private->image),
                            picman_viewable_get_stock_id (PICMAN_VIEWABLE (info)),
                            GTK_ICON_SIZE_BUTTON);

  if (! picman_device_info_get_device (info, NULL))
    delete_sensitive = TRUE;

  gtk_widget_set_sensitive (private->delete_button, delete_sensitive);

  g_object_unref (info);
}

static void
picman_device_editor_delete_response (GtkWidget        *dialog,
                                    gint              response_id,
                                    PicmanDeviceEditor *editor)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (editor);

  gtk_widget_destroy (dialog);

  if (response_id == GTK_RESPONSE_OK)
    {
      GList *selected;

      if (picman_container_view_get_selected (PICMAN_CONTAINER_VIEW (private->treeview),
                                            &selected))
        {
          PicmanContainer *devices;

          devices = PICMAN_CONTAINER (picman_devices_get_manager (private->picman));

          picman_container_remove (devices, selected->data);

          g_list_free (selected);
        }
    }

  gtk_widget_set_sensitive (GTK_WIDGET (editor), TRUE);
}

static void
picman_device_editor_delete_clicked (GtkWidget        *button,
                                   PicmanDeviceEditor *editor)
{
  PicmanDeviceEditorPrivate *private = PICMAN_DEVICE_EDITOR_GET_PRIVATE (editor);
  GtkWidget               *dialog;
  GList                   *selected;

  if (! picman_container_view_get_selected (PICMAN_CONTAINER_VIEW (private->treeview),
                                          &selected))
    return;

  dialog = picman_message_dialog_new (_("Delete Device Settings"),
                                    PICMAN_STOCK_QUESTION,
                                    gtk_widget_get_toplevel (GTK_WIDGET (editor)),
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    picman_standard_help_func, NULL,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_DELETE, GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (picman_device_editor_delete_response),
                    editor);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("Delete \"%s\"?"),
                                     picman_object_get_name (selected->data));
  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                             _("You are about to delete this device's "
                               "stored settings.\n"
                               "The next time this device is plugged, "
                               "default settings will be used."));

  g_list_free (selected);

  gtk_widget_set_sensitive (GTK_WIDGET (editor), FALSE);

  gtk_widget_show (dialog);
}


/*  public functions  */

GtkWidget *
picman_device_editor_new (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return g_object_new (PICMAN_TYPE_DEVICE_EDITOR,
                       "picman", picman,
                       NULL);
}
