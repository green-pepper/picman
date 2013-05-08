/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasitem.c
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

#include "libpicmanmath/picmanmath.h"

#include "display-types.h"

#include "core/picmanmarshal.h"

#include "picmancanvas-style.h"
#include "picmancanvasitem.h"
#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-transform.h"


enum
{
  PROP_0,
  PROP_SHELL,
  PROP_VISIBLE,
  PROP_LINE_CAP,
  PROP_HIGHLIGHT
};

enum
{
  UPDATE,
  LAST_SIGNAL
};


typedef struct _PicmanCanvasItemPrivate PicmanCanvasItemPrivate;

struct _PicmanCanvasItemPrivate
{
  PicmanDisplayShell *shell;
  gboolean          visible;
  cairo_line_cap_t  line_cap;
  gboolean          highlight;
  gint              suspend_stroking;
  gint              suspend_filling;
  gint              change_count;
  cairo_region_t   *change_region;
};

#define GET_PRIVATE(item) \
        G_TYPE_INSTANCE_GET_PRIVATE (item, \
                                     PICMAN_TYPE_CANVAS_ITEM, \
                                     PicmanCanvasItemPrivate)


/*  local function prototypes  */

static void             picman_canvas_item_constructed      (GObject         *object);
static void             picman_canvas_item_set_property     (GObject         *object,
                                                           guint            property_id,
                                                           const GValue    *value,
                                                           GParamSpec      *pspec);
static void             picman_canvas_item_get_property     (GObject         *object,
                                                           guint            property_id,
                                                           GValue          *value,
                                                           GParamSpec      *pspec);
static void  picman_canvas_item_dispatch_properties_changed (GObject         *object,
                                                           guint            n_pspecs,
                                                           GParamSpec     **pspecs);

static void             picman_canvas_item_real_draw        (PicmanCanvasItem  *item,
                                                           cairo_t         *cr);
static cairo_region_t * picman_canvas_item_real_get_extents (PicmanCanvasItem  *item);
static void             picman_canvas_item_real_stroke      (PicmanCanvasItem  *item,
                                                           cairo_t         *cr);
static void             picman_canvas_item_real_fill        (PicmanCanvasItem  *item,
                                                           cairo_t         *cr);
static gboolean         picman_canvas_item_real_hit         (PicmanCanvasItem  *item,
                                                           gdouble          x,
                                                           gdouble          y);


G_DEFINE_TYPE (PicmanCanvasItem, picman_canvas_item,
               PICMAN_TYPE_OBJECT)

#define parent_class picman_canvas_item_parent_class

static guint item_signals[LAST_SIGNAL] = { 0 };


static void
picman_canvas_item_class_init (PicmanCanvasItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed                 = picman_canvas_item_constructed;
  object_class->set_property                = picman_canvas_item_set_property;
  object_class->get_property                = picman_canvas_item_get_property;
  object_class->dispatch_properties_changed = picman_canvas_item_dispatch_properties_changed;

  klass->update                             = NULL;
  klass->draw                               = picman_canvas_item_real_draw;
  klass->get_extents                        = picman_canvas_item_real_get_extents;
  klass->stroke                             = picman_canvas_item_real_stroke;
  klass->fill                               = picman_canvas_item_real_fill;
  klass->hit                                = picman_canvas_item_real_hit;

  item_signals[UPDATE] =
    g_signal_new ("update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanCanvasItemClass, update),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  g_object_class_install_property (object_class, PROP_SHELL,
                                   g_param_spec_object ("shell",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DISPLAY_SHELL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_VISIBLE,
                                   g_param_spec_boolean ("visible",
                                                         NULL, NULL,
                                                         TRUE,
                                                         PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_LINE_CAP,
                                   g_param_spec_int ("line-cap",
                                                     NULL, NULL,
                                                     CAIRO_LINE_CAP_BUTT,
                                                     CAIRO_LINE_CAP_SQUARE,
                                                     CAIRO_LINE_CAP_ROUND,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_HIGHLIGHT,
                                   g_param_spec_boolean ("highlight",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasItemPrivate));
}

static void
picman_canvas_item_init (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private = GET_PRIVATE (item);

  private->shell            = NULL;
  private->visible          = TRUE;
  private->line_cap         = CAIRO_LINE_CAP_ROUND;
  private->highlight        = FALSE;
  private->suspend_stroking = 0;
  private->suspend_filling  = 0;
  private->change_count     = 1; /* avoid emissions during construction */
  private->change_region    = NULL;
}

static void
picman_canvas_item_constructed (GObject *object)
{
  PicmanCanvasItemPrivate *private = GET_PRIVATE (object);

  g_assert (PICMAN_IS_DISPLAY_SHELL (private->shell));

  private->change_count = 0; /* undo hack from init() */

  G_OBJECT_CLASS (parent_class)->constructed (object);
}

static void
picman_canvas_item_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanCanvasItemPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_SHELL:
      private->shell = g_value_get_object (value); /* don't ref */
      break;
    case PROP_VISIBLE:
      private->visible = g_value_get_boolean (value);
      break;
    case PROP_LINE_CAP:
      private->line_cap = g_value_get_int (value);
      break;
    case PROP_HIGHLIGHT:
      private->highlight = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_item_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanCanvasItemPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_SHELL:
      g_value_set_object (value, private->shell);
      break;
    case PROP_VISIBLE:
      g_value_set_boolean (value, private->visible);
      break;
    case PROP_LINE_CAP:
      g_value_set_int (value, private->line_cap);
      break;
    case PROP_HIGHLIGHT:
      g_value_set_boolean (value, private->highlight);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_item_dispatch_properties_changed (GObject     *object,
                                              guint        n_pspecs,
                                              GParamSpec **pspecs)
{
  PicmanCanvasItem *item = PICMAN_CANVAS_ITEM (object);

  G_OBJECT_CLASS (parent_class)->dispatch_properties_changed (object,
                                                              n_pspecs,
                                                              pspecs);

  if (_picman_canvas_item_needs_update (item))
    {
      cairo_region_t *region = picman_canvas_item_get_extents (item);

      if (region)
        {
          g_signal_emit (object, item_signals[UPDATE], 0,
                         region);
          cairo_region_destroy (region);
        }
    }
}

static void
picman_canvas_item_real_draw (PicmanCanvasItem *item,
                            cairo_t        *cr)
{
  g_warn_if_reached ();
}

static cairo_region_t *
picman_canvas_item_real_get_extents (PicmanCanvasItem *item)
{
  return NULL;
}

static void
picman_canvas_item_real_stroke (PicmanCanvasItem *item,
                              cairo_t        *cr)
{
  PicmanCanvasItemPrivate *private = GET_PRIVATE (item);

  cairo_set_line_cap (cr, private->line_cap);

  picman_canvas_set_tool_bg_style (picman_canvas_item_get_canvas (item), cr);
  cairo_stroke_preserve (cr);

  picman_canvas_set_tool_fg_style (picman_canvas_item_get_canvas (item), cr,
                                 private->highlight);
  cairo_stroke (cr);
}

static void
picman_canvas_item_real_fill (PicmanCanvasItem *item,
                            cairo_t        *cr)
{
  PicmanCanvasItemPrivate *private = GET_PRIVATE (item);

  picman_canvas_set_tool_bg_style (picman_canvas_item_get_canvas (item), cr);
  cairo_set_line_width (cr, 2.0);
  cairo_stroke_preserve (cr);

  picman_canvas_set_tool_fg_style (picman_canvas_item_get_canvas (item), cr,
                                 private->highlight);
  cairo_fill (cr);
}

static gboolean
picman_canvas_item_real_hit (PicmanCanvasItem *item,
                           gdouble         x,
                           gdouble         y)
{
  return FALSE;
}


/*  public functions  */

PicmanDisplayShell *
picman_canvas_item_get_shell (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CANVAS_ITEM (item), NULL);

  private = GET_PRIVATE (item);

  return private->shell;
}

PicmanImage *
picman_canvas_item_get_image (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CANVAS_ITEM (item), NULL);

  private = GET_PRIVATE (item);

  return picman_display_get_image (private->shell->display);
}

GtkWidget *
picman_canvas_item_get_canvas (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CANVAS_ITEM (item), NULL);

  private = GET_PRIVATE (item);

  return private->shell->canvas;
}

void
picman_canvas_item_draw (PicmanCanvasItem *item,
                       cairo_t        *cr)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));
  g_return_if_fail (cr != NULL);

  private = GET_PRIVATE (item);

  if (private->visible)
    {
      cairo_save (cr);
      PICMAN_CANVAS_ITEM_GET_CLASS (item)->draw (item, cr);
      cairo_restore (cr);
    }
}

cairo_region_t *
picman_canvas_item_get_extents (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CANVAS_ITEM (item), NULL);

  private = GET_PRIVATE (item);

  if (private->visible)
    return PICMAN_CANVAS_ITEM_GET_CLASS (item)->get_extents (item);

  return NULL;
}

gboolean
picman_canvas_item_hit (PicmanCanvasItem   *item,
                      gdouble           x,
                      gdouble           y)
{
  g_return_val_if_fail (PICMAN_IS_CANVAS_ITEM (item), FALSE);

  return PICMAN_CANVAS_ITEM_GET_CLASS (item)->hit (item, x, y);
}

void
picman_canvas_item_set_visible (PicmanCanvasItem *item,
                              gboolean        visible)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  if (private->visible != visible)
    {
      picman_canvas_item_begin_change (item);
      g_object_set (G_OBJECT (item),
                    "visible", visible,
                    NULL);
      picman_canvas_item_end_change (item);
    }
}

gboolean
picman_canvas_item_get_visible (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CANVAS_ITEM (item), FALSE);

  private = GET_PRIVATE (item);

  return private->visible;
}

void
picman_canvas_item_set_line_cap (PicmanCanvasItem   *item,
                               cairo_line_cap_t  line_cap)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  if (private->line_cap != line_cap)
    {
      picman_canvas_item_begin_change (item);
      g_object_set (G_OBJECT (item),
                    "line-cap", line_cap,
                    NULL);
      picman_canvas_item_end_change (item);
    }
}

void
picman_canvas_item_set_highlight (PicmanCanvasItem *item,
                                gboolean        highlight)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  if (private->highlight != highlight)
    {
      g_object_set (G_OBJECT (item),
                    "highlight", highlight,
                    NULL);
    }
}

gboolean
picman_canvas_item_get_highlight (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CANVAS_ITEM (item), FALSE);

  private = GET_PRIVATE (item);

  return private->highlight;
}

void
picman_canvas_item_begin_change (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  private->change_count++;

  if (private->change_count == 1 &&
      g_signal_has_handler_pending (item, item_signals[UPDATE], 0, FALSE))
    {
      private->change_region = picman_canvas_item_get_extents (item);
    }
}

void
picman_canvas_item_end_change (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  g_return_if_fail (private->change_count > 0);

  private->change_count--;

  if (private->change_count == 0)
    {
      if (g_signal_has_handler_pending (item, item_signals[UPDATE], 0, FALSE))
        {
          cairo_region_t *region = picman_canvas_item_get_extents (item);

          if (! region)
            {
              region = private->change_region;
            }
          else if (private->change_region)
            {
              cairo_region_union (region, private->change_region);
              cairo_region_destroy (private->change_region);
            }

          private->change_region = NULL;

          if (region)
            {
              g_signal_emit (item, item_signals[UPDATE], 0,
                             region);
              cairo_region_destroy (region);
            }
        }
      else if (private->change_region)
        {
          cairo_region_destroy (private->change_region);
          private->change_region = NULL;
        }
    }
}

void
picman_canvas_item_suspend_stroking (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  private->suspend_stroking++;
}

void
picman_canvas_item_resume_stroking (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  g_return_if_fail (private->suspend_stroking > 0);

  private->suspend_stroking--;
}

void
picman_canvas_item_suspend_filling (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  private->suspend_filling++;
}

void
picman_canvas_item_resume_filling (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  g_return_if_fail (private->suspend_filling > 0);

  private->suspend_filling--;
}

void
picman_canvas_item_transform (PicmanCanvasItem *item,
                            cairo_t        *cr)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));
  g_return_if_fail (cr != NULL);

  private = GET_PRIVATE (item);

  cairo_translate (cr, -private->shell->offset_x, -private->shell->offset_y);
  cairo_scale (cr, private->shell->scale_x, private->shell->scale_y);
}

void
picman_canvas_item_transform_xy (PicmanCanvasItem *item,
                               gdouble         x,
                               gdouble         y,
                               gint           *tx,
                               gint           *ty)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  picman_display_shell_zoom_xy (private->shell, x, y, tx, ty);
}

void
picman_canvas_item_transform_xy_f (PicmanCanvasItem *item,
                                 gdouble         x,
                                 gdouble         y,
                                 gdouble        *tx,
                                 gdouble        *ty)
{
  PicmanCanvasItemPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (item);

  picman_display_shell_zoom_xy_f (private->shell, x, y, tx, ty);
}


/*  protected functions  */

void
_picman_canvas_item_update (PicmanCanvasItem *item,
                          cairo_region_t *region)
{
  g_signal_emit (item, item_signals[UPDATE], 0,
                 region);
}

gboolean
_picman_canvas_item_needs_update (PicmanCanvasItem *item)
{
  PicmanCanvasItemPrivate *private = GET_PRIVATE (item);

  return (private->change_count == 0 &&
          g_signal_has_handler_pending (item, item_signals[UPDATE], 0, FALSE));
}

void
_picman_canvas_item_stroke (PicmanCanvasItem *item,
                          cairo_t        *cr)
{
  PicmanCanvasItemPrivate *private = GET_PRIVATE (item);

  if (private->suspend_filling > 0)
    g_warning ("_picman_canvas_item_stroke() on an item that is in a filling group");

  if (private->suspend_stroking == 0)
    {
      PICMAN_CANVAS_ITEM_GET_CLASS (item)->stroke (item, cr);
    }
  else
    {
      cairo_new_sub_path (cr);
    }
}

void
_picman_canvas_item_fill (PicmanCanvasItem   *item,
                        cairo_t          *cr)
{
  PicmanCanvasItemPrivate *private = GET_PRIVATE (item);

  if (private->suspend_stroking > 0)
    g_warning ("_picman_canvas_item_fill() on an item that is in a stroking group");

  if (private->suspend_filling == 0)
    {
      PICMAN_CANVAS_ITEM_GET_CLASS (item)->fill (item, cr);
    }
  else
    {
      cairo_new_sub_path (cr);
    }
}
