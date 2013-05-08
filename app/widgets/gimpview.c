/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanview.c
 * Copyright (C) 2001-2006 Michael Natterer <mitch@picman.org>
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
#include "libpicmanmath/picmanmath.h"
#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontext.h"
#include "core/picmanmarshal.h"
#include "core/picmanviewable.h"

#include "picmandnd.h"
#include "picmanview.h"
#include "picmanview-popup.h"
#include "picmanviewrenderer.h"
#include "picmanviewrenderer-utils.h"


enum
{
  SET_VIEWABLE,
  CLICKED,
  DOUBLE_CLICKED,
  CONTEXT,
  LAST_SIGNAL
};


static void        picman_view_dispose              (GObject          *object);

static void        picman_view_realize              (GtkWidget        *widget);
static void        picman_view_unrealize            (GtkWidget        *widget);
static void        picman_view_map                  (GtkWidget        *widget);
static void        picman_view_unmap                (GtkWidget        *widget);
static void        picman_view_size_request         (GtkWidget        *widget,
                                                   GtkRequisition   *requisition);
static void        picman_view_size_allocate        (GtkWidget        *widget,
                                                   GtkAllocation    *allocation);
static gboolean    picman_view_expose_event         (GtkWidget        *widget,
                                                   GdkEventExpose   *event);
static gboolean    picman_view_button_press_event   (GtkWidget        *widget,
                                                   GdkEventButton   *bevent);
static gboolean    picman_view_button_release_event (GtkWidget        *widget,
                                                   GdkEventButton   *bevent);
static gboolean    picman_view_enter_notify_event   (GtkWidget        *widget,
                                                   GdkEventCrossing *event);
static gboolean    picman_view_leave_notify_event   (GtkWidget        *widget,
                                                   GdkEventCrossing *event);

static void        picman_view_real_set_viewable    (PicmanView         *view,
                                                   PicmanViewable     *old,
                                                   PicmanViewable     *viewable);

static void        picman_view_update_callback      (PicmanViewRenderer *renderer,
                                                   PicmanView         *view);

static PicmanViewable * picman_view_drag_viewable     (GtkWidget        *widget,
                                                   PicmanContext     **context,
                                                   gpointer          data);
static GdkPixbuf    * picman_view_drag_pixbuf       (GtkWidget        *widget,
                                                   gpointer          data);


G_DEFINE_TYPE (PicmanView, picman_view, GTK_TYPE_WIDGET)

#define parent_class picman_view_parent_class

static guint view_signals[LAST_SIGNAL] = { 0 };


static void
picman_view_class_init (PicmanViewClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  view_signals[SET_VIEWABLE] =
    g_signal_new ("set-viewable",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanViewClass, set_viewable),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT_OBJECT,
                  G_TYPE_NONE, 2,
                  PICMAN_TYPE_VIEWABLE, PICMAN_TYPE_VIEWABLE);

  view_signals[CLICKED] =
    g_signal_new ("clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanViewClass, clicked),
                  NULL, NULL,
                  picman_marshal_VOID__FLAGS,
                  G_TYPE_NONE, 1,
                  GDK_TYPE_MODIFIER_TYPE);

  view_signals[DOUBLE_CLICKED] =
    g_signal_new ("double-clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanViewClass, double_clicked),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  view_signals[CONTEXT] =
    g_signal_new ("context",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanViewClass, context),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->dispose              = picman_view_dispose;

  widget_class->activate_signal      = view_signals[CLICKED];
  widget_class->realize              = picman_view_realize;
  widget_class->unrealize            = picman_view_unrealize;
  widget_class->map                  = picman_view_map;
  widget_class->unmap                = picman_view_unmap;
  widget_class->size_request         = picman_view_size_request;
  widget_class->size_allocate        = picman_view_size_allocate;
  widget_class->expose_event         = picman_view_expose_event;
  widget_class->button_press_event   = picman_view_button_press_event;
  widget_class->button_release_event = picman_view_button_release_event;
  widget_class->enter_notify_event   = picman_view_enter_notify_event;
  widget_class->leave_notify_event   = picman_view_leave_notify_event;

  klass->set_viewable                = picman_view_real_set_viewable;
  klass->clicked                     = NULL;
  klass->double_clicked              = NULL;
  klass->context                     = NULL;
}

static void
picman_view_init (PicmanView *view)
{
  gtk_widget_set_has_window (GTK_WIDGET (view), FALSE);
  gtk_widget_add_events (GTK_WIDGET (view),
                         GDK_BUTTON_PRESS_MASK   |
                         GDK_BUTTON_RELEASE_MASK |
                         GDK_ENTER_NOTIFY_MASK   |
                         GDK_LEAVE_NOTIFY_MASK);

  view->event_window      = NULL;
  view->viewable          = NULL;
  view->renderer          = NULL;

  view->clickable         = FALSE;
  view->eat_button_events = TRUE;
  view->show_popup        = FALSE;
  view->expand            = FALSE;

  view->in_button         = FALSE;
  view->has_grab          = FALSE;
  view->press_state       = 0;
}

static void
picman_view_dispose (GObject *object)
{
  PicmanView *view = PICMAN_VIEW (object);

  if (view->viewable)
    picman_view_set_viewable (view, NULL);

  if (view->renderer)
    {
      g_object_unref (view->renderer);
      view->renderer = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_view_realize (GtkWidget *widget)
{
  PicmanView      *view = PICMAN_VIEW (widget);
  GtkAllocation  allocation;
  GdkWindowAttr  attributes;
  gint           attributes_mask;

  GTK_WIDGET_CLASS (parent_class)->realize (widget);

  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x           = allocation.x;
  attributes.y           = allocation.y;
  attributes.width       = allocation.width;
  attributes.height      = allocation.height;
  attributes.wclass      = GDK_INPUT_ONLY;
  attributes.event_mask  = gtk_widget_get_events (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  view->event_window = gdk_window_new (gtk_widget_get_window (widget),
                                       &attributes, attributes_mask);
  gdk_window_set_user_data (view->event_window, view);
}

static void
picman_view_unrealize (GtkWidget *widget)
{
  PicmanView *view = PICMAN_VIEW (widget);

  if (view->event_window)
    {
      gdk_window_set_user_data (view->event_window, NULL);
      gdk_window_destroy (view->event_window);
      view->event_window = NULL;
    }

  GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
picman_view_map (GtkWidget *widget)
{
  PicmanView *view = PICMAN_VIEW (widget);

  GTK_WIDGET_CLASS (parent_class)->map (widget);

  if (view->event_window)
    gdk_window_show (view->event_window);
}

static void
picman_view_unmap (GtkWidget *widget)
{
  PicmanView *view = PICMAN_VIEW (widget);

  if (view->has_grab)
    {
      gtk_grab_remove (widget);
      view->has_grab = FALSE;
    }

  if (view->event_window)
    gdk_window_hide (view->event_window);

  GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static void
picman_view_size_request (GtkWidget      *widget,
                        GtkRequisition *requisition)
{
  PicmanView *view = PICMAN_VIEW (widget);

  if (view->expand)
    {
      requisition->width  = 2 * view->renderer->border_width + 1;
      requisition->height = 2 * view->renderer->border_width + 1;
    }
  else
    {
      requisition->width  = (view->renderer->width +
                             2 * view->renderer->border_width);
      requisition->height = (view->renderer->height +
                             2 * view->renderer->border_width);
    }
}

static void
picman_view_size_allocate (GtkWidget     *widget,
                         GtkAllocation *allocation)
{
  PicmanView *view = PICMAN_VIEW (widget);
  gint      width;
  gint      height;

  if (view->expand)
    {
      width  = MIN (PICMAN_VIEWABLE_MAX_PREVIEW_SIZE,
                    allocation->width - 2 * view->renderer->border_width);
      height = MIN (PICMAN_VIEWABLE_MAX_PREVIEW_SIZE,
                    allocation->height - 2 * view->renderer->border_width);

      if (view->renderer->width  != width ||
          view->renderer->height != height)
        {
          gint border_width = view->renderer->border_width;

          if (view->renderer->size != -1 && view->renderer->viewable)
            {
              gint view_width;
              gint view_height;
              gint scaled_width;
              gint scaled_height;

              picman_viewable_get_preview_size (view->renderer->viewable,
                                              PICMAN_VIEWABLE_MAX_PREVIEW_SIZE,
                                              view->renderer->is_popup,
                                              view->renderer->dot_for_dot,
                                              &view_width,
                                              &view_height);

              picman_viewable_calc_preview_size (view_width, view_height,
                                               width, height,
                                               TRUE, 1.0, 1.0,
                                               &scaled_width, &scaled_height,
                                               NULL);

              if (scaled_width > width)
                {
                  scaled_height = scaled_height * width / scaled_width;
                  scaled_width  = scaled_width  * width / scaled_width;
                }
              else if (scaled_height > height)
                {
                  scaled_width  = scaled_width  * height / scaled_height;
                  scaled_height = scaled_height * height / scaled_height;
                }

              picman_view_renderer_set_size (view->renderer,
                                           MAX (scaled_width, scaled_height),
                                           border_width);
            }
          else
            {
              picman_view_renderer_set_size_full (view->renderer,
                                                width, height,
                                                border_width);
            }

          picman_view_renderer_remove_idle (view->renderer);
        }
    }

  width  = (view->renderer->width +
            2 * view->renderer->border_width);
  height = (view->renderer->height +
            2 * view->renderer->border_width);

  if (allocation->width > width)
    allocation->x += (allocation->width - width) / 2;

  if (allocation->height > height)
    allocation->y += (allocation->height - height) / 2;

  allocation->width  = width;
  allocation->height = height;

  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (widget))
    gdk_window_move_resize (view->event_window,
                            allocation->x,
                            allocation->y,
                            allocation->width,
                            allocation->height);
}

static gboolean
picman_view_expose_event (GtkWidget      *widget,
                        GdkEventExpose *event)
{
  if (gtk_widget_is_drawable (widget))
    {
      GtkAllocation  allocation;
      cairo_t       *cr;

      gtk_widget_get_allocation (widget, &allocation);

      cr = gdk_cairo_create (event->window);
      gdk_cairo_region (cr, event->region);
      cairo_clip (cr);

      cairo_translate (cr, allocation.x, allocation.y);

      picman_view_renderer_draw (PICMAN_VIEW (widget)->renderer,
                               widget, cr,
                               allocation.width,
                               allocation.height);

      cairo_destroy (cr);
    }

  return FALSE;
}


#define DEBUG_MEMSIZE 1

#ifdef DEBUG_MEMSIZE
extern gboolean picman_debug_memsize;
#endif

static gboolean
picman_view_button_press_event (GtkWidget      *widget,
                              GdkEventButton *bevent)
{
  PicmanView *view = PICMAN_VIEW (widget);

#ifdef DEBUG_MEMSIZE
  if (bevent->type == GDK_BUTTON_PRESS && bevent->button == 2)
    {
      picman_debug_memsize = TRUE;

      picman_object_get_memsize (PICMAN_OBJECT (view->viewable), NULL);

      picman_debug_memsize = FALSE;
    }
#endif /* DEBUG_MEMSIZE */

  if (! view->clickable &&
      ! view->show_popup)
    return FALSE;

  if (! gtk_widget_get_realized (widget))
    return FALSE;

  if (bevent->type == GDK_BUTTON_PRESS)
    {
      if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
        {
          view->press_state = 0;

          g_signal_emit (widget, view_signals[CONTEXT], 0);
        }
      else if (bevent->button == 1)
        {
          gtk_grab_add (widget);

          view->has_grab    = TRUE;
          view->press_state = bevent->state;

          if (view->show_popup && view->viewable)
            {
              picman_view_popup_show (widget, bevent,
                                    view->renderer->context,
                                    view->viewable,
                                    view->renderer->width,
                                    view->renderer->height,
                                    view->renderer->dot_for_dot);
            }
        }
      else
        {
          view->press_state = 0;

          if (bevent->button == 2)
            picman_view_popup_show (widget, bevent,
                                  view->renderer->context,
                                  view->viewable,
                                  view->renderer->width,
                                  view->renderer->height,
                                  view->renderer->dot_for_dot);

          return FALSE;
        }
    }
  else if (bevent->type == GDK_2BUTTON_PRESS)
    {
      if (bevent->button == 1)
        g_signal_emit (widget, view_signals[DOUBLE_CLICKED], 0);
    }

  return view->eat_button_events ? TRUE : FALSE;
}

static gboolean
picman_view_button_release_event (GtkWidget      *widget,
                                GdkEventButton *bevent)
{
  PicmanView *view = PICMAN_VIEW (widget);

  if (! view->clickable &&
      ! view->show_popup)
    return FALSE;

  if (bevent->button == 1)
    {
      gtk_grab_remove (widget);
      view->has_grab = FALSE;

      if (view->clickable && view->in_button)
        {
          g_signal_emit (widget, view_signals[CLICKED], 0, view->press_state);
        }
    }
  else
    {
      return FALSE;
    }

  return view->eat_button_events ? TRUE : FALSE;
}

static gboolean
picman_view_enter_notify_event (GtkWidget        *widget,
                              GdkEventCrossing *event)
{
  PicmanView  *view         = PICMAN_VIEW (widget);
  GtkWidget *event_widget = gtk_get_event_widget ((GdkEvent *) event);

  if ((event_widget == widget) &&
      (event->detail != GDK_NOTIFY_INFERIOR))
    {
      view->in_button = TRUE;
    }

  return FALSE;
}

static gboolean
picman_view_leave_notify_event (GtkWidget        *widget,
                              GdkEventCrossing *event)
{
  PicmanView  *view         = PICMAN_VIEW (widget);
  GtkWidget *event_widget = gtk_get_event_widget ((GdkEvent *) event);

  if ((event_widget == widget) &&
      (event->detail != GDK_NOTIFY_INFERIOR))
    {
      view->in_button = FALSE;
    }

  return FALSE;
}

static void
picman_view_real_set_viewable (PicmanView     *view,
                             PicmanViewable *old,
                             PicmanViewable *viewable)
{
  GType viewable_type = G_TYPE_NONE;

  if (viewable == view->viewable)
    return;

  if (viewable)
    {
      viewable_type = G_TYPE_FROM_INSTANCE (viewable);

      g_return_if_fail (g_type_is_a (viewable_type,
                                     view->renderer->viewable_type));
    }

  if (view->viewable)
    {
      g_object_remove_weak_pointer (G_OBJECT (view->viewable),
                                    (gpointer) &view->viewable);

      if (! viewable && ! view->renderer->is_popup)
        {
          if (picman_dnd_viewable_source_remove (GTK_WIDGET (view),
                                               G_TYPE_FROM_INSTANCE (view->viewable)))
            {
              if (picman_viewable_get_size (view->viewable, NULL, NULL))
                picman_dnd_pixbuf_source_remove (GTK_WIDGET (view));

              gtk_drag_source_unset (GTK_WIDGET (view));
            }
        }
    }
  else if (viewable && ! view->renderer->is_popup)
    {
      if (picman_dnd_drag_source_set_by_type (GTK_WIDGET (view),
                                            GDK_BUTTON1_MASK | GDK_BUTTON2_MASK,
                                            viewable_type,
                                            GDK_ACTION_COPY))
        {
          picman_dnd_viewable_source_add (GTK_WIDGET (view),
                                        viewable_type,
                                        picman_view_drag_viewable,
                                        NULL);

          if (picman_viewable_get_size (viewable, NULL, NULL))
            picman_dnd_pixbuf_source_add (GTK_WIDGET (view),
                                        picman_view_drag_pixbuf,
                                        NULL);
        }
    }

  picman_view_renderer_set_viewable (view->renderer, viewable);
  view->viewable = viewable;

  if (view->viewable)
    {
      g_object_add_weak_pointer (G_OBJECT (view->viewable),
                                 (gpointer) &view->viewable);
    }
}

/*  public functions  */

GtkWidget *
picman_view_new (PicmanContext  *context,
               PicmanViewable *viewable,
               gint          size,
               gint          border_width,
               gboolean      is_popup)
{
  GtkWidget *view;

  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);

  view = picman_view_new_by_types (context,
                                 PICMAN_TYPE_VIEW,
                                 G_TYPE_FROM_INSTANCE (viewable),
                                 size, border_width, is_popup);

  if (view)
    picman_view_set_viewable (PICMAN_VIEW (view), viewable);

  picman_view_renderer_remove_idle (PICMAN_VIEW (view)->renderer);

  return view;
}

GtkWidget *
picman_view_new_full (PicmanContext  *context,
                    PicmanViewable *viewable,
                    gint          width,
                    gint          height,
                    gint          border_width,
                    gboolean      is_popup,
                    gboolean      clickable,
                    gboolean      show_popup)
{
  GtkWidget *view;

  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);

  view = picman_view_new_full_by_types (context,
                                      PICMAN_TYPE_VIEW,
                                      G_TYPE_FROM_INSTANCE (viewable),
                                      width, height, border_width,
                                      is_popup, clickable, show_popup);

  if (view)
    picman_view_set_viewable (PICMAN_VIEW (view), viewable);

  picman_view_renderer_remove_idle (PICMAN_VIEW (view)->renderer);

  return view;
}

GtkWidget *
picman_view_new_by_types (PicmanContext *context,
                        GType        view_type,
                        GType        viewable_type,
                        gint         size,
                        gint         border_width,
                        gboolean     is_popup)
{
  PicmanViewRenderer *renderer;
  PicmanView         *view;

  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (g_type_is_a (view_type, PICMAN_TYPE_VIEW), NULL);
  g_return_val_if_fail (g_type_is_a (viewable_type, PICMAN_TYPE_VIEWABLE), NULL);
  g_return_val_if_fail (size >  0 &&
                        size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (border_width >= 0 &&
                        border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH, NULL);

  renderer = picman_view_renderer_new (context, viewable_type, size,
                                     border_width, is_popup);

  g_return_val_if_fail (renderer != NULL, NULL);

  view = g_object_new (view_type, NULL);

  g_signal_connect (renderer, "update",
                    G_CALLBACK (picman_view_update_callback),
                    view);

  view->renderer = renderer;

  return GTK_WIDGET (view);
}

GtkWidget *
picman_view_new_full_by_types (PicmanContext *context,
                             GType        view_type,
                             GType        viewable_type,
                             gint         width,
                             gint         height,
                             gint         border_width,
                             gboolean     is_popup,
                             gboolean     clickable,
                             gboolean     show_popup)
{
  PicmanViewRenderer *renderer;
  PicmanView         *view;

  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (g_type_is_a (view_type, PICMAN_TYPE_VIEW), NULL);
  g_return_val_if_fail (g_type_is_a (viewable_type, PICMAN_TYPE_VIEWABLE), NULL);
  g_return_val_if_fail (width >  0 &&
                        width <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (height >  0 &&
                        height <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (border_width >= 0 &&
                        border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH, NULL);

  renderer = picman_view_renderer_new_full (context, viewable_type,
                                          width, height, border_width,
                                          is_popup);

  g_return_val_if_fail (renderer != NULL, NULL);

  view = g_object_new (view_type, NULL);

  g_signal_connect (renderer, "update",
                    G_CALLBACK (picman_view_update_callback),
                    view);

  view->renderer   = renderer;
  view->clickable  = clickable  ? TRUE : FALSE;
  view->show_popup = show_popup ? TRUE : FALSE;

  return GTK_WIDGET (view);
}

PicmanViewable *
picman_view_get_viewable (PicmanView *view)
{
  g_return_val_if_fail (PICMAN_IS_VIEW (view), NULL);

  return view->viewable;
}

void
picman_view_set_viewable (PicmanView     *view,
                        PicmanViewable *viewable)
{
  g_return_if_fail (PICMAN_IS_VIEW (view));
  g_return_if_fail (viewable == NULL || PICMAN_IS_VIEWABLE (viewable));

  if (viewable == view->viewable)
    return;

  g_signal_emit (view, view_signals[SET_VIEWABLE], 0, view->viewable, viewable);
}

void
picman_view_set_expand (PicmanView *view,
                      gboolean  expand)
{
  g_return_if_fail (PICMAN_IS_VIEW (view));

  if (view->expand != expand)
    {
      view->expand = expand ? TRUE : FALSE;
      gtk_widget_queue_resize (GTK_WIDGET (view));
    }
}


/*  private functions  */

static void
picman_view_update_callback (PicmanViewRenderer *renderer,
                           PicmanView         *view)
{
  GtkWidget      *widget = GTK_WIDGET (view);
  GtkRequisition  requisition;
  gint            width;
  gint            height;

  gtk_widget_get_requisition (widget, &requisition);

  width  = renderer->width  + 2 * renderer->border_width;
  height = renderer->height + 2 * renderer->border_width;

  if (width  != requisition.width ||
      height != requisition.height)
    {
      gtk_widget_queue_resize (widget);
    }
  else
    {
      gtk_widget_queue_draw (widget);
    }
}

static PicmanViewable *
picman_view_drag_viewable (GtkWidget    *widget,
                         PicmanContext **context,
                         gpointer      data)
{
  if (context)
    *context = PICMAN_VIEW (widget)->renderer->context;

  return PICMAN_VIEW (widget)->viewable;
}

static GdkPixbuf *
picman_view_drag_pixbuf (GtkWidget *widget,
                       gpointer   data)
{
  PicmanView     *view     = PICMAN_VIEW (widget);
  PicmanViewable *viewable = view->viewable;
  gint          width;
  gint          height;

  if (viewable && picman_viewable_get_size (viewable, &width, &height))
    return picman_viewable_get_new_pixbuf (viewable, view->renderer->context,
                                         width, height);

  return NULL;
}
