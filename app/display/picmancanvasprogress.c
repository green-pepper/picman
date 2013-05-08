/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasprogress.c
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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
#include "libpicmanmath/picmanmath.h"

#include "display-types.h"

#include "core/picmanprogress.h"

#include "picmancanvas.h"
#include "picmancanvas-style.h"
#include "picmancanvasitem-utils.h"
#include "picmancanvasprogress.h"
#include "picmandisplayshell.h"


#define BORDER   5
#define RADIUS  20


enum
{
  PROP_0,
  PROP_ANCHOR,
  PROP_X,
  PROP_Y
};


typedef struct _PicmanCanvasProgressPrivate PicmanCanvasProgressPrivate;

struct _PicmanCanvasProgressPrivate
{
  PicmanHandleAnchor  anchor;
  gdouble           x;
  gdouble           y;

  gchar            *text;
  gdouble           value;
};

#define GET_PRIVATE(progress) \
        G_TYPE_INSTANCE_GET_PRIVATE (progress, \
                                     PICMAN_TYPE_CANVAS_PROGRESS, \
                                     PicmanCanvasProgressPrivate)


/*  local function prototypes  */

static void             picman_canvas_progress_iface_init   (PicmanProgressInterface *iface);

static void             picman_canvas_progress_finalize     (GObject          *object);
static void             picman_canvas_progress_set_property (GObject          *object,
                                                           guint             property_id,
                                                           const GValue     *value,
                                                           GParamSpec       *pspec);
static void             picman_canvas_progress_get_property (GObject          *object,
                                                           guint             property_id,
                                                           GValue           *value,
                                                           GParamSpec       *pspec);
static void             picman_canvas_progress_draw         (PicmanCanvasItem   *item,
                                                           cairo_t          *cr);
static cairo_region_t * picman_canvas_progress_get_extents  (PicmanCanvasItem   *item);

static PicmanProgress   * picman_canvas_progress_start        (PicmanProgress      *progress,
                                                           const gchar       *message,
                                                           gboolean           cancelable);
static void             picman_canvas_progress_end          (PicmanProgress      *progress);
static gboolean         picman_canvas_progress_is_active    (PicmanProgress      *progress);
static void             picman_canvas_progress_set_text     (PicmanProgress      *progress,
                                                           const gchar       *message);
static void             picman_canvas_progress_set_value    (PicmanProgress      *progress,
                                                           gdouble            percentage);
static gdouble          picman_canvas_progress_get_value    (PicmanProgress      *progress);
static void             picman_canvas_progress_pulse        (PicmanProgress      *progress);
static gboolean         picman_canvas_progress_message      (PicmanProgress      *progress,
                                                           Picman              *picman,
                                                           PicmanMessageSeverity severity,
                                                           const gchar       *domain,
                                                           const gchar       *message);


G_DEFINE_TYPE_WITH_CODE (PicmanCanvasProgress, picman_canvas_progress,
                         PICMAN_TYPE_CANVAS_ITEM,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROGRESS,
                                                picman_canvas_progress_iface_init))

#define parent_class picman_canvas_progress_parent_class


static void
picman_canvas_progress_class_init (PicmanCanvasProgressClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->finalize     = picman_canvas_progress_finalize;
  object_class->set_property = picman_canvas_progress_set_property;
  object_class->get_property = picman_canvas_progress_get_property;

  item_class->draw           = picman_canvas_progress_draw;
  item_class->get_extents    = picman_canvas_progress_get_extents;

  g_object_class_install_property (object_class, PROP_ANCHOR,
                                   g_param_spec_enum ("anchor", NULL, NULL,
                                                      PICMAN_TYPE_HANDLE_ANCHOR,
                                                      PICMAN_HANDLE_ANCHOR_CENTER,
                                                      PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_X,
                                   g_param_spec_double ("x", NULL, NULL,
                                                        -PICMAN_MAX_IMAGE_SIZE,
                                                        PICMAN_MAX_IMAGE_SIZE, 0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_Y,
                                   g_param_spec_double ("y", NULL, NULL,
                                                        -PICMAN_MAX_IMAGE_SIZE,
                                                        PICMAN_MAX_IMAGE_SIZE, 0,
                                                        PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasProgressPrivate));
}

static void
picman_canvas_progress_iface_init (PicmanProgressInterface *iface)
{
  iface->start     = picman_canvas_progress_start;
  iface->end       = picman_canvas_progress_end;
  iface->is_active = picman_canvas_progress_is_active;
  iface->set_text  = picman_canvas_progress_set_text;
  iface->set_value = picman_canvas_progress_set_value;
  iface->get_value = picman_canvas_progress_get_value;
  iface->pulse     = picman_canvas_progress_pulse;
  iface->message   = picman_canvas_progress_message;
}

static void
picman_canvas_progress_init (PicmanCanvasProgress *progress)
{
}

static void
picman_canvas_progress_finalize (GObject *object)
{
  PicmanCanvasProgressPrivate *private = GET_PRIVATE (object);

  if (private->text)
    {
      g_free (private->text);
      private->text = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_canvas_progress_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanCanvasProgressPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_ANCHOR:
      private->anchor = g_value_get_enum (value);
      break;
    case PROP_X:
      private->x = g_value_get_double (value);
      break;
    case PROP_Y:
      private->y = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_progress_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanCanvasProgressPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_ANCHOR:
      g_value_set_enum (value, private->anchor);
      break;
    case PROP_X:
      g_value_set_double (value, private->x);
      break;
    case PROP_Y:
      g_value_set_double (value, private->y);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static PangoLayout *
picman_canvas_progress_transform (PicmanCanvasItem *item,
                                gdouble        *x,
                                gdouble        *y,
                                gint           *width,
                                gint           *height)
{
  PicmanCanvasProgressPrivate *private = GET_PRIVATE (item);
  GtkWidget                 *canvas  = picman_canvas_item_get_canvas (item);
  PangoLayout               *layout;

  layout = picman_canvas_get_layout (PICMAN_CANVAS (canvas), "%s",
                                   private->text);

  pango_layout_get_pixel_size (layout, width, height);

  *width  += 2 * BORDER;
  *height += 3 * BORDER + 2 * RADIUS;

  picman_canvas_item_transform_xy_f (item,
                                   private->x, private->y,
                                   x, y);

  picman_canvas_item_shift_to_north_west (private->anchor,
                                        *x, *y,
                                        *width,
                                        *height,
                                        x, y);

  *x = floor (*x);
  *y = floor (*y);

  return layout;
}

static void
picman_canvas_progress_draw (PicmanCanvasItem *item,
                           cairo_t        *cr)
{
  PicmanCanvasProgressPrivate *private = GET_PRIVATE (item);
  GtkWidget                 *canvas  = picman_canvas_item_get_canvas (item);
  gdouble                    x, y;
  gint                       width, height;

  picman_canvas_progress_transform (item, &x, &y, &width, &height);

  cairo_move_to (cr, x, y);
  cairo_line_to (cr, x + width, y);
  cairo_line_to (cr, x + width, y + height - BORDER - 2 * RADIUS);
  cairo_line_to (cr, x + 2 * BORDER + 2 * RADIUS, y + height - BORDER - 2 * RADIUS);
  cairo_arc (cr, x + BORDER + RADIUS, y + height - BORDER - RADIUS,
             BORDER + RADIUS, 0, G_PI);
  cairo_close_path (cr);

  _picman_canvas_item_fill (item, cr);

  cairo_move_to (cr, x + BORDER, y + BORDER);
  cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
  pango_cairo_show_layout (cr,
                           picman_canvas_get_layout (PICMAN_CANVAS (canvas),
                                                   "%s", private->text));

  picman_canvas_set_tool_bg_style (picman_canvas_item_get_canvas (item), cr);
  cairo_arc (cr, x + BORDER + RADIUS, y + height - BORDER - RADIUS,
             RADIUS, - G_PI / 2.0, 2 * G_PI - G_PI / 2.0);
  cairo_fill (cr);

  cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, 1.0);
  cairo_move_to (cr, x + BORDER + RADIUS, y + height - BORDER - RADIUS);
  cairo_arc (cr, x + BORDER + RADIUS, y + height - BORDER - RADIUS,
             RADIUS, - G_PI / 2.0, 2 * G_PI * private->value - G_PI / 2.0);
  cairo_fill (cr);
}

static cairo_region_t *
picman_canvas_progress_get_extents (PicmanCanvasItem *item)
{
  cairo_rectangle_int_t rectangle;
  gdouble               x, y;
  gint                  width, height;

  picman_canvas_progress_transform (item, &x, &y, &width, &height);

  /*  add 1px on each side because fill()'s default impl does the same  */
  rectangle.x      = (gint) x - 1;
  rectangle.y      = (gint) y - 1;
  rectangle.width  = width  + 2;
  rectangle.height = height + 2;

  return cairo_region_create_rectangle (&rectangle);
}

static PicmanProgress *
picman_canvas_progress_start (PicmanProgress *progress,
                            const gchar  *message,
                            gboolean      cancelable)
{
  picman_canvas_progress_set_text (progress, message);

  return progress;
}

static void
picman_canvas_progress_end (PicmanProgress *progress)
{
}

static gboolean
picman_canvas_progress_is_active (PicmanProgress *progress)
{
  return TRUE;
}

static void
picman_canvas_progress_set_text (PicmanProgress *progress,
                               const gchar  *message)
{
  PicmanCanvasProgressPrivate *private = GET_PRIVATE (progress);
  cairo_region_t            *old_region;
  cairo_region_t            *new_region;

  old_region = picman_canvas_item_get_extents (PICMAN_CANVAS_ITEM (progress));

  if (private->text)
    g_free (private->text);

  private->text = g_strdup (message);

  new_region = picman_canvas_item_get_extents (PICMAN_CANVAS_ITEM (progress));

  cairo_region_union (new_region, old_region);
  cairo_region_destroy (old_region);

  _picman_canvas_item_update (PICMAN_CANVAS_ITEM (progress), new_region);

  cairo_region_destroy (new_region);
}

static void
picman_canvas_progress_set_value (PicmanProgress *progress,
                                gdouble       percentage)
{
  PicmanCanvasProgressPrivate *private = GET_PRIVATE (progress);

  if (percentage != private->value)
    {
      cairo_region_t *region;

      private->value = percentage;

      region = picman_canvas_item_get_extents (PICMAN_CANVAS_ITEM (progress));

      _picman_canvas_item_update (PICMAN_CANVAS_ITEM (progress), region);

      cairo_region_destroy (region);
    }
}

static gdouble
picman_canvas_progress_get_value (PicmanProgress *progress)
{
  PicmanCanvasProgressPrivate *private = GET_PRIVATE (progress);

  return private->value;
}

static void
picman_canvas_progress_pulse (PicmanProgress *progress)
{
}

static gboolean
picman_canvas_progress_message (PicmanProgress        *progress,
                              Picman                *picman,
                              PicmanMessageSeverity  severity,
                              const gchar         *domain,
                              const gchar         *message)
{
  return FALSE;
}

PicmanCanvasItem *
picman_canvas_progress_new (PicmanDisplayShell *shell,
                          PicmanHandleAnchor  anchor,
                          gdouble           x,
                          gdouble           y)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_PROGRESS,
                       "shell",  shell,
                       "anchor", anchor,
                       "x",      x,
                       "y",      y,
                       NULL);
}
