/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanvectorstreeview.c
 * Copyright (C) 2001-2009 Michael Natterer <mitch@picman.org>
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

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"

#include "vectors/picmanvectors.h"
#include "vectors/picmanvectors-export.h"
#include "vectors/picmanvectors-import.h"

#include "picmanactiongroup.h"
#include "picmancontainerview.h"
#include "picmandnd.h"
#include "picmanhelp-ids.h"
#include "picmanuimanager.h"
#include "picmanvectorstreeview.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


static void    picman_vectors_tree_view_view_iface_init (PicmanContainerViewInterface *iface);

static void      picman_vectors_tree_view_constructed   (GObject                  *object);

static void      picman_vectors_tree_view_set_container (PicmanContainerView        *view,
                                                       PicmanContainer            *container);
static void      picman_vectors_tree_view_drop_svg      (PicmanContainerTreeView    *tree_view,
                                                       const gchar              *svg_data,
                                                       gsize                     svg_data_len,
                                                       PicmanViewable             *dest_viewable,
                                                       GtkTreeViewDropPosition   drop_pos);
static PicmanItem * picman_vectors_tree_view_item_new     (PicmanImage                *image);
static guchar   * picman_vectors_tree_view_drag_svg     (GtkWidget                *widget,
                                                       gsize                    *svg_data_len,
                                                       gpointer                  data);


G_DEFINE_TYPE_WITH_CODE (PicmanVectorsTreeView, picman_vectors_tree_view,
                         PICMAN_TYPE_ITEM_TREE_VIEW,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONTAINER_VIEW,
                                                picman_vectors_tree_view_view_iface_init))

#define parent_class picman_vectors_tree_view_parent_class

static PicmanContainerViewInterface *parent_view_iface = NULL;


static void
picman_vectors_tree_view_class_init (PicmanVectorsTreeViewClass *klass)
{
  GObjectClass               *object_class = G_OBJECT_CLASS (klass);
  PicmanContainerTreeViewClass *view_class   = PICMAN_CONTAINER_TREE_VIEW_CLASS (klass);
  PicmanItemTreeViewClass      *iv_class     = PICMAN_ITEM_TREE_VIEW_CLASS (klass);

  object_class->constructed = picman_vectors_tree_view_constructed;

  view_class->drop_svg      = picman_vectors_tree_view_drop_svg;

  iv_class->item_type       = PICMAN_TYPE_VECTORS;
  iv_class->signal_name     = "active-vectors-changed";

  iv_class->get_container   = picman_image_get_vectors;
  iv_class->get_active_item = (PicmanGetItemFunc) picman_image_get_active_vectors;
  iv_class->set_active_item = (PicmanSetItemFunc) picman_image_set_active_vectors;
  iv_class->add_item        = (PicmanAddItemFunc) picman_image_add_vectors;
  iv_class->remove_item     = (PicmanRemoveItemFunc) picman_image_remove_vectors;
  iv_class->new_item        = picman_vectors_tree_view_item_new;

  iv_class->action_group           = "vectors";
  iv_class->activate_action        = "vectors-path-tool";
  iv_class->edit_action            = "vectors-edit-attributes";
  iv_class->new_action             = "vectors-new";
  iv_class->new_default_action     = "vectors-new-last-values";
  iv_class->raise_action           = "vectors-raise";
  iv_class->raise_top_action       = "vectors-raise-to-top";
  iv_class->lower_action           = "vectors-lower";
  iv_class->lower_bottom_action    = "vectors-lower-to-bottom";
  iv_class->duplicate_action       = "vectors-duplicate";
  iv_class->delete_action          = "vectors-delete";
  iv_class->lock_content_stock_id  = PICMAN_STOCK_TOOL_PATH;
  iv_class->lock_content_tooltip   = _("Lock path strokes");
  iv_class->lock_content_help_id   = PICMAN_HELP_PATH_LOCK_STROKES;
  iv_class->lock_position_stock_id = PICMAN_STOCK_TOOL_MOVE;
  iv_class->lock_position_tooltip  = _("Lock path position");
  iv_class->lock_position_help_id  = PICMAN_HELP_PATH_LOCK_POSITION;
}

static void
picman_vectors_tree_view_view_iface_init (PicmanContainerViewInterface *iface)
{
  parent_view_iface = g_type_interface_peek_parent (iface);

  iface->set_container = picman_vectors_tree_view_set_container;
}

static void
picman_vectors_tree_view_init (PicmanVectorsTreeView *view)
{
}

static void
picman_vectors_tree_view_constructed (GObject *object)
{
  PicmanEditor            *editor    = PICMAN_EDITOR (object);
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (object);
  PicmanVectorsTreeView   *view      = PICMAN_VECTORS_TREE_VIEW (object);
  GdkModifierType        extend_mask;
  GdkModifierType        modify_mask;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  extend_mask = gtk_widget_get_modifier_mask (GTK_WIDGET (object),
                                              GDK_MODIFIER_INTENT_EXTEND_SELECTION);
  modify_mask = gtk_widget_get_modifier_mask (GTK_WIDGET (object),
                                              GDK_MODIFIER_INTENT_MODIFY_SELECTION);

  /*  hide basically useless edit button  */
  gtk_widget_hide (picman_item_tree_view_get_edit_button (PICMAN_ITEM_TREE_VIEW (view)));

  view->toselection_button =
    picman_editor_add_action_button (editor, "vectors",
                                   "vectors-selection-replace",
                                   "vectors-selection-add",
                                   extend_mask,
                                   "vectors-selection-subtract",
                                   modify_mask,
                                   "vectors-selection-intersect",
                                   extend_mask | modify_mask,
                                   NULL);
  picman_container_view_enable_dnd (PICMAN_CONTAINER_VIEW (editor),
                                  GTK_BUTTON (view->toselection_button),
                                  PICMAN_TYPE_VECTORS);
  gtk_box_reorder_child (picman_editor_get_button_box (editor),
                         view->toselection_button, 5);

  view->tovectors_button =
    picman_editor_add_action_button (editor, "vectors",
                                   "vectors-selection-to-vectors",
                                   "vectors-selection-to-vectors-advanced",
                                   GDK_SHIFT_MASK,
                                   NULL);
  gtk_box_reorder_child (picman_editor_get_button_box (editor),
                         view->tovectors_button, 6);

  view->stroke_button =
    picman_editor_add_action_button (editor, "vectors",
                                   "vectors-stroke",
                                   "vectors-stroke-last-values",
                                   GDK_SHIFT_MASK,
                                   NULL);
  picman_container_view_enable_dnd (PICMAN_CONTAINER_VIEW (editor),
                                  GTK_BUTTON (view->stroke_button),
                                  PICMAN_TYPE_VECTORS);
  gtk_box_reorder_child (picman_editor_get_button_box (editor),
                         view->stroke_button, 7);

  picman_dnd_svg_dest_add (GTK_WIDGET (tree_view->view), NULL, view);
}

static void
picman_vectors_tree_view_set_container (PicmanContainerView *view,
                                      PicmanContainer     *container)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  PicmanContainer         *old_container;

  old_container = picman_container_view_get_container (PICMAN_CONTAINER_VIEW (view));

  if (old_container && ! container)
    {
      picman_dnd_svg_source_remove (GTK_WIDGET (tree_view->view));
    }

  parent_view_iface->set_container (view, container);

  if (! old_container && container)
    {
      picman_dnd_svg_source_add (GTK_WIDGET (tree_view->view),
                               picman_vectors_tree_view_drag_svg,
                               tree_view);
    }
}

static void
picman_vectors_tree_view_drop_svg (PicmanContainerTreeView   *tree_view,
                                 const gchar             *svg_data,
                                 gsize                    svg_data_len,
                                 PicmanViewable            *dest_viewable,
                                 GtkTreeViewDropPosition  drop_pos)
{
  PicmanItemTreeView *item_view = PICMAN_ITEM_TREE_VIEW (tree_view);
  PicmanImage        *image     = picman_item_tree_view_get_image (item_view);
  PicmanVectors      *parent;
  gint              index;
  GError           *error = NULL;

  if (image->picman->be_verbose)
    g_print ("%s: SVG dropped (len = %d)\n", G_STRFUNC, (gint) svg_data_len);

  index = picman_item_tree_view_get_drop_index (item_view, dest_viewable,
                                              drop_pos,
                                              (PicmanViewable **) &parent);

  if (! picman_vectors_import_buffer (image, svg_data, svg_data_len,
                                    TRUE, FALSE, parent, index, NULL, &error))
    {
      picman_message_literal (image->picman,
			    G_OBJECT (tree_view), PICMAN_MESSAGE_ERROR,
			    error->message);
      g_clear_error (&error);
    }
  else
    {
      picman_image_flush (image);
    }
}

static PicmanItem *
picman_vectors_tree_view_item_new (PicmanImage *image)
{
  PicmanVectors *new_vectors;

  new_vectors = picman_vectors_new (image, _("Path"));

  picman_image_add_vectors (image, new_vectors,
                          PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  return PICMAN_ITEM (new_vectors);
}

static guchar *
picman_vectors_tree_view_drag_svg (GtkWidget *widget,
                                 gsize     *svg_data_len,
                                 gpointer   data)
{
  PicmanItemTreeView *view  = PICMAN_ITEM_TREE_VIEW (data);
  PicmanImage        *image = picman_item_tree_view_get_image (view);
  PicmanItem         *item;
  gchar            *svg_data = NULL;

  item = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (image);

  *svg_data_len = 0;

  if (item)
    {
      svg_data = picman_vectors_export_string (image, PICMAN_VECTORS (item));

      if (svg_data)
        *svg_data_len = strlen (svg_data);
    }

  return (guchar *) svg_data;
}
