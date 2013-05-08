/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfgbgeditor.c
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#include "core/picmancontext.h"
#include "core/picmanmarshal.h"

#include "picmandnd.h"
#include "picmanfgbgeditor.h"


enum
{
  PROP_0,
  PROP_CONTEXT,
  PROP_ACTIVE_COLOR
};

enum
{
  COLOR_CLICKED,
  LAST_SIGNAL
};

typedef enum
{
  INVALID_AREA,
  FOREGROUND_AREA,
  BACKGROUND_AREA,
  SWAP_AREA,
  DEFAULT_AREA
} FgBgTarget;


static void     picman_fg_bg_editor_dispose         (GObject        *object);
static void     picman_fg_bg_editor_set_property    (GObject        *object,
                                                   guint           property_id,
                                                   const GValue   *value,
                                                   GParamSpec     *pspec);
static void     picman_fg_bg_editor_get_property    (GObject        *object,
                                                   guint           property_id,
                                                   GValue         *value,
                                                   GParamSpec     *pspec);

static gboolean picman_fg_bg_editor_expose          (GtkWidget      *widget,
                                                   GdkEventExpose *eevent);
static gboolean picman_fg_bg_editor_button_press    (GtkWidget      *widget,
                                                   GdkEventButton *bevent);
static gboolean picman_fg_bg_editor_button_release  (GtkWidget      *widget,
                                                   GdkEventButton *bevent);
static gboolean picman_fg_bg_editor_drag_motion     (GtkWidget      *widget,
                                                   GdkDragContext *context,
                                                   gint            x,
                                                   gint            y,
                                                   guint           time);

static void     picman_fg_bg_editor_drag_color      (GtkWidget      *widget,
                                                   PicmanRGB        *color,
                                                   gpointer        data);
static void     picman_fg_bg_editor_drop_color      (GtkWidget      *widget,
                                                   gint            x,
                                                   gint            y,
                                                   const PicmanRGB  *color,
                                                   gpointer        data);


G_DEFINE_TYPE (PicmanFgBgEditor, picman_fg_bg_editor, GTK_TYPE_DRAWING_AREA)

#define parent_class picman_fg_bg_editor_parent_class

static guint  editor_signals[LAST_SIGNAL] = { 0 };


static void
picman_fg_bg_editor_class_init (PicmanFgBgEditorClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  editor_signals[COLOR_CLICKED] =
    g_signal_new ("color-clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanFgBgEditorClass, color_clicked),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_ACTIVE_COLOR);

  object_class->dispose              = picman_fg_bg_editor_dispose;
  object_class->set_property         = picman_fg_bg_editor_set_property;
  object_class->get_property         = picman_fg_bg_editor_get_property;

  widget_class->expose_event         = picman_fg_bg_editor_expose;
  widget_class->button_press_event   = picman_fg_bg_editor_button_press;
  widget_class->button_release_event = picman_fg_bg_editor_button_release;
  widget_class->drag_motion          = picman_fg_bg_editor_drag_motion;

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_ACTIVE_COLOR,
                                   g_param_spec_enum ("active-color",
                                                      NULL, NULL,
                                                      PICMAN_TYPE_ACTIVE_COLOR,
                                                      PICMAN_ACTIVE_COLOR_FOREGROUND,
                                                      PICMAN_PARAM_READWRITE));
}

static void
picman_fg_bg_editor_init (PicmanFgBgEditor *editor)
{
  editor->context      = NULL;
  editor->active_color = PICMAN_ACTIVE_COLOR_FOREGROUND;

  gtk_widget_add_events (GTK_WIDGET (editor),
                         GDK_BUTTON_PRESS_MASK |
                         GDK_BUTTON_RELEASE_MASK);

  picman_dnd_color_source_add (GTK_WIDGET (editor),
                             picman_fg_bg_editor_drag_color, NULL);
  picman_dnd_color_dest_add (GTK_WIDGET (editor),
                           picman_fg_bg_editor_drop_color, NULL);
}

static void
picman_fg_bg_editor_dispose (GObject *object)
{
  PicmanFgBgEditor *editor = PICMAN_FG_BG_EDITOR (object);

  if (editor->context)
    picman_fg_bg_editor_set_context (editor, NULL);

  if (editor->default_icon)
    {
      g_object_unref (editor->default_icon);
      editor->default_icon = NULL;
    }

  if (editor->swap_icon)
    {
      g_object_unref (editor->swap_icon);
      editor->swap_icon = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_fg_bg_editor_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanFgBgEditor *editor = PICMAN_FG_BG_EDITOR (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      picman_fg_bg_editor_set_context (editor, g_value_get_object (value));
      break;
    case PROP_ACTIVE_COLOR:
      picman_fg_bg_editor_set_active (editor, g_value_get_enum (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_fg_bg_editor_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanFgBgEditor *editor = PICMAN_FG_BG_EDITOR (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (value, editor->context);
      break;
    case PROP_ACTIVE_COLOR:
      g_value_set_enum (value, editor->active_color);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_fg_bg_editor_expose (GtkWidget      *widget,
                          GdkEventExpose *eevent)
{
  PicmanFgBgEditor *editor = PICMAN_FG_BG_EDITOR (widget);
  GtkStyle       *style  = gtk_widget_get_style (widget);
  GdkWindow      *window = gtk_widget_get_window (widget);
  cairo_t        *cr;
  GtkAllocation   allocation;
  gint            width, height;
  gint            default_w, default_h;
  gint            swap_w, swap_h;
  gint            rect_w, rect_h;
  PicmanRGB         color;

  if (! gtk_widget_is_drawable (widget))
    return FALSE;

  cr = gdk_cairo_create (eevent->window);

  gdk_cairo_region (cr, eevent->region);
  cairo_clip (cr);

  gtk_widget_get_allocation (widget, &allocation);

  width  = allocation.width;
  height = allocation.height;

  /*  draw the default colors pixbuf  */
  if (! editor->default_icon)
    editor->default_icon = gtk_widget_render_icon (widget,
                                                   PICMAN_STOCK_DEFAULT_COLORS,
                                                   GTK_ICON_SIZE_MENU, NULL);

  default_w = gdk_pixbuf_get_width  (editor->default_icon);
  default_h = gdk_pixbuf_get_height (editor->default_icon);

  if (default_w < width / 2 && default_h < height / 2)
    {
      gdk_cairo_set_source_pixbuf (cr, editor->default_icon,
                                   0, height - default_h);
      cairo_paint (cr);
    }
  else
    {
      default_w = default_h = 0;
    }

  /*  draw the swap colors pixbuf  */
  if (! editor->swap_icon)
    editor->swap_icon = gtk_widget_render_icon (widget,
                                                PICMAN_STOCK_SWAP_COLORS,
                                                GTK_ICON_SIZE_MENU, NULL);

  swap_w = gdk_pixbuf_get_width  (editor->swap_icon);
  swap_h = gdk_pixbuf_get_height (editor->swap_icon);

  if (swap_w < width / 2 && swap_h < height / 2)
    {
      gdk_cairo_set_source_pixbuf (cr, editor->swap_icon,
                                   width - swap_w, 0);
      cairo_paint (cr);
    }
  else
    {
      swap_w = swap_h = 0;
    }

  rect_h = height - MAX (default_h, swap_h) - 2;
  rect_w = width  - MAX (default_w, swap_w) - 4;

  if (rect_h > (height * 3 / 4))
    rect_w = MAX (rect_w - (rect_h - ((height * 3 / 4))),
                  width * 2 / 3);

  editor->rect_width  = rect_w;
  editor->rect_height = rect_h;


  /*  draw the background area  */

  if (editor->context)
    {
      picman_context_get_background (editor->context, &color);
      picman_cairo_set_source_rgb (cr, &color);

      cairo_rectangle (cr,
                       width - rect_w,
                       height - rect_h,
                       rect_w,
                       rect_h);
      cairo_fill (cr);
    }

  gtk_paint_shadow (style, window, GTK_STATE_NORMAL,
                    editor->active_color == PICMAN_ACTIVE_COLOR_FOREGROUND ?
                    GTK_SHADOW_OUT : GTK_SHADOW_IN,
                    NULL, widget, NULL,
                    (width - rect_w),
                    (height - rect_h),
                    rect_w, rect_h);


  /*  draw the foreground area  */

  if (editor->context)
    {
      picman_context_get_foreground (editor->context, &color);
      picman_cairo_set_source_rgb (cr, &color);

      cairo_rectangle (cr,
                       0, 0,
                       rect_w, rect_h);
      cairo_fill (cr);
    }

  gtk_paint_shadow (style, window, GTK_STATE_NORMAL,
                    editor->active_color == PICMAN_ACTIVE_COLOR_BACKGROUND ?
                    GTK_SHADOW_OUT : GTK_SHADOW_IN,
                    NULL, widget, NULL,
                    0, 0,
                    rect_w, rect_h);

  cairo_destroy (cr);

  return TRUE;
}

static FgBgTarget
picman_fg_bg_editor_target (PicmanFgBgEditor *editor,
                          gint            x,
                          gint            y)
{
  GtkAllocation allocation;
  gint          width;
  gint          height;
  gint          rect_w = editor->rect_width;
  gint          rect_h = editor->rect_height;

  gtk_widget_get_allocation (GTK_WIDGET (editor), &allocation);

  width  = allocation.width;
  height = allocation.height;

  if (x > 0 && x < rect_w && y > 0 && y < rect_h)
    return FOREGROUND_AREA;
  else if (x > (width - rect_w)  && x < width  &&
           y > (height - rect_h) && y < height)
    return BACKGROUND_AREA;
  else if (x > 0      && x < (width - rect_w) &&
           y > rect_h && y < height)
    return DEFAULT_AREA;
  else if (x > rect_w && x < width &&
           y > 0      && y < (height - rect_h))
    return SWAP_AREA;

  return INVALID_AREA;
}

static gboolean
picman_fg_bg_editor_button_press (GtkWidget      *widget,
                                GdkEventButton *bevent)
{
  PicmanFgBgEditor *editor = PICMAN_FG_BG_EDITOR (widget);

  if (bevent->button == 1 && bevent->type == GDK_BUTTON_PRESS)
    {
      FgBgTarget target = picman_fg_bg_editor_target (editor,
                                                    bevent->x, bevent->y);

      editor->click_target = INVALID_AREA;

      switch (target)
        {
        case FOREGROUND_AREA:
          if (editor->active_color != PICMAN_ACTIVE_COLOR_FOREGROUND)
            picman_fg_bg_editor_set_active (editor,
                                          PICMAN_ACTIVE_COLOR_FOREGROUND);
          editor->click_target = FOREGROUND_AREA;
          break;

        case BACKGROUND_AREA:
          if (editor->active_color != PICMAN_ACTIVE_COLOR_BACKGROUND)
            picman_fg_bg_editor_set_active (editor,
                                          PICMAN_ACTIVE_COLOR_BACKGROUND);
          editor->click_target = BACKGROUND_AREA;
          break;

        case SWAP_AREA:
          if (editor->context)
            picman_context_swap_colors (editor->context);
          break;

        case DEFAULT_AREA:
          if (editor->context)
            picman_context_set_default_colors (editor->context);
          break;

        default:
          break;
        }
    }

  return FALSE;
}

static gboolean
picman_fg_bg_editor_button_release (GtkWidget      *widget,
                                  GdkEventButton *bevent)
{
  PicmanFgBgEditor *editor = PICMAN_FG_BG_EDITOR (widget);

  if (bevent->button == 1)
    {
      FgBgTarget target = picman_fg_bg_editor_target (editor,
                                                    bevent->x, bevent->y);

      if (target == editor->click_target)
        {
          switch (target)
            {
            case FOREGROUND_AREA:
              g_signal_emit (editor, editor_signals[COLOR_CLICKED], 0,
                             PICMAN_ACTIVE_COLOR_FOREGROUND);
              break;

            case BACKGROUND_AREA:
              g_signal_emit (editor, editor_signals[COLOR_CLICKED], 0,
                             PICMAN_ACTIVE_COLOR_BACKGROUND);
              break;

            default:
              break;
            }
        }

      editor->click_target = INVALID_AREA;
    }

  return FALSE;
}

static gboolean
picman_fg_bg_editor_drag_motion (GtkWidget      *widget,
                               GdkDragContext *context,
                               gint            x,
                               gint            y,
                               guint           time)
{
  PicmanFgBgEditor *editor = PICMAN_FG_BG_EDITOR (widget);
  FgBgTarget      target = picman_fg_bg_editor_target (editor, x, y);

  if (target == FOREGROUND_AREA || target == BACKGROUND_AREA)
    {
      gdk_drag_status (context, GDK_ACTION_COPY, time);

      return TRUE;
    }

  gdk_drag_status (context, 0, time);

  return FALSE;
}


/*  public functions  */

GtkWidget *
picman_fg_bg_editor_new (PicmanContext *context)
{
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);

  return g_object_new (PICMAN_TYPE_FG_BG_EDITOR,
                       "context", context,
                       NULL);
}

void
picman_fg_bg_editor_set_context (PicmanFgBgEditor *editor,
                               PicmanContext    *context)
{
  g_return_if_fail (PICMAN_IS_FG_BG_EDITOR (editor));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  if (context == editor->context)
    return;

  if (editor->context)
    {
      g_signal_handlers_disconnect_by_func (editor->context,
                                            gtk_widget_queue_draw,
                                            editor);
      g_object_unref (editor->context);
      editor->context = NULL;
    }

  editor->context = context;

  if (context)
    {
      g_object_ref (context);

      g_signal_connect_swapped (context, "foreground-changed",
                                G_CALLBACK (gtk_widget_queue_draw),
                                editor);
      g_signal_connect_swapped (context, "background-changed",
                                G_CALLBACK (gtk_widget_queue_draw),
                                editor);
    }

  g_object_notify (G_OBJECT (editor), "context");
}

void
picman_fg_bg_editor_set_active (PicmanFgBgEditor  *editor,
                              PicmanActiveColor  active)
{
  g_return_if_fail (PICMAN_IS_FG_BG_EDITOR (editor));

  editor->active_color = active;
  gtk_widget_queue_draw (GTK_WIDGET (editor));
  g_object_notify (G_OBJECT (editor), "active-color");
}


/*  private functions  */

static void
picman_fg_bg_editor_drag_color (GtkWidget *widget,
                              PicmanRGB   *color,
                              gpointer   data)
{
  PicmanFgBgEditor *editor = PICMAN_FG_BG_EDITOR (widget);

  if (editor->context)
    {
      switch (editor->active_color)
        {
        case PICMAN_ACTIVE_COLOR_FOREGROUND:
          picman_context_get_foreground (editor->context, color);
          break;

        case PICMAN_ACTIVE_COLOR_BACKGROUND:
          picman_context_get_background (editor->context, color);
          break;
        }
    }
}

static void
picman_fg_bg_editor_drop_color (GtkWidget     *widget,
                              gint           x,
                              gint           y,
                              const PicmanRGB *color,
                              gpointer       data)
{
  PicmanFgBgEditor *editor = PICMAN_FG_BG_EDITOR (widget);

  if (editor->context)
    {
      switch (picman_fg_bg_editor_target (editor, x, y))
        {
        case FOREGROUND_AREA:
          picman_context_set_foreground (editor->context, color);
          break;

        case BACKGROUND_AREA:
          picman_context_set_background (editor->context, color);
          break;

        default:
          break;
        }
    }
}
