/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancellrendererdashes.c
 * Copyright (C) 2005 Sven Neumann <sven@picman.org>
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

#include <gtk/gtk.h>

#include "widgets-types.h"

#include "core/picmandashpattern.h"

#include "picmancellrendererdashes.h"


#define DASHES_WIDTH   96
#define DASHES_HEIGHT   4

#define N_SEGMENTS     24
#define BLOCK_WIDTH    (DASHES_WIDTH / (2 * N_SEGMENTS))


enum
{
  PROP_0,
  PROP_PATTERN
};


static void picman_cell_renderer_dashes_finalize     (GObject         *object);
static void picman_cell_renderer_dashes_get_property (GObject         *object,
                                                    guint            param_id,
                                                    GValue          *value,
                                                    GParamSpec      *pspec);
static void picman_cell_renderer_dashes_set_property (GObject         *object,
                                                    guint            param_id,
                                                    const GValue    *value,
                                                    GParamSpec      *pspec);
static void picman_cell_renderer_dashes_get_size     (GtkCellRenderer *cell,
                                                    GtkWidget       *widget,
                                                    GdkRectangle    *rectangle,
                                                    gint            *x_offset,
                                                    gint            *y_offset,
                                                    gint            *width,
                                                    gint            *height);
static void picman_cell_renderer_dashes_render       (GtkCellRenderer *cell,
                                                    GdkWindow       *window,
                                                    GtkWidget       *widget,
                                                    GdkRectangle    *background_area,
                                                    GdkRectangle    *cell_area,
                                                    GdkRectangle    *expose_area,
                                                    GtkCellRendererState flags);


G_DEFINE_TYPE (PicmanCellRendererDashes, picman_cell_renderer_dashes,
               GTK_TYPE_CELL_RENDERER)

#define parent_class picman_cell_renderer_dashes_parent_class


static void
picman_cell_renderer_dashes_class_init (PicmanCellRendererDashesClass *klass)
{
  GObjectClass         *object_class = G_OBJECT_CLASS (klass);
  GtkCellRendererClass *cell_class   = GTK_CELL_RENDERER_CLASS (klass);

  object_class->finalize     = picman_cell_renderer_dashes_finalize;
  object_class->get_property = picman_cell_renderer_dashes_get_property;
  object_class->set_property = picman_cell_renderer_dashes_set_property;

  cell_class->get_size       = picman_cell_renderer_dashes_get_size;
  cell_class->render         = picman_cell_renderer_dashes_render;

  g_object_class_install_property (object_class, PROP_PATTERN,
                                   g_param_spec_boxed ("pattern", NULL, NULL,
                                                       PICMAN_TYPE_DASH_PATTERN,
                                                       PICMAN_PARAM_WRITABLE));
}

static void
picman_cell_renderer_dashes_init (PicmanCellRendererDashes *dashes)
{
  dashes->segments = g_new0 (gboolean, N_SEGMENTS);
}

static void
picman_cell_renderer_dashes_finalize (GObject *object)
{
  PicmanCellRendererDashes *dashes = PICMAN_CELL_RENDERER_DASHES (object);

  g_free (dashes->segments);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_cell_renderer_dashes_get_property (GObject    *object,
                                        guint       param_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
}

static void
picman_cell_renderer_dashes_set_property (GObject      *object,
                                        guint         param_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  PicmanCellRendererDashes *dashes = PICMAN_CELL_RENDERER_DASHES (object);

  switch (param_id)
    {
    case PROP_PATTERN:
      picman_dash_pattern_fill_segments (g_value_get_boxed (value),
                                       dashes->segments, N_SEGMENTS);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
picman_cell_renderer_dashes_get_size (GtkCellRenderer *cell,
                                    GtkWidget       *widget,
                                    GdkRectangle    *cell_area,
                                    gint            *x_offset,
                                    gint            *y_offset,
                                    gint            *width,
                                    gint            *height)
{
  gfloat xalign, yalign;
  gint   xpad, ypad;

  gtk_cell_renderer_get_alignment (cell, &xalign, &yalign);
  gtk_cell_renderer_get_padding (cell, &xpad, &ypad);

  if (cell_area)
    {
      if (x_offset)
        {
          gdouble align;

          align = ((gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) ?
                   1.0 - xalign : xalign);

          *x_offset = align * (cell_area->width - DASHES_WIDTH);
          *x_offset = MAX (*x_offset, 0) + xpad;
        }

      if (y_offset)
        {
          *y_offset = yalign * (cell_area->height - DASHES_HEIGHT);
          *y_offset = MAX (*y_offset, 0) + ypad;
        }
    }
  else
    {
      if (x_offset)
        *x_offset = 0;

      if (y_offset)
        *y_offset = 0;
    }

  *width  = DASHES_WIDTH  + 2 * xpad;
  *height = DASHES_HEIGHT + 2 * ypad;
}

static void
picman_cell_renderer_dashes_render (GtkCellRenderer      *cell,
                                  GdkWindow            *window,
                                  GtkWidget            *widget,
                                  GdkRectangle         *background_area,
                                  GdkRectangle         *cell_area,
                                  GdkRectangle         *expose_area,
                                  GtkCellRendererState  flags)
{
  PicmanCellRendererDashes *dashes = PICMAN_CELL_RENDERER_DASHES (cell);
  GtkStyle               *style  = gtk_widget_get_style (widget);
  GtkStateType            state;
  gint                    xpad, ypad;
  cairo_t                *cr;
  gint                    width;
  gint                    x, y;

  gtk_cell_renderer_get_padding (cell, &xpad, &ypad);

  if (! gtk_cell_renderer_get_sensitive (cell))
    {
      state = GTK_STATE_INSENSITIVE;
    }
  else if ((flags & GTK_CELL_RENDERER_SELECTED) == GTK_CELL_RENDERER_SELECTED)
    {
      if (gtk_widget_has_focus (widget))
        state = GTK_STATE_SELECTED;
      else
        state = GTK_STATE_ACTIVE;
    }
  else if ((flags & GTK_CELL_RENDERER_PRELIT) == GTK_CELL_RENDERER_PRELIT &&
           gtk_widget_get_state (widget) == GTK_STATE_PRELIGHT)
    {
      state = GTK_STATE_PRELIGHT;
    }
  else
    {
      if (gtk_widget_is_sensitive (widget))
        state = GTK_STATE_NORMAL;
      else
        state = GTK_STATE_INSENSITIVE;
    }

  y = cell_area->y + (cell_area->height - DASHES_HEIGHT) / 2;
  width = cell_area->width - 2 * xpad;

  cr = gdk_cairo_create (window);

  gdk_cairo_rectangle (cr, expose_area);
  cairo_clip (cr);

  for (x = 0; x < width + BLOCK_WIDTH; x += BLOCK_WIDTH)
    {
      guint index = ((guint) x / BLOCK_WIDTH) % N_SEGMENTS;

      if (dashes->segments[index])
        {
          cairo_rectangle (cr,
                           cell_area->x + xpad + x, y,
                           MIN (BLOCK_WIDTH, width - x), DASHES_HEIGHT);
        }
    }

  gdk_cairo_set_source_color (cr, &style->text[state]);
  cairo_fill (cr);

  cairo_destroy (cr);
}

GtkCellRenderer *
picman_cell_renderer_dashes_new (void)
{
  return g_object_new (PICMAN_TYPE_CELL_RENDERER_DASHES, NULL);
}
