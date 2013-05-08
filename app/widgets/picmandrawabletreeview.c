/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandrawabletreeview.c
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picman-edit.h"
#include "core/picmancontext.h"
#include "core/picmandrawable.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanpattern.h"

#include "picmancontainerview.h"
#include "picmandnd.h"
#include "picmandrawabletreeview.h"

#include "picman-intl.h"


static void   picman_drawable_tree_view_view_iface_init (PicmanContainerViewInterface *iface);

static void     picman_drawable_tree_view_constructed (GObject           *object);

static gboolean picman_drawable_tree_view_select_item (PicmanContainerView *view,
                                                     PicmanViewable      *item,
                                                     gpointer           insert_data);

static gboolean picman_drawable_tree_view_drop_possible(PicmanContainerTreeView *view,
                                                      PicmanDndType          src_type,
                                                      PicmanViewable        *src_viewable,
                                                      PicmanViewable        *dest_viewable,
                                                      GtkTreePath         *drop_path,
                                                      GtkTreeViewDropPosition  drop_pos,
                                                      GtkTreeViewDropPosition *return_drop_pos,
                                                      GdkDragAction       *return_drag_action);
static void   picman_drawable_tree_view_drop_viewable (PicmanContainerTreeView *view,
                                                     PicmanViewable     *src_viewable,
                                                     PicmanViewable     *dest_viewable,
                                                     GtkTreeViewDropPosition  drop_pos);
static void   picman_drawable_tree_view_drop_color (PicmanContainerTreeView *view,
                                                  const PicmanRGB       *color,
                                                  PicmanViewable        *dest_viewable,
                                                  GtkTreeViewDropPosition  drop_pos);

static void   picman_drawable_tree_view_set_image  (PicmanItemTreeView     *view,
                                                  PicmanImage            *image);

static void   picman_drawable_tree_view_floating_selection_changed
                                                 (PicmanImage            *image,
                                                  PicmanDrawableTreeView *view);

static void   picman_drawable_tree_view_new_pattern_dropped
                                                 (GtkWidget            *widget,
                                                  gint                  x,
                                                  gint                  y,
                                                  PicmanViewable         *viewable,
                                                  gpointer              data);
static void   picman_drawable_tree_view_new_color_dropped
                                                 (GtkWidget            *widget,
                                                  gint                  x,
                                                  gint                  y,
                                                  const PicmanRGB        *color,
                                                  gpointer              data);


G_DEFINE_TYPE_WITH_CODE (PicmanDrawableTreeView, picman_drawable_tree_view,
                         PICMAN_TYPE_ITEM_TREE_VIEW,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONTAINER_VIEW,
                                                picman_drawable_tree_view_view_iface_init))

#define parent_class picman_drawable_tree_view_parent_class

static PicmanContainerViewInterface *parent_view_iface = NULL;


static void
picman_drawable_tree_view_class_init (PicmanDrawableTreeViewClass *klass)
{
  GObjectClass               *object_class;
  PicmanContainerTreeViewClass *tree_view_class;
  PicmanItemTreeViewClass      *item_view_class;

  object_class    = G_OBJECT_CLASS (klass);
  tree_view_class = PICMAN_CONTAINER_TREE_VIEW_CLASS (klass);
  item_view_class = PICMAN_ITEM_TREE_VIEW_CLASS (klass);

  object_class->constructed      = picman_drawable_tree_view_constructed;

  tree_view_class->drop_possible = picman_drawable_tree_view_drop_possible;
  tree_view_class->drop_viewable = picman_drawable_tree_view_drop_viewable;
  tree_view_class->drop_color    = picman_drawable_tree_view_drop_color;

  item_view_class->set_image     = picman_drawable_tree_view_set_image;

  item_view_class->lock_content_stock_id  = PICMAN_STOCK_TOOL_PAINTBRUSH;
  item_view_class->lock_content_tooltip   = _("Lock pixels");
  item_view_class->lock_position_stock_id = PICMAN_STOCK_TOOL_MOVE;
  item_view_class->lock_position_tooltip  = _("Lock position and size");
}

static void
picman_drawable_tree_view_view_iface_init (PicmanContainerViewInterface *iface)
{
  parent_view_iface = g_type_interface_peek_parent (iface);

  iface->select_item = picman_drawable_tree_view_select_item;
}

static void
picman_drawable_tree_view_init (PicmanDrawableTreeView *view)
{
}

static void
picman_drawable_tree_view_constructed (GObject *object)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (object);
  PicmanItemTreeView      *item_view = PICMAN_ITEM_TREE_VIEW (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_dnd_viewable_dest_add (picman_item_tree_view_get_new_button (item_view),
                              PICMAN_TYPE_PATTERN,
                              picman_drawable_tree_view_new_pattern_dropped,
                              item_view);
  picman_dnd_color_dest_add (picman_item_tree_view_get_new_button (item_view),
                           picman_drawable_tree_view_new_color_dropped,
                           item_view);

  picman_dnd_color_dest_add    (GTK_WIDGET (tree_view->view),
                              NULL, tree_view);
  picman_dnd_viewable_dest_add (GTK_WIDGET (tree_view->view), PICMAN_TYPE_PATTERN,
                              NULL, tree_view);
}


/*  PicmanContainerView methods  */

static gboolean
picman_drawable_tree_view_select_item (PicmanContainerView *view,
                                     PicmanViewable      *item,
                                     gpointer           insert_data)
{
  PicmanItemTreeView *item_view = PICMAN_ITEM_TREE_VIEW (view);
  gboolean          success   = TRUE;

  if (picman_item_tree_view_get_image (item_view))
    {
      PicmanLayer *floating_sel =
        picman_image_get_floating_selection (picman_item_tree_view_get_image (item_view));

      success = (item         == NULL ||
                 floating_sel == NULL ||
                 item         == PICMAN_VIEWABLE (floating_sel));
    }

  if (success)
    success = parent_view_iface->select_item (view, item, insert_data);

  return success;
}


/*  PicmanContainerTreeView methods  */

static gboolean
picman_drawable_tree_view_drop_possible (PicmanContainerTreeView   *tree_view,
                                       PicmanDndType              src_type,
                                       PicmanViewable            *src_viewable,
                                       PicmanViewable            *dest_viewable,
                                       GtkTreePath             *drop_path,
                                       GtkTreeViewDropPosition  drop_pos,
                                       GtkTreeViewDropPosition *return_drop_pos,
                                       GdkDragAction           *return_drag_action)
{
  if (PICMAN_CONTAINER_TREE_VIEW_CLASS (parent_class)->drop_possible (tree_view,
                                                                    src_type,
                                                                    src_viewable,
                                                                    dest_viewable,
                                                                    drop_path,
                                                                    drop_pos,
                                                                    return_drop_pos,
                                                                    return_drag_action))
    {
      if (src_type == PICMAN_DND_TYPE_COLOR ||
          src_type == PICMAN_DND_TYPE_PATTERN)
        {
          if (! dest_viewable ||
              picman_item_is_content_locked (PICMAN_ITEM (dest_viewable)) ||
              picman_viewable_get_children (PICMAN_VIEWABLE (dest_viewable)))
            return FALSE;

          if (return_drop_pos)
            {
              *return_drop_pos = GTK_TREE_VIEW_DROP_INTO_OR_AFTER;
            }
        }

      return TRUE;
    }

  return FALSE;
}

static void
picman_drawable_tree_view_drop_viewable (PicmanContainerTreeView   *view,
                                       PicmanViewable            *src_viewable,
                                       PicmanViewable            *dest_viewable,
                                       GtkTreeViewDropPosition  drop_pos)
{
  if (dest_viewable && PICMAN_IS_PATTERN (src_viewable))
    {
      picman_edit_fill_full (picman_item_get_image (PICMAN_ITEM (dest_viewable)),
                           PICMAN_DRAWABLE (dest_viewable),
                           NULL, PICMAN_PATTERN (src_viewable),
                           PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE,
                           C_("undo-type", "Drop pattern to layer"));

      picman_image_flush (picman_item_get_image (PICMAN_ITEM (dest_viewable)));
      return;
    }

  PICMAN_CONTAINER_TREE_VIEW_CLASS (parent_class)->drop_viewable (view,
                                                                src_viewable,
                                                                dest_viewable,
                                                                drop_pos);
}

static void
picman_drawable_tree_view_drop_color (PicmanContainerTreeView   *view,
                                    const PicmanRGB           *color,
                                    PicmanViewable            *dest_viewable,
                                    GtkTreeViewDropPosition  drop_pos)
{
  if (dest_viewable)
    {
      picman_edit_fill_full (picman_item_get_image (PICMAN_ITEM (dest_viewable)),
                           PICMAN_DRAWABLE (dest_viewable),
                           color, NULL,
                           PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE,
                           C_("undo-type", "Drop color to layer"));

      picman_image_flush (picman_item_get_image (PICMAN_ITEM (dest_viewable)));
    }
}


/*  PicmanItemTreeView methods  */

static void
picman_drawable_tree_view_set_image (PicmanItemTreeView *view,
                                   PicmanImage        *image)
{
  if (picman_item_tree_view_get_image (view))
    g_signal_handlers_disconnect_by_func (picman_item_tree_view_get_image (view),
                                          picman_drawable_tree_view_floating_selection_changed,
                                          view);

  PICMAN_ITEM_TREE_VIEW_CLASS (parent_class)->set_image (view, image);

  if (picman_item_tree_view_get_image (view))
    g_signal_connect (picman_item_tree_view_get_image (view),
                      "floating-selection-changed",
                      G_CALLBACK (picman_drawable_tree_view_floating_selection_changed),
                      view);
}


/*  callbacks  */

static void
picman_drawable_tree_view_floating_selection_changed (PicmanImage            *image,
                                                    PicmanDrawableTreeView *view)
{
  PicmanItem *item;

  item = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (image);

  /*  update button states  */
  picman_container_view_select_item (PICMAN_CONTAINER_VIEW (view),
                                   (PicmanViewable *) item);
}

static void
picman_drawable_tree_view_new_dropped (PicmanItemTreeView   *view,
                                     gint                x,
                                     gint                y,
                                     const PicmanRGB      *color,
                                     PicmanPattern        *pattern)
{
  PicmanItem *item;

  picman_image_undo_group_start (picman_item_tree_view_get_image (view), PICMAN_UNDO_GROUP_EDIT_PASTE,
                               _("New Layer"));

  item = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->new_item (picman_item_tree_view_get_image (view));

  if (item)
    {
      picman_edit_fill_full (picman_item_get_image (item),
                           PICMAN_DRAWABLE (item),
                           color, pattern,
                           PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE,
                           pattern ?
                           C_("undo-type", "Drop pattern to layer") :
                           C_("undo-type", "Drop color to layer"));
    }

  picman_image_undo_group_end (picman_item_tree_view_get_image (view));

  picman_image_flush (picman_item_tree_view_get_image (view));
}

static void
picman_drawable_tree_view_new_pattern_dropped (GtkWidget    *widget,
                                             gint          x,
                                             gint          y,
                                             PicmanViewable *viewable,
                                             gpointer      data)
{
  picman_drawable_tree_view_new_dropped (PICMAN_ITEM_TREE_VIEW (data), x, y,
                                       NULL, PICMAN_PATTERN (viewable));
}

static void
picman_drawable_tree_view_new_color_dropped (GtkWidget     *widget,
                                           gint           x,
                                           gint           y,
                                           const PicmanRGB *color,
                                           gpointer       data)
{
  picman_drawable_tree_view_new_dropped (PICMAN_ITEM_TREE_VIEW (data), x, y,
                                       color, NULL);
}
