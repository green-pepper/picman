/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpaletteview.c
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

#include <gegl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanpalette.h"
#include "core/picmanmarshal.h"

#include "picmandnd.h"
#include "picmanpaletteview.h"
#include "picmanviewrendererpalette.h"


enum
{
  ENTRY_CLICKED,
  ENTRY_SELECTED,
  ENTRY_ACTIVATED,
  ENTRY_CONTEXT,
  COLOR_DROPPED,
  LAST_SIGNAL
};


static gboolean picman_palette_view_expose         (GtkWidget        *widget,
                                                  GdkEventExpose   *eevent);
static gboolean picman_palette_view_button_press   (GtkWidget        *widget,
                                                  GdkEventButton   *bevent);
static gboolean picman_palette_view_key_press      (GtkWidget        *widget,
                                                  GdkEventKey      *kevent);
static gboolean picman_palette_view_focus          (GtkWidget        *widget,
                                                  GtkDirectionType  direction);
static void     picman_palette_view_set_viewable   (PicmanView         *view,
                                                  PicmanViewable     *old_viewable,
                                                  PicmanViewable     *new_viewable);
static PicmanPaletteEntry *
                picman_palette_view_find_entry     (PicmanPaletteView *view,
                                                  gint             x,
                                                  gint             y);
static void     picman_palette_view_expose_entry   (PicmanPaletteView  *view,
                                                  PicmanPaletteEntry *entry);
static void     picman_palette_view_invalidate     (PicmanPalette      *palette,
                                                  PicmanPaletteView  *view);
static void     picman_palette_view_drag_color     (GtkWidget        *widget,
                                                  PicmanRGB          *color,
                                                  gpointer          data);
static void     picman_palette_view_drop_color     (GtkWidget        *widget,
                                                  gint              x,
                                                  gint              y,
                                                  const PicmanRGB    *color,
                                                  gpointer          data);


G_DEFINE_TYPE (PicmanPaletteView, picman_palette_view, PICMAN_TYPE_VIEW)

#define parent_class picman_palette_view_parent_class

static guint view_signals[LAST_SIGNAL] = { 0 };


static void
picman_palette_view_class_init (PicmanPaletteViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  PicmanViewClass  *view_class   = PICMAN_VIEW_CLASS (klass);

  view_signals[ENTRY_CLICKED] =
    g_signal_new ("entry-clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPaletteViewClass, entry_clicked),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER_ENUM,
                  G_TYPE_NONE, 2,
                  G_TYPE_POINTER,
                  GDK_TYPE_MODIFIER_TYPE);

  view_signals[ENTRY_SELECTED] =
    g_signal_new ("entry-selected",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPaletteViewClass, entry_selected),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  view_signals[ENTRY_ACTIVATED] =
    g_signal_new ("entry-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPaletteViewClass, entry_activated),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  view_signals[ENTRY_CONTEXT] =
    g_signal_new ("entry-context",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPaletteViewClass, entry_context),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  view_signals[COLOR_DROPPED] =
    g_signal_new ("color-dropped",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPaletteViewClass, color_dropped),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER_BOXED,
                  G_TYPE_NONE, 2,
                  G_TYPE_POINTER,
                  PICMAN_TYPE_RGB);

  widget_class->expose_event       = picman_palette_view_expose;
  widget_class->button_press_event = picman_palette_view_button_press;
  widget_class->key_press_event    = picman_palette_view_key_press;
  widget_class->focus              = picman_palette_view_focus;

  view_class->set_viewable         = picman_palette_view_set_viewable;
}

static void
picman_palette_view_init (PicmanPaletteView *view)
{
  gtk_widget_set_can_focus (GTK_WIDGET (view), TRUE);

  view->selected  = NULL;
  view->dnd_entry = NULL;
}

static gboolean
picman_palette_view_expose (GtkWidget      *widget,
                          GdkEventExpose *eevent)
{
  PicmanPaletteView *pal_view = PICMAN_PALETTE_VIEW (widget);
  PicmanView        *view     = PICMAN_VIEW (widget);

  if (! gtk_widget_is_drawable (widget))
    return FALSE;

  GTK_WIDGET_CLASS (parent_class)->expose_event (widget, eevent);

  if (view->renderer->viewable && pal_view->selected)
    {
      PicmanViewRendererPalette *renderer;
      GtkStyle                *style = gtk_widget_get_style (widget);
      GtkAllocation            allocation;
      cairo_t                 *cr;
      gint                     row, col;

      renderer = PICMAN_VIEW_RENDERER_PALETTE (view->renderer);

      gtk_widget_get_allocation (widget, &allocation);

      row = pal_view->selected->position / renderer->columns;
      col = pal_view->selected->position % renderer->columns;

      cr = gdk_cairo_create (gtk_widget_get_window (widget));
      gdk_cairo_region (cr, eevent->region);
      cairo_clip (cr);

      cairo_translate (cr, allocation.x, allocation.y);

      cairo_rectangle (cr,
                       col * renderer->cell_width  + 0.5,
                       row * renderer->cell_height + 0.5,
                       renderer->cell_width,
                       renderer->cell_height);

      cairo_set_line_width (cr, 1.0);
      gdk_cairo_set_source_color (cr, &style->fg[GTK_STATE_SELECTED]);
      cairo_stroke_preserve (cr);

      if (picman_cairo_set_focus_line_pattern (cr, widget))
        {
          gdk_cairo_set_source_color (cr, &style->fg[GTK_STATE_NORMAL]);
          cairo_stroke (cr);
        }

      cairo_destroy (cr);
    }

  return FALSE;
}

static gboolean
picman_palette_view_button_press (GtkWidget      *widget,
                                GdkEventButton *bevent)
{
  PicmanPaletteView  *view = PICMAN_PALETTE_VIEW (widget);
  PicmanPaletteEntry *entry;

  if (gtk_widget_get_can_focus (widget) && ! gtk_widget_has_focus (widget))
    gtk_widget_grab_focus (widget);

  entry = picman_palette_view_find_entry (view, bevent->x, bevent->y);

  view->dnd_entry = entry;

  if (! entry || bevent->button == 2)
    return TRUE;

  if (bevent->type == GDK_BUTTON_PRESS)
    g_signal_emit (view, view_signals[ENTRY_CLICKED], 0,
                   entry, bevent->state);

  if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
    {
      if (entry != view->selected)
        picman_palette_view_select_entry (view, entry);

      g_signal_emit (view, view_signals[ENTRY_CONTEXT], 0, entry);
    }
  else if (bevent->button == 1)
    {
      if (bevent->type == GDK_BUTTON_PRESS)
        {
          picman_palette_view_select_entry (view, entry);
        }
      else if (bevent->type == GDK_2BUTTON_PRESS && entry == view->selected)
        {
          g_signal_emit (view, view_signals[ENTRY_ACTIVATED], 0, entry);
        }
    }

  return TRUE;
}

static gboolean
picman_palette_view_key_press (GtkWidget   *widget,
                             GdkEventKey *kevent)
{
  PicmanPaletteView *view = PICMAN_PALETTE_VIEW (widget);

  if (view->selected &&
      (kevent->keyval == GDK_KEY_space    ||
       kevent->keyval == GDK_KEY_KP_Space ||
       kevent->keyval == GDK_KEY_Return   ||
       kevent->keyval == GDK_KEY_KP_Enter ||
       kevent->keyval == GDK_KEY_ISO_Enter))
    {
      g_signal_emit (view, view_signals[ENTRY_CLICKED], 0,
                     view->selected, kevent->state);
    }

  return FALSE;
}

static gboolean
picman_palette_view_focus (GtkWidget        *widget,
                         GtkDirectionType  direction)
{
  PicmanPaletteView *view = PICMAN_PALETTE_VIEW (widget);
  PicmanPalette     *palette;

  palette = PICMAN_PALETTE (PICMAN_VIEW (view)->renderer->viewable);

  if (gtk_widget_get_can_focus (widget) &&
      ! gtk_widget_has_focus (widget))
    {
      gtk_widget_grab_focus (widget);

      if (! view->selected &&
          palette && picman_palette_get_n_colors (palette) > 0)
        {
          PicmanPaletteEntry *entry = picman_palette_get_entry (palette, 0);

          picman_palette_view_select_entry (view, entry);
        }

      return TRUE;
    }

  if (view->selected)
    {
      PicmanViewRendererPalette *renderer;
      gint                     skip = 0;

      renderer = PICMAN_VIEW_RENDERER_PALETTE (PICMAN_VIEW (view)->renderer);

      switch (direction)
        {
        case GTK_DIR_UP:
          skip = -renderer->columns;
          break;
        case GTK_DIR_DOWN:
          skip = renderer->columns;
          break;
        case GTK_DIR_LEFT:
          skip = -1;
          break;
        case GTK_DIR_RIGHT:
          skip = 1;
          break;

        case GTK_DIR_TAB_FORWARD:
        case GTK_DIR_TAB_BACKWARD:
          return FALSE;
        }

      if (skip != 0)
        {
          PicmanPaletteEntry *entry;
          gint              position;

          position = view->selected->position + skip;

          entry = picman_palette_get_entry (palette, position);

          if (entry)
            picman_palette_view_select_entry (view, entry);
        }

      return TRUE;
    }

  return FALSE;
}

static void
picman_palette_view_set_viewable (PicmanView     *view,
                                PicmanViewable *old_viewable,
                                PicmanViewable *new_viewable)
{
  PicmanPaletteView *pal_view = PICMAN_PALETTE_VIEW (view);

  pal_view->dnd_entry = NULL;
  picman_palette_view_select_entry (pal_view, NULL);

  if (old_viewable)
    {
      g_signal_handlers_disconnect_by_func (old_viewable,
                                            picman_palette_view_invalidate,
                                            view);

      if (! new_viewable)
        {
          picman_dnd_color_source_remove (GTK_WIDGET (view));
          picman_dnd_color_dest_remove (GTK_WIDGET (view));
        }
    }

  PICMAN_VIEW_CLASS (parent_class)->set_viewable (view,
                                                old_viewable, new_viewable);

  if (new_viewable)
    {
      g_signal_connect (new_viewable, "invalidate-preview",
                        G_CALLBACK (picman_palette_view_invalidate),
                        view);

      /*  unset the palette drag handler set by PicmanView  */
      picman_dnd_viewable_source_remove (GTK_WIDGET (view), PICMAN_TYPE_PALETTE);

      if (! old_viewable)
        {
          picman_dnd_color_source_add (GTK_WIDGET (view),
                                     picman_palette_view_drag_color,
                                     view);
          picman_dnd_color_dest_add (GTK_WIDGET (view),
                                   picman_palette_view_drop_color,
                                   view);
        }
    }
}


/*  public functions  */

void
picman_palette_view_select_entry (PicmanPaletteView  *view,
                                PicmanPaletteEntry *entry)
{
  g_return_if_fail (PICMAN_IS_PALETTE_VIEW (view));

  if (entry == view->selected)
    return;

  if (view->selected)
    picman_palette_view_expose_entry (view, view->selected);

  view->selected = entry;

  if (view->selected)
    picman_palette_view_expose_entry (view, view->selected);

  g_signal_emit (view, view_signals[ENTRY_SELECTED], 0, view->selected);
}


/*  private funcions  */

static PicmanPaletteEntry *
picman_palette_view_find_entry (PicmanPaletteView *view,
                              gint             x,
                              gint             y)
{
  PicmanPalette             *palette;
  PicmanViewRendererPalette *renderer;
  PicmanPaletteEntry        *entry = NULL;
  gint                     col, row;

  palette  = PICMAN_PALETTE (PICMAN_VIEW (view)->renderer->viewable);
  renderer = PICMAN_VIEW_RENDERER_PALETTE (PICMAN_VIEW (view)->renderer);

  if (! palette)
    return NULL;

  col = x / renderer->cell_width;
  row = y / renderer->cell_height;

  if (col >= 0 && col < renderer->columns &&
      row >= 0 && row < renderer->rows)
    {
      entry = picman_palette_get_entry (palette,
                                      row * renderer->columns + col);
    }

  return entry;
}

static void
picman_palette_view_expose_entry (PicmanPaletteView  *view,
                                PicmanPaletteEntry *entry)
{
  PicmanViewRendererPalette *renderer;
  gint                     row, col;
  GtkWidget               *widget = GTK_WIDGET (view);
  GtkAllocation            allocation;

  renderer = PICMAN_VIEW_RENDERER_PALETTE (PICMAN_VIEW (view)->renderer);

  gtk_widget_get_allocation (widget, &allocation);

  row = entry->position / renderer->columns;
  col = entry->position % renderer->columns;

  gtk_widget_queue_draw_area (GTK_WIDGET (view),
                              allocation.x + col * renderer->cell_width,
                              allocation.y + row * renderer->cell_height,
                              renderer->cell_width  + 1,
                              renderer->cell_height + 1);
}

static void
picman_palette_view_invalidate (PicmanPalette     *palette,
                              PicmanPaletteView *view)
{
  view->dnd_entry = NULL;

  if (view->selected &&
      ! g_list_find (picman_palette_get_colors (palette), view->selected))
    {
      picman_palette_view_select_entry (view, NULL);
    }
}

static void
picman_palette_view_drag_color (GtkWidget *widget,
                              PicmanRGB   *color,
                              gpointer   data)
{
  PicmanPaletteView *view = PICMAN_PALETTE_VIEW (data);

  if (view->dnd_entry)
    *color = view->dnd_entry->color;
  else
    picman_rgba_set (color, 0.0, 0.0, 0.0, 1.0);
}

static void
picman_palette_view_drop_color (GtkWidget     *widget,
                              gint           x,
                              gint           y,
                              const PicmanRGB *color,
                              gpointer       data)
{
  PicmanPaletteView  *view = PICMAN_PALETTE_VIEW (data);
  PicmanPaletteEntry *entry;

  entry = picman_palette_view_find_entry (view, x, y);

  g_signal_emit (view, view_signals[COLOR_DROPPED], 0,
                 entry, color);
}
