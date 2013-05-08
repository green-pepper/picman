/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancellrendererviewable.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#include <config.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "widgets-types.h"

#include "core/picmanmarshal.h"
#include "core/picmanviewable.h"

#include "picmancellrendererviewable.h"
#include "picmanview-popup.h"
#include "picmanviewrenderer.h"


enum
{
  PRE_CLICKED,
  CLICKED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_RENDERER
};


static void picman_cell_renderer_viewable_finalize     (GObject         *object);
static void picman_cell_renderer_viewable_get_property (GObject         *object,
                                                      guint            param_id,
                                                      GValue          *value,
                                                      GParamSpec      *pspec);
static void picman_cell_renderer_viewable_set_property (GObject         *object,
                                                      guint            param_id,
                                                      const GValue    *value,
                                                      GParamSpec      *pspec);
static void picman_cell_renderer_viewable_get_size     (GtkCellRenderer *cell,
                                                      GtkWidget       *widget,
                                                      GdkRectangle    *rectangle,
                                                      gint            *x_offset,
                                                      gint            *y_offset,
                                                      gint            *width,
                                                      gint            *height);
static void picman_cell_renderer_viewable_render       (GtkCellRenderer *cell,
                                                      GdkWindow       *window,
                                                      GtkWidget       *widget,
                                                      GdkRectangle    *background_area,
                                                      GdkRectangle    *cell_area,
                                                      GdkRectangle    *expose_area,
                                                      GtkCellRendererState flags);
static gboolean picman_cell_renderer_viewable_activate (GtkCellRenderer *cell,
                                                      GdkEvent        *event,
                                                      GtkWidget       *widget,
                                                      const gchar     *path,
                                                      GdkRectangle    *background_area,
                                                      GdkRectangle    *cell_area,
                                                      GtkCellRendererState flags);


G_DEFINE_TYPE (PicmanCellRendererViewable, picman_cell_renderer_viewable,
               GTK_TYPE_CELL_RENDERER)

#define parent_class picman_cell_renderer_viewable_parent_class

static guint viewable_cell_signals[LAST_SIGNAL] = { 0 };


static void
picman_cell_renderer_viewable_class_init (PicmanCellRendererViewableClass *klass)
{
  GObjectClass         *object_class = G_OBJECT_CLASS (klass);
  GtkCellRendererClass *cell_class   = GTK_CELL_RENDERER_CLASS (klass);

  /**
   * PicmanCellRendererViewable::pre-clicked:
   * @cell:
   * @path:
   * @state:
   *
   * Called early on a viewable cell when it is clicked, typically
   * before selection code is invoked for example.
   *
   * Returns: %TRUE if the signal handled the event and event
   *          propagation should stop, for example preventing a
   *          selection from happening, %FALSE to continue as normal
   **/
  viewable_cell_signals[PRE_CLICKED] =
    g_signal_new ("pre-clicked",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanCellRendererViewableClass, pre_clicked),
                  g_signal_accumulator_true_handled, NULL,
                  picman_marshal_BOOLEAN__STRING_FLAGS,
                  G_TYPE_BOOLEAN, 2,
                  G_TYPE_STRING,
                  GDK_TYPE_MODIFIER_TYPE);

  /**
   * PicmanCellRendererViewable::clicked:
   * @cell:
   * @path:
   * @state:
   *
   * Called late on a viewable cell when it is clicked, typically
   * after selection code has been invoked for example.
   **/
  viewable_cell_signals[CLICKED] =
    g_signal_new ("clicked",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanCellRendererViewableClass, clicked),
                  NULL, NULL,
                  picman_marshal_VOID__STRING_FLAGS,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  GDK_TYPE_MODIFIER_TYPE);

  object_class->finalize     = picman_cell_renderer_viewable_finalize;
  object_class->get_property = picman_cell_renderer_viewable_get_property;
  object_class->set_property = picman_cell_renderer_viewable_set_property;

  cell_class->get_size       = picman_cell_renderer_viewable_get_size;
  cell_class->render         = picman_cell_renderer_viewable_render;
  cell_class->activate       = picman_cell_renderer_viewable_activate;

  klass->clicked             = NULL;

  g_object_class_install_property (object_class, PROP_RENDERER,
                                   g_param_spec_object ("renderer",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_VIEW_RENDERER,
                                                        PICMAN_PARAM_READWRITE));
}

static void
picman_cell_renderer_viewable_init (PicmanCellRendererViewable *cellviewable)
{
  g_object_set (cellviewable,
                "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
                NULL);
}

static void
picman_cell_renderer_viewable_finalize (GObject *object)
{
  PicmanCellRendererViewable *cell = PICMAN_CELL_RENDERER_VIEWABLE (object);

  if (cell->renderer)
    {
      g_object_unref (cell->renderer);
      cell->renderer = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_cell_renderer_viewable_get_property (GObject    *object,
                                          guint       param_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  PicmanCellRendererViewable *cell = PICMAN_CELL_RENDERER_VIEWABLE (object);

  switch (param_id)
    {
    case PROP_RENDERER:
      g_value_set_object (value, cell->renderer);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
picman_cell_renderer_viewable_set_property (GObject      *object,
                                          guint         param_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  PicmanCellRendererViewable *cell = PICMAN_CELL_RENDERER_VIEWABLE (object);

  switch (param_id)
    {
    case PROP_RENDERER:
      {
        PicmanViewRenderer *renderer = g_value_dup_object (value);

        if (cell->renderer)
          g_object_unref (cell->renderer);

        cell->renderer = renderer;
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
picman_cell_renderer_viewable_get_size (GtkCellRenderer *cell,
                                      GtkWidget       *widget,
                                      GdkRectangle    *cell_area,
                                      gint            *x_offset,
                                      gint            *y_offset,
                                      gint            *width,
                                      gint            *height)
{
  PicmanCellRendererViewable *cellviewable;
  gfloat                    xalign, yalign;
  gint                      xpad, ypad;
  gint                      view_width  = 0;
  gint                      view_height = 0;
  gint                      calc_width;
  gint                      calc_height;

  gtk_cell_renderer_get_alignment (cell, &xalign, &yalign);
  gtk_cell_renderer_get_padding (cell, &xpad, &ypad);

  cellviewable = PICMAN_CELL_RENDERER_VIEWABLE (cell);

  if (cellviewable->renderer)
    {
      view_width  = (cellviewable->renderer->width  +
                     2 * cellviewable->renderer->border_width);
      view_height = (cellviewable->renderer->height +
                     2 * cellviewable->renderer->border_width);
    }

  calc_width  = (gint) xpad * 2 + view_width;
  calc_height = (gint) ypad * 2 + view_height;

  if (x_offset) *x_offset = 0;
  if (y_offset) *y_offset = 0;

  if (cell_area && view_width > 0 && view_height > 0)
    {
      if (x_offset)
        {
          *x_offset = (((gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) ?
                        1.0 - xalign : xalign) *
                       (cell_area->width - calc_width - 2 * xpad));
          *x_offset = (MAX (*x_offset, 0) + xpad);
        }
      if (y_offset)
        {
          *y_offset = (yalign * (cell_area->height - calc_height - 2 * ypad));
          *y_offset = (MAX (*y_offset, 0) + ypad);
        }
    }

  if (width)  *width  = calc_width;
  if (height) *height = calc_height;
}

static void
picman_cell_renderer_viewable_render (GtkCellRenderer      *cell,
                                    GdkWindow            *window,
                                    GtkWidget            *widget,
                                    GdkRectangle         *background_area,
                                    GdkRectangle         *cell_area,
                                    GdkRectangle         *expose_area,
                                    GtkCellRendererState  flags)
{
  PicmanCellRendererViewable *cellviewable;

  cellviewable = PICMAN_CELL_RENDERER_VIEWABLE (cell);

  if (cellviewable->renderer)
    {
      cairo_t *cr;

      if (! (flags & GTK_CELL_RENDERER_SELECTED))
        {
          /* this is an ugly hack. The cell state should be passed to
           * the view renderer, so that it can adjust its border.
           * (or something like this) */
          if (cellviewable->renderer->border_type == PICMAN_VIEW_BORDER_WHITE)
            picman_view_renderer_set_border_type (cellviewable->renderer,
                                                PICMAN_VIEW_BORDER_BLACK);

          picman_view_renderer_remove_idle (cellviewable->renderer);
        }

      cr = gdk_cairo_create (window);
      gdk_cairo_rectangle (cr, expose_area);
      cairo_clip (cr);

      cairo_translate (cr, cell_area->x, cell_area->y);

      picman_view_renderer_draw (cellviewable->renderer, widget, cr,
                               cell_area->width,
                               cell_area->height);

      cairo_destroy (cr);
    }
}

static gboolean
picman_cell_renderer_viewable_activate (GtkCellRenderer      *cell,
                                      GdkEvent             *event,
                                      GtkWidget            *widget,
                                      const gchar          *path,
                                      GdkRectangle         *background_area,
                                      GdkRectangle         *cell_area,
                                      GtkCellRendererState  flags)
{
  PicmanCellRendererViewable *cellviewable;

  cellviewable = PICMAN_CELL_RENDERER_VIEWABLE (cell);

  if (cellviewable->renderer)
    {
      GdkModifierType state = 0;

      if (event && ((GdkEventAny *) event)->type == GDK_BUTTON_PRESS)
        state = ((GdkEventButton *) event)->state;

      if (! event ||
          (((GdkEventAny *) event)->type == GDK_BUTTON_PRESS &&
           ((GdkEventButton *) event)->button == 1))
        {
          picman_cell_renderer_viewable_clicked (cellviewable, path, state);

          return TRUE;
        }
    }

  return FALSE;
}

GtkCellRenderer *
picman_cell_renderer_viewable_new (void)
{
  return g_object_new (PICMAN_TYPE_CELL_RENDERER_VIEWABLE, NULL);
}

gboolean
picman_cell_renderer_viewable_pre_clicked (PicmanCellRendererViewable *cell,
                                         const gchar              *path,
                                         GdkModifierType           state)
{
  gboolean handled = FALSE;

  g_return_val_if_fail (PICMAN_IS_CELL_RENDERER_VIEWABLE (cell), FALSE);
  g_return_val_if_fail (path != NULL, FALSE);

  g_signal_emit (cell,
                 viewable_cell_signals[PRE_CLICKED],
                 0 /*detail*/,
                 path, state,
                 &handled);

  return handled;
}

void
picman_cell_renderer_viewable_clicked (PicmanCellRendererViewable *cell,
                                     const gchar              *path,
                                     GdkModifierType           state)
{

  g_return_if_fail (PICMAN_IS_CELL_RENDERER_VIEWABLE (cell));
  g_return_if_fail (path != NULL);

  if (cell->renderer)
    {
      GdkEvent *event = gtk_get_current_event ();

      if (event)
        {
          GdkEventButton *bevent = (GdkEventButton *) event;

          if (bevent->type == GDK_BUTTON_PRESS &&
              (bevent->button == 1 || bevent->button == 2))
            {
              picman_view_popup_show (gtk_get_event_widget (event),
                                    bevent,
                                    cell->renderer->context,
                                    cell->renderer->viewable,
                                    cell->renderer->width,
                                    cell->renderer->height,
                                    cell->renderer->dot_for_dot);
            }

          gdk_event_free (event);
        }
    }

  /*  emit the signal last so no callback effects can set
   *  cell->renderer to NULL.
   */
  g_signal_emit (cell, viewable_cell_signals[CLICKED], 0, path, state);
}
