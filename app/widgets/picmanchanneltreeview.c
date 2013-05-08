/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanchanneltreeview.c
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanchannel.h"
#include "core/picmancontainer.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanlayer.h"
#include "core/picmanlayermask.h"

#include "picmanactiongroup.h"
#include "picmanchanneltreeview.h"
#include "picmancomponenteditor.h"
#include "picmancontainerview.h"
#include "picmandnd.h"
#include "picmandocked.h"
#include "picmanhelp-ids.h"
#include "picmanuimanager.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


struct _PicmanChannelTreeViewPriv
{
  GtkWidget *component_editor;

  GtkWidget *toselection_button;
};


static void  picman_channel_tree_view_view_iface_init   (PicmanContainerViewInterface *iface);

static void   picman_channel_tree_view_constructed      (GObject                 *object);

static void   picman_channel_tree_view_drop_viewable    (PicmanContainerTreeView   *view,
                                                       PicmanViewable            *src_viewable,
                                                       PicmanViewable            *dest_viewable,
                                                       GtkTreeViewDropPosition  drop_pos);
static void   picman_channel_tree_view_drop_component   (PicmanContainerTreeView   *tree_view,
                                                       PicmanImage               *image,
                                                       PicmanChannelType          component,
                                                       PicmanViewable            *dest_viewable,
                                                       GtkTreeViewDropPosition  drop_pos);
static void   picman_channel_tree_view_set_image        (PicmanItemTreeView        *item_view,
                                                       PicmanImage               *image);
static PicmanItem * picman_channel_tree_view_item_new     (PicmanImage               *image);

static void   picman_channel_tree_view_set_context      (PicmanContainerView       *view,
                                                       PicmanContext             *context);
static void   picman_channel_tree_view_set_view_size    (PicmanContainerView       *view);


G_DEFINE_TYPE_WITH_CODE (PicmanChannelTreeView, picman_channel_tree_view,
                         PICMAN_TYPE_DRAWABLE_TREE_VIEW,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONTAINER_VIEW,
                                                picman_channel_tree_view_view_iface_init))

#define parent_class picman_channel_tree_view_parent_class

static PicmanContainerViewInterface *parent_view_iface = NULL;


static void
picman_channel_tree_view_class_init (PicmanChannelTreeViewClass *klass)
{
  GObjectClass               *object_class = G_OBJECT_CLASS (klass);
  PicmanContainerTreeViewClass *view_class   = PICMAN_CONTAINER_TREE_VIEW_CLASS (klass);
  PicmanItemTreeViewClass      *iv_class     = PICMAN_ITEM_TREE_VIEW_CLASS (klass);

  object_class->constructed  = picman_channel_tree_view_constructed;

  view_class->drop_viewable  = picman_channel_tree_view_drop_viewable;
  view_class->drop_component = picman_channel_tree_view_drop_component;

  iv_class->set_image        = picman_channel_tree_view_set_image;

  iv_class->item_type        = PICMAN_TYPE_CHANNEL;
  iv_class->signal_name      = "active-channel-changed";

  iv_class->get_container    = picman_image_get_channels;
  iv_class->get_active_item  = (PicmanGetItemFunc) picman_image_get_active_channel;
  iv_class->set_active_item  = (PicmanSetItemFunc) picman_image_set_active_channel;
  iv_class->add_item         = (PicmanAddItemFunc) picman_image_add_channel;
  iv_class->remove_item      = (PicmanRemoveItemFunc) picman_image_remove_channel;
  iv_class->new_item         = picman_channel_tree_view_item_new;

  iv_class->action_group          = "channels";
  iv_class->activate_action       = "channels-edit-attributes";
  iv_class->edit_action           = "channels-edit-attributes";
  iv_class->new_action            = "channels-new";
  iv_class->new_default_action    = "channels-new-last-values";
  iv_class->raise_action          = "channels-raise";
  iv_class->raise_top_action      = "channels-raise-to-top";
  iv_class->lower_action          = "channels-lower";
  iv_class->lower_bottom_action   = "channels-lower-to-bottom";
  iv_class->duplicate_action      = "channels-duplicate";
  iv_class->delete_action         = "channels-delete";
  iv_class->lock_content_help_id  = PICMAN_HELP_CHANNEL_LOCK_PIXELS;
  iv_class->lock_position_help_id = PICMAN_HELP_CHANNEL_LOCK_POSITION;

  g_type_class_add_private (klass, sizeof (PicmanChannelTreeViewPriv));
}

static void
picman_channel_tree_view_view_iface_init (PicmanContainerViewInterface *view_iface)
{
  parent_view_iface = g_type_interface_peek_parent (view_iface);

  view_iface->set_context   = picman_channel_tree_view_set_context;
  view_iface->set_view_size = picman_channel_tree_view_set_view_size;
}

static void
picman_channel_tree_view_init (PicmanChannelTreeView *view)
{
  view->priv = G_TYPE_INSTANCE_GET_PRIVATE (view,
                                            PICMAN_TYPE_CHANNEL_TREE_VIEW,
                                            PicmanChannelTreeViewPriv);

  view->priv->component_editor   = NULL;
  view->priv->toselection_button = NULL;
}

static void
picman_channel_tree_view_constructed (GObject *object)
{
  PicmanChannelTreeView   *view      = PICMAN_CHANNEL_TREE_VIEW (object);
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (object);
  GdkModifierType        extend_mask;
  GdkModifierType        modify_mask;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  extend_mask = gtk_widget_get_modifier_mask (GTK_WIDGET (object),
                                              GDK_MODIFIER_INTENT_EXTEND_SELECTION);
  modify_mask = gtk_widget_get_modifier_mask (GTK_WIDGET (object),
                                              GDK_MODIFIER_INTENT_MODIFY_SELECTION);

  picman_dnd_viewable_dest_add  (GTK_WIDGET (tree_view->view), PICMAN_TYPE_LAYER,
                               NULL, tree_view);
  picman_dnd_viewable_dest_add  (GTK_WIDGET (tree_view->view), PICMAN_TYPE_LAYER_MASK,
                               NULL, tree_view);
  picman_dnd_component_dest_add (GTK_WIDGET (tree_view->view),
                               NULL, tree_view);

  view->priv->toselection_button =
    picman_editor_add_action_button (PICMAN_EDITOR (view), "channels",
                                   "channels-selection-replace",
                                   "channels-selection-add",
                                   extend_mask,
                                   "channels-selection-subtract",
                                   modify_mask,
                                   "channels-selection-intersect",
                                   extend_mask | modify_mask,
                                   NULL);
  picman_container_view_enable_dnd (PICMAN_CONTAINER_VIEW (view),
                                  GTK_BUTTON (view->priv->toselection_button),
                                  PICMAN_TYPE_CHANNEL);
  gtk_box_reorder_child (picman_editor_get_button_box (PICMAN_EDITOR (view)),
                         view->priv->toselection_button, 5);
}


/*  PicmanContainerTreeView methods  */

static void
picman_channel_tree_view_drop_viewable (PicmanContainerTreeView   *tree_view,
                                      PicmanViewable            *src_viewable,
                                      PicmanViewable            *dest_viewable,
                                      GtkTreeViewDropPosition  drop_pos)
{
  PicmanItemTreeView      *item_view = PICMAN_ITEM_TREE_VIEW (tree_view);
  PicmanImage             *image     = picman_item_tree_view_get_image (item_view);
  PicmanItemTreeViewClass *item_view_class;

  item_view_class = PICMAN_ITEM_TREE_VIEW_GET_CLASS (item_view);

  if (PICMAN_IS_DRAWABLE (src_viewable) &&
      (image != picman_item_get_image (PICMAN_ITEM (src_viewable)) ||
       G_TYPE_FROM_INSTANCE (src_viewable) != item_view_class->item_type))
    {
      PicmanItem *new_item;
      PicmanItem *parent;
      gint      index;

      index = picman_item_tree_view_get_drop_index (item_view, dest_viewable,
                                                  drop_pos,
                                                  (PicmanViewable **) &parent);

      new_item = picman_item_convert (PICMAN_ITEM (src_viewable),
                                    picman_item_tree_view_get_image (item_view),
                                    item_view_class->item_type);

      picman_item_set_linked (new_item, FALSE, FALSE);

      item_view_class->add_item (image, new_item, parent, index, TRUE);

      picman_image_flush (image);

      return;
    }

  PICMAN_CONTAINER_TREE_VIEW_CLASS (parent_class)->drop_viewable (tree_view,
                                                                src_viewable,
                                                                dest_viewable,
                                                                drop_pos);
}

static void
picman_channel_tree_view_drop_component (PicmanContainerTreeView   *tree_view,
                                       PicmanImage               *src_image,
                                       PicmanChannelType          component,
                                       PicmanViewable            *dest_viewable,
                                       GtkTreeViewDropPosition  drop_pos)
{
  PicmanItemTreeView *item_view = PICMAN_ITEM_TREE_VIEW (tree_view);
  PicmanImage        *image     = picman_item_tree_view_get_image (item_view);
  PicmanItem         *new_item;
  PicmanChannel      *parent;
  gint              index;
  const gchar      *desc;
  gchar            *name;

  index = picman_item_tree_view_get_drop_index (item_view, dest_viewable,
                                              drop_pos,
                                              (PicmanViewable **) &parent);

  picman_enum_get_value (PICMAN_TYPE_CHANNEL_TYPE, component,
                       NULL, NULL, &desc, NULL);
  name = g_strdup_printf (_("%s Channel Copy"), desc);

  new_item = PICMAN_ITEM (picman_channel_new_from_component (src_image, component,
                                                         name, NULL));

  /*  copied components are invisible by default so subsequent copies
   *  of components don't affect each other
   */
  picman_item_set_visible (new_item, FALSE, FALSE);

  g_free (name);

  if (src_image != image)
    PICMAN_ITEM_GET_CLASS (new_item)->convert (new_item, image);

  picman_image_add_channel (image, PICMAN_CHANNEL (new_item), parent, index, TRUE);

  picman_image_flush (image);
}


/*  PicmanItemTreeView methods  */

static void
picman_channel_tree_view_set_image (PicmanItemTreeView *item_view,
                                  PicmanImage        *image)
{
  PicmanChannelTreeView *channel_view = PICMAN_CHANNEL_TREE_VIEW (item_view);

  if (! channel_view->priv->component_editor)
    {
      PicmanContainerView *view = PICMAN_CONTAINER_VIEW (item_view);
      gint               view_size;

      view_size = picman_container_view_get_view_size (view, NULL);

      channel_view->priv->component_editor =
        picman_component_editor_new (view_size,
                                   picman_editor_get_menu_factory (PICMAN_EDITOR (item_view)));
      picman_docked_set_context (PICMAN_DOCKED (channel_view->priv->component_editor),
                               picman_container_view_get_context (view));
      gtk_box_pack_start (GTK_BOX (item_view), channel_view->priv->component_editor,
                          FALSE, FALSE, 0);
      gtk_box_reorder_child (GTK_BOX (item_view),
                             channel_view->priv->component_editor, 0);
    }

  if (! image)
    gtk_widget_hide (channel_view->priv->component_editor);

  picman_image_editor_set_image (PICMAN_IMAGE_EDITOR (channel_view->priv->component_editor),
                               image);

  PICMAN_ITEM_TREE_VIEW_CLASS (parent_class)->set_image (item_view, image);

  if (picman_item_tree_view_get_image (item_view))
    gtk_widget_show (channel_view->priv->component_editor);
}

static PicmanItem *
picman_channel_tree_view_item_new (PicmanImage *image)
{
  PicmanChannel *new_channel;
  PicmanRGB      color;

  picman_rgba_set (&color, 0.0, 0.0, 0.0, 0.5);

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_EDIT_PASTE,
                               _("New Channel"));

  new_channel = picman_channel_new (image,
                                  picman_image_get_width (image),
                                  picman_image_get_height (image),
                                  _("Channel"), &color);

  picman_image_add_channel (image, new_channel,
                          PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  picman_image_undo_group_end (image);

  return PICMAN_ITEM (new_channel);
}


/*  PicmanContainerView methods  */

static void
picman_channel_tree_view_set_context (PicmanContainerView *view,
                                    PicmanContext       *context)
{
  PicmanChannelTreeView *channel_view = PICMAN_CHANNEL_TREE_VIEW (view);

  parent_view_iface->set_context (view, context);

  if (channel_view->priv->component_editor)
    picman_docked_set_context (PICMAN_DOCKED (channel_view->priv->component_editor),
                             context);
}

static void
picman_channel_tree_view_set_view_size (PicmanContainerView *view)
{
  PicmanChannelTreeView *channel_view = PICMAN_CHANNEL_TREE_VIEW (view);
  gint                 view_size;

  parent_view_iface->set_view_size (view);

  view_size = picman_container_view_get_view_size (view, NULL);

  if (channel_view->priv->component_editor)
    picman_component_editor_set_view_size (PICMAN_COMPONENT_EDITOR (channel_view->priv->component_editor),
                                         view_size);
}
