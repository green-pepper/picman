/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpanedbox.c
 * Copyright (C) 2001-2005 Michael Natterer <mitch@picman.org>
 * Copyright (C) 2009-2011 Martin Nordholts <martinn@src.gnome.org>
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
#include "core/picmancontext.h"
#include "core/picmanmarshal.h"

#include "picmandialogfactory.h"
#include "picmandnd.h"
#include "picmandockable.h"
#include "picmandockbook.h"
#include "picmanmenudock.h"
#include "picmanpanedbox.h"
#include "picmantoolbox.h"
#include "picmanwidgets-utils.h"

#include "picman-log.h"

#include "picman-intl.h"


/**
 * Defines the size of the area that dockables can be dropped on in
 * order to be inserted and get space on their own (rather than
 * inserted among others and sharing space)
 */
#define DROP_AREA_SIZE            6

#define INSERT_INDEX_UNUSED       G_MININT

#define INSTRUCTIONS_TEXT_PADDING 6
#define INSTRUCTIONS_TEXT         _("You can drop dockable dialogs here")


struct _PicmanPanedBoxPrivate
{
  /* Widgets that are separated by panes */
  GList                  *widgets;

  /* Displays INSTRUCTIONS_TEXT when there are no dockables */
  GtkWidget              *instructions;

  /* Window used for drag-and-drop output */
  GdkWindow              *dnd_window;

  /* The insert index to use on drop */
  gint                    insert_index;

  /* Callback on drop */
  PicmanPanedBoxDroppedFunc dropped_cb;
  gpointer                dropped_cb_data;

  /* A drag handler offered to handle drag events */
  PicmanPanedBox           *drag_handler;
};


static void      picman_paned_box_dispose                 (GObject        *object);

static void      picman_paned_box_drag_leave              (GtkWidget      *widget,
                                                         GdkDragContext *context,
                                                         guint           time);
static gboolean  picman_paned_box_drag_motion             (GtkWidget      *widget,
                                                         GdkDragContext *context,
                                                         gint            x,
                                                         gint            y,
                                                         guint           time);
static gboolean  picman_paned_box_drag_drop               (GtkWidget      *widget,
                                                         GdkDragContext *context,
                                                         gint            x,
                                                         gint            y,
                                                         guint           time);
static void      picman_paned_box_realize                 (GtkWidget      *widget);
static void      picman_paned_box_unrealize               (GtkWidget      *widget);
static void      picman_paned_box_set_widget_drag_handler (GtkWidget      *widget,
                                                         PicmanPanedBox   *handler);
static gint      picman_paned_box_get_drop_area_size      (PicmanPanedBox   *paned_box);


G_DEFINE_TYPE (PicmanPanedBox, picman_paned_box, GTK_TYPE_BOX)

#define parent_class picman_paned_box_parent_class

static const GtkTargetEntry dialog_target_table[] = { PICMAN_TARGET_DIALOG };


static void
picman_paned_box_class_init (PicmanPanedBoxClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose     = picman_paned_box_dispose;

  widget_class->drag_leave  = picman_paned_box_drag_leave;
  widget_class->drag_motion = picman_paned_box_drag_motion;
  widget_class->drag_drop   = picman_paned_box_drag_drop;
  widget_class->realize     = picman_paned_box_realize;
  widget_class->unrealize   = picman_paned_box_unrealize;

  g_type_class_add_private (klass, sizeof (PicmanPanedBoxPrivate));
}

static void
picman_paned_box_init (PicmanPanedBox *paned_box)
{
  paned_box->p = G_TYPE_INSTANCE_GET_PRIVATE (paned_box,
                                              PICMAN_TYPE_PANED_BOX,
                                              PicmanPanedBoxPrivate);

  /* Instructions label
   *
   * Size a small size request so it don't mess up dock window layouts
   * during startup; in particular, set its height request to 0 so it
   * doesn't contribute to the minimum height of the toolbox.
   */
  paned_box->p->instructions = gtk_label_new (INSTRUCTIONS_TEXT);
  gtk_misc_set_padding (GTK_MISC (paned_box->p->instructions),
                        INSTRUCTIONS_TEXT_PADDING, INSTRUCTIONS_TEXT_PADDING);
  gtk_label_set_line_wrap (GTK_LABEL (paned_box->p->instructions), TRUE);
  gtk_label_set_justify (GTK_LABEL (paned_box->p->instructions),
                         GTK_JUSTIFY_CENTER);
  gtk_widget_set_size_request (paned_box->p->instructions, 16, DROP_AREA_SIZE);
  picman_label_set_attributes (GTK_LABEL (paned_box->p->instructions),
                             PANGO_ATTR_STYLE, PANGO_STYLE_ITALIC,
                             -1);
  gtk_box_pack_start (GTK_BOX (paned_box), paned_box->p->instructions,
                      TRUE, TRUE, 0);
  gtk_widget_show (paned_box->p->instructions);


  /* Setup DND */
  gtk_drag_dest_set (GTK_WIDGET (paned_box),
                     0,
                     dialog_target_table, G_N_ELEMENTS (dialog_target_table),
                     GDK_ACTION_MOVE);
}

static void
picman_paned_box_dispose (GObject *object)
{
  PicmanPanedBox *paned_box = PICMAN_PANED_BOX (object);

  while (paned_box->p->widgets)
    {
      GtkWidget *widget = paned_box->p->widgets->data;

      g_object_ref (widget);
      picman_paned_box_remove_widget (paned_box, widget);
      gtk_widget_destroy (widget);
      g_object_unref (widget);
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_paned_box_realize (GtkWidget *widget)
{
  GTK_WIDGET_CLASS (parent_class)->realize (widget);

  /* We realize() dnd_window on demand in
   * picman_paned_box_show_separators()
   */
}

static void
picman_paned_box_unrealize (GtkWidget *widget)
{
  PicmanPanedBox *paned_box = PICMAN_PANED_BOX (widget);

  if (paned_box->p->dnd_window)
    {
      gdk_window_set_user_data (paned_box->p->dnd_window, NULL);
      gdk_window_destroy (paned_box->p->dnd_window);
      paned_box->p->dnd_window = NULL;
    }

  GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
picman_paned_box_set_widget_drag_handler (GtkWidget    *widget,
                                        PicmanPanedBox *drag_handler)
{
  /* Hook us in for drag events. We could abstract this properly and
   * put picman_paned_box_will_handle_drag() in an interface for
   * example, but it doesn't feel worth it at this point
   *
   * Note that we don't have 'else if's because a widget can be both a
   * dock and a toolbox for example, in which case we want to set a
   * drag handler in two ways
   *
   * We so need to introduce som abstractions here...
   */

  if (PICMAN_IS_DOCKBOOK (widget))
    {
      picman_dockbook_set_drag_handler (PICMAN_DOCKBOOK (widget),
                                      drag_handler);
    }

  if (PICMAN_IS_DOCK (widget))
    {
      PicmanPanedBox *dock_paned_box = NULL;
      dock_paned_box = PICMAN_PANED_BOX (picman_dock_get_vbox (PICMAN_DOCK (widget)));
      picman_paned_box_set_drag_handler (dock_paned_box, drag_handler);
    }

  if (PICMAN_IS_TOOLBOX (widget))
    {
      PicmanToolbox *toolbox = PICMAN_TOOLBOX (widget);
      picman_toolbox_set_drag_handler (toolbox, drag_handler);
    }
}

static gint
picman_paned_box_get_drop_area_size (PicmanPanedBox *paned_box)
{
  gint drop_area_size = 0;

  if (! paned_box->p->widgets)
    {
      GtkAllocation  allocation;
      GtkOrientation orientation;

      gtk_widget_get_allocation (GTK_WIDGET (paned_box), &allocation);
      orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (paned_box));

      if (orientation == GTK_ORIENTATION_HORIZONTAL)
        drop_area_size = allocation.width;
      else if (orientation == GTK_ORIENTATION_VERTICAL)
        drop_area_size = allocation.height;
    }

  drop_area_size = MAX (drop_area_size, DROP_AREA_SIZE);

  return drop_area_size;
}

static void
picman_paned_box_position_drop_indicator (PicmanPanedBox *paned_box,
                                        gint          x,
                                        gint          y,
                                        gint          width,
                                        gint          height)
{
  GtkWidget *widget = GTK_WIDGET (paned_box);

  if (! gtk_widget_is_drawable (widget))
    return;

  /* Create or move the GdkWindow in place */
  if (! paned_box->p->dnd_window)
    {
      GtkStyle      *style = gtk_widget_get_style (widget);
      GtkAllocation  allocation;
      GdkWindowAttr  attributes;

      gtk_widget_get_allocation (widget, &allocation);

      attributes.x           = x;
      attributes.y           = y;
      attributes.width       = width;
      attributes.height      = height;
      attributes.window_type = GDK_WINDOW_CHILD;
      attributes.wclass      = GDK_INPUT_OUTPUT;
      attributes.event_mask  = gtk_widget_get_events (widget);

      paned_box->p->dnd_window = gdk_window_new (gtk_widget_get_window (widget),
                                                 &attributes,
                                                 GDK_WA_X | GDK_WA_Y);
      gdk_window_set_user_data (paned_box->p->dnd_window, widget);

      gdk_window_set_background (paned_box->p->dnd_window,
                                 &style->bg[GTK_STATE_SELECTED]);
    }
  else
    {
      gdk_window_move_resize (paned_box->p->dnd_window,
                              x, y,
                              width, height);
    }

  gdk_window_show (paned_box->p->dnd_window);
}

static void
picman_paned_box_hide_drop_indicator (PicmanPanedBox *paned_box)
{
  if (! paned_box->p->dnd_window)
    return;

  gdk_window_hide (paned_box->p->dnd_window);
}

static void
picman_paned_box_drag_leave (GtkWidget      *widget,
                           GdkDragContext *context,
                           guint           time)
{
  picman_paned_box_hide_drop_indicator (PICMAN_PANED_BOX (widget));
  picman_highlight_widget (widget, FALSE);
}

static gboolean
picman_paned_box_drag_motion (GtkWidget      *widget,
                            GdkDragContext *context,
                            gint            x,
                            gint            y,
                            guint           time)
{
  PicmanPanedBox  *paned_box      = PICMAN_PANED_BOX (widget);
  gint           insert_index   = INSERT_INDEX_UNUSED;
  gint           dnd_window_x   = 0;
  gint           dnd_window_y   = 0;
  gint           dnd_window_w   = 0;
  gint           dnd_window_h   = 0;
  GtkAllocation  allocation     = { 0, };
  GtkOrientation orientation    = 0;
  gboolean       handle         = FALSE;
  gint           drop_area_size = picman_paned_box_get_drop_area_size (paned_box);

  if (picman_paned_box_will_handle_drag (paned_box->p->drag_handler,
                                       widget,
                                       context,
                                       x, y,
                                       time))
    {
      /* A parent widget will handle the event, just go to the end */
      handle = FALSE;
      goto finish;
    }

  gtk_widget_get_allocation (widget, &allocation);

  /* See if we're at the edge of the dock If there are no dockables,
   * the entire paned box is a drop area
   */
  orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (paned_box));
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      dnd_window_y = 0;
      dnd_window_h = allocation.height;

      /* If there are no widgets, the drop area is as big as the paned
       * box
       */
      if (! paned_box->p->widgets)
        dnd_window_w = allocation.width;
      else
        dnd_window_w = drop_area_size;

      if (x < drop_area_size)
        {
          insert_index = 0;
          dnd_window_x = 0;
        }
      if (x > allocation.width - drop_area_size)
        {
          insert_index = -1;
          dnd_window_x = allocation.width - drop_area_size;
        }
    }
  else /* if (orientation = GTK_ORIENTATION_VERTICAL) */
    {
      dnd_window_x = 0;
      dnd_window_w = allocation.width;

      /* If there are no widgets, the drop area is as big as the paned
       * box
       */
      if (! paned_box->p->widgets)
        dnd_window_h = allocation.height;
      else
        dnd_window_h = drop_area_size;

      if (y < drop_area_size)
        {
          insert_index = 0;
          dnd_window_y = 0;
        }
      if (y > allocation.height - drop_area_size)
        {
          insert_index = -1;
          dnd_window_y = allocation.height - drop_area_size;
        }
    }

  /* If we are at the edge, show a GdkWindow to communicate that a
   * drop will create a new dock column
   */
  handle = (insert_index != INSERT_INDEX_UNUSED);
  if (handle)
    {
      picman_paned_box_position_drop_indicator (paned_box,
                                              allocation.x + dnd_window_x,
                                              allocation.y + dnd_window_y,
                                              dnd_window_w,
                                              dnd_window_h);
    }
  else
    {
      picman_paned_box_hide_drop_indicator (paned_box);
    }

  /* Save the insert index for drag-drop */
  paned_box->p->insert_index = insert_index;

 finish:
  gdk_drag_status (context, handle ? GDK_ACTION_MOVE : 0, time);
  picman_highlight_widget (widget, handle);
  return handle;
}

static gboolean
picman_paned_box_drag_drop (GtkWidget      *widget,
                          GdkDragContext *context,
                          gint            x,
                          gint            y,
                          guint           time)
{
  gboolean      found   = FALSE;
  PicmanPanedBox *paned_box = PICMAN_PANED_BOX (widget);

  if (picman_paned_box_will_handle_drag (paned_box->p->drag_handler,
                                       widget,
                                       context,
                                       x, y,
                                       time))
    {
      /* A parent widget will handle the event, just go to the end */
      found = FALSE;
      goto finish;
    }
  
  if (paned_box->p->dropped_cb)
    {
      GtkWidget *source = gtk_drag_get_source_widget (context);

      if (source)
        found = paned_box->p->dropped_cb (source,
                                          paned_box->p->insert_index,
                                          paned_box->p->dropped_cb_data);
    }

 finish:
  if (found)
    gtk_drag_finish (context, TRUE, TRUE, time);
  return found;
}


GtkWidget *
picman_paned_box_new (gboolean       homogeneous,
                    gint           spacing,
                    GtkOrientation orientation)
{
  return g_object_new (PICMAN_TYPE_PANED_BOX,
                       "homogeneous",  homogeneous,
                       "spacing",      0,
                       "orientation",  orientation,
                       NULL);
}

void
picman_paned_box_set_dropped_cb (PicmanPanedBox            *paned_box,
                               PicmanPanedBoxDroppedFunc  dropped_cb,
                               gpointer                 dropped_cb_data)
{
  g_return_if_fail (PICMAN_IS_PANED_BOX (paned_box));

  paned_box->p->dropped_cb      = dropped_cb;
  paned_box->p->dropped_cb_data = dropped_cb_data;
}

/**
 * picman_paned_box_add_widget:
 * @paned_box: A #PicmanPanedBox
 * @widget:    The #GtkWidget to add
 * @index:     Where to add the @widget
 *
 * Add a #GtkWidget to the #PicmanPanedBox in a hierarchy of #GtkPaned:s
 * so the space can be manually distributed between the widgets.
 **/
void
picman_paned_box_add_widget (PicmanPanedBox *paned_box,
                           GtkWidget    *widget,
                           gint          index)
{
  gint old_length = 0;

  g_return_if_fail (PICMAN_IS_PANED_BOX (paned_box));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  PICMAN_LOG (DND, "Adding GtkWidget %p to PicmanPanedBox %p", widget, paned_box);

  /* Calculate length */
  old_length = g_list_length (paned_box->p->widgets);

  /* If index is invalid append at the end */
  if (index >= old_length || index < 0)
    {
      index = old_length;
    }

  /* Insert into the list */
  paned_box->p->widgets = g_list_insert (paned_box->p->widgets, widget, index);

  /* Hook us in for drag events. We could abstract this but it doesn't
   * seem worth it at this point
   */
  picman_paned_box_set_widget_drag_handler (widget, paned_box);

  /* Insert into the GtkPaned hierarchy */
  if (old_length == 0)
    {
      /* A widget is added, hide the instructions */
      gtk_widget_hide (paned_box->p->instructions);

      gtk_box_pack_start (GTK_BOX (paned_box), widget, TRUE, TRUE, 0);
    }
  else
    {
      GtkWidget      *old_widget;
      GtkWidget      *parent;
      GtkWidget      *paned;
      GtkOrientation  orientation;

      /* Figure out what widget to detach */
      if (index == 0)
        {
          old_widget = g_list_nth_data (paned_box->p->widgets, index + 1);
        }
      else
        {
          old_widget = g_list_nth_data (paned_box->p->widgets, index - 1);
        }

      parent = gtk_widget_get_parent (old_widget);

      if (old_length > 1 && index > 0)
        {
          GtkWidget *grandparent = gtk_widget_get_parent (parent);

          old_widget = parent;
          parent     = grandparent;
        }

      /* Deatch the widget and bulid up a new hierarchy */
      g_object_ref (old_widget);
      gtk_container_remove (GTK_CONTAINER (parent), old_widget);

      /* GtkPaned is abstract :( */
      orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (paned_box));
      paned = gtk_paned_new (orientation);

      if (GTK_IS_PANED (parent))
        {
          gtk_paned_pack1 (GTK_PANED (parent), paned, TRUE, FALSE);
        }
      else
        {
          gtk_box_pack_start (GTK_BOX (parent), paned, TRUE, TRUE, 0);
        }
      gtk_widget_show (paned);

      if (index == 0)
        {
          gtk_paned_pack1 (GTK_PANED (paned), widget,
                           TRUE, FALSE);
          gtk_paned_pack2 (GTK_PANED (paned), old_widget,
                           TRUE, FALSE);
        }
      else
        {
          gtk_paned_pack1 (GTK_PANED (paned), old_widget,
                           TRUE, FALSE);
          gtk_paned_pack2 (GTK_PANED (paned), widget,
                           TRUE, FALSE);
        }

      g_object_unref (old_widget);
    }
}

/**
 * picman_paned_box_remove_widget:
 * @paned_box: A #PicmanPanedBox
 * @widget:    The #GtkWidget to remove
 *
 * Remove a #GtkWidget from a #PicmanPanedBox added with
 * picman_widgets_add_paned_widget().
 **/
void
picman_paned_box_remove_widget (PicmanPanedBox *paned_box,
                              GtkWidget    *widget)
{
  gint       old_length   = 0;
  gint       index        = 0;
  GtkWidget *other_widget = NULL;
  GtkWidget *parent       = NULL;
  GtkWidget *grandparent  = NULL;

  g_return_if_fail (PICMAN_IS_PANED_BOX (paned_box));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  PICMAN_LOG (DND, "Removing GtkWidget %p from PicmanPanedBox %p", widget, paned_box);

  /* Calculate length and index */
  old_length = g_list_length (paned_box->p->widgets);
  index      = g_list_index (paned_box->p->widgets, widget);

  /* Remove from list */
  paned_box->p->widgets = g_list_remove (paned_box->p->widgets, widget);

  /* Reset the drag events hook */
  picman_paned_box_set_widget_drag_handler (widget, NULL);

  /* Remove from widget hierarchy */
  if (old_length == 1)
    {
      /* The widget might already be parent-less if we are in
       * destruction, .e.g when closing a dock window.
       */
      if (gtk_widget_get_parent (widget) != NULL)
        gtk_container_remove (GTK_CONTAINER (paned_box), widget);

      /* The last widget is removed, show the instructions */
      gtk_widget_show (paned_box->p->instructions);
    }
  else
    {
      g_object_ref (widget);

      parent      = gtk_widget_get_parent (GTK_WIDGET (widget));
      grandparent = gtk_widget_get_parent (parent);

      if (index == 0)
        other_widget = gtk_paned_get_child2 (GTK_PANED (parent));
      else
        other_widget = gtk_paned_get_child1 (GTK_PANED (parent));

      g_object_ref (other_widget);

      gtk_container_remove (GTK_CONTAINER (parent), other_widget);
      gtk_container_remove (GTK_CONTAINER (parent), GTK_WIDGET (widget));

      gtk_container_remove (GTK_CONTAINER (grandparent), parent);

      if (GTK_IS_PANED (grandparent))
        gtk_paned_pack1 (GTK_PANED (grandparent), other_widget, TRUE, FALSE);
      else
        gtk_box_pack_start (GTK_BOX (paned_box), other_widget, TRUE, TRUE, 0);

      g_object_unref (other_widget);

      g_object_unref (widget);
    }
}

/**
 * picman_paned_box_will_handle_drag:
 * @paned_box: A #PicmanPanedBox
 * @widget:    The widget that got the drag event
 * @context:   Context from drag event
 * @x:         x from drag event
 * @y:         y from drag event
 * @time:      time from drag event
 *
 * Returns: %TRUE if the drag event on @widget will be handled by
 *          @paned_box.
 **/
gboolean
picman_paned_box_will_handle_drag (PicmanPanedBox   *paned_box,
                                 GtkWidget      *widget,
                                 GdkDragContext *context,
                                 gint            x,
                                 gint            y,
                                 gint            time)
{
  gint           paned_box_x    = 0;
  gint           paned_box_y    = 0;
  GtkAllocation  allocation     = { 0, };
  GtkOrientation orientation    = 0;
  gboolean       will_handle    = FALSE;
  gint           drop_area_size = 0;

  g_return_val_if_fail (paned_box == NULL ||
                        PICMAN_IS_PANED_BOX (paned_box), FALSE);

  /* Check for NULL to allow cleaner client code */
  if (paned_box == NULL)
    return FALSE;

  /* Our handler might handle it */
  if (picman_paned_box_will_handle_drag (paned_box->p->drag_handler,
                                       widget,
                                       context,
                                       x, y,
                                       time))
    {
      /* Return TRUE so the client will pass on the drag event */
      return TRUE;
    }

  /* If we don't have a common ancenstor we will not handle it */
  if (! gtk_widget_translate_coordinates (widget,
                                          GTK_WIDGET (paned_box),
                                          x, y,
                                          &paned_box_x, &paned_box_y))
    {
      /* Return FALSE so the client can take care of the drag event */
      return FALSE;
    }

  /* We now have paned_box coordinates, see if the paned_box will
   * handle the event
   */
  gtk_widget_get_allocation (GTK_WIDGET (paned_box), &allocation);
  orientation    = gtk_orientable_get_orientation (GTK_ORIENTABLE (paned_box));
  drop_area_size = picman_paned_box_get_drop_area_size (paned_box);
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      will_handle = (paned_box_x < drop_area_size ||
                     paned_box_x > allocation.width - drop_area_size);
    }
  else /*if (orientation = GTK_ORIENTATION_VERTICAL)*/
    {
      will_handle = (paned_box_y < drop_area_size ||
                     paned_box_y > allocation.height - drop_area_size);
    }

  return will_handle;
}

void
picman_paned_box_set_drag_handler (PicmanPanedBox *paned_box,
                                 PicmanPanedBox *drag_handler)
{
  g_return_if_fail (PICMAN_IS_PANED_BOX (paned_box));

  paned_box->p->drag_handler = drag_handler;
}
