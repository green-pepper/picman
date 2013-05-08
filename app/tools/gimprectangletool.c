/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 2007 Martin Nordholts
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
#include <gdk/gdkkeysyms.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman-utils.h"
#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanmarshal.h"
#include "core/picmanpickable.h"
#include "core/picmanpickable-auto-shrink.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvasgroup.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-scroll.h"
#include "display/picmandisplayshell-transform.h"

#include "picmandrawtool.h"
#include "picmanrectangleoptions.h"
#include "picmanrectangletool.h"
#include "picmantoolcontrol.h"

#include "picman-log.h"

#include "picman-intl.h"


enum
{
  RECTANGLE_CHANGE_COMPLETE,
  LAST_SIGNAL
};

/*  speed of key movement  */
#define ARROW_VELOCITY   25

#define MAX_HANDLE_SIZE         50
#define MIN_HANDLE_SIZE         15
#define NARROW_MODE_HANDLE_SIZE 15
#define NARROW_MODE_THRESHOLD   45

typedef enum
{
  CLAMPED_NONE   = 0,
  CLAMPED_LEFT   = 1 << 0,
  CLAMPED_RIGHT  = 1 << 1,
  CLAMPED_TOP    = 1 << 2,
  CLAMPED_BOTTOM = 1 << 3
} ClampedSide;

typedef enum
{
  SIDE_TO_RESIZE_NONE,
  SIDE_TO_RESIZE_LEFT,
  SIDE_TO_RESIZE_RIGHT,
  SIDE_TO_RESIZE_TOP,
  SIDE_TO_RESIZE_BOTTOM,
  SIDE_TO_RESIZE_LEFT_AND_RIGHT_SYMMETRICALLY,
  SIDE_TO_RESIZE_TOP_AND_BOTTOM_SYMMETRICALLY,
} SideToResize;


#define FEQUAL(a,b)       (fabs ((a) - (b)) < 0.0001)
#define PIXEL_FEQUAL(a,b) (fabs ((a) - (b)) < 0.5)

#define PICMAN_RECTANGLE_TOOL_GET_PRIVATE(obj) \
  (picman_rectangle_tool_get_private (PICMAN_RECTANGLE_TOOL (obj)))


typedef struct _PicmanRectangleToolPrivate PicmanRectangleToolPrivate;

struct _PicmanRectangleToolPrivate
{
  /* The following members are "constants", that is, variables that are setup
   * during picman_rectangle_tool_button_press and then only read.
   */

  /* Whether or not the rectangle currently being rubber-banded was
   * created from scatch.
   */
  gboolean                is_new;

  /* Holds the coordinate that should be used as the "other side" when
   * fixed-center is turned off.
   */
  gdouble                 other_side_x;
  gdouble                 other_side_y;

  /* Holds the coordinate to be used as center when fixed-center is used. */
  gdouble                 center_x_on_fixed_center;
  gdouble                 center_y_on_fixed_center;

  /* True when the rectangle is being adjusted (moved or
   * rubber-banded).
   */
  gboolean                rect_adjusting;


  /* The rest of the members are internal state variables, that is, variables
   * that might change during the manipulation session of the rectangle. Make
   * sure these variables are in consistent states.
   */

  /* Coordinates of upper left and lower right rectangle corners. */
  gdouble                 x1, y1;
  gdouble                 x2, y2;

  /* Integer coordinats of upper left corner and size. We must
   * calculate this separately from the gdouble ones because sometimes
   * we don't want to affect the integer size (e.g. when moving the
   * rectangle), but that will be the case if we always calculate the
   * integer coordinates based on rounded values of the gdouble
   * coordinates even if the gdouble width remains constant.
   *
   * TODO: Change the internal double-representation of the rectangle
   * to x,y width,height instead of x1,y1 x2,y2. That way we don't
   * need to keep a separate representation of the integer version of
   * the rectangle; rounding width an height will yield consistent
   * results and not depend on position of the rectangle.
   */
  gint                    x1_int,    y1_int;
  gint                    width_int, height_int;

  /* What modification state the rectangle is in. What corner are we resizing,
   * or are we moving the rectangle? etc.
   */
  guint                   function;

  /* How to constrain the rectangle. */
  PicmanRectangleConstraint constraint;

  /* What precision the rectangle will apear to have externally (it
   * will always be double internally)
   */
  PicmanRectanglePrecision  precision;

  /* Previous coordinate applied to the rectangle. */
  gdouble                 lastx;
  gdouble                 lasty;

  /* Width and height of corner handles. */
  gint                    corner_handle_w;
  gint                    corner_handle_h;

  /* Width and height of side handles. */
  gint                    top_and_bottom_handle_w;
  gint                    left_and_right_handle_h;

  /* Whether or not the rectangle is in a 'narrow situation' i.e. it is
   * too small for reasonable sized handle to be inside. In this case
   * we put handles on the outside.
   */
  gboolean                narrow_mode;

  /* For what scale the handle sizes is calculated. We must cache this
   * so that we can differentiate between when the tool is resumed
   * because of zoom level just has changed or because the highlight
   * has just been updated.
   */
  gdouble                 scale_x_used_for_handle_size_calculations;
  gdouble                 scale_y_used_for_handle_size_calculations;

  /* For saving in case of cancelation. */
  gdouble                 saved_x1;
  gdouble                 saved_y1;
  gdouble                 saved_x2;
  gdouble                 saved_y2;

  gint                    suppress_updates;
};


static void          picman_rectangle_tool_iface_base_init      (PicmanRectangleToolInterface *iface);

static PicmanRectangleToolPrivate *
                     picman_rectangle_tool_get_private          (PicmanRectangleTool        *rect_tool);

static void          picman_rectangle_tool_start                (PicmanRectangleTool        *rect_tool,
                                                               PicmanDisplay              *display);
static void          picman_rectangle_tool_halt                 (PicmanRectangleTool        *rect_tool);

static void          picman_rectangle_tool_update_options       (PicmanRectangleTool        *rect_tool,
                                                               PicmanDisplay              *display);

static void          picman_rectangle_tool_options_notify       (PicmanRectangleOptions     *options,
                                                               GParamSpec               *pspec,
                                                               PicmanRectangleTool        *rect_tool);
static void          picman_rectangle_tool_shell_scrolled       (PicmanDisplayShell         *options,
                                                               PicmanRectangleTool        *rect_tool);

static void          picman_rectangle_tool_check_function       (PicmanRectangleTool        *rect_tool);

static void          picman_rectangle_tool_rectangle_change_complete
                                                              (PicmanRectangleTool        *rect_tool);

static void          picman_rectangle_tool_auto_shrink          (PicmanRectangleTool        *rect_tool);

static gboolean      picman_rectangle_tool_coord_outside        (PicmanRectangleTool        *rect_tool,
                                                               const PicmanCoords         *coords);

static gboolean      picman_rectangle_tool_coord_on_handle      (PicmanRectangleTool        *rect_tool,
                                                               const PicmanCoords         *coords,
                                                               PicmanHandleAnchor          anchor);

static PicmanHandleAnchor picman_rectangle_tool_get_anchor        (PicmanRectangleToolPrivate *private);

static void          picman_rectangle_tool_update_highlight     (PicmanRectangleTool        *rect_tool);

static gboolean      picman_rectangle_tool_rect_rubber_banding_func
                                                              (PicmanRectangleTool        *rect_tool);
static gboolean      picman_rectangle_tool_rect_adjusting_func  (PicmanRectangleTool        *rect_tool);

static void          picman_rectangle_tool_update_handle_sizes  (PicmanRectangleTool        *rect_tool);

static gboolean      picman_rectangle_tool_scale_has_changed    (PicmanRectangleTool        *rect_tool);

static void          picman_rectangle_tool_get_other_side       (PicmanRectangleTool        *rect_tool,
                                                               gdouble                 **other_x,
                                                               gdouble                 **other_y);
static void          picman_rectangle_tool_get_other_side_coord (PicmanRectangleTool        *rect_tool,
                                                               gdouble                  *other_side_x,
                                                               gdouble                  *other_side_y);
static void          picman_rectangle_tool_set_other_side_coord (PicmanRectangleTool        *rect_tool,
                                                               gdouble                   other_side_x,
                                                               gdouble                   other_side_y);

static void          picman_rectangle_tool_apply_coord          (PicmanRectangleTool        *rect_tool,
                                                               gdouble                   coord_x,
                                                               gdouble                   coord_y);
static void          picman_rectangle_tool_setup_snap_offsets   (PicmanRectangleTool        *rect_tool,
                                                               const PicmanCoords         *coords);

static void          picman_rectangle_tool_clamp                (PicmanRectangleTool        *rect_tool,
                                                               ClampedSide              *clamped_sides,
                                                               PicmanRectangleConstraint   constraint,
                                                               gboolean                  symmetrically);
static void          picman_rectangle_tool_clamp_width          (PicmanRectangleTool        *rect_tool,
                                                               ClampedSide              *clamped_sides,
                                                               PicmanRectangleConstraint   constraint,
                                                               gboolean                  symmetrically);
static void          picman_rectangle_tool_clamp_height         (PicmanRectangleTool        *rect_tool,
                                                               ClampedSide              *clamped_sides,
                                                               PicmanRectangleConstraint   constraint,
                                                               gboolean                  symmetrically);

static void          picman_rectangle_tool_keep_inside          (PicmanRectangleTool        *rect_tool,
                                                               PicmanRectangleConstraint   constraint);
static void          picman_rectangle_tool_keep_inside_horizontally
                                                              (PicmanRectangleTool        *rect_tool,
                                                               PicmanRectangleConstraint   constraint);
static void          picman_rectangle_tool_keep_inside_vertically
                                                              (PicmanRectangleTool        *rect_tool,
                                                               PicmanRectangleConstraint   constraint);

static void          picman_rectangle_tool_apply_fixed_width    (PicmanRectangleTool        *rect_tool,
                                                               PicmanRectangleConstraint   constraint,
                                                               gdouble                   width);
static void          picman_rectangle_tool_apply_fixed_height   (PicmanRectangleTool        *rect_tool,
                                                               PicmanRectangleConstraint   constraint,
                                                               gdouble                   height);

static void          picman_rectangle_tool_apply_aspect         (PicmanRectangleTool        *rect_tool,
                                                               gdouble                   aspect,
                                                               gint                      clamped_sides);

static void          picman_rectangle_tool_update_with_coord    (PicmanRectangleTool        *rect_tool,
                                                               gdouble                   new_x,
                                                               gdouble                   new_y);
static void          picman_rectangle_tool_apply_fixed_rule     (PicmanRectangleTool        *rect_tool);

static void          picman_rectangle_tool_get_constraints      (PicmanRectangleTool        *rect_tool,
                                                               gint                     *min_x,
                                                               gint                     *min_y,
                                                               gint                     *max_x,
                                                               gint                     *max_y,
                                                               PicmanRectangleConstraint   constraint);

static void          picman_rectangle_tool_handle_general_clamping
                                                              (PicmanRectangleTool        *rect_tool);
static void          picman_rectangle_tool_update_int_rect      (PicmanRectangleTool        *rect_tool);
static void          picman_rectangle_tool_get_public_rect      (PicmanRectangleTool        *rect_tool,
                                                               gdouble                  *pub_x1,
                                                               gdouble                  *pub_y1,
                                                               gdouble                  *pub_x2,
                                                               gdouble                  *pub_y2);
static void          picman_rectangle_tool_adjust_coord         (PicmanRectangleTool        *rect_tool,
                                                               gdouble                   coord_x_input,
                                                               gdouble                   coord_y_input,
                                                               gdouble                  *coord_x_output,
                                                               gdouble                  *coord_y_output);


static guint picman_rectangle_tool_signals[LAST_SIGNAL] = { 0 };


GType
picman_rectangle_tool_interface_get_type (void)
{
  static GType iface_type = 0;

  if (! iface_type)
    {
      const GTypeInfo iface_info =
      {
        sizeof (PicmanRectangleToolInterface),
        (GBaseInitFunc)     picman_rectangle_tool_iface_base_init,
        (GBaseFinalizeFunc) NULL,
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                           "PicmanRectangleToolInterface",
                                           &iface_info, 0);

      g_type_interface_add_prerequisite (iface_type, PICMAN_TYPE_DRAW_TOOL);
    }

  return iface_type;
}

static void
picman_rectangle_tool_iface_base_init (PicmanRectangleToolInterface *iface)
{
  static gboolean initialized = FALSE;

  if (! initialized)
    {
      picman_rectangle_tool_signals[RECTANGLE_CHANGE_COMPLETE] =
        g_signal_new ("rectangle-change-complete",
                      G_TYPE_FROM_INTERFACE (iface),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (PicmanRectangleToolInterface,
                                       rectangle_change_complete),
                      NULL, NULL,
                      picman_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

      g_object_interface_install_property (iface,
                                           g_param_spec_int ("x1",
                                                             NULL, NULL,
                                                             -PICMAN_MAX_IMAGE_SIZE,
                                                             PICMAN_MAX_IMAGE_SIZE,
                                                             0,
                                                             PICMAN_PARAM_READWRITE));

      g_object_interface_install_property (iface,
                                           g_param_spec_int ("y1",
                                                             NULL, NULL,
                                                             -PICMAN_MAX_IMAGE_SIZE,
                                                             PICMAN_MAX_IMAGE_SIZE,
                                                             0,
                                                             PICMAN_PARAM_READWRITE));

      g_object_interface_install_property (iface,
                                           g_param_spec_int ("x2",
                                                             NULL, NULL,
                                                             -PICMAN_MAX_IMAGE_SIZE,
                                                             PICMAN_MAX_IMAGE_SIZE,
                                                             0,
                                                             PICMAN_PARAM_READWRITE));

      g_object_interface_install_property (iface,
                                           g_param_spec_int ("y2",
                                                             NULL, NULL,
                                                             -PICMAN_MAX_IMAGE_SIZE,
                                                             PICMAN_MAX_IMAGE_SIZE,
                                                             0,
                                                             PICMAN_PARAM_READWRITE));

      g_object_interface_install_property (iface,
                                           g_param_spec_enum ("constraint",
                                                              NULL, NULL,
                                                              PICMAN_TYPE_RECTANGLE_CONSTRAINT,
                                                              PICMAN_RECTANGLE_CONSTRAIN_NONE,
                                                              PICMAN_PARAM_READWRITE));

      g_object_interface_install_property (iface,
                                           g_param_spec_enum ("precision",
                                                              NULL, NULL,
                                                              PICMAN_TYPE_RECTANGLE_PRECISION,
                                                              PICMAN_RECTANGLE_PRECISION_INT,
                                                              PICMAN_PARAM_READWRITE));
      g_object_interface_install_property (iface,
                                           g_param_spec_boolean ("narrow-mode",
                                                                 NULL, NULL,
                                                                 FALSE,
                                                                 PICMAN_PARAM_READWRITE));

      iface->execute                   = NULL;
      iface->cancel                    = NULL;
      iface->rectangle_change_complete = NULL;

      initialized = TRUE;
    }
}

static void
picman_rectangle_tool_private_finalize (PicmanRectangleToolPrivate *private)
{
  g_slice_free (PicmanRectangleToolPrivate, private);
}

static PicmanRectangleToolPrivate *
picman_rectangle_tool_get_private (PicmanRectangleTool *tool)
{
  static GQuark private_key = 0;

  PicmanRectangleToolPrivate *private;

  if (G_UNLIKELY (private_key == 0))
    private_key = g_quark_from_static_string ("picman-rectangle-tool-private");

  private = g_object_get_qdata (G_OBJECT (tool), private_key);

  if (! private)
    {
      private = g_slice_new0 (PicmanRectangleToolPrivate);

      g_object_set_qdata_full (G_OBJECT (tool), private_key, private,
                               (GDestroyNotify)
                               picman_rectangle_tool_private_finalize);
    }

  return private;
}

/**
 * picman_rectangle_tool_init:
 * @rect_tool:
 *
 * Initializes the PicmanRectangleTool.
 **/
void
picman_rectangle_tool_init (PicmanRectangleTool *rect_tool)
{
  /* No need to initialize anything yet. */
}

/**
 * picman_rectangle_tool_install_properties:
 * @klass: the class structure for a type deriving from #GObject
 *
 * Installs the necessary properties for a class implementing
 * #PicmanToolOptions. A #PicmanRectangleToolProp property is installed
 * for each property, using the values from the #PicmanRectangleToolProp
 * enumeration. The caller must make sure itself that the enumeration
 * values don't collide with some other property values they
 * are using (that's what %PICMAN_RECTANGLE_TOOL_PROP_LAST is good for).
 **/
void
picman_rectangle_tool_install_properties (GObjectClass *klass)
{
  g_object_class_override_property (klass,
                                    PICMAN_RECTANGLE_TOOL_PROP_X1,
                                    "x1");
  g_object_class_override_property (klass,
                                    PICMAN_RECTANGLE_TOOL_PROP_Y1,
                                    "y1");
  g_object_class_override_property (klass,
                                    PICMAN_RECTANGLE_TOOL_PROP_X2,
                                    "x2");
  g_object_class_override_property (klass,
                                    PICMAN_RECTANGLE_TOOL_PROP_Y2,
                                    "y2");
  g_object_class_override_property (klass,
                                    PICMAN_RECTANGLE_TOOL_PROP_CONSTRAINT,
                                    "constraint");
  g_object_class_override_property (klass,
                                    PICMAN_RECTANGLE_TOOL_PROP_PRECISION,
                                    "precision");
  g_object_class_override_property (klass,
                                    PICMAN_RECTANGLE_TOOL_PROP_NARROW_MODE,
                                    "narrow-mode");
}

void
picman_rectangle_tool_set_constraint (PicmanRectangleTool       *tool,
                                    PicmanRectangleConstraint  constraint)
{
  PicmanRectangleToolPrivate *private;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (tool));

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);

  private->constraint = constraint;

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  picman_rectangle_tool_clamp (tool,
                             NULL,
                             constraint,
                             FALSE);

  picman_rectangle_tool_update_highlight (tool);
  picman_rectangle_tool_update_handle_sizes (tool);

  picman_rectangle_tool_rectangle_change_complete (tool);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));

  g_object_notify (G_OBJECT (tool), "constraint");
}

PicmanRectangleConstraint
picman_rectangle_tool_get_constraint (PicmanRectangleTool *tool)
{
  PicmanRectangleToolPrivate *private;

  g_return_val_if_fail (PICMAN_IS_RECTANGLE_TOOL (tool), 0);

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);

  return private->constraint;
}

/**
 * picman_rectangle_tool_pending_size_set:
 * @width_property:  Option property to set to pending rectangle width.
 * @height_property: Option property to set to pending rectangle height.
 *
 * Sets specified rectangle tool options properties to the width and
 * height of the current pending rectangle.
 */
void
picman_rectangle_tool_pending_size_set (PicmanRectangleTool *rect_tool,
                                      GObject           *object,
                                      const gchar       *width_property,
                                      const gchar       *height_property)
{
  PicmanRectangleToolPrivate *private;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (rect_tool));
  g_return_if_fail (width_property  != NULL);
  g_return_if_fail (height_property != NULL);

  private = picman_rectangle_tool_get_private (rect_tool);

  g_object_set (object,
                width_property,  MAX (private->x2 - private->x1, 1.0),
                height_property, MAX (private->y2 - private->y1, 1.0),
                NULL);
}

/**
 * picman_rectangle_tool_constraint_size_set:
 * @width_property:  Option property to set to current constraint width.
 * @height_property: Option property to set to current constraint height.
 *
 * Sets specified rectangle tool options properties to the width and
 * height of the current contraint size.
 */
void
picman_rectangle_tool_constraint_size_set (PicmanRectangleTool *rect_tool,
                                         GObject           *object,
                                         const gchar       *width_property,
                                         const gchar       *height_property)
{
  PicmanTool    *tool;
  PicmanContext *context;
  PicmanImage   *image;
  gdouble      width;
  gdouble      height;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (rect_tool));

  tool    = PICMAN_TOOL (rect_tool);
  context = picman_get_user_context (tool->tool_info->picman);
  image   = picman_context_get_image (context);

  if (! image)
    {
      width  = 1.0;
      height = 1.0;
    }
  else
    {
      PicmanRectangleConstraint constraint;

      constraint = picman_rectangle_tool_get_constraint (rect_tool);

      switch (constraint)
        {
        case PICMAN_RECTANGLE_CONSTRAIN_DRAWABLE:
          {
            PicmanItem *item = PICMAN_ITEM (picman_image_get_active_layer (image));

            if (! item)
              {
                width  = 1.0;
                height = 1.0;
              }
            else
              {
                width  = picman_item_get_width  (item);
                height = picman_item_get_height (item);
              }
          }
          break;

        case PICMAN_RECTANGLE_CONSTRAIN_IMAGE:
        default:
          {
            width  = picman_image_get_width  (image);
            height = picman_image_get_height (image);
          }
          break;
        }
    }

  g_object_set (object,
                width_property,  width,
                height_property, height,
                NULL);
}

/**
 * picman_rectangle_tool_rectangle_is_new:
 * @rect_tool:
 *
 * Returns: %TRUE if the user is creating a new rectangle from
 * scratch, %FALSE if modifying n previously existing rectangle. This
 * function is only meaningful in _motion and _button_release.
 */
gboolean
picman_rectangle_tool_rectangle_is_new (PicmanRectangleTool *rect_tool)
{
  g_return_val_if_fail (PICMAN_IS_RECTANGLE_TOOL (rect_tool), FALSE);

  return PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool)->is_new;
}

/**
 * picman_rectangle_tool_point_in_rectangle:
 * @rect_tool:
 * @x:         X-coord of point to test (in image coordinates)
 * @y:         Y-coord of point to test (in image coordinates)
 *
 * Returns: %TRUE if the passed point was within the rectangle
 **/
gboolean
picman_rectangle_tool_point_in_rectangle (PicmanRectangleTool *rect_tool,
                                        gdouble            x,
                                        gdouble            y)
{
  gboolean inside = FALSE;

  g_return_val_if_fail (PICMAN_IS_RECTANGLE_TOOL (rect_tool), FALSE);

  if (PICMAN_TOOL (rect_tool)->display)
    {
      gdouble pub_x1, pub_y1, pub_x2, pub_y2;

      picman_rectangle_tool_get_public_rect (rect_tool,
                                           &pub_x1, &pub_y1, &pub_x2, &pub_y2);

      inside = x >= pub_x1 && x <= pub_x2 &&
               y >= pub_y1 && y <= pub_y2;
    }

  return inside;
}

/**
 * picman_rectangle_tool_frame_item:
 * @rect_tool: a #PicmanRectangleTool interface
 * @item:      a #PicmanItem attached to the image on which a
 *             rectangle is being shown.
 *
 * Convenience function to set the corners of the rectangle to
 * match the bounds of the specified item.  The rectangle interface
 * must be active (i.e., showing a rectangle), and the item must be
 * attached to the image on which the rectangle is active.
 **/
void
picman_rectangle_tool_frame_item (PicmanRectangleTool *rect_tool,
                                PicmanItem          *item)
{
  PicmanDisplay *display;
  gint         offset_x;
  gint         offset_y;
  gint         width;
  gint         height;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (rect_tool));
  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_is_attached (item));

  display = PICMAN_TOOL (rect_tool)->display;

  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (picman_display_get_image (display) ==
                    picman_item_get_image (item));

  width  = picman_item_get_width  (item);
  height = picman_item_get_height (item);

  picman_item_get_offset (item, &offset_x, &offset_y);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (rect_tool));

  picman_rectangle_tool_set_function (rect_tool,
                                    PICMAN_RECTANGLE_TOOL_CREATING);

  g_object_set (rect_tool,
                "x1", offset_x,
                "y1", offset_y,
                "x2", offset_x + width,
                "y2", offset_y + height,
                NULL);

  /* kludge to force handle sizes to update.  This call may be
   * harmful if this function is ever moved out of the text tool code.
   */
  picman_rectangle_tool_set_constraint (rect_tool,
                                      PICMAN_RECTANGLE_CONSTRAIN_NONE);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (rect_tool));
}

void
picman_rectangle_tool_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PicmanRectangleTool        *rect_tool = PICMAN_RECTANGLE_TOOL (object);
  PicmanRectangleToolPrivate *private;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  switch (property_id)
    {
    case PICMAN_RECTANGLE_TOOL_PROP_X1:
      private->x1 = g_value_get_int (value);
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_Y1:
      private->y1 = g_value_get_int (value);
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_X2:
      private->x2 = g_value_get_int (value);
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_Y2:
      private->y2 = g_value_get_int (value);
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_CONSTRAINT:
      picman_rectangle_tool_set_constraint (rect_tool, g_value_get_enum (value));
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_PRECISION:
      private->precision = g_value_get_enum (value);
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_NARROW_MODE:
      private->narrow_mode = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }

  picman_rectangle_tool_update_int_rect (rect_tool);
}

void
picman_rectangle_tool_get_property (GObject      *object,
                                  guint         property_id,
                                  GValue       *value,
                                  GParamSpec   *pspec)
{
  PicmanRectangleTool        *rect_tool = PICMAN_RECTANGLE_TOOL (object);
  PicmanRectangleToolPrivate *private;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  switch (property_id)
    {
    case PICMAN_RECTANGLE_TOOL_PROP_X1:
      g_value_set_int (value, private->x1_int);
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_Y1:
      g_value_set_int (value, private->y1_int);
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_X2:
      g_value_set_int (value, private->x1_int + private->width_int);
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_Y2:
      g_value_set_int (value, private->y1_int + private->height_int);
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_CONSTRAINT:
      g_value_set_enum (value, picman_rectangle_tool_get_constraint (rect_tool));
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_PRECISION:
      g_value_set_enum (value, private->precision);
      break;
    case PICMAN_RECTANGLE_TOOL_PROP_NARROW_MODE:
      g_value_set_boolean (value, private->narrow_mode);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

void
picman_rectangle_tool_constructor (GObject *object)
{
  PicmanRectangleTool    *rect_tool = PICMAN_RECTANGLE_TOOL (object);
  PicmanRectangleOptions *options;

  options = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (object);

  g_signal_connect_object (options, "notify",
                           G_CALLBACK (picman_rectangle_tool_options_notify),
                           rect_tool, 0);
}

void
picman_rectangle_tool_control (PicmanTool       *tool,
                             PicmanToolAction  action,
                             PicmanDisplay    *display)
{
  PicmanRectangleTool *rect_tool = PICMAN_RECTANGLE_TOOL (tool);

  PICMAN_LOG (RECTANGLE_TOOL, "action = %s",
            picman_enum_get_value_name (PICMAN_TYPE_TOOL_ACTION, action));

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
      break;

    case PICMAN_TOOL_ACTION_RESUME:
      /* When highlightning is on, the shell gets paused/unpaused which means we
       * will get here, but we only want to recalculate handle sizes when the
       * zoom has changed.
       */
      if (picman_rectangle_tool_scale_has_changed (rect_tool))
        picman_rectangle_tool_update_handle_sizes (rect_tool);

      break;

    case PICMAN_TOOL_ACTION_HALT:
      picman_rectangle_tool_halt (rect_tool);
      break;

    default:
      break;
    }
}

void
picman_rectangle_tool_button_press (PicmanTool         *tool,
                                  const PicmanCoords *coords,
                                  guint32           time,
                                  GdkModifierType   state,
                                  PicmanDisplay      *display)
{
  PicmanRectangleTool        *rect_tool;
  PicmanDrawTool             *draw_tool;
  PicmanRectangleToolPrivate *private;
  gdouble                   snapped_x, snapped_y;
  gint                      snap_x, snap_y;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (tool));

  rect_tool = PICMAN_RECTANGLE_TOOL (tool);
  draw_tool = PICMAN_DRAW_TOOL (tool);
  private   = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);

  picman_draw_tool_pause (draw_tool);

  PICMAN_LOG (RECTANGLE_TOOL, "coords->x = %f, coords->y = %f",
            coords->x, coords->y);

  if (display != tool->display)
    {
      if (picman_draw_tool_is_active (draw_tool))
        {
          PicmanDisplayShell *shell = picman_display_get_shell (draw_tool->display);

          picman_display_shell_set_highlight (shell, NULL);
          picman_draw_tool_stop (draw_tool);
        }

      picman_rectangle_tool_set_function (rect_tool,
                                        PICMAN_RECTANGLE_TOOL_CREATING);

      private->x1 = private->x2 = coords->x;
      private->y1 = private->y2 = coords->y;

      picman_rectangle_tool_start (rect_tool, display);
    }

  /* save existing shape in case of cancellation */
  private->saved_x1 = private->x1;
  private->saved_y1 = private->y1;
  private->saved_x2 = private->x2;
  private->saved_y2 = private->y2;

  picman_rectangle_tool_setup_snap_offsets (rect_tool,
                                          coords);

  picman_tool_control_get_snap_offsets (tool->control,
                                      &snap_x, &snap_y, NULL, NULL);

  snapped_x = coords->x + snap_x;
  snapped_y = coords->y + snap_y;

  private->lastx = snapped_x;
  private->lasty = snapped_y;

  if (private->function == PICMAN_RECTANGLE_TOOL_CREATING)
    {
      /* Remember that this rectangle was created from scratch. */
      private->is_new = TRUE;

      private->x1 = private->x2 = snapped_x;
      private->y1 = private->y2 = snapped_y;

      picman_rectangle_tool_update_handle_sizes (rect_tool);

      /* Created rectangles should not be started in narrow-mode */
      private->narrow_mode = FALSE;

      /* If the rectangle is being modified we want the center on
       * fixed_center to be at the center of the currently existing
       * rectangle, otherwise we want the point where the user clicked
       * to be the center on fixed_center.
       */
      private->center_x_on_fixed_center = snapped_x;
      private->center_y_on_fixed_center = snapped_y;

      /* When the user toggles modifier keys, we want to keep track of
       * what coordinates the "other side" should have. If we are
       * creating a rectangle, use the current mouse coordinates as
       * the coordinate of the "other side", otherwise use the
       * immidiate "other side" for that.
       */
      private->other_side_x = snapped_x;
      private->other_side_y = snapped_y;

    }
  else
    {
      /* This rectangle was not created from scratch. */
      private->is_new = FALSE;

      private->center_x_on_fixed_center = (private->x1 + private->x2) / 2;
      private->center_y_on_fixed_center = (private->y1 + private->y2) / 2;

      picman_rectangle_tool_get_other_side_coord (rect_tool,
                                                &private->other_side_x,
                                                &private->other_side_y);
    }

  picman_rectangle_tool_update_int_rect (rect_tool);

  /* Is the rectangle being rubber-banded? */
  private->rect_adjusting = picman_rectangle_tool_rect_adjusting_func (rect_tool);

  picman_rectangle_tool_update_highlight (rect_tool);

  picman_draw_tool_resume (draw_tool);
}

void
picman_rectangle_tool_button_release (PicmanTool              *tool,
                                    const PicmanCoords      *coords,
                                    guint32                time,
                                    GdkModifierType        state,
                                    PicmanButtonReleaseType  release_type,
                                    PicmanDisplay           *display)
{
  PicmanRectangleTool        *rect_tool;
  PicmanRectangleToolPrivate *private;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (tool));

  rect_tool = PICMAN_RECTANGLE_TOOL (tool);
  private   = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  PICMAN_LOG (RECTANGLE_TOOL, "coords->x = %f, coords->y = %f",
            coords->x, coords->y);

  if (private->function == PICMAN_RECTANGLE_TOOL_EXECUTING)
    picman_tool_pop_status (tool, display);

  switch (release_type)
    {
    case PICMAN_BUTTON_RELEASE_NORMAL:
      picman_rectangle_tool_rectangle_change_complete (rect_tool);
      break;

    case PICMAN_BUTTON_RELEASE_CANCEL:
      private->x1 = private->saved_x1;
      private->y1 = private->saved_y1;
      private->x2 = private->saved_x2;
      private->y2 = private->saved_y2;
      picman_rectangle_tool_update_int_rect (rect_tool);

      /* If the first created rectangle was canceled, halt the tool */
      if (picman_rectangle_tool_rectangle_is_new (rect_tool))
        {
          picman_rectangle_tool_halt (rect_tool);
        }

      break;

    case PICMAN_BUTTON_RELEASE_CLICK:

      /* When a dead area is clicked, don't execute. */
      if (private->function == PICMAN_RECTANGLE_TOOL_DEAD)
        break;

      if (picman_rectangle_tool_execute (rect_tool))
        picman_rectangle_tool_halt (rect_tool);
      break;

    case PICMAN_BUTTON_RELEASE_NO_MOTION:
      break;
    }

  /* We must update this. */
  private->center_x_on_fixed_center = (private->x1 + private->x2) / 2;
  private->center_y_on_fixed_center = (private->y1 + private->y2) / 2;

  picman_tool_control_set_snap_offsets (tool->control, 0, 0, 0, 0);

  /* On button release, we are not rubber-banding the rectangle any longer. */
  private->rect_adjusting = FALSE;

  picman_rectangle_tool_update_highlight (rect_tool);
  picman_rectangle_tool_update_handle_sizes (rect_tool);
  picman_rectangle_tool_update_options (rect_tool, display);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

void
picman_rectangle_tool_motion (PicmanTool         *tool,
                            const PicmanCoords *coords,
                            guint32           time,
                            GdkModifierType   state,
                            PicmanDisplay      *display)
{
  PicmanRectangleTool        *rect_tool;
  PicmanRectangleToolPrivate *private;
  PicmanRectangleOptions     *options;
  gdouble                   snapped_x;
  gdouble                   snapped_y;
  gint                      snap_x, snap_y;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (tool));

  rect_tool = PICMAN_RECTANGLE_TOOL (tool);
  private   = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);
  options   = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (tool);

  /* Motion events should be ignored when we're just waiting for the
   * button release event to execute or if the user has grabbed a dead
   * area of the rectangle.
   */
  if (private->function == PICMAN_RECTANGLE_TOOL_EXECUTING ||
      private->function == PICMAN_RECTANGLE_TOOL_DEAD)
    return;

  PICMAN_LOG (RECTANGLE_TOOL, "coords->x = %f, coords->y = %f",
            coords->x, coords->y);

  /* Handle snapping. */
  picman_tool_control_get_snap_offsets (tool->control,
                                      &snap_x, &snap_y, NULL, NULL);
  snapped_x = coords->x + snap_x;
  snapped_y = coords->y + snap_y;

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));


  /* This is the core rectangle shape updating function: */
  picman_rectangle_tool_update_with_coord (rect_tool,
                                         snapped_x,
                                         snapped_y);

  /* Update the highlight, but only if it is not being adjusted. If it
   * is not being adjusted, the highlight is not shown anyway.
   */
  if (picman_rectangle_tool_rect_adjusting_func (rect_tool))
    picman_rectangle_tool_update_highlight (rect_tool);

  if (private->function != PICMAN_RECTANGLE_TOOL_MOVING &&
      private->function != PICMAN_RECTANGLE_TOOL_EXECUTING)
    {
      gdouble pub_x1, pub_y1, pub_x2, pub_y2;
      gint    w, h;

      picman_tool_pop_status (tool, display);

      picman_rectangle_tool_get_public_rect (rect_tool,
                                           &pub_x1, &pub_y1, &pub_x2, &pub_y2);
      w = pub_x2 - pub_x1;
      h = pub_y2 - pub_y1;

      if (w > 0.0 && h > 0.0)
        {
          gchar *aspect_text;

          aspect_text = g_strdup_printf ("  (%.2f:1)", w / (gdouble) h);

          picman_tool_push_status_coords (tool, display,
                                        picman_tool_control_get_precision (tool->control),
                                        _("Rectangle: "),
                                        w, " × ", h, aspect_text);
          g_free (aspect_text);
        }
    }

  if (private->function == PICMAN_RECTANGLE_TOOL_CREATING)
    {
      PicmanRectangleFunction function = PICMAN_RECTANGLE_TOOL_CREATING;
      gdouble               dx       = snapped_x - private->lastx;
      gdouble               dy       = snapped_y - private->lasty;

      /* When the user starts to move the cursor, set the current
       * function to one of the corner-grabbed functions, depending on
       * in what direction the user starts dragging the rectangle.
       */
      if (dx < 0)
        {
          function = (dy < 0 ?
                      PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT :
                      PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT);
        }
      else if (dx > 0)
        {
          function = (dy < 0 ?
                      PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT :
                      PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT);
        }
      else if (dy < 0)
        {
          function = (dx < 0 ?
                      PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT :
                      PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT);
        }
      else if (dy > 0)
        {
          function = (dx < 0 ?
                      PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT :
                      PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT);
        }

      picman_rectangle_tool_set_function (rect_tool, function);

      if (picman_rectangle_options_fixed_rule_active (options,
                                                    PICMAN_RECTANGLE_TOOL_FIXED_SIZE))
        {
          /* For fixed size, set the function to moving immediately since the
           * rectangle can not be resized anyway.
           */

          /* We fake a coord update to get the right size. */
          picman_rectangle_tool_update_with_coord (rect_tool,
                                                 snapped_x,
                                                 snapped_y);

          picman_tool_control_set_snap_offsets (tool->control,
                                              -(private->x2 - private->x1) / 2,
                                              -(private->y2 - private->y1) / 2,
                                              private->x2 - private->x1,
                                              private->y2 - private->y1);

          picman_rectangle_tool_set_function (rect_tool,
                                            PICMAN_RECTANGLE_TOOL_MOVING);
        }
    }

  picman_rectangle_tool_update_options (rect_tool, display);

  private->lastx = snapped_x;
  private->lasty = snapped_y;

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

void
picman_rectangle_tool_active_modifier_key (PicmanTool        *tool,
                                         GdkModifierType  key,
                                         gboolean         press,
                                         GdkModifierType  state,
                                         PicmanDisplay     *display)
{
  PicmanDrawTool                *draw_tool;
  PicmanRectangleTool           *rect_tool;
  PicmanRectangleOptions        *options;
  PicmanRectangleOptionsPrivate *options_private;
  PicmanRectangleToolPrivate    *private;
  gboolean                     button1_down;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (tool));

  draw_tool       = PICMAN_DRAW_TOOL (tool);
  rect_tool       = PICMAN_RECTANGLE_TOOL (tool);
  private         = picman_rectangle_tool_get_private (rect_tool);
  options         = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (tool);
  options_private = PICMAN_RECTANGLE_OPTIONS_GET_PRIVATE (options);
  button1_down    = state & GDK_BUTTON1_MASK;

  picman_draw_tool_pause (draw_tool);

  if (key == GDK_SHIFT_MASK)
    {
      /* Here we want to handle manualy when to update the rectangle, so we
       * don't want picman_rectangle_tool_options_notify to do anything.
       */
      g_signal_handlers_block_by_func (options,
                                       picman_rectangle_tool_options_notify,
                                       rect_tool);

      g_object_set (options,
                    "fixed-rule-active", ! options_private->fixed_rule_active,
                    NULL);

      g_signal_handlers_unblock_by_func (options,
                                         picman_rectangle_tool_options_notify,
                                         rect_tool);

      /* Only change the shape if the mouse is still down (i.e. the user is
       * still editing the rectangle.
       */
      if (button1_down)
        {
          if (!options_private->fixed_rule_active)
            {
              /* Reset anchor point */
              picman_rectangle_tool_set_other_side_coord (rect_tool,
                                                        private->other_side_x,
                                                        private->other_side_y);
            }

          picman_rectangle_tool_update_with_coord (rect_tool,
                                                 private->lastx,
                                                 private->lasty);

          picman_rectangle_tool_update_highlight (rect_tool);
        }
    }

  if (key == picman_get_toggle_behavior_mask ())
    {
      g_object_set (options,
                    "fixed-center", ! options_private->fixed_center,
                    NULL);

      if (options_private->fixed_center)
        {
          picman_rectangle_tool_update_with_coord (rect_tool,
                                                 private->lastx,
                                                 private->lasty);

          picman_rectangle_tool_update_highlight (rect_tool);

          /* Only emit the rectangle-changed signal if the button is
           * not down. If it is down, the signal will and shall be
           * emitted on _button_release instead.
           */
          if (! button1_down)
            {
              picman_rectangle_tool_rectangle_change_complete (rect_tool);
            }
        }
      else if (button1_down)
        {
          /* If we are leaving fixed_center mode we want to set the
           * "other side" where it should be. Don't do anything if we
           * came here by a mouse-click though, since then the user
           * has confirmed the shape and we don't want to modify it
           * afterwards.
           */
          picman_rectangle_tool_set_other_side_coord (rect_tool,
                                                    private->other_side_x,
                                                    private->other_side_y);

          picman_rectangle_tool_update_highlight (rect_tool);
        }
    }

  picman_draw_tool_resume (draw_tool);

  picman_rectangle_tool_update_options (rect_tool, tool->display);
}

static void
swap_doubles (gdouble *i,
              gdouble *j)
{
  gdouble tmp;

  tmp = *i;
  *i = *j;
  *j = tmp;
}

/* picman_rectangle_tool_check_function() is needed to deal with
 * situations where the user drags a corner or edge across one of the
 * existing edges, thereby changing its function.  Ugh.
 */
static void
picman_rectangle_tool_check_function (PicmanRectangleTool *rect_tool)

{
  PicmanRectangleToolPrivate *private;
  PicmanRectangleFunction     function;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  function = private->function;

  if (private->x2 < private->x1)
    {
      swap_doubles (&private->x1, &private->x2);
      switch (function)
        {
          case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT;
            break;
          case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT;
            break;
          case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT;
            break;
          case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT;
            break;
          case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT;
            break;
          case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_LEFT;
            break;
          /* avoid annoying warnings about unhandled enums */
          default:
            break;
        }
    }

  if (private->y2 < private->y1)
    {
      swap_doubles (&private->y1, &private->y2);
      switch (function)
        {
          case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
           function = PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT;
            break;
          case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT;
            break;
          case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT;
            break;
          case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT;
            break;
          case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM;
            break;
          case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
            function = PICMAN_RECTANGLE_TOOL_RESIZING_TOP;
            break;
          default:
            break;
        }
    }

  picman_rectangle_tool_set_function (rect_tool, function);
}

gboolean
picman_rectangle_tool_key_press (PicmanTool    *tool,
                               GdkEventKey *kevent,
                               PicmanDisplay *display)
{
  PicmanRectangleTool        *rect_tool;
  PicmanRectangleToolPrivate *private;
  gint                      dx = 0;
  gint                      dy = 0;
  gdouble                   new_x = 0;
  gdouble                   new_y = 0;

  g_return_val_if_fail (PICMAN_IS_RECTANGLE_TOOL (tool), FALSE);

  if (display != tool->display)
    return FALSE;

  rect_tool = PICMAN_RECTANGLE_TOOL (tool);
  private   = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);

  switch (kevent->keyval)
    {
    case GDK_KEY_Up:
      dy = -1;
      break;
    case GDK_KEY_Left:
      dx = -1;
      break;
    case GDK_KEY_Right:
      dx = 1;
      break;
    case GDK_KEY_Down:
      dy = 1;
      break;

    case GDK_KEY_Return:
    case GDK_KEY_KP_Enter:
    case GDK_KEY_ISO_Enter:
      if (picman_rectangle_tool_execute (rect_tool))
        picman_rectangle_tool_halt (rect_tool);
      return TRUE;

    case GDK_KEY_Escape:
      picman_rectangle_tool_cancel (rect_tool);
      picman_rectangle_tool_halt (rect_tool);
      return TRUE;

    default:
      return FALSE;
    }

  /*  If the shift key is down, move by an accelerated increment  */
  if (kevent->state & GDK_SHIFT_MASK)
    {
      dx *= ARROW_VELOCITY;
      dy *= ARROW_VELOCITY;
    }

  picman_tool_control_set_snap_offsets (PICMAN_TOOL (rect_tool)->control,
                                      0, 0, 0, 0);

  /*  Resize the rectangle if the mouse is over a handle, otherwise move it  */
  switch (private->function)
    {
    case PICMAN_RECTANGLE_TOOL_MOVING:
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
      new_x = private->x1 + dx;
      new_y = private->y1 + dy;
      private->lastx = new_x;
      private->lasty = new_y;
      break;
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
      new_x = private->x2 + dx;
      new_y = private->y1 + dy;
      private->lastx = new_x;
      private->lasty = new_y;
      break;
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
      new_x = private->x1 + dx;
      new_y = private->y2 + dy;
      private->lastx = new_x;
      private->lasty = new_y;
      break;
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
      new_x = private->x2 + dx;
      new_y = private->y2 + dy;
      private->lastx = new_x;
      private->lasty = new_y;
      break;
    case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
      new_x = private->x1 + dx;
      private->lastx = new_x;
      break;
    case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
      new_x = private->x2 + dx;
      private->lastx = new_x;
      break;
    case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
      new_y = private->y1 + dy;
      private->lasty = new_y;
      break;
    case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
      new_y = private->y2 + dy;
      private->lasty = new_y;
      break;

    default:
      return TRUE;
    }

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  picman_rectangle_tool_update_with_coord (rect_tool,
                                         new_x,
                                         new_y);

  private->center_x_on_fixed_center = (private->x1 + private->x2) / 2;
  private->center_y_on_fixed_center = (private->y1 + private->y2) / 2;

  picman_rectangle_tool_update_highlight (rect_tool);
  picman_rectangle_tool_update_handle_sizes (rect_tool);

  picman_rectangle_tool_update_options (rect_tool, tool->display);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));

  picman_rectangle_tool_rectangle_change_complete (rect_tool);

  /*  Evil hack to suppress oper updates. We do this because we don't
   *  want the rectangle tool to change function while the rectangle
   *  is being resized or moved using the keyboard.
   */
  private->suppress_updates = 2;

  return TRUE;
}

void
picman_rectangle_tool_oper_update (PicmanTool         *tool,
                                 const PicmanCoords *coords,
                                 GdkModifierType   state,
                                 gboolean          proximity,
                                 PicmanDisplay      *display)
{
  PicmanRectangleToolPrivate *private;
  PicmanRectangleTool        *rect_tool;
  gint                      function;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (tool));

  private   = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);
  rect_tool = PICMAN_RECTANGLE_TOOL (tool);

  if (tool->display != display)
    return;

  if (private->suppress_updates)
    {
      private->suppress_updates--;
      return;
    }

  if (! proximity)
    {
      function = PICMAN_RECTANGLE_TOOL_DEAD;
    }
  else if (picman_rectangle_tool_coord_outside (rect_tool, coords))
    {
      /* The cursor is outside of the rectangle, clicking should
       * create a new rectangle.
       */
      function = PICMAN_RECTANGLE_TOOL_CREATING;
    }
  else if (picman_rectangle_tool_coord_on_handle (rect_tool,
                                                coords,
                                                PICMAN_HANDLE_ANCHOR_NORTH_WEST))
    {
      function = PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT;
    }
  else if (picman_rectangle_tool_coord_on_handle (rect_tool,
                                                coords,
                                                PICMAN_HANDLE_ANCHOR_SOUTH_EAST))
    {
      function = PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT;
    }
  else if  (picman_rectangle_tool_coord_on_handle (rect_tool,
                                                 coords,
                                                 PICMAN_HANDLE_ANCHOR_NORTH_EAST))
    {
      function = PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT;
    }
  else if (picman_rectangle_tool_coord_on_handle (rect_tool,
                                                coords,
                                                PICMAN_HANDLE_ANCHOR_SOUTH_WEST))
    {
      function = PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT;
    }
  else if (picman_rectangle_tool_coord_on_handle (rect_tool,
                                                coords,
                                                PICMAN_HANDLE_ANCHOR_WEST))
    {
      function = PICMAN_RECTANGLE_TOOL_RESIZING_LEFT;
    }
  else if (picman_rectangle_tool_coord_on_handle (rect_tool,
                                                coords,
                                                PICMAN_HANDLE_ANCHOR_EAST))
    {
      function = PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT;
    }
  else if (picman_rectangle_tool_coord_on_handle (rect_tool,
                                                coords,
                                                PICMAN_HANDLE_ANCHOR_NORTH))
    {
      function = PICMAN_RECTANGLE_TOOL_RESIZING_TOP;
    }
  else if (picman_rectangle_tool_coord_on_handle (rect_tool,
                                                coords,
                                                PICMAN_HANDLE_ANCHOR_SOUTH))
    {
      function = PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM;
    }
  else if (picman_rectangle_tool_coord_on_handle (rect_tool,
                                                coords,
                                                PICMAN_HANDLE_ANCHOR_CENTER))
    {
      function = PICMAN_RECTANGLE_TOOL_MOVING;
    }
  else
    {
      function = PICMAN_RECTANGLE_TOOL_DEAD;
    }

  picman_rectangle_tool_set_function (PICMAN_RECTANGLE_TOOL (tool), function);
}

void
picman_rectangle_tool_cursor_update (PicmanTool         *tool,
                                   const PicmanCoords *coords,
                                   GdkModifierType   state,
                                   PicmanDisplay      *display)
{
  PicmanRectangleToolPrivate *private;
  PicmanCursorType            cursor   = PICMAN_CURSOR_CROSSHAIR_SMALL;
  PicmanCursorModifier        modifier = PICMAN_CURSOR_MODIFIER_NONE;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (tool));

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);

  if (tool->display == display)
    {
      switch (private->function)
        {
        case PICMAN_RECTANGLE_TOOL_CREATING:
          cursor = PICMAN_CURSOR_CROSSHAIR_SMALL;
          break;
        case PICMAN_RECTANGLE_TOOL_MOVING:
          cursor   = PICMAN_CURSOR_MOVE;
          modifier = PICMAN_CURSOR_MODIFIER_MOVE;
          break;
        case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
          cursor = PICMAN_CURSOR_CORNER_TOP_LEFT;
          break;
        case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
          cursor = PICMAN_CURSOR_CORNER_TOP_RIGHT;
          break;
        case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
          cursor = PICMAN_CURSOR_CORNER_BOTTOM_LEFT;
          break;
        case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
          cursor = PICMAN_CURSOR_CORNER_BOTTOM_RIGHT;
          break;
        case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
          cursor = PICMAN_CURSOR_SIDE_LEFT;
          break;
        case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
          cursor = PICMAN_CURSOR_SIDE_RIGHT;
          break;
        case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
          cursor = PICMAN_CURSOR_SIDE_TOP;
          break;
        case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
          cursor = PICMAN_CURSOR_SIDE_BOTTOM;
          break;

        default:
          break;
        }
    }

  picman_tool_control_set_cursor          (tool->control, cursor);
  picman_tool_control_set_cursor_modifier (tool->control, modifier);
}

void
picman_rectangle_tool_draw (PicmanDrawTool    *draw_tool,
                          PicmanCanvasGroup *stroke_group)
{
  PicmanTool                    *tool;
  PicmanRectangleToolPrivate    *private;
  PicmanRectangleOptions        *options;
  PicmanRectangleOptionsPrivate *options_private;
  gdouble                      x1, y1, x2, y2;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (draw_tool));
  g_return_if_fail (stroke_group == NULL || PICMAN_IS_CANVAS_GROUP (stroke_group));

  tool            = PICMAN_TOOL (draw_tool);
  private         = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);
  options         = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (tool);
  options_private = PICMAN_RECTANGLE_OPTIONS_GET_PRIVATE (options);

  picman_rectangle_tool_get_public_rect (PICMAN_RECTANGLE_TOOL (draw_tool),
                                       &x1, &y1, &x2, &y2);

  if (private->function == PICMAN_RECTANGLE_TOOL_INACTIVE)
    return;

  if (! stroke_group)
    stroke_group = PICMAN_CANVAS_GROUP (picman_draw_tool_add_stroke_group (draw_tool));

  picman_draw_tool_push_group (draw_tool, stroke_group);

  picman_draw_tool_add_rectangle_guides (draw_tool,
                                       options_private->guide,
                                       x1, y1,
                                       x2 - x1,
                                       y2 - y1);

  picman_draw_tool_add_rectangle (draw_tool, FALSE,
                                x1, y1,
                                x2 - x1,
                                y2 - y1);

  picman_draw_tool_pop_group (draw_tool);

  switch (private->function)
    {
    case PICMAN_RECTANGLE_TOOL_MOVING:

      if (picman_tool_control_is_active (tool->control))
        {
          /* Mark the center because we snap to it */
          picman_draw_tool_add_handle (draw_tool,
                                     PICMAN_HANDLE_CROSS,
                                     (x1 + x2) / 2.0,
                                     (y1 + y2) / 2.0,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_HANDLE_ANCHOR_CENTER);
          break;
        }
      else
        {
          /* Fallthrough */
        }

    case PICMAN_RECTANGLE_TOOL_DEAD:
    case PICMAN_RECTANGLE_TOOL_CREATING:
    case PICMAN_RECTANGLE_TOOL_AUTO_SHRINK:
      picman_draw_tool_push_group (draw_tool, stroke_group);

      picman_draw_tool_add_corner (draw_tool, FALSE, private->narrow_mode,
                                 x1, y1,
                                 x2, y2,
                                 private->corner_handle_w,
                                 private->corner_handle_h,
                                 PICMAN_HANDLE_ANCHOR_NORTH_WEST);
      picman_draw_tool_add_corner (draw_tool, FALSE, private->narrow_mode,
                                 x1, y1,
                                 x2, y2,
                                 private->corner_handle_w,
                                 private->corner_handle_h,
                                 PICMAN_HANDLE_ANCHOR_NORTH_EAST);
      picman_draw_tool_add_corner (draw_tool, FALSE, private->narrow_mode,
                                 x1, y1,
                                 x2, y2,
                                 private->corner_handle_w,
                                 private->corner_handle_h,
                                 PICMAN_HANDLE_ANCHOR_SOUTH_WEST);
      picman_draw_tool_add_corner (draw_tool, FALSE, private->narrow_mode,
                                 x1, y1,
                                 x2, y2,
                                 private->corner_handle_w,
                                 private->corner_handle_h,
                                 PICMAN_HANDLE_ANCHOR_SOUTH_EAST);

      picman_draw_tool_pop_group (draw_tool);
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
    case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
      if (picman_tool_control_is_active (tool->control))
        picman_draw_tool_push_group (draw_tool, stroke_group);

      picman_draw_tool_add_corner (draw_tool,
                                 ! picman_tool_control_is_active (tool->control),
                                 private->narrow_mode,
                                 x1, y1,
                                 x2, y2,
                                 private->top_and_bottom_handle_w,
                                 private->corner_handle_h,
                                 picman_rectangle_tool_get_anchor (private));

      if (picman_tool_control_is_active (tool->control))
        picman_draw_tool_pop_group (draw_tool);
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
      if (picman_tool_control_is_active (tool->control))
        picman_draw_tool_push_group (draw_tool, stroke_group);

      picman_draw_tool_add_corner (draw_tool,
                                 ! picman_tool_control_is_active (tool->control),
                                 private->narrow_mode,
                                 x1, y1,
                                 x2, y2,
                                 private->corner_handle_w,
                                 private->left_and_right_handle_h,
                                 picman_rectangle_tool_get_anchor (private));

      if (picman_tool_control_is_active (tool->control))
        picman_draw_tool_pop_group (draw_tool);
      break;

    default:
      if (picman_tool_control_is_active (tool->control))
        picman_draw_tool_push_group (draw_tool, stroke_group);

      picman_draw_tool_add_corner (draw_tool,
                                 ! picman_tool_control_is_active (tool->control),
                                 private->narrow_mode,
                                 x1, y1,
                                 x2, y2,
                                 private->corner_handle_w,
                                 private->corner_handle_h,
                                 picman_rectangle_tool_get_anchor (private));

      if (picman_tool_control_is_active (tool->control))
        picman_draw_tool_pop_group (draw_tool);
      break;
    }
}

static void
picman_rectangle_tool_update_handle_sizes (PicmanRectangleTool *rect_tool)
{
  PicmanTool                 *tool;
  PicmanRectangleToolPrivate *private;
  PicmanDisplayShell         *shell;
  gint                      visible_rectangle_width;
  gint                      visible_rectangle_height;
  gint                      rectangle_width;
  gint                      rectangle_height;
  gdouble                   pub_x1, pub_y1;
  gdouble                   pub_x2, pub_y2;

  tool    = PICMAN_TOOL (rect_tool);
  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);

  if (! (tool && tool->display))
    return;

  shell   = picman_display_get_shell (tool->display);

  picman_rectangle_tool_get_public_rect (rect_tool,
                                       &pub_x1, &pub_y1, &pub_x2, &pub_y2);
  {
    /* Calculate rectangles of the selection rectangle and the display shell,
     * with origin at (0, 0) of image, and in screen coordinate scale.
     */
    gint x1 =  pub_x1 * shell->scale_x;
    gint y1 =  pub_y1 * shell->scale_y;
    gint w1 = (pub_x2 - pub_x1) * shell->scale_x;
    gint h1 = (pub_y2 - pub_y1) * shell->scale_y;

    gint x2, y2, w2, h2;

    picman_display_shell_scroll_get_scaled_viewport (shell, &x2, &y2, &w2, &h2);

    rectangle_width  = w1;
    rectangle_height = h1;

    /* Handle size calculations shall be based on the visible part of the
     * rectangle, so calculate the size for the visible rectangle by
     * intersecting with the viewport rectangle.
     */
    picman_rectangle_intersect (x1, y1,
                              w1, h1,
                              x2, y2,
                              w2, h2,
                              NULL, NULL,
                              &visible_rectangle_width,
                              &visible_rectangle_height);

    /* Determine if we are in narrow-mode or not. */
    private->narrow_mode = (visible_rectangle_width  < NARROW_MODE_THRESHOLD ||
                            visible_rectangle_height < NARROW_MODE_THRESHOLD);
  }

  if (private->narrow_mode)
    {
      /* Corner handles always have the same (on-screen) size in
       * narrow-mode.
       */
      private->corner_handle_w = NARROW_MODE_HANDLE_SIZE;
      private->corner_handle_h = NARROW_MODE_HANDLE_SIZE;

      private->top_and_bottom_handle_w = CLAMP (rectangle_width,
                                                MIN (rectangle_width - 2,
                                                     NARROW_MODE_HANDLE_SIZE),
                                                G_MAXINT);
      private->left_and_right_handle_h = CLAMP (rectangle_height,
                                                MIN (rectangle_height - 2,
                                                     NARROW_MODE_HANDLE_SIZE),
                                                G_MAXINT);
    }
  else
    {
      /* Calculate and clamp corner handle size. */

      private->corner_handle_w = visible_rectangle_width  / 4;
      private->corner_handle_h = visible_rectangle_height / 4;

      private->corner_handle_w = CLAMP (private->corner_handle_w,
                                        MIN_HANDLE_SIZE,
                                        MAX_HANDLE_SIZE);
      private->corner_handle_h = CLAMP (private->corner_handle_h,
                                        MIN_HANDLE_SIZE,
                                        MAX_HANDLE_SIZE);

      /* Calculate and clamp side handle size. */

      private->top_and_bottom_handle_w = rectangle_width  - 3 * private->corner_handle_w;
      private->left_and_right_handle_h = rectangle_height - 3 * private->corner_handle_h;

      private->top_and_bottom_handle_w = CLAMP (private->top_and_bottom_handle_w,
                                                MIN_HANDLE_SIZE,
                                                G_MAXINT);
      private->left_and_right_handle_h = CLAMP (private->left_and_right_handle_h,
                                                MIN_HANDLE_SIZE,
                                                G_MAXINT);
    }

  /* Keep track of when we need to calculate handle sizes because of a display
   * shell change.
   */
  private->scale_x_used_for_handle_size_calculations = shell->scale_x;
  private->scale_y_used_for_handle_size_calculations = shell->scale_y;
}

/**
 * picman_rectangle_tool_scale_has_changed:
 * @rect_tool: A #PicmanRectangleTool.
 *
 * Returns: %TRUE if the scale that was used to calculate handle sizes
 *          is not the same as the current shell scale.
 */
static gboolean
picman_rectangle_tool_scale_has_changed (PicmanRectangleTool *rect_tool)
{
  PicmanTool                 *tool    = PICMAN_TOOL (rect_tool);
  PicmanRectangleToolPrivate *private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);
  PicmanDisplayShell         *shell;

  if (! tool->display)
    return TRUE;

  shell = picman_display_get_shell (tool->display);

  return (shell->scale_x != private->scale_x_used_for_handle_size_calculations
          ||
          shell->scale_y != private->scale_y_used_for_handle_size_calculations);
}

static void
picman_rectangle_tool_start (PicmanRectangleTool *rect_tool,
                           PicmanDisplay       *display)
{
  PicmanTool                    *tool = PICMAN_TOOL (rect_tool);
  PicmanRectangleOptionsPrivate *options_private;
  PicmanImage                   *image;
  gdouble                      xres;
  gdouble                      yres;

  options_private =
    PICMAN_RECTANGLE_OPTIONS_GET_PRIVATE (picman_tool_get_options (tool));

  image = picman_display_get_image (display);

  tool->display = display;

  g_signal_connect_object (picman_display_get_shell (tool->display), "scrolled",
                           G_CALLBACK (picman_rectangle_tool_shell_scrolled),
                           rect_tool, 0);

  picman_rectangle_tool_update_highlight (rect_tool);
  picman_rectangle_tool_update_handle_sizes (rect_tool);

  /* initialize the statusbar display */
  picman_tool_push_status_coords (tool, tool->display,
                                picman_tool_control_get_precision (tool->control),
                                _("Rectangle: "), 0, " × ", 0, NULL);

  picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), tool->display);

  picman_image_get_resolution (image, &xres, &yres);

  if (options_private->fixed_width_entry)
    {
      GtkWidget *entry = options_private->fixed_width_entry;

      picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (entry), 0, xres, FALSE);
      picman_size_entry_set_size (PICMAN_SIZE_ENTRY (entry), 0,
                                0, picman_image_get_width (image));
    }

  if (options_private->fixed_height_entry)
    {
      GtkWidget *entry = options_private->fixed_height_entry;

      picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (entry), 0, yres, FALSE);
      picman_size_entry_set_size (PICMAN_SIZE_ENTRY (entry), 0,
                                0, picman_image_get_height (image));
    }

  if (options_private->x_entry)
    {
      GtkWidget *entry = options_private->x_entry;

      picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (entry), 0, xres, FALSE);
      picman_size_entry_set_size (PICMAN_SIZE_ENTRY (entry), 0,
                                0, picman_image_get_width (image));
    }

  if (options_private->y_entry)
    {
      GtkWidget *entry = options_private->y_entry;

      picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (entry), 0, yres, FALSE);
      picman_size_entry_set_size (PICMAN_SIZE_ENTRY (entry), 0,
                                0, picman_image_get_height (image));
    }

  if (options_private->width_entry)
    {
      GtkWidget *entry = options_private->width_entry;

      picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (entry), 0, xres, FALSE);
      picman_size_entry_set_size (PICMAN_SIZE_ENTRY (entry), 0,
                                0, picman_image_get_width (image));
    }

  if (options_private->height_entry)
    {
      GtkWidget *entry = options_private->height_entry;

      picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (entry), 0, yres, FALSE);
      picman_size_entry_set_size (PICMAN_SIZE_ENTRY (entry), 0,
                                0, picman_image_get_height (image));
    }

  if (options_private->auto_shrink_button)
    {
      g_signal_connect_swapped (options_private->auto_shrink_button, "clicked",
                                G_CALLBACK (picman_rectangle_tool_auto_shrink),
                                rect_tool);

      gtk_widget_set_sensitive (options_private->auto_shrink_button, TRUE);
    }
}

static void
picman_rectangle_tool_halt (PicmanRectangleTool *rect_tool)
{
  PicmanTool                    *tool = PICMAN_TOOL (rect_tool);
  PicmanRectangleOptionsPrivate *options_private;

  options_private =
    PICMAN_RECTANGLE_OPTIONS_GET_PRIVATE (picman_tool_get_options (tool));

  if (tool->display)
    {
      PicmanDisplayShell *shell = picman_display_get_shell (tool->display);

      picman_display_shell_set_highlight (shell, NULL);

      g_signal_handlers_disconnect_by_func (shell,
                                            picman_rectangle_tool_shell_scrolled,
                                            rect_tool);
    }

  if (picman_draw_tool_is_active (PICMAN_DRAW_TOOL (rect_tool)))
    picman_draw_tool_stop (PICMAN_DRAW_TOOL (rect_tool));

  tool->display  = NULL;
  tool->drawable = NULL;

  picman_rectangle_tool_set_function (rect_tool, PICMAN_RECTANGLE_TOOL_INACTIVE);

  if (options_private->auto_shrink_button)
    {
      gtk_widget_set_sensitive (options_private->auto_shrink_button, FALSE);

      g_signal_handlers_disconnect_by_func (options_private->auto_shrink_button,
                                            picman_rectangle_tool_auto_shrink,
                                            rect_tool);
    }
}

gboolean
picman_rectangle_tool_execute (PicmanRectangleTool *rect_tool)
{
  PicmanRectangleToolInterface *iface;
  gboolean                    retval = FALSE;

  iface = PICMAN_RECTANGLE_TOOL_GET_INTERFACE (rect_tool);

  if (iface->execute)
    {
      gdouble pub_x1, pub_y1;
      gdouble pub_x2, pub_y2;

      picman_rectangle_tool_get_public_rect (rect_tool,
                                           &pub_x1, &pub_y1, &pub_x2, &pub_y2);

      picman_draw_tool_pause (PICMAN_DRAW_TOOL (rect_tool));

      retval = iface->execute (rect_tool,
                               pub_x1,
                               pub_y1,
                               pub_x2 - pub_x1,
                               pub_y2 - pub_y1);

      picman_rectangle_tool_update_highlight (rect_tool);

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (rect_tool));
    }

  return retval;
}

void
picman_rectangle_tool_cancel (PicmanRectangleTool *rect_tool)
{
  PicmanRectangleToolInterface *iface;

  iface = PICMAN_RECTANGLE_TOOL_GET_INTERFACE (rect_tool);

  if (iface->cancel)
    iface->cancel (rect_tool);
}

static void
picman_rectangle_tool_update_options (PicmanRectangleTool *rect_tool,
                                    PicmanDisplay       *display)
{
  PicmanRectangleOptions *options;
  gdouble               x1, y1;
  gdouble               x2, y2;
  gdouble               old_x;
  gdouble               old_y;
  gdouble               old_width;
  gdouble               old_height;

  options = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (rect_tool);

  picman_rectangle_tool_get_public_rect (rect_tool, &x1, &y1, &x2, &y2);

  g_signal_handlers_block_by_func (options,
                                   picman_rectangle_tool_options_notify,
                                   rect_tool);

  g_object_get (options,
                "x",      &old_x,
                "y",      &old_y,
                "width",  &old_width,
                "height", &old_height,
                NULL);

  g_object_freeze_notify (G_OBJECT (options));

  if (! FEQUAL (old_x, x1))
    g_object_set (options, "x", x1, NULL);

  if (! FEQUAL (old_y, y1))
    g_object_set (options, "y", y1, NULL);

  if (! FEQUAL (old_width, x2 - x1))
    g_object_set (options, "width", x2 - x1, NULL);

  if (! FEQUAL (old_height, y2 - y1))
    g_object_set (options, "height", y2 - y1, NULL);

  g_object_thaw_notify (G_OBJECT (options));

  g_signal_handlers_unblock_by_func (options,
                                     picman_rectangle_tool_options_notify,
                                     rect_tool);
}

static void
picman_rectangle_tool_synthesize_motion (PicmanRectangleTool *rect_tool,
                                       gint               function,
                                       gdouble            new_x,
                                       gdouble            new_y)
{
  PicmanTool                 *tool;
  PicmanDrawTool             *draw_tool;
  PicmanRectangleToolPrivate *private;
  PicmanRectangleFunction     old_function;

  tool      = PICMAN_TOOL (rect_tool);
  draw_tool = PICMAN_DRAW_TOOL (rect_tool);
  private   = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  /* We don't want to synthesize motions if the tool control is active
   * since that means the mouse button is down and the rectangle will
   * get updated in _motion anyway. The reason we want to prevent this
   * function from executing is that is emits the
   * rectangle-changed-complete signal which we don't want in the
   * middle of a rectangle change.
   *
   * In addition to that, we don't want to synthesize a motion if
   * there is no pending rectangle because that doesn't make any
   * sense.
   */
  if (picman_tool_control_is_active (tool->control) ||
      ! tool->display)
    return;

  old_function = private->function;

  picman_draw_tool_pause (draw_tool);

  picman_rectangle_tool_set_function (rect_tool, function);

  picman_rectangle_tool_update_with_coord (rect_tool,
                                         new_x,
                                         new_y);

  /* We must update this. */
  private->center_x_on_fixed_center = (private->x1 + private->x2) / 2;
  private->center_y_on_fixed_center = (private->y1 + private->y2) / 2;

  picman_rectangle_tool_update_options (rect_tool,
                                      tool->display);

  picman_rectangle_tool_set_function (rect_tool, old_function);

  picman_rectangle_tool_update_highlight (rect_tool);
  picman_rectangle_tool_update_handle_sizes (rect_tool);

  picman_draw_tool_resume (draw_tool);

  picman_rectangle_tool_rectangle_change_complete (rect_tool);
}

static void
picman_rectangle_tool_options_notify (PicmanRectangleOptions *options,
                                    GParamSpec           *pspec,
                                    PicmanRectangleTool    *rect_tool)
{
  PicmanTool                    *tool;
  PicmanRectangleToolPrivate    *private;
  PicmanRectangleOptionsPrivate *options_private;

  tool            = PICMAN_TOOL (rect_tool);
  private         = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);
  options_private = PICMAN_RECTANGLE_OPTIONS_GET_PRIVATE (options);

  if (strcmp (pspec->name, "guide") == 0)
    {
      picman_draw_tool_pause (PICMAN_DRAW_TOOL (rect_tool));
      picman_draw_tool_resume (PICMAN_DRAW_TOOL (rect_tool));
    }
  else if (strcmp  (pspec->name, "x") == 0 &&
           !PIXEL_FEQUAL (private->x1, options_private->x))
    {
      picman_rectangle_tool_synthesize_motion (rect_tool,
                                             PICMAN_RECTANGLE_TOOL_MOVING,
                                             options_private->x,
                                             private->y1);
    }
  else if (strcmp  (pspec->name, "y") == 0 &&
           !PIXEL_FEQUAL (private->y1, options_private->y))
    {
      picman_rectangle_tool_synthesize_motion (rect_tool,
                                             PICMAN_RECTANGLE_TOOL_MOVING,
                                             private->x1,
                                             options_private->y);
    }
  else if (strcmp  (pspec->name, "width") == 0 &&
           !PIXEL_FEQUAL (private->x2 - private->x1, options_private->width))
    {
      /* Calculate x2, y2 that will create a rectangle of given width, for the
       * current options.
       */
      gdouble x2;

      if (options_private->fixed_center)
        {
          x2 = private->center_x_on_fixed_center +
               options_private->width / 2;
        }
      else
        {
          x2 = private->x1 + options_private->width;
        }

      picman_rectangle_tool_synthesize_motion (rect_tool,
                                             PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT,
                                             x2,
                                             private->y2);
    }
  else if (strcmp  (pspec->name, "height") == 0 &&
           !PIXEL_FEQUAL (private->y2 - private->y1, options_private->height))
    {
      /* Calculate x2, y2 that will create a rectangle of given height, for the
       * current options.
       */
      gdouble y2;

      if (options_private->fixed_center)
        {
          y2 = private->center_y_on_fixed_center +
               options_private->height / 2;
        }
      else
        {
          y2 = private->y1 + options_private->height;
        }

      picman_rectangle_tool_synthesize_motion (rect_tool,
                                             PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM,
                                             private->x2,
                                             y2);
    }
  else if (strcmp (pspec->name, "desired-fixed-size-width") == 0)
    {
      /* We are only interested in when width and height swaps, so
       * it's enough to only check e.g. for width.
       */

      gdouble width  = private->x2 - private->x1;
      gdouble height = private->y2 - private->y1;

      /* Depending on a bunch of conditions, we might want to
       * immedieately switch width and height of the pending
       * rectangle.
       */
      if (options_private->fixed_rule_active                          &&
          tool->display                                       != NULL &&
          tool->button_press_state                            == 0    &&
          tool->active_modifier_state                         == 0    &&
          FEQUAL (options_private->desired_fixed_size_width,  height) &&
          FEQUAL (options_private->desired_fixed_size_height, width))
        {
          gdouble x = private->x1;
          gdouble y = private->y1;

          picman_rectangle_tool_synthesize_motion (rect_tool,
                                                 PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT,
                                                 private->x2,
                                                 private->y2);

          /* For some reason these needs to be set separately... */
          g_object_set (options,
                        "x", x,
                        NULL);
          g_object_set (options,
                        "y", y,
                        NULL);
        }
    }
  else if (strcmp (pspec->name, "aspect-numerator") == 0)
    {
      /* We are only interested in when numerator and denominator
       * swaps, so it's enough to only check e.g. for numerator.
       */

      double    width             = private->x2 - private->x1;
      double    height            = private->y2 - private->y1;
      gdouble   new_inverse_ratio = options_private->aspect_denominator /
                                    options_private->aspect_numerator;
      gdouble   lower_ratio;
      gdouble   higher_ratio;

      /* The ratio of the Fixed: Aspect ratio rule and the pending
       * rectangle is very rarely exactly the same so use an
       * interval. For small rectangles the below code will
       * automatically yield a more generous accepted ratio interval
       * which is exactly what we want.
       */
      if (width > height && height > 1.0)
        {
          lower_ratio  = width / (height + 1.0);
          higher_ratio = width / (height - 1.0);
        }
      else
        {
          lower_ratio  = (width - 1.0) / height;
          higher_ratio = (width + 1.0) / height;
        }

      /* Depending on a bunch of conditions, we might want to
       * immedieately switch width and height of the pending
       * rectangle.
       */
      if (options_private->fixed_rule_active               &&
          tool->display               != NULL              &&
          tool->button_press_state    == 0                 &&
          tool->active_modifier_state == 0                 &&
          lower_ratio                 <  new_inverse_ratio &&
          higher_ratio                >  new_inverse_ratio)
        {
          gdouble new_x2 = private->x1 + private->y2 - private->y1;
          gdouble new_y2 = private->y1 + private->x2 - private->x1;

          picman_rectangle_tool_synthesize_motion (rect_tool,
                                                 PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT,
                                                 new_x2,
                                                 new_y2);
        }
    }
  else if (strcmp (pspec->name, "highlight") == 0)
    {
      picman_rectangle_tool_update_highlight (rect_tool);
    }
}

static void
picman_rectangle_tool_shell_scrolled (PicmanDisplayShell  *shell,
                                    PicmanRectangleTool *rect_tool)
{
  PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (rect_tool);

  picman_draw_tool_pause (draw_tool);

  picman_rectangle_tool_update_handle_sizes (rect_tool);

  picman_draw_tool_resume (draw_tool);
}

PicmanRectangleFunction
picman_rectangle_tool_get_function (PicmanRectangleTool *rect_tool)
{
  g_return_val_if_fail (PICMAN_IS_RECTANGLE_TOOL (rect_tool),
                        PICMAN_RECTANGLE_TOOL_INACTIVE);

  return PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool)->function;
}

void
picman_rectangle_tool_set_function (PicmanRectangleTool     *rect_tool,
                                  PicmanRectangleFunction  function)
{
  PicmanRectangleToolPrivate *private;

  g_return_if_fail (PICMAN_IS_RECTANGLE_TOOL (rect_tool));

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  /* redraw the tool when the function changes */
  /* FIXME: should also update the cursor      */
  if (private->function != function)
    {
      PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (rect_tool);

      picman_draw_tool_pause (draw_tool);

      private->function = function;

      picman_draw_tool_resume (draw_tool);
    }
}

static void
picman_rectangle_tool_rectangle_change_complete (PicmanRectangleTool *rect_tool)
{
  g_signal_emit (rect_tool,
                 picman_rectangle_tool_signals[RECTANGLE_CHANGE_COMPLETE], 0);
}

static void
picman_rectangle_tool_auto_shrink (PicmanRectangleTool *rect_tool)
{
  PicmanTool                 *tool    = PICMAN_TOOL (rect_tool);
  PicmanRectangleToolPrivate *private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);
  PicmanDisplay              *display = tool->display;
  PicmanImage                *image;
  PicmanPickable             *pickable;
  gint                      offset_x = 0;
  gint                      offset_y = 0;
  gint                      x1, y1;
  gint                      x2, y2;
  gint                      shrunk_x1;
  gint                      shrunk_y1;
  gint                      shrunk_x2;
  gint                      shrunk_y2;
  gboolean                  shrink_merged;

  if (! display)
    return;

  image = picman_display_get_image (display);

  g_object_get (picman_tool_get_options (tool),
                "shrink-merged", &shrink_merged,
                NULL);

  if (shrink_merged)
    {
      pickable = PICMAN_PICKABLE (picman_image_get_projection (image));

      x1 = private->x1;
      y1 = private->y1;
      x2 = private->x2;
      y2 = private->y2;
    }
  else
    {
      pickable = PICMAN_PICKABLE (picman_image_get_active_drawable (image));

      if (! pickable)
        return;

      picman_item_get_offset (PICMAN_ITEM (pickable), &offset_x, &offset_y);

      x1 = private->x1 - offset_x;
      y1 = private->y1 - offset_y;
      x2 = private->x2 - offset_x;
      y2 = private->y2 - offset_y;
    }

  if (picman_pickable_auto_shrink (pickable,
                                 x1, y1, x2, y2,
                                 &shrunk_x1,
                                 &shrunk_y1,
                                 &shrunk_x2,
                                 &shrunk_y2))
    {
      PicmanRectangleFunction original_function = private->function;

      picman_draw_tool_pause (PICMAN_DRAW_TOOL (rect_tool));
      private->function = PICMAN_RECTANGLE_TOOL_AUTO_SHRINK;

      private->x1 = offset_x + shrunk_x1;
      private->y1 = offset_y + shrunk_y1;
      private->x2 = offset_x + shrunk_x2;
      private->y2 = offset_y + shrunk_y2;

      picman_rectangle_tool_update_int_rect (rect_tool);

      picman_rectangle_tool_rectangle_change_complete (rect_tool);

      picman_rectangle_tool_update_handle_sizes (rect_tool);
      picman_rectangle_tool_update_highlight (rect_tool);

      private->function = original_function;
      picman_draw_tool_resume (PICMAN_DRAW_TOOL (rect_tool));
    }

  picman_rectangle_tool_update_options (rect_tool, tool->display);
}

/**
 * picman_rectangle_tool_coord_outside:
 *
 * Returns: %TRUE if the coord is outside the rectange bounds
 *          including any outside handles.
 */
static gboolean
picman_rectangle_tool_coord_outside (PicmanRectangleTool *rect_tool,
                                   const PicmanCoords  *coord)
{
  PicmanRectangleToolPrivate *private;
  PicmanDisplayShell         *shell;
  gboolean                  narrow_mode;
  gdouble                   pub_x1, pub_y1, pub_x2, pub_y2;
  gdouble                   x1_b, y1_b, x2_b, y2_b;

  private     = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);
  narrow_mode = private->narrow_mode;
  shell       = picman_display_get_shell (PICMAN_TOOL (rect_tool)->display);

  picman_rectangle_tool_get_public_rect (rect_tool,
                                       &pub_x1, &pub_y1, &pub_x2, &pub_y2);

  x1_b = pub_x1 - (narrow_mode ? private->corner_handle_w / shell->scale_x : 0);
  x2_b = pub_x2 + (narrow_mode ? private->corner_handle_w / shell->scale_x : 0);
  y1_b = pub_y1 - (narrow_mode ? private->corner_handle_h / shell->scale_y : 0);
  y2_b = pub_y2 + (narrow_mode ? private->corner_handle_h / shell->scale_y : 0);

  return (coord->x < x1_b ||
          coord->x > x2_b ||
          coord->y < y1_b ||
          coord->y > y2_b);
}

/**
 * picman_rectangle_tool_coord_on_handle:
 *
 * Returns: %TRUE if the coord is on the handle that corresponds to
 *          @anchor.
 */
static gboolean
picman_rectangle_tool_coord_on_handle (PicmanRectangleTool *rect_tool,
                                     const PicmanCoords  *coords,
                                     PicmanHandleAnchor   anchor)
{
  PicmanRectangleToolPrivate *private;
  PicmanDisplayShell         *shell;
  PicmanDrawTool             *draw_tool;
  PicmanTool                 *tool;
  gdouble                   pub_x1, pub_y1, pub_x2, pub_y2;
  gdouble                   rect_w, rect_h;
  gdouble                   handle_x          = 0;
  gdouble                   handle_y          = 0;
  gdouble                   handle_width      = 0;
  gdouble                   handle_height     = 0;
  gint                      narrow_mode_x_dir = 0;
  gint                      narrow_mode_y_dir = 0;

  tool      = PICMAN_TOOL (rect_tool);
  draw_tool = PICMAN_DRAW_TOOL (tool);
  shell     = picman_display_get_shell (tool->display);
  private   = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);

  picman_rectangle_tool_get_public_rect (rect_tool,
                                       &pub_x1, &pub_y1, &pub_x2, &pub_y2);

  rect_w = pub_x2 - pub_x1;
  rect_h = pub_y2 - pub_y1;

  switch (anchor)
    {
    case PICMAN_HANDLE_ANCHOR_NORTH_WEST:
      handle_x      = pub_x1;
      handle_y      = pub_y1;
      handle_width  = private->corner_handle_w;
      handle_height = private->corner_handle_h;

      narrow_mode_x_dir = -1;
      narrow_mode_y_dir = -1;
      break;

    case PICMAN_HANDLE_ANCHOR_SOUTH_EAST:
      handle_x      = pub_x2;
      handle_y      = pub_y2;
      handle_width  = private->corner_handle_w;
      handle_height = private->corner_handle_h;

      narrow_mode_x_dir =  1;
      narrow_mode_y_dir =  1;
      break;

    case PICMAN_HANDLE_ANCHOR_NORTH_EAST:
      handle_x      = pub_x2;
      handle_y      = pub_y1;
      handle_width  = private->corner_handle_w;
      handle_height = private->corner_handle_h;

      narrow_mode_x_dir =  1;
      narrow_mode_y_dir = -1;
      break;

    case PICMAN_HANDLE_ANCHOR_SOUTH_WEST:
      handle_x      = pub_x1;
      handle_y      = pub_y2;
      handle_width  = private->corner_handle_w;
      handle_height = private->corner_handle_h;

      narrow_mode_x_dir = -1;
      narrow_mode_y_dir =  1;
      break;

    case PICMAN_HANDLE_ANCHOR_WEST:
      handle_x      = pub_x1;
      handle_y      = pub_y1 + rect_h / 2;
      handle_width  = private->corner_handle_w;
      handle_height = private->left_and_right_handle_h;

      narrow_mode_x_dir = -1;
      narrow_mode_y_dir =  0;
      break;

    case PICMAN_HANDLE_ANCHOR_EAST:
      handle_x      = pub_x2;
      handle_y      = pub_y1 + rect_h / 2;
      handle_width  = private->corner_handle_w;
      handle_height = private->left_and_right_handle_h;

      narrow_mode_x_dir =  1;
      narrow_mode_y_dir =  0;
      break;

    case PICMAN_HANDLE_ANCHOR_NORTH:
      handle_x      = pub_x1 + rect_w / 2;
      handle_y      = pub_y1;
      handle_width  = private->top_and_bottom_handle_w;
      handle_height = private->corner_handle_h;

      narrow_mode_x_dir =  0;
      narrow_mode_y_dir = -1;
      break;

    case PICMAN_HANDLE_ANCHOR_SOUTH:
      handle_x      = pub_x1 + rect_w / 2;
      handle_y      = pub_y2;
      handle_width  = private->top_and_bottom_handle_w;
      handle_height = private->corner_handle_h;

      narrow_mode_x_dir =  0;
      narrow_mode_y_dir =  1;
      break;

    case PICMAN_HANDLE_ANCHOR_CENTER:
      handle_x      = pub_x1 + rect_w / 2;
      handle_y      = pub_y1 + rect_h / 2;

      if (private->narrow_mode)
        {
          handle_width  = rect_w * shell->scale_x;
          handle_height = rect_h * shell->scale_y;
        }
      else
        {
          handle_width  = rect_w * shell->scale_x - private->corner_handle_w * 2;
          handle_height = rect_h * shell->scale_y - private->corner_handle_h * 2;
        }

      narrow_mode_x_dir =  0;
      narrow_mode_y_dir =  0;
      break;
    }

  if (private->narrow_mode)
    {
      handle_x += narrow_mode_x_dir * handle_width  / shell->scale_x;
      handle_y += narrow_mode_y_dir * handle_height / shell->scale_y;
    }

  return picman_draw_tool_on_handle (draw_tool, shell->display,
                                   coords->x, coords->y,
                                   PICMAN_HANDLE_SQUARE,
                                   handle_x,     handle_y,
                                   handle_width, handle_height,
                                   anchor);
}

static PicmanHandleAnchor
picman_rectangle_tool_get_anchor (PicmanRectangleToolPrivate *private)
{
  switch (private->function)
    {
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
      return PICMAN_HANDLE_ANCHOR_NORTH_WEST;

    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
      return PICMAN_HANDLE_ANCHOR_NORTH_EAST;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
      return PICMAN_HANDLE_ANCHOR_SOUTH_WEST;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
      return PICMAN_HANDLE_ANCHOR_SOUTH_EAST;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
      return PICMAN_HANDLE_ANCHOR_WEST;

    case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
      return PICMAN_HANDLE_ANCHOR_EAST;

    case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
      return PICMAN_HANDLE_ANCHOR_NORTH;

    case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
      return PICMAN_HANDLE_ANCHOR_SOUTH;

    default:
      return PICMAN_HANDLE_ANCHOR_CENTER;
    }
}

static void
picman_rectangle_tool_update_highlight (PicmanRectangleTool *rect_tool)
{
  PicmanRectangleToolPrivate *private;
  PicmanTool                 *tool;
  PicmanRectangleOptions     *options;
  PicmanDisplayShell         *shell;
  gboolean                  highlight;

  tool      = PICMAN_TOOL (rect_tool);
  options   = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (tool);
  private   = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);
  highlight = FALSE;

  if (! tool->display)
    return;

  shell = picman_display_get_shell (tool->display);

  g_object_get (options, "highlight", &highlight, NULL);

  /* Don't show the highlight when the mouse is down. */
  if (! highlight || private->rect_adjusting)
    {
      picman_display_shell_set_highlight (shell, NULL);
    }
  else
    {
      GdkRectangle rect;
      gdouble      pub_x1, pub_y1;
      gdouble      pub_x2, pub_y2;

      picman_rectangle_tool_get_public_rect (rect_tool,
                                           &pub_x1, &pub_y1, &pub_x2, &pub_y2);

      rect.x      = pub_x1;
      rect.y      = pub_y1;
      rect.width  = pub_x2 - pub_x1;
      rect.height = pub_y2 - pub_y1;

      picman_display_shell_set_highlight (shell, &rect);
    }
}

static gboolean
picman_rectangle_tool_rect_rubber_banding_func (PicmanRectangleTool *rect_tool)
{
  PicmanRectangleToolPrivate *private;
  gboolean                  rect_rubber_banding_func;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  switch (private->function)
    {
      case PICMAN_RECTANGLE_TOOL_CREATING:
      case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
      case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
      case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
      case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
      case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
      case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
      case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
      case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
      case PICMAN_RECTANGLE_TOOL_AUTO_SHRINK:
        rect_rubber_banding_func = TRUE;
        break;

      case PICMAN_RECTANGLE_TOOL_MOVING:
      case PICMAN_RECTANGLE_TOOL_INACTIVE:
      case PICMAN_RECTANGLE_TOOL_DEAD:
      default:
        rect_rubber_banding_func = FALSE;
        break;
    }

  return rect_rubber_banding_func;
}

/**
 * picman_rectangle_tool_rect_adjusting_func:
 * @rect_tool:
 *
 * Returns: %TRUE if the current function is a rectangle adjusting
 *          function.
 */
static gboolean
picman_rectangle_tool_rect_adjusting_func (PicmanRectangleTool *rect_tool)
{
  PicmanRectangleToolPrivate *private;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  return (picman_rectangle_tool_rect_rubber_banding_func (rect_tool) ||
          private->function == PICMAN_RECTANGLE_TOOL_MOVING);
}

/**
 * picman_rectangle_tool_get_other_side:
 * @rect_tool: A #PicmanRectangleTool.
 * @other_x:   Pointer to double of the other-x double.
 * @other_y:   Pointer to double of the other-y double.
 *
 * Calculates pointers to member variables that hold the coordinates
 * of the opposite side (either the opposite corner or literally the
 * opposite side), based on the current function. The opposite of a
 * corner needs two coordinates, the opposite of a side only needs
 * one.
 */
static void
picman_rectangle_tool_get_other_side (PicmanRectangleTool  *rect_tool,
                                    gdouble           **other_x,
                                    gdouble           **other_y)
{
  PicmanRectangleToolPrivate *private;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  switch (private->function)
    {
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
      *other_x = &private->x1;
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
      *other_x = &private->x2;
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
    case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
    default:
      *other_x = NULL;
      break;
    }

  switch (private->function)
    {
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
      *other_y = &private->y1;
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
      *other_y = &private->y2;
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
    default:
      *other_y = NULL;
      break;
    }
}

static void
picman_rectangle_tool_get_other_side_coord (PicmanRectangleTool *rect_tool,
                                          gdouble           *other_side_x,
                                          gdouble           *other_side_y)
{
  gdouble *other_x = NULL;
  gdouble *other_y = NULL;

  picman_rectangle_tool_get_other_side (rect_tool,
                                      &other_x,
                                      &other_y);
  if (other_x)
    *other_side_x = *other_x;
  if (other_y)
    *other_side_y = *other_y;
}

static void
picman_rectangle_tool_set_other_side_coord (PicmanRectangleTool *rect_tool,
                                          gdouble            other_side_x,
                                          gdouble            other_side_y)
{
  gdouble *other_x = NULL;
  gdouble *other_y = NULL;

  picman_rectangle_tool_get_other_side (rect_tool,
                                      &other_x,
                                      &other_y);
  if (other_x)
    *other_x = other_side_x;
  if (other_y)
    *other_y = other_side_y;

  picman_rectangle_tool_check_function (rect_tool);

  picman_rectangle_tool_update_int_rect (rect_tool);
}

/**
 * picman_rectangle_tool_apply_coord:
 * @param:     A #PicmanRectangleTool.
 * @coord_x:   X of coord.
 * @coord_y:   Y of coord.
 *
 * Adjust the rectangle to the new position specified by passed
 * coordinate, taking fixed_center into account, which means it
 * expands the rectagle around the center point.
 */
static void
picman_rectangle_tool_apply_coord (PicmanRectangleTool *rect_tool,
                                 gdouble            coord_x,
                                 gdouble            coord_y)
{
  PicmanRectangleToolPrivate    *private;
  PicmanRectangleOptions        *options;
  PicmanRectangleOptionsPrivate *options_private;

  private         = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);
  options         = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (rect_tool);
  options_private = PICMAN_RECTANGLE_OPTIONS_GET_PRIVATE (options);

  if (private->function == PICMAN_RECTANGLE_TOOL_INACTIVE)
    g_warning ("function is PICMAN_RECTANGLE_TOOL_INACTIVE while mouse is moving");

  if (private->function == PICMAN_RECTANGLE_TOOL_MOVING)
    {
      /* Preserve width and height while moving the grab-point to where the
       * cursor is.
       */
      gdouble w = private->x2 - private->x1;
      gdouble h = private->y2 - private->y1;

      private->x1 = coord_x;
      private->y1 = coord_y;

      private->x2 = private->x1 + w;
      private->y2 = private->y1 + h;

      /* We are done already. */
      return;
    }

  switch (private->function)
    {
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
      private->x1 = coord_x;

      if (options_private->fixed_center)
        private->x2 = 2 * private->center_x_on_fixed_center - private->x1;

      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
      private->x2 = coord_x;

      if (options_private->fixed_center)
        private->x1 = 2 * private->center_x_on_fixed_center - private->x2;

      break;
    }

  switch (private->function)
    {
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
      private->y1 = coord_y;

      if (options_private->fixed_center)
        private->y2 = 2 * private->center_y_on_fixed_center - private->y1;

      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
      private->y2 = coord_y;

      if (options_private->fixed_center)
        private->y1 = 2 * private->center_y_on_fixed_center - private->y2;

      break;
    }
}

static void
picman_rectangle_tool_setup_snap_offsets (PicmanRectangleTool *rect_tool,
                                        const PicmanCoords  *coords)
{
  PicmanTool                 *tool;
  PicmanRectangleToolPrivate *private;
  gdouble                   pub_x1, pub_y1, pub_x2, pub_y2;
  gdouble                   pub_coord_x, pub_coord_y;

  tool    = PICMAN_TOOL (rect_tool);
  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  picman_rectangle_tool_get_public_rect (rect_tool,
                                       &pub_x1, &pub_y1, &pub_x2, &pub_y2);
  picman_rectangle_tool_adjust_coord (rect_tool,
                                    coords->x, coords->y,
                                    &pub_coord_x, &pub_coord_y);

  switch (private->function)
    {
      picman_tool_control_set_snap_offsets (tool->control, 0, 0, 0, 0);
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
      picman_tool_control_set_snap_offsets (tool->control,
                                          pub_x1 - pub_coord_x,
                                          pub_y1 - pub_coord_y,
                                          0, 0);
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
      picman_tool_control_set_snap_offsets (tool->control,
                                          pub_x2 - pub_coord_x,
                                          pub_y1 - pub_coord_y,
                                          0, 0);
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
      picman_tool_control_set_snap_offsets (tool->control,
                                          pub_x1 - pub_coord_x,
                                          pub_y2 - pub_coord_y,
                                          0, 0);
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
      picman_tool_control_set_snap_offsets (tool->control,
                                          pub_x2 - pub_coord_x,
                                          pub_y2 - pub_coord_y,
                                          0, 0);
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
      picman_tool_control_set_snap_offsets (tool->control,
                                          pub_x1 - pub_coord_x, 0,
                                          0, 0);
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
      picman_tool_control_set_snap_offsets (tool->control,
                                          pub_x2 - pub_coord_x, 0,
                                          0, 0);
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
      picman_tool_control_set_snap_offsets (tool->control,
                                          0, pub_y1 - pub_coord_y,
                                          0, 0);
      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
      picman_tool_control_set_snap_offsets (tool->control,
                                          0, pub_y2 - pub_coord_y,
                                          0, 0);
      break;

    case PICMAN_RECTANGLE_TOOL_MOVING:
      picman_tool_control_set_snap_offsets (tool->control,
                                          pub_x1 - pub_coord_x,
                                          pub_y1 - pub_coord_y,
                                          pub_x2 - pub_x1,
                                          pub_y2 - pub_y1);
      break;

    default:
      break;
    }
}

/**
 * picman_rectangle_tool_clamp_width:
 * @rect_tool:      A #PicmanRectangleTool.
 * @clamped_sides:  Where to put contrainment information.
 * @constraint:     Constraint to use.
 * @symmetrically:  Whether or not to clamp symmetrically.
 *
 * Clamps rectangle inside specified bounds, providing information of
 * where clamping was done. Can also clamp symmetrically.
 */
static void
picman_rectangle_tool_clamp (PicmanRectangleTool       *rect_tool,
                           ClampedSide             *clamped_sides,
                           PicmanRectangleConstraint  constraint,
                           gboolean                 symmetrically)
{
  picman_rectangle_tool_clamp_width (rect_tool,
                                   clamped_sides,
                                   constraint,
                                   symmetrically);

  picman_rectangle_tool_clamp_height (rect_tool,
                                    clamped_sides,
                                    constraint,
                                    symmetrically);
}

/**
 * picman_rectangle_tool_clamp_width:
 * @rect_tool:      A #PicmanRectangleTool.
 * @clamped_sides:  Where to put contrainment information.
 * @constraint:     Constraint to use.
 * @symmetrically:  Whether or not to clamp symmetrically.
 *
 * Clamps height of rectangle. Set symmetrically to true when using
 * for fixed_center:ed rectangles, since that will clamp symmetrically
 * which is just what is needed.
 *
 * When this function constrains, it puts what it constrains in
 * @constraint. This information is essential when an aspect ratio is
 * to be applied.
 */
static void
picman_rectangle_tool_clamp_width (PicmanRectangleTool       *rect_tool,
                                 ClampedSide             *clamped_sides,
                                 PicmanRectangleConstraint  constraint,
                                 gboolean                 symmetrically)
{
  PicmanRectangleToolPrivate *private;
  gint                      min_x;
  gint                      max_x;

  if (constraint == PICMAN_RECTANGLE_CONSTRAIN_NONE)
    return;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  picman_rectangle_tool_get_constraints (rect_tool,
                                       &min_x,
                                       NULL,
                                       &max_x,
                                       NULL,
                                       constraint);
  if (private->x1 < min_x)
    {
      gdouble dx = min_x - private->x1;

      private->x1 += dx;

      if (symmetrically)
        private->x2 -= dx;

      if (private->x2 < min_x)
        private->x2 = min_x;

      if (clamped_sides)
        *clamped_sides |= CLAMPED_LEFT;
    }

  if (private->x2 > max_x)
    {
      gdouble dx = max_x - private->x2;

      private->x2 += dx;

      if (symmetrically)
        private->x1 -= dx;

      if (private->x1 > max_x)
        private->x1 = max_x;

      if (clamped_sides)
        *clamped_sides |= CLAMPED_RIGHT;
    }
}

/**
 * picman_rectangle_tool_clamp_height:
 * @rect_tool:      A #PicmanRectangleTool.
 * @clamped_sides:  Where to put contrainment information.
 * @constraint:     Constraint to use.
 * @symmetrically:  Whether or not to clamp symmetrically.
 *
 * Clamps height of rectangle. Set symmetrically to true when using for
 * fixed_center:ed rectangles, since that will clamp symmetrically which is just
 * what is needed.
 *
 * When this function constrains, it puts what it constrains in
 * @constraint. This information is essential when an aspect ratio is to be
 * applied.
 */
static void
picman_rectangle_tool_clamp_height (PicmanRectangleTool       *rect_tool,
                                  ClampedSide             *clamped_sides,
                                  PicmanRectangleConstraint  constraint,
                                  gboolean                 symmetrically)
{
  PicmanRectangleToolPrivate *private;
  gint                      min_y;
  gint                      max_y;

  if (constraint == PICMAN_RECTANGLE_CONSTRAIN_NONE)
    return;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  picman_rectangle_tool_get_constraints (rect_tool,
                                       NULL,
                                       &min_y,
                                       NULL,
                                       &max_y,
                                       constraint);
  if (private->y1 < min_y)
    {
      gdouble dy = min_y - private->y1;

      private->y1 += dy;

      if (symmetrically)
        private->y2 -= dy;

      if (private->y2 < min_y)
        private->y2 = min_y;

      if (clamped_sides)
        *clamped_sides |= CLAMPED_TOP;
    }

  if (private->y2 > max_y)
    {
      gdouble dy = max_y - private->y2;

      private->y2 += dy;

      if (symmetrically)
        private->y1 -= dy;

      if (private->y1 > max_y)
        private->y1 = max_y;

      if (clamped_sides)
        *clamped_sides |= CLAMPED_BOTTOM;
    }
}

/**
 * picman_rectangle_tool_keep_inside:
 * @rect_tool: A #PicmanRectangleTool.
 *
 * If the rectangle is outside of the canvas, move it into it. If the rectangle is
 * larger than the canvas in any direction, make it fill the canvas in that direction.
 */
static void
picman_rectangle_tool_keep_inside (PicmanRectangleTool      *rect_tool,
                                 PicmanRectangleConstraint constraint)
{
  picman_rectangle_tool_keep_inside_horizontally (rect_tool,
                                                constraint);

  picman_rectangle_tool_keep_inside_vertically (rect_tool,
                                              constraint);
}

/**
 * picman_rectangle_tool_keep_inside_horizontally:
 * @rect_tool:      A #PicmanRectangleTool.
 * @constraint:     Constraint to use.
 *
 * If the rectangle is outside of the given constraint horizontally, move it
 * inside. If it is too big to fit inside, make it just as big as the width
 * limit.
 */
static void
picman_rectangle_tool_keep_inside_horizontally (PicmanRectangleTool       *rect_tool,
                                              PicmanRectangleConstraint  constraint)
{
  PicmanRectangleToolPrivate *private;
  gint                      min_x;
  gint                      max_x;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  if (constraint == PICMAN_RECTANGLE_CONSTRAIN_NONE)
    return;

  picman_rectangle_tool_get_constraints (rect_tool,
                                       &min_x,
                                       NULL,
                                       &max_x,
                                       NULL,
                                       constraint);

  if (max_x - min_x < private->x2 - private->x1)
    {
      private->x1 = min_x;
      private->x2 = max_x;
    }
  else
    {
      if (private->x1 < min_x)
        {
          gdouble dx = min_x - private->x1;

          private->x1 += dx;
          private->x2 += dx;
        }
      if (private->x2 > max_x)
        {
          gdouble dx = max_x - private->x2;

          private->x1 += dx;
          private->x2 += dx;
        }
    }
}

/**
 * picman_rectangle_tool_keep_inside_vertically:
 * @rect_tool:      A #PicmanRectangleTool.
 * @constraint:     Constraint to use.
 *
 * If the rectangle is outside of the given constraint vertically,
 * move it inside. If it is too big to fit inside, make it just as big
 * as the width limit.
 */
static void
picman_rectangle_tool_keep_inside_vertically (PicmanRectangleTool       *rect_tool,
                                            PicmanRectangleConstraint  constraint)
{
  PicmanRectangleToolPrivate *private;
  gint                      min_y;
  gint                      max_y;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  if (constraint == PICMAN_RECTANGLE_CONSTRAIN_NONE)
    return;

  picman_rectangle_tool_get_constraints (rect_tool,
                                       NULL,
                                       &min_y,
                                       NULL,
                                       &max_y,
                                       constraint);

  if (max_y - min_y < private->y2 - private->y1)
    {
      private->y1 = min_y;
      private->y2 = max_y;
    }
  else
    {
      if (private->y1 < min_y)
        {
          gdouble dy = min_y - private->y1;

          private->y1 += dy;
          private->y2 += dy;
        }
      if (private->y2 > max_y)
        {
          gdouble dy = max_y - private->y2;

          private->y1 += dy;
          private->y2 += dy;
        }
    }
}

/**
 * picman_rectangle_tool_apply_fixed_width:
 * @rect_tool:      A #PicmanRectangleTool.
 * @constraint:     Constraint to use.
 * @width:
 *
 * Makes the rectangle have a fixed_width, following the constrainment
 * rules of fixed widths as well. Please refer to the rectangle tools
 * spec.
 */
static void
picman_rectangle_tool_apply_fixed_width (PicmanRectangleTool      *rect_tool,
                                       PicmanRectangleConstraint constraint,
                                       gdouble                 width)
{
  PicmanRectangleToolPrivate *private;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  switch (private->function)
    {
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:

      /* We always want to center around fixed_center here, since we want the
       * anchor point to be directly on the opposite side.
       */
      private->x1 = private->center_x_on_fixed_center -
                    width / 2;
      private->x2 = private->x1 + width;

      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:

      /* We always want to center around fixed_center here, since we want the
       * anchor point to be directly on the opposite side.
       */
      private->x1 = private->center_x_on_fixed_center -
                    width / 2;
      private->x2 = private->x1 + width;

      break;
    }

  /* Width shall be kept even after constraints, so we move the
   * rectangle sideways rather than adjusting a side.
   */
  picman_rectangle_tool_keep_inside_horizontally (rect_tool,
                                                constraint);
}

/**
 * picman_rectangle_tool_apply_fixed_height:
 * @rect_tool:      A #PicmanRectangleTool.
 * @constraint:     Constraint to use.
 * @height:
 *
 * Makes the rectangle have a fixed_height, following the
 * constrainment rules of fixed heights as well. Please refer to the
 * rectangle tools spec.
 */
static void
picman_rectangle_tool_apply_fixed_height (PicmanRectangleTool      *rect_tool,
                                        PicmanRectangleConstraint constraint,
                                        gdouble                 height)

{
  PicmanRectangleToolPrivate *private;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  switch (private->function)
    {
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:

      /* We always want to center around fixed_center here, since we
       * want the anchor point to be directly on the opposite side.
       */
      private->y1 = private->center_y_on_fixed_center -
                    height / 2;
      private->y2 = private->y1 + height;

      break;

    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
    case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:

      /* We always want to center around fixed_center here, since we
       * want the anchor point to be directly on the opposite side.
       */
      private->y1 = private->center_y_on_fixed_center -
                    height / 2;
      private->y2 = private->y1 + height;

      break;
    }

  /* Width shall be kept even after constraints, so we move the
   * rectangle sideways rather than adjusting a side.
   */
  picman_rectangle_tool_keep_inside_vertically (rect_tool,
                                              constraint);
}

/**
 * picman_rectangle_tool_apply_aspect:
 * @rect_tool:      A #PicmanRectangleTool.
 * @aspect:         The desired aspect.
 * @clamped_sides:  Bitfield of sides that have been clamped.
 *
 * Adjust the rectangle to the desired aspect.
 *
 * Sometimes, a side must not be moved outwards, for example if a the
 * RIGHT side has been clamped previously, we must not move the RIGHT
 * side to the right, since that would violate the constraint
 * again. The clamped_sides bitfield keeps track of sides that have
 * previously been clamped.
 *
 * If fixed_center is used, the function adjusts the aspect by
 * symmetrically adjusting the left and right, or top and bottom side.
 */
static void
picman_rectangle_tool_apply_aspect (PicmanRectangleTool *rect_tool,
                                  gdouble            aspect,
                                  gint               clamped_sides)
{
  PicmanRectangleToolPrivate    *private;
  PicmanRectangleOptions        *options;
  PicmanRectangleOptionsPrivate *options_private;
  gdouble                      current_w;
  gdouble                      current_h;
  gdouble                      current_aspect;
  SideToResize                 side_to_resize = SIDE_TO_RESIZE_NONE;

  private         = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);
  options         = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (rect_tool);
  options_private = PICMAN_RECTANGLE_OPTIONS_GET_PRIVATE (options);

  current_w = private->x2 - private->x1;
  current_h = private->y2 - private->y1;

  current_aspect = current_w / (gdouble) current_h;

  /* Do we have to do anything? */
  if (current_aspect == aspect)
    return;

  if (options_private->fixed_center)
    {
      /* We may only adjust the sides symmetrically to get desired aspect. */
      if (current_aspect > aspect)
        {
          /* We prefer to use top and bottom (since that will make the
           * cursor remain on the rectangle edge), unless that is what
           * the user has grabbed.
           */
          switch (private->function)
            {
            case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
            case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
            case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
            case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
            case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
            case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
              if (!(clamped_sides & CLAMPED_TOP) &&
                  !(clamped_sides & CLAMPED_BOTTOM))
                {
                  side_to_resize = SIDE_TO_RESIZE_TOP_AND_BOTTOM_SYMMETRICALLY;
                }
              else
                {
                  side_to_resize = SIDE_TO_RESIZE_LEFT_AND_RIGHT_SYMMETRICALLY;
                }
              break;

            case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
            case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
            default:
              side_to_resize = SIDE_TO_RESIZE_LEFT_AND_RIGHT_SYMMETRICALLY;
              break;
            }
        }
      else /* (current_aspect < aspect) */
        {
          /* We prefer to use left and right (since that will make the
           * cursor remain on the rectangle edge), unless that is what
           * the user has grabbed.
           */
          switch (private->function)
            {
            case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
            case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
            case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
            case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
            case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
            case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
              if (!(clamped_sides & CLAMPED_LEFT) &&
                  !(clamped_sides & CLAMPED_RIGHT))
                {
                  side_to_resize = SIDE_TO_RESIZE_LEFT_AND_RIGHT_SYMMETRICALLY;
                }
              else
                {
                  side_to_resize = SIDE_TO_RESIZE_TOP_AND_BOTTOM_SYMMETRICALLY;
                }
              break;

            case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
            case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
            default:
              side_to_resize = SIDE_TO_RESIZE_TOP_AND_BOTTOM_SYMMETRICALLY;
              break;
            }
        }
    }
  else if (current_aspect > aspect)
    {
      /* We can safely pick LEFT or RIGHT, since using those sides
       * will make the rectangle smaller, so we don't need to check
       * for clamped_sides. We may only use TOP and BOTTOM if not
       * those sides have been clamped, since using them will make the
       * rectangle bigger.
       */
      switch (private->function)
        {
        case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
          if (!(clamped_sides & CLAMPED_TOP))
            side_to_resize = SIDE_TO_RESIZE_TOP;
          else
            side_to_resize = SIDE_TO_RESIZE_LEFT;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
          if (!(clamped_sides & CLAMPED_TOP))
            side_to_resize = SIDE_TO_RESIZE_TOP;
          else
            side_to_resize = SIDE_TO_RESIZE_RIGHT;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
          if (!(clamped_sides & CLAMPED_BOTTOM))
            side_to_resize = SIDE_TO_RESIZE_BOTTOM;
          else
            side_to_resize = SIDE_TO_RESIZE_LEFT;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
          if (!(clamped_sides & CLAMPED_BOTTOM))
            side_to_resize = SIDE_TO_RESIZE_BOTTOM;
          else
            side_to_resize = SIDE_TO_RESIZE_RIGHT;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
          if (!(clamped_sides & CLAMPED_TOP) &&
              !(clamped_sides & CLAMPED_BOTTOM))
            side_to_resize = SIDE_TO_RESIZE_TOP_AND_BOTTOM_SYMMETRICALLY;
          else
            side_to_resize = SIDE_TO_RESIZE_LEFT;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
          if (!(clamped_sides & CLAMPED_TOP) &&
              !(clamped_sides & CLAMPED_BOTTOM))
            side_to_resize = SIDE_TO_RESIZE_TOP_AND_BOTTOM_SYMMETRICALLY;
          else
            side_to_resize = SIDE_TO_RESIZE_RIGHT;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
        case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
          side_to_resize = SIDE_TO_RESIZE_LEFT_AND_RIGHT_SYMMETRICALLY;
          break;

        case PICMAN_RECTANGLE_TOOL_MOVING:
        default:
          if (!(clamped_sides & CLAMPED_BOTTOM))
            side_to_resize = SIDE_TO_RESIZE_BOTTOM;
          else if (!(clamped_sides & CLAMPED_RIGHT))
            side_to_resize = SIDE_TO_RESIZE_RIGHT;
          else if (!(clamped_sides & CLAMPED_TOP))
            side_to_resize = SIDE_TO_RESIZE_TOP;
          else if (!(clamped_sides & CLAMPED_LEFT))
            side_to_resize = SIDE_TO_RESIZE_LEFT;
          break;
        }
    }
  else /* (current_aspect < aspect) */
    {
      /* We can safely pick TOP or BOTTOM, since using those sides
       * will make the rectangle smaller, so we don't need to check
       * for clamped_sides. We may only use LEFT and RIGHT if not
       * those sides have been clamped, since using them will make the
       * rectangle bigger.
       */
      switch (private->function)
        {
        case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT:
          if (!(clamped_sides & CLAMPED_LEFT))
            side_to_resize = SIDE_TO_RESIZE_LEFT;
          else
            side_to_resize = SIDE_TO_RESIZE_TOP;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT:
          if (!(clamped_sides & CLAMPED_RIGHT))
            side_to_resize = SIDE_TO_RESIZE_RIGHT;
          else
            side_to_resize = SIDE_TO_RESIZE_TOP;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT:
          if (!(clamped_sides & CLAMPED_LEFT))
            side_to_resize = SIDE_TO_RESIZE_LEFT;
          else
            side_to_resize = SIDE_TO_RESIZE_BOTTOM;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT:
          if (!(clamped_sides & CLAMPED_RIGHT))
            side_to_resize = SIDE_TO_RESIZE_RIGHT;
          else
            side_to_resize = SIDE_TO_RESIZE_BOTTOM;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_TOP:
          if (!(clamped_sides & CLAMPED_LEFT) &&
              !(clamped_sides & CLAMPED_RIGHT))
            side_to_resize = SIDE_TO_RESIZE_LEFT_AND_RIGHT_SYMMETRICALLY;
          else
            side_to_resize = SIDE_TO_RESIZE_TOP;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM:
          if (!(clamped_sides & CLAMPED_LEFT) &&
              !(clamped_sides & CLAMPED_RIGHT))
            side_to_resize = SIDE_TO_RESIZE_LEFT_AND_RIGHT_SYMMETRICALLY;
          else
            side_to_resize = SIDE_TO_RESIZE_BOTTOM;
          break;

        case PICMAN_RECTANGLE_TOOL_RESIZING_LEFT:
        case PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT:
          side_to_resize = SIDE_TO_RESIZE_TOP_AND_BOTTOM_SYMMETRICALLY;
          break;

        case PICMAN_RECTANGLE_TOOL_MOVING:
        default:
          if (!(clamped_sides & CLAMPED_BOTTOM))
            side_to_resize = SIDE_TO_RESIZE_BOTTOM;
          else if (!(clamped_sides & CLAMPED_RIGHT))
            side_to_resize = SIDE_TO_RESIZE_RIGHT;
          else if (!(clamped_sides & CLAMPED_TOP))
            side_to_resize = SIDE_TO_RESIZE_TOP;
          else if (!(clamped_sides & CLAMPED_LEFT))
            side_to_resize = SIDE_TO_RESIZE_LEFT;
          break;
        }
    }

  /* We now know what side(s) we should resize, so now we just solve
   * the aspect equation for that side(s).
   */
  switch (side_to_resize)
    {
    case SIDE_TO_RESIZE_NONE:
      return;

    case SIDE_TO_RESIZE_LEFT:
      private->x1 = private->x2 - aspect * current_h;
      break;

    case SIDE_TO_RESIZE_RIGHT:
      private->x2 = private->x1 + aspect * current_h;
      break;

    case SIDE_TO_RESIZE_TOP:
      private->y1 = private->y2 - current_w / aspect;
      break;

    case SIDE_TO_RESIZE_BOTTOM:
      private->y2 = private->y1 + current_w / aspect;
      break;

    case SIDE_TO_RESIZE_TOP_AND_BOTTOM_SYMMETRICALLY:
      {
        gdouble correct_h = current_w / aspect;

        private->y1 = private->center_y_on_fixed_center - correct_h / 2;
        private->y2 = private->y1 + correct_h;
      }
      break;

    case SIDE_TO_RESIZE_LEFT_AND_RIGHT_SYMMETRICALLY:
      {
        gdouble correct_w = current_h * aspect;

        private->x1 = private->center_x_on_fixed_center - correct_w / 2;
        private->x2 = private->x1 + correct_w;
      }
      break;
    }
}

/**
 * picman_rectangle_tool_update_with_coord:
 * @rect_tool:      A #PicmanRectangleTool.
 * @new_x:          New X-coordinate in the context of the current function.
 * @new_y:          New Y-coordinate in the context of the current function.
 *
 * The core rectangle adjustment function. It updates the rectangle
 * for the passed cursor coordinate, taking current function and tool
 * options into account.  It also updates the current
 * private->function if necessary.
 */
static void
picman_rectangle_tool_update_with_coord (PicmanRectangleTool *rect_tool,
                                       gdouble            new_x,
                                       gdouble            new_y)
{
  PicmanRectangleToolPrivate *private;

  private = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  /* Move the corner or edge the user currently has grabbed. */
  picman_rectangle_tool_apply_coord (rect_tool,
                                   new_x,
                                   new_y);

  /* Update private->function. The function changes if the user
   * "flips" the rectangle.
   */
  picman_rectangle_tool_check_function (rect_tool);

  /* Clamp the rectangle if necessary */
  picman_rectangle_tool_handle_general_clamping (rect_tool);

  /* If the rectangle is being moved, do not run through any further
   * rectangle adjusting functions since it's shape should not change
   * then.
   */
  if (private->function != PICMAN_RECTANGLE_TOOL_MOVING)
    {
      picman_rectangle_tool_apply_fixed_rule (rect_tool);
    }

  picman_rectangle_tool_update_int_rect (rect_tool);
}

static void
picman_rectangle_tool_apply_fixed_rule (PicmanRectangleTool *rect_tool)
{
  PicmanTool                    *tool;
  PicmanRectangleToolPrivate    *private;
  PicmanRectangleOptions        *options;
  PicmanRectangleOptionsPrivate *options_private;
  PicmanRectangleConstraint      constraint_to_use;
  PicmanImage                   *image;

  tool            = PICMAN_TOOL (rect_tool);
  private         = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (tool);
  options         = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (tool);
  options_private = PICMAN_RECTANGLE_OPTIONS_GET_PRIVATE (options);
  image           = picman_display_get_image (tool->display);

  /* Calculate what constraint to use when needed. */
  constraint_to_use = picman_rectangle_tool_get_constraint (rect_tool);

  if (picman_rectangle_options_fixed_rule_active (options,
                                                PICMAN_RECTANGLE_TOOL_FIXED_ASPECT))
    {
      gdouble aspect;

      aspect = CLAMP (options_private->aspect_numerator /
                      options_private->aspect_denominator,
                      1.0 / picman_image_get_height (image),
                      picman_image_get_width (image));

      if (constraint_to_use == PICMAN_RECTANGLE_CONSTRAIN_NONE)
        {
          picman_rectangle_tool_apply_aspect (rect_tool,
                                            aspect,
                                            CLAMPED_NONE);
        }
      else
        {
          if (private->function != PICMAN_RECTANGLE_TOOL_MOVING)
            {
              ClampedSide clamped_sides = CLAMPED_NONE;

              picman_rectangle_tool_apply_aspect (rect_tool,
                                                aspect,
                                                clamped_sides);

              /* After we have applied aspect, we might have taken the
               * rectangle outside of constraint, so clamp and apply
               * aspect again. We will get the right result this time,
               * since 'clamped_sides' will be setup correctly now.
               */
              picman_rectangle_tool_clamp (rect_tool,
                                         &clamped_sides,
                                         constraint_to_use,
                                         options_private->fixed_center);

              picman_rectangle_tool_apply_aspect (rect_tool,
                                                aspect,
                                                clamped_sides);
            }
          else
            {
              picman_rectangle_tool_apply_aspect (rect_tool,
                                                aspect,
                                                CLAMPED_NONE);

              picman_rectangle_tool_keep_inside (rect_tool,
                                               constraint_to_use);
            }
        }
    }
  else if (picman_rectangle_options_fixed_rule_active (options,
                                                     PICMAN_RECTANGLE_TOOL_FIXED_SIZE))
    {
      picman_rectangle_tool_apply_fixed_width (rect_tool,
                                             constraint_to_use,
                                             options_private->desired_fixed_size_width);
      picman_rectangle_tool_apply_fixed_height (rect_tool,
                                              constraint_to_use,
                                              options_private->desired_fixed_size_height);
    }
  else if (picman_rectangle_options_fixed_rule_active (options,
                                                     PICMAN_RECTANGLE_TOOL_FIXED_WIDTH))
    {
      picman_rectangle_tool_apply_fixed_width (rect_tool,
                                             constraint_to_use,
                                             options_private->desired_fixed_width);
    }
  else if (picman_rectangle_options_fixed_rule_active (options,
                                                     PICMAN_RECTANGLE_TOOL_FIXED_HEIGHT))
    {
      picman_rectangle_tool_apply_fixed_height (rect_tool,
                                              constraint_to_use,
                                              options_private->desired_fixed_height);
    }
}

/**
 * picman_rectangle_tool_get_constraints:
 * @rect_tool:      A #PicmanRectangleTool.
 * @min_x:
 * @min_y:
 * @max_x:
 * @max_y:          Pointers of where to put constraints. NULL allowed.
 * @constraint:     Whether to return image or layer constraints.
 *
 * Calculates constraint coordinates for image or layer.
 */
static void
picman_rectangle_tool_get_constraints (PicmanRectangleTool       *rect_tool,
                                     gint                    *min_x,
                                     gint                    *min_y,
                                     gint                    *max_x,
                                     gint                    *max_y,
                                     PicmanRectangleConstraint  constraint)
{
  PicmanTool  *tool = PICMAN_TOOL (rect_tool);
  PicmanImage *image;
  gint       min_x_dummy;
  gint       min_y_dummy;
  gint       max_x_dummy;
  gint       max_y_dummy;

  if (! min_x) min_x = &min_x_dummy;
  if (! min_y) min_y = &min_y_dummy;
  if (! max_x) max_x = &max_x_dummy;
  if (! max_y) max_y = &max_y_dummy;

  *min_x = 0;
  *min_y = 0;
  *max_x = 0;
  *max_y = 0;

  if (! tool->display)
    return;

  image = picman_display_get_image (tool->display);

  switch (constraint)
    {
    case PICMAN_RECTANGLE_CONSTRAIN_IMAGE:
      *min_x = 0;
      *min_y = 0;
      *max_x = picman_image_get_width  (image);
      *max_y = picman_image_get_height (image);
      break;

    case PICMAN_RECTANGLE_CONSTRAIN_DRAWABLE:
      {
        PicmanItem *item = PICMAN_ITEM (tool->drawable);

        picman_item_get_offset (item, min_x, min_y);
        *max_x = *min_x + picman_item_get_width  (item);
        *max_y = *min_y + picman_item_get_height (item);
      }
      break;

    default:
      g_warning ("Invalid rectangle constraint.\n");
      return;
    }
}

/**
 * picman_rectangle_tool_handle_general_clamping:
 * @rect_tool: A #PicmanRectangleTool.
 *
 * Make sure that contraints are applied to the rectangle, either by
 * manually doing it, or by looking at the rectangle tool options and
 * concluding it will be done later.
 */
static void
picman_rectangle_tool_handle_general_clamping (PicmanRectangleTool *rect_tool)
{
  PicmanRectangleToolPrivate    *private;
  PicmanRectangleOptions        *options;
  PicmanRectangleOptionsPrivate *options_private;
  PicmanRectangleConstraint      constraint;

  private         = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);
  options         = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (rect_tool);
  options_private = PICMAN_RECTANGLE_OPTIONS_GET_PRIVATE (options);

  constraint = picman_rectangle_tool_get_constraint (rect_tool);

  /* fixed_aspect takes care of clamping by it self, so just return in
   * case that is in use. Also return if no constraints should be
   * enforced.
   */
  if (constraint == PICMAN_RECTANGLE_CONSTRAIN_NONE)
    return;

  if (private->function != PICMAN_RECTANGLE_TOOL_MOVING)
    {
      picman_rectangle_tool_clamp (rect_tool,
                                 NULL,
                                 constraint,
                                 options_private->fixed_center);
    }
  else
    {
      picman_rectangle_tool_keep_inside (rect_tool,
                                       constraint);
    }
}

/**
 * picman_rectangle_tool_update_int_rect:
 * @rect_tool:
 *
 * Update integer representation of rectangle.
 **/
static void
picman_rectangle_tool_update_int_rect (PicmanRectangleTool *rect_tool)
{
  PicmanRectangleToolPrivate *priv = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  priv->x1_int = SIGNED_ROUND (priv->x1);
  priv->y1_int = SIGNED_ROUND (priv->y1);

  if (picman_rectangle_tool_rect_rubber_banding_func (rect_tool))
    {
      priv->width_int  = (gint) SIGNED_ROUND (priv->x2) - priv->x1_int;
      priv->height_int = (gint) SIGNED_ROUND (priv->y2) - priv->y1_int;
    }
}

/**
 * picman_rectangle_tool_get_public_rect:
 * @rect_tool:
 * @pub_x1:
 * @pub_y1:
 * @pub_x2:
 * @pub_y2:
 *
 * This function returns the rectangle as it appears to be publicly
 * (based on integer or double precision-mode).
 **/
static void
picman_rectangle_tool_get_public_rect (PicmanRectangleTool *rect_tool,
                                     gdouble           *pub_x1,
                                     gdouble           *pub_y1,
                                     gdouble           *pub_x2,
                                     gdouble           *pub_y2)
{
  PicmanRectangleToolPrivate *priv;

  priv = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  switch (priv->precision)
    {
      case PICMAN_RECTANGLE_PRECISION_INT:
        *pub_x1 = priv->x1_int;
        *pub_y1 = priv->y1_int;
        *pub_x2 = priv->x1_int + priv->width_int;
        *pub_y2 = priv->y1_int + priv->height_int;

        break;

      case PICMAN_RECTANGLE_PRECISION_DOUBLE:
      default:
        *pub_x1 = priv->x1;
        *pub_y1 = priv->y1;
        *pub_x2 = priv->x2;
        *pub_y2 = priv->y2;
        break;
    }
}

/**
 * picman_rectangle_tool_adjust_coord:
 * @rect_tool:
 * @ccoord_x_input:
 * @ccoord_x_input:
 * @ccoord_x_output:
 * @ccoord_x_output:
 *
 * Transforms a coordinate to better fit the public behaviour of the
 * rectangle.
 */
static void
picman_rectangle_tool_adjust_coord (PicmanRectangleTool *rect_tool,
                                  gdouble            coord_x_input,
                                  gdouble            coord_y_input,
                                  gdouble           *coord_x_output,
                                  gdouble           *coord_y_output)
{
  PicmanRectangleToolPrivate *priv;

  priv = PICMAN_RECTANGLE_TOOL_GET_PRIVATE (rect_tool);

  switch (priv->precision)
    {
      case PICMAN_RECTANGLE_PRECISION_INT:
        *coord_x_output = RINT (coord_x_input);
        *coord_y_output = RINT (coord_y_input);
        break;

      case PICMAN_RECTANGLE_PRECISION_DOUBLE:
      default:
        *coord_x_output = coord_x_input;
        *coord_y_output = coord_y_input;
        break;
    }
}
