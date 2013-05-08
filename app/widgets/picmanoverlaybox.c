/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanOverlayBox
 * Copyright (C) 2009 Michael Natterer <mitch@picman.org>
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

#include <gtk/gtk.h>

#include "widgets-types.h"

#include "picmanoverlaybox.h"
#include "picmanoverlaychild.h"


/*  local function prototypes  */

static void        picman_overlay_box_set_property        (GObject        *object,
                                                         guint           property_id,
                                                         const GValue   *value,
                                                         GParamSpec     *pspec);
static void        picman_overlay_box_get_property        (GObject        *object,
                                                         guint           property_id,
                                                         GValue         *value,
                                                         GParamSpec     *pspec);

static void        picman_overlay_box_realize             (GtkWidget      *widget);
static void        picman_overlay_box_unrealize           (GtkWidget      *widget);
static void        picman_overlay_box_size_request        (GtkWidget      *widget,
                                                         GtkRequisition *requisition);
static void        picman_overlay_box_size_allocate       (GtkWidget      *widget,
                                                         GtkAllocation  *allocation);
static gboolean    picman_overlay_box_expose              (GtkWidget      *widget,
                                                         GdkEventExpose *event);
static gboolean    picman_overlay_box_damage              (GtkWidget      *widget,
                                                         GdkEventExpose *event);

static void        picman_overlay_box_add                 (GtkContainer   *container,
                                                         GtkWidget      *widget);
static void        picman_overlay_box_remove              (GtkContainer   *container,
                                                         GtkWidget      *widget);
static void        picman_overlay_box_forall              (GtkContainer   *container,
                                                         gboolean        include_internals,
                                                         GtkCallback     callback,
                                                         gpointer        callback_data);
static GType       picman_overlay_box_child_type          (GtkContainer   *container);

static GdkWindow * picman_overlay_box_pick_embedded_child (GdkWindow      *window,
                                                         gdouble         x,
                                                         gdouble         y,
                                                         PicmanOverlayBox *box);


G_DEFINE_TYPE (PicmanOverlayBox, picman_overlay_box, GTK_TYPE_CONTAINER)

#define parent_class picman_overlay_box_parent_class


static void
picman_overlay_box_class_init (PicmanOverlayBoxClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  GtkWidgetClass    *widget_class    = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->set_property  = picman_overlay_box_set_property;
  object_class->get_property  = picman_overlay_box_get_property;

  widget_class->realize       = picman_overlay_box_realize;
  widget_class->unrealize     = picman_overlay_box_unrealize;
  widget_class->size_request  = picman_overlay_box_size_request;
  widget_class->size_allocate = picman_overlay_box_size_allocate;
  widget_class->expose_event  = picman_overlay_box_expose;

  g_signal_override_class_handler ("damage-event",
                                   PICMAN_TYPE_OVERLAY_BOX,
                                   G_CALLBACK (picman_overlay_box_damage));

  container_class->add        = picman_overlay_box_add;
  container_class->remove     = picman_overlay_box_remove;
  container_class->forall     = picman_overlay_box_forall;
  container_class->child_type = picman_overlay_box_child_type;
}

static void
picman_overlay_box_init (PicmanOverlayBox *box)
{
  gtk_widget_set_has_window (GTK_WIDGET (box), TRUE);
}

static void
picman_overlay_box_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_overlay_box_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_overlay_box_realize (GtkWidget *widget)
{
  PicmanOverlayBox *box = PICMAN_OVERLAY_BOX (widget);
  GtkAllocation   allocation;
  GdkWindowAttr   attributes;
  gint            attributes_mask;
  GList          *list;

  gtk_widget_set_realized (widget, TRUE);

  gtk_widget_get_allocation (widget, &allocation);

  attributes.x           = allocation.x;
  attributes.y           = allocation.y;
  attributes.width       = allocation.width;
  attributes.height      = allocation.height;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass      = GDK_INPUT_OUTPUT;
  attributes.visual      = gtk_widget_get_visual (widget);
  attributes.colormap    = gtk_widget_get_colormap (widget);
  attributes.event_mask  = gtk_widget_get_events (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  gtk_widget_set_window (widget,
                         gdk_window_new (gtk_widget_get_parent_window (widget),
                                         &attributes, attributes_mask));
  gdk_window_set_user_data (gtk_widget_get_window (widget), widget);

  g_signal_connect (gtk_widget_get_window (widget), "pick-embedded-child",
                    G_CALLBACK (picman_overlay_box_pick_embedded_child),
                    widget);

  gtk_widget_style_attach (widget);
  gtk_style_set_background (gtk_widget_get_style (widget),
                            gtk_widget_get_window (widget),
                            GTK_STATE_NORMAL);

  for (list = box->children; list; list = g_list_next (list))
    picman_overlay_child_realize (box, list->data);
}

static void
picman_overlay_box_unrealize (GtkWidget *widget)
{
  PicmanOverlayBox *box = PICMAN_OVERLAY_BOX (widget);
  GList          *list;

  for (list = box->children; list; list = g_list_next (list))
    picman_overlay_child_unrealize (box, list->data);

  GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
picman_overlay_box_size_request (GtkWidget      *widget,
                               GtkRequisition *requisition)
{
  PicmanOverlayBox *box = PICMAN_OVERLAY_BOX (widget);
  GList          *list;
  gint            border_width;

  border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));

  requisition->width  = 1 + 2 * border_width;
  requisition->height = 1 + 2 * border_width;

  for (list = box->children; list; list = g_list_next (list))
    picman_overlay_child_size_request (box, list->data);
}

static void
picman_overlay_box_size_allocate (GtkWidget     *widget,
                                GtkAllocation *allocation)
{
  PicmanOverlayBox *box = PICMAN_OVERLAY_BOX (widget);
  GList          *list;

  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (widget))
    gdk_window_move_resize (gtk_widget_get_window (widget),
                            allocation->x,
                            allocation->y,
                            allocation->width,
                            allocation->height);

  for (list = box->children; list; list = g_list_next (list))
    picman_overlay_child_size_allocate (box, list->data);
}

static gboolean
picman_overlay_box_expose (GtkWidget      *widget,
                         GdkEventExpose *event)
{
  if (gtk_widget_is_drawable (widget))
    {
      PicmanOverlayBox *box = PICMAN_OVERLAY_BOX (widget);
      GList          *list;

      for (list = box->children; list; list = g_list_next (list))
        {
          if (picman_overlay_child_expose (box, list->data, event))
            return FALSE;
        }
    }

  return FALSE;
}

static gboolean
picman_overlay_box_damage (GtkWidget      *widget,
                         GdkEventExpose *event)
{
  PicmanOverlayBox *box = PICMAN_OVERLAY_BOX (widget);
  GList          *list;

  for (list = box->children; list; list = g_list_next (list))
    {
      if (picman_overlay_child_damage (box, list->data, event))
        return FALSE;
    }

  return FALSE;
}

static void
picman_overlay_box_add (GtkContainer *container,
                      GtkWidget    *widget)
{
  picman_overlay_box_add_child (PICMAN_OVERLAY_BOX (container), widget, 0.5, 0.5);
}

static void
picman_overlay_box_remove (GtkContainer *container,
                         GtkWidget    *widget)
{
  PicmanOverlayBox   *box   = PICMAN_OVERLAY_BOX (container);
  PicmanOverlayChild *child = picman_overlay_child_find (box, widget);

  if (child)
    {
      if (gtk_widget_get_visible (widget))
        picman_overlay_child_invalidate (box, child);

      box->children = g_list_remove (box->children, child);

      picman_overlay_child_free (box, child);
    }
}

static void
picman_overlay_box_forall (GtkContainer *container,
                         gboolean      include_internals,
                         GtkCallback   callback,
                         gpointer      callback_data)
{
  PicmanOverlayBox *box = PICMAN_OVERLAY_BOX (container);
  GList          *list;

  list = box->children;
  while (list)
    {
      PicmanOverlayChild *child = list->data;

      list = list->next;

      (* callback) (child->widget, callback_data);
    }
}

static GType
picman_overlay_box_child_type (GtkContainer *container)
{
  return GTK_TYPE_WIDGET;
}

static GdkWindow *
picman_overlay_box_pick_embedded_child (GdkWindow      *parent,
                                      gdouble         parent_x,
                                      gdouble         parent_y,
                                      PicmanOverlayBox *box)
{
  GList *list;

  for (list = box->children; list; list = g_list_next (list))
    {
      PicmanOverlayChild *child = list->data;

      if (picman_overlay_child_pick (box, child, parent_x, parent_y))
        return child->window;
    }

  return NULL;
}


/*  public functions  */

/**
 * picman_overlay_box_new:
 *
 * Creates a new #PicmanOverlayBox widget.
 *
 * Return value: a new #PicmanOverlayBox widget
 **/
GtkWidget *
picman_overlay_box_new (void)
{
  return g_object_new (PICMAN_TYPE_OVERLAY_BOX, NULL);
}

static void
unset_double_buffered (GtkWidget *widget)
{
  gtk_widget_set_double_buffered (widget, FALSE);

  if (GTK_IS_CONTAINER (widget))
    {
      GList *children = gtk_container_get_children (GTK_CONTAINER (widget));
      GList *list;

      for (list = children; list; list = g_list_next (list))
        {
          unset_double_buffered (list->data);
        }

      g_list_free (children);
    }
}

void
picman_overlay_box_add_child (PicmanOverlayBox *box,
                            GtkWidget      *widget,
                            gdouble         xalign,
                            gdouble         yalign)
{
  PicmanOverlayChild *child;

  g_return_if_fail (PICMAN_IS_OVERLAY_BOX (box));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  unset_double_buffered (widget);

  child = picman_overlay_child_new (box, widget, xalign, yalign, 0.0, 0.7);

  box->children = g_list_append (box->children, child);
}

void
picman_overlay_box_set_child_alignment (PicmanOverlayBox *box,
                                      GtkWidget      *widget,
                                      gdouble         xalign,
                                      gdouble         yalign)
{
  PicmanOverlayChild *child = picman_overlay_child_find (box, widget);

  if (child)
    {
      xalign = CLAMP (xalign, 0.0, 1.0);
      yalign = CLAMP (yalign, 0.0, 1.0);

      if (child->has_position     ||
          child->xalign != xalign ||
          child->yalign != yalign)
        {
          picman_overlay_child_invalidate (box, child);

          child->has_position = FALSE;
          child->xalign       = xalign;
          child->yalign       = yalign;

          gtk_widget_queue_resize (widget);
        }
    }
}

void
picman_overlay_box_set_child_position (PicmanOverlayBox *box,
                                     GtkWidget      *widget,
                                     gdouble         x,
                                     gdouble         y)
{
  PicmanOverlayChild *child = picman_overlay_child_find (box, widget);

  if (child)
    {
      if (! child->has_position ||
          child->x != x         ||
          child->y != y)
        {
          picman_overlay_child_invalidate (box, child);

          child->has_position = TRUE;
          child->x            = x;
          child->y            = y;

          gtk_widget_queue_resize (widget);
        }
    }
}

void
picman_overlay_box_set_child_angle (PicmanOverlayBox *box,
                                  GtkWidget      *widget,
                                  gdouble         angle)
{
  PicmanOverlayChild *child = picman_overlay_child_find (box, widget);

  if (child)
    {
      if (child->angle != angle)
        {
          picman_overlay_child_invalidate (box, child);

          child->angle = angle;

          gtk_widget_queue_draw (widget);
        }
    }
}

void
picman_overlay_box_set_child_opacity (PicmanOverlayBox *box,
                                    GtkWidget      *widget,
                                    gdouble         opacity)
{
  PicmanOverlayChild *child = picman_overlay_child_find (box, widget);

  if (child)
    {
      opacity = CLAMP (opacity, 0.0, 1.0);

      if (child->opacity != opacity)
        {
          child->opacity = opacity;

          gtk_widget_queue_draw (widget);
        }
    }
}

/**
 * picman_overlay_box_scroll:
 * @box: the #PicmanOverlayBox widget to scroll.
 * @offset_x: the x scroll amount.
 * @offset_y: the y scroll amount.
 *
 * Scrolls the box using gdk_window_scroll() and makes sure the result
 * is displayed immediately by calling gdk_window_process_updates().
 **/
void
picman_overlay_box_scroll (PicmanOverlayBox *box,
                         gint            offset_x,
                         gint            offset_y)
{
  GtkWidget *widget;
  GdkWindow *window;
  GList     *list;

  g_return_if_fail (PICMAN_IS_OVERLAY_BOX (box));

  widget = GTK_WIDGET (box);
  window = gtk_widget_get_window (widget);

  /*  Undraw all overlays  */
  for (list = box->children; list; list = g_list_next (list))
    {
      PicmanOverlayChild *child = list->data;

      picman_overlay_child_invalidate (box, child);
    }

  gdk_window_scroll (window, offset_x, offset_y);

  /*  Re-draw all overlays  */
  for (list = box->children; list; list = g_list_next (list))
    {
      PicmanOverlayChild *child = list->data;

      picman_overlay_child_invalidate (box, child);
    }

  /*  Make sure expose events are processed before scrolling again  */
  gdk_window_process_updates (window, FALSE);
}
