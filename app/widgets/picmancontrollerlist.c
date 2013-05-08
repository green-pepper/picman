/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmancontrollerlist.c
 * Copyright (C) 2005 Michael Natterer <mitch@picman.org>
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

#include "libpicmanwidgets/picmanwidgets.h"

#define PICMAN_ENABLE_CONTROLLER_UNDER_CONSTRUCTION
#include "libpicmanwidgets/picmancontroller.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"

#include "picmancontainertreeview.h"
#include "picmancontainerview.h"
#include "picmancontrollereditor.h"
#include "picmancontrollerlist.h"
#include "picmancontrollerinfo.h"
#include "picmancontrollerkeyboard.h"
#include "picmancontrollermouse.h"
#include "picmancontrollerwheel.h"
#include "picmancontrollers.h"
#include "picmandialogfactory.h"
#include "picmanhelp-ids.h"
#include "picmanmessagebox.h"
#include "picmanmessagedialog.h"
#include "picmanpropwidgets.h"
#include "picmanuimanager.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_PICMAN
};

enum
{
  COLUMN_ICON,
  COLUMN_NAME,
  COLUMN_TYPE,
  N_COLUMNS
};


static void picman_controller_list_constructed     (GObject            *object);
static void picman_controller_list_finalize        (GObject            *object);
static void picman_controller_list_set_property    (GObject            *object,
                                                  guint               property_id,
                                                  const GValue       *value,
                                                  GParamSpec         *pspec);
static void picman_controller_list_get_property    (GObject            *object,
                                                  guint               property_id,
                                                  GValue             *value,
                                                  GParamSpec         *pspec);

static void picman_controller_list_src_sel_changed (GtkTreeSelection   *sel,
                                                  PicmanControllerList *list);
static void picman_controller_list_row_activated   (GtkTreeView        *tv,
                                                  GtkTreePath        *path,
                                                  GtkTreeViewColumn  *column,
                                                  PicmanControllerList *list);

static void picman_controller_list_select_item     (PicmanContainerView  *view,
                                                  PicmanViewable       *viewable,
                                                  gpointer            insert_data,
                                                  PicmanControllerList *list);
static void picman_controller_list_activate_item   (PicmanContainerView  *view,
                                                  PicmanViewable       *viewable,
                                                  gpointer            insert_data,
                                                  PicmanControllerList *list);

static void picman_controller_list_add_clicked     (GtkWidget          *button,
                                                  PicmanControllerList *list);
static void picman_controller_list_remove_clicked  (GtkWidget          *button,
                                                  PicmanControllerList *list);

static void picman_controller_list_edit_clicked    (GtkWidget          *button,
                                                  PicmanControllerList *list);
static void picman_controller_list_edit_destroy    (GtkWidget          *widget,
                                                  PicmanControllerInfo *info);
static void picman_controller_list_up_clicked      (GtkWidget          *button,
                                                  PicmanControllerList *list);
static void picman_controller_list_down_clicked    (GtkWidget          *button,
                                                  PicmanControllerList *list);


G_DEFINE_TYPE (PicmanControllerList, picman_controller_list, GTK_TYPE_BOX)

#define parent_class picman_controller_list_parent_class


static void
picman_controller_list_class_init (PicmanControllerListClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_controller_list_constructed;
  object_class->finalize     = picman_controller_list_finalize;
  object_class->set_property = picman_controller_list_set_property;
  object_class->get_property = picman_controller_list_get_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_controller_list_init (PicmanControllerList *list)
{
  GtkWidget         *hbox;
  GtkWidget         *sw;
  GtkWidget         *tv;
  GtkTreeViewColumn *column;
  GtkCellRenderer   *cell;
  GtkWidget         *vbox;
  GtkWidget         *image;
  GtkIconSize        icon_size;
  gint               icon_width;
  gint               icon_height;
  GType             *controller_types;
  guint              n_controller_types;
  gint               i;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (list),
                                  GTK_ORIENTATION_VERTICAL);

  list->picman = NULL;

  list->hbox = hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (list), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (hbox), sw, TRUE, TRUE, 0);
  gtk_widget_show (sw);

  list->src = gtk_list_store_new (N_COLUMNS,
                                  G_TYPE_STRING,
                                  G_TYPE_STRING,
                                  G_TYPE_GTYPE);
  tv = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list->src));
  g_object_unref (list->src);

  gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (tv), FALSE);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, _("Available Controllers"));
  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);

  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "stock-id", COLUMN_ICON,
                                       NULL);

  g_object_get (cell, "stock-size", &icon_size, NULL);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, TRUE);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "text", COLUMN_NAME,
                                       NULL);

  gtk_container_add (GTK_CONTAINER (sw), tv);
  gtk_widget_show (tv);

  g_signal_connect_object (tv, "row-activated",
                           G_CALLBACK (picman_controller_list_row_activated),
                           G_OBJECT (list), 0);

  list->src_sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
  gtk_tree_selection_set_mode (list->src_sel, GTK_SELECTION_BROWSE);

  g_signal_connect_object (list->src_sel, "changed",
                           G_CALLBACK (picman_controller_list_src_sel_changed),
                           G_OBJECT (list), 0);

  controller_types = g_type_children (PICMAN_TYPE_CONTROLLER,
                                      &n_controller_types);

  for (i = 0; i < n_controller_types; i++)
    {
      PicmanControllerClass *controller_class;
      GtkTreeIter          iter;

      controller_class = g_type_class_ref (controller_types[i]);

      gtk_list_store_append (list->src, &iter);
      gtk_list_store_set (list->src, &iter,
                          COLUMN_ICON, controller_class->stock_id,
                          COLUMN_NAME, controller_class->name,
                          COLUMN_TYPE, controller_types[i],
                          -1);

      g_type_class_unref (controller_class);
    }

  g_free (controller_types);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_set_homogeneous (GTK_BOX (vbox), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  list->add_button = gtk_button_new ();
  gtk_box_pack_start (GTK_BOX (vbox), list->add_button, TRUE, FALSE, 0);
  gtk_widget_set_sensitive (list->add_button, FALSE);
  gtk_widget_show (list->add_button);

  image = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_BUTTON);
  gtk_container_add (GTK_CONTAINER (list->add_button), image);
  gtk_widget_show (image);

  g_signal_connect (list->add_button, "clicked",
                    G_CALLBACK (picman_controller_list_add_clicked),
                    list);

  g_object_add_weak_pointer (G_OBJECT (list->add_button),
                             (gpointer) &list->add_button);

  list->remove_button = gtk_button_new ();
  gtk_box_pack_start (GTK_BOX (vbox), list->remove_button, TRUE, FALSE, 0);
  gtk_widget_set_sensitive (list->remove_button, FALSE);
  gtk_widget_show (list->remove_button);

  image = gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_BUTTON);
  gtk_container_add (GTK_CONTAINER (list->remove_button), image);
  gtk_widget_show (image);

  g_signal_connect (list->remove_button, "clicked",
                    G_CALLBACK (picman_controller_list_remove_clicked),
                    list);

  g_object_add_weak_pointer (G_OBJECT (list->remove_button),
                             (gpointer) &list->remove_button);

  gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (GTK_WIDGET (list)),
                                     icon_size, &icon_width, &icon_height);

  list->dest = picman_container_tree_view_new (NULL, NULL, icon_height, 0);
  picman_container_tree_view_set_main_column_title (PICMAN_CONTAINER_TREE_VIEW (list->dest),
                                                  _("Active Controllers"));
  gtk_tree_view_set_headers_visible (PICMAN_CONTAINER_TREE_VIEW (list->dest)->view,
                                     TRUE);
  gtk_box_pack_start (GTK_BOX (list->hbox), list->dest, TRUE, TRUE, 0);
  gtk_widget_show (list->dest);

  g_signal_connect_object (list->dest, "select-item",
                           G_CALLBACK (picman_controller_list_select_item),
                           G_OBJECT (list), 0);
  g_signal_connect_object (list->dest, "activate-item",
                           G_CALLBACK (picman_controller_list_activate_item),
                           G_OBJECT (list), 0);

  list->edit_button =
    picman_editor_add_button (PICMAN_EDITOR (list->dest),
                            GTK_STOCK_PROPERTIES,
                            _("Configure the selected controller"),
                            NULL,
                            G_CALLBACK (picman_controller_list_edit_clicked),
                            NULL,
                            list);
  list->up_button =
    picman_editor_add_button (PICMAN_EDITOR (list->dest),
                            GTK_STOCK_GO_UP,
                            _("Move the selected controller up"),
                            NULL,
                            G_CALLBACK (picman_controller_list_up_clicked),
                            NULL,
                            list);
  list->down_button =
    picman_editor_add_button (PICMAN_EDITOR (list->dest),
                            GTK_STOCK_GO_DOWN,
                            _("Move the selected controller down"),
                            NULL,
                            G_CALLBACK (picman_controller_list_down_clicked),
                            NULL,
                            list);

  gtk_widget_set_sensitive (list->edit_button, FALSE);
  gtk_widget_set_sensitive (list->up_button,   FALSE);
  gtk_widget_set_sensitive (list->down_button, FALSE);
}

static void
picman_controller_list_constructed (GObject *object)
{
  PicmanControllerList *list = PICMAN_CONTROLLER_LIST (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (list->picman));

  picman_container_view_set_container (PICMAN_CONTAINER_VIEW (list->dest),
                                     picman_controllers_get_list (list->picman));

  picman_container_view_set_context (PICMAN_CONTAINER_VIEW (list->dest),
                                   picman_get_user_context (list->picman));
}

static void
picman_controller_list_finalize (GObject *object)
{
  PicmanControllerList *list = PICMAN_CONTROLLER_LIST (object);

  if (list->picman)
    {
      g_object_unref (list->picman);
      list->picman = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_controller_list_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanControllerList *list = PICMAN_CONTROLLER_LIST (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      list->picman = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_controller_list_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanControllerList *list = PICMAN_CONTROLLER_LIST (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, list->picman);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  public functions  */

GtkWidget *
picman_controller_list_new (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return g_object_new (PICMAN_TYPE_CONTROLLER_LIST,
                       "picman", picman,
                       NULL);
}


/*  private functions  */

static void
picman_controller_list_src_sel_changed (GtkTreeSelection   *sel,
                                      PicmanControllerList *list)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gchar        *tip = NULL;

  if (gtk_tree_selection_get_selected (sel, &model, &iter))
    {
      gchar *name;

      gtk_tree_model_get (model, &iter,
                          COLUMN_NAME, &name,
                          COLUMN_TYPE, &list->src_gtype,
                          -1);

      if (list->add_button)
        {
          tip =
            g_strdup_printf (_("Add '%s' to the list of active controllers"),
                             name);
          gtk_widget_set_sensitive (list->add_button, TRUE);
        }

      g_free (name);
    }
  else
    {
      if (list->add_button)
        gtk_widget_set_sensitive (list->add_button, FALSE);
    }

  if (list->add_button)
    {
      picman_help_set_help_data (list->add_button, tip, NULL);
      g_free (tip);
    }
}

static void
picman_controller_list_row_activated (GtkTreeView        *tv,
                                    GtkTreePath        *path,
                                    GtkTreeViewColumn  *column,
                                    PicmanControllerList *list)
{
  if (gtk_widget_is_sensitive (list->add_button))
    gtk_button_clicked (GTK_BUTTON (list->add_button));
}

static void
picman_controller_list_select_item (PicmanContainerView  *view,
                                  PicmanViewable       *viewable,
                                  gpointer            insert_data,
                                  PicmanControllerList *list)
{
  gboolean selected;

  list->dest_info = PICMAN_CONTROLLER_INFO (viewable);

  selected = PICMAN_IS_CONTROLLER_INFO (list->dest_info);

  if (list->remove_button)
    {
      PicmanObject *object = PICMAN_OBJECT (list->dest_info);
      gchar      *tip    = NULL;

      gtk_widget_set_sensitive (list->remove_button, selected);

      if (selected)
        tip =
          g_strdup_printf (_("Remove '%s' from the list of active controllers"),
                           picman_object_get_name (object));

      picman_help_set_help_data (list->remove_button, tip, NULL);
      g_free (tip);
    }

  gtk_widget_set_sensitive (list->edit_button, selected);
  gtk_widget_set_sensitive (list->up_button,   selected);
  gtk_widget_set_sensitive (list->down_button, selected);
}

static void
picman_controller_list_activate_item (PicmanContainerView  *view,
                                    PicmanViewable       *viewable,
                                    gpointer            insert_data,
                                    PicmanControllerList *list)
{
  if (gtk_widget_is_sensitive (list->edit_button))
    gtk_button_clicked (GTK_BUTTON (list->edit_button));
}

static void
picman_controller_list_add_clicked (GtkWidget          *button,
                                  PicmanControllerList *list)
{
  PicmanControllerInfo *info;
  PicmanContainer      *container;

  if (list->src_gtype == PICMAN_TYPE_CONTROLLER_KEYBOARD &&
      picman_controllers_get_keyboard (list->picman) != NULL)
    {
      picman_message_literal (list->picman,
			    G_OBJECT (button), PICMAN_MESSAGE_WARNING,
			    _("There can only be one active keyboard "
			      "controller.\n\n"
			      "You already have a keyboard controller in "
			      "your list of active controllers."));
      return;
    }
  else if (list->src_gtype == PICMAN_TYPE_CONTROLLER_WHEEL &&
           picman_controllers_get_wheel (list->picman) != NULL)
    {
      picman_message_literal (list->picman,
			    G_OBJECT (button), PICMAN_MESSAGE_WARNING,
			    _("There can only be one active wheel "
			      "controller.\n\n"
			      "You already have a wheel controller in "
			      "your list of active controllers."));
      return;
    }
  else if (list->src_gtype == PICMAN_TYPE_CONTROLLER_MOUSE &&
           picman_controllers_get_mouse (list->picman) != NULL)
    {
      picman_message_literal (list->picman,
			    G_OBJECT (button), PICMAN_MESSAGE_WARNING,
			    _("There can only be one active mouse "
			      "controller.\n\n"
			      "You already have a mouse controller in "
			      "your list of active controllers."));
      return;
    }

  info = picman_controller_info_new (list->src_gtype);
  container = picman_controllers_get_list (list->picman);
  picman_container_add (container, PICMAN_OBJECT (info));
  g_object_unref (info);

  picman_container_view_select_item (PICMAN_CONTAINER_VIEW (list->dest),
                                   PICMAN_VIEWABLE (info));
  picman_controller_list_edit_clicked (NULL, list);
}

static void
picman_controller_list_remove_clicked (GtkWidget          *button,
                                     PicmanControllerList *list)
{
  GtkWidget   *dialog;
  const gchar *name;

#define RESPONSE_DISABLE 1

  dialog = picman_message_dialog_new (_("Remove Controller?"),
                                    PICMAN_STOCK_WARNING,
                                    GTK_WIDGET (list), GTK_DIALOG_MODAL,
                                    NULL, NULL,

                                    _("Disable Controller"), RESPONSE_DISABLE,
                                    GTK_STOCK_CANCEL,        GTK_RESPONSE_CANCEL,
                                    _("Remove Controller"),  GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           RESPONSE_DISABLE,
                                           -1);

  name = picman_object_get_name (list->dest_info);
  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("Remove Controller '%s'?"), name);

  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
			     "%s",
                             _("Removing this controller from the list of "
			       "active controllers will permanently delete "
			       "all event mappings you have configured.\n\n"
			       "Selecting \"Disable Controller\" will disable "
			       "the controller without removing it."));

  switch (picman_dialog_run (PICMAN_DIALOG (dialog)))
    {
    case RESPONSE_DISABLE:
      picman_controller_info_set_enabled (list->dest_info, FALSE);
      break;

    case GTK_RESPONSE_OK:
      {
        GtkWidget     *editor_dialog;
        PicmanContainer *container;

        editor_dialog = g_object_get_data (G_OBJECT (list->dest_info),
                                           "picman-controller-editor-dialog");

        if (editor_dialog)
          gtk_dialog_response (GTK_DIALOG (editor_dialog),
                               GTK_RESPONSE_DELETE_EVENT);

        container = picman_controllers_get_list (list->picman);
        picman_container_remove (container, PICMAN_OBJECT (list->dest_info));
      }
      break;

    default:
      break;
    }

  gtk_widget_destroy (dialog);
}

static void
picman_controller_list_edit_clicked (GtkWidget          *button,
                                   PicmanControllerList *list)
{
  GtkWidget *dialog;
  GtkWidget *editor;

  dialog = g_object_get_data (G_OBJECT (list->dest_info),
                              "picman-controller-editor-dialog");

  if (dialog)
    {
      gtk_window_present (GTK_WINDOW (dialog));
      return;
    }

  dialog = picman_dialog_new (_("Configure Input Controller"),
                            "picman-controller-editor-dialog",
                            gtk_widget_get_toplevel (GTK_WIDGET (list)),
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            picman_standard_help_func,
                            PICMAN_HELP_PREFS_INPUT_CONTROLLERS,

                            GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,

                            NULL);

  picman_dialog_factory_add_foreign (picman_dialog_factory_get_singleton (),
                                   "picman-controller-editor-dialog",
                                   dialog);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);

  editor = picman_controller_editor_new (list->dest_info,
                                       picman_get_user_context (list->picman));
  gtk_container_set_border_width (GTK_CONTAINER (editor), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      editor, TRUE, TRUE, 0);
  gtk_widget_show (editor);

  g_object_set_data (G_OBJECT (list->dest_info), "picman-controller-editor-dialog",
                     dialog);

  g_signal_connect_object (dialog, "destroy",
                           G_CALLBACK (picman_controller_list_edit_destroy),
                           G_OBJECT (list->dest_info), 0);

  g_signal_connect_object (list, "destroy",
                           G_CALLBACK (gtk_widget_destroy),
                           G_OBJECT (dialog),
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (list, "unmap",
                           G_CALLBACK (gtk_widget_destroy),
                           G_OBJECT (dialog),
                           G_CONNECT_SWAPPED);

  gtk_widget_show (dialog);
}

static void
picman_controller_list_edit_destroy (GtkWidget          *widget,
                                   PicmanControllerInfo *info)
{
  g_object_set_data (G_OBJECT (info), "picman-controller-editor-dialog", NULL);
}

static void
picman_controller_list_up_clicked (GtkWidget          *button,
                                 PicmanControllerList *list)
{
  PicmanContainer *container;
  gint           index;

  container = picman_controllers_get_list (list->picman);

  index = picman_container_get_child_index (container,
                                          PICMAN_OBJECT (list->dest_info));

  if (index > 0)
    picman_container_reorder (container, PICMAN_OBJECT (list->dest_info),
                            index - 1);
}

static void
picman_controller_list_down_clicked (GtkWidget          *button,
                                   PicmanControllerList *list)
{
  PicmanContainer *container;
  gint           index;

  container = picman_controllers_get_list (list->picman);

  index = picman_container_get_child_index (container,
                                          PICMAN_OBJECT (list->dest_info));

  if (index < picman_container_get_n_children (container) - 1)
    picman_container_reorder (container, PICMAN_OBJECT (list->dest_info),
                            index + 1);
}
