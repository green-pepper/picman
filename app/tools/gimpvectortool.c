/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Vector tool
 * Copyright (C) 2003 Simon Budig  <simon@picman.org>
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanimage-undo-push.h"
#include "core/picmanlist.h"
#include "core/picmantoolinfo.h"
#include "core/picmanundostack.h"

#include "paint/picmanpaintoptions.h" /* PICMAN_PAINT_OPTIONS_CONTEXT_MASK */

#include "vectors/picmananchor.h"
#include "vectors/picmanvectors.h"
#include "vectors/picmanbezierstroke.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvasitem.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-scale.h"

#include "picmantoolcontrol.h"
#include "picmanvectoroptions.h"
#include "picmanvectortool.h"

#include "dialogs/stroke-dialog.h"

#include "picman-intl.h"


#define TOGGLE_MASK  GDK_SHIFT_MASK
#define MOVE_MASK    GDK_MOD1_MASK
#define INSDEL_MASK  picman_get_toggle_behavior_mask ()


/*  local function prototypes  */

static void     picman_vector_tool_control         (PicmanTool              *tool,
                                                  PicmanToolAction         action,
                                                  PicmanDisplay           *display);
static void     picman_vector_tool_button_press    (PicmanTool              *tool,
                                                  const PicmanCoords      *coords,
                                                  guint32                time,
                                                  GdkModifierType        state,
                                                  PicmanButtonPressType    press_type,
                                                  PicmanDisplay           *display);
static void     picman_vector_tool_button_release  (PicmanTool              *tool,
                                                  const PicmanCoords      *coords,
                                                  guint32                time,
                                                  GdkModifierType        state,
                                                  PicmanButtonReleaseType  release_type,
                                                  PicmanDisplay           *display);
static void     picman_vector_tool_motion          (PicmanTool              *tool,
                                                  const PicmanCoords      *coords,
                                                  guint32                time,
                                                  GdkModifierType        state,
                                                  PicmanDisplay           *display);
static gboolean picman_vector_tool_key_press       (PicmanTool              *tool,
                                                  GdkEventKey           *kevent,
                                                  PicmanDisplay           *display);
static void     picman_vector_tool_modifier_key    (PicmanTool              *tool,
                                                  GdkModifierType        key,
                                                  gboolean               press,
                                                  GdkModifierType        state,
                                                  PicmanDisplay           *display);
static void     picman_vector_tool_oper_update     (PicmanTool              *tool,
                                                  const PicmanCoords      *coords,
                                                  GdkModifierType        state,
                                                  gboolean               proximity,
                                                  PicmanDisplay           *display);
static void     picman_vector_tool_status_update   (PicmanTool              *tool,
                                                  PicmanDisplay           *display,
                                                  GdkModifierType        state,
                                                  gboolean               proximity);
static void     picman_vector_tool_cursor_update   (PicmanTool              *tool,
                                                  const PicmanCoords      *coords,
                                                  GdkModifierType        state,
                                                  PicmanDisplay           *display);

static void     picman_vector_tool_draw            (PicmanDrawTool          *draw_tool);

static void     picman_vector_tool_vectors_changed (PicmanImage             *image,
                                                  PicmanVectorTool        *vector_tool);
static void     picman_vector_tool_vectors_removed (PicmanVectors           *vectors,
                                                  PicmanVectorTool        *vector_tool);
static void     picman_vector_tool_vectors_visible (PicmanVectors           *vectors,
                                                  PicmanVectorTool        *vector_tool);
static void     picman_vector_tool_vectors_freeze  (PicmanVectors           *vectors,
                                                  PicmanVectorTool        *vector_tool);
static void     picman_vector_tool_vectors_thaw    (PicmanVectors           *vectors,
                                                  PicmanVectorTool        *vector_tool);

static void     picman_vector_tool_move_selected_anchors
                                                 (PicmanVectorTool        *vector_tool,
                                                  gdouble                x,
                                                  gdouble                y);
static void     picman_vector_tool_delete_selected_anchors
                                                 (PicmanVectorTool        *vector_tool);
static void     picman_vector_tool_verify_state    (PicmanVectorTool        *vector_tool);
static void     picman_vector_tool_undo_push       (PicmanVectorTool        *vector_tool,
                                                  const gchar           *desc);

static void     picman_vector_tool_to_selection    (PicmanVectorTool        *vector_tool);
static void     picman_vector_tool_to_selection_extended
                                                 (PicmanVectorTool        *vector_tool,
                                                  gint                   state);
static void     picman_vector_tool_stroke_vectors  (PicmanVectorTool        *vector_tool,
                                                  GtkWidget             *button);


G_DEFINE_TYPE (PicmanVectorTool, picman_vector_tool, PICMAN_TYPE_DRAW_TOOL)

#define parent_class picman_vector_tool_parent_class


void
picman_vector_tool_register (PicmanToolRegisterCallback callback,
                           gpointer                 data)
{
  (* callback) (PICMAN_TYPE_VECTOR_TOOL,
                PICMAN_TYPE_VECTOR_OPTIONS,
                picman_vector_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK |
                PICMAN_CONTEXT_PATTERN_MASK |
                PICMAN_CONTEXT_GRADIENT_MASK, /* for stroking */
                "picman-vector-tool",
                _("Paths"),
                _("Paths Tool: Create and edit paths"),
                N_("Pat_hs"), "b",
                NULL, PICMAN_HELP_TOOL_PATH,
                PICMAN_STOCK_TOOL_PATH,
                data);
}

static void
picman_vector_tool_class_init (PicmanVectorToolClass *klass)
{
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  tool_class->control        = picman_vector_tool_control;
  tool_class->button_press   = picman_vector_tool_button_press;
  tool_class->button_release = picman_vector_tool_button_release;
  tool_class->motion         = picman_vector_tool_motion;
  tool_class->key_press      = picman_vector_tool_key_press;
  tool_class->modifier_key   = picman_vector_tool_modifier_key;
  tool_class->oper_update    = picman_vector_tool_oper_update;
  tool_class->cursor_update  = picman_vector_tool_cursor_update;

  draw_tool_class->draw      = picman_vector_tool_draw;
}

static void
picman_vector_tool_init (PicmanVectorTool *vector_tool)
{
  PicmanTool *tool = PICMAN_TOOL (vector_tool);

  picman_tool_control_set_handle_empty_image (tool->control, TRUE);
  picman_tool_control_set_motion_mode        (tool->control,
                                            PICMAN_MOTION_MODE_COMPRESS);
  picman_tool_control_set_precision          (tool->control,
                                            PICMAN_CURSOR_PRECISION_SUBPIXEL);
  picman_tool_control_set_tool_cursor        (tool->control,
                                            PICMAN_TOOL_CURSOR_PATHS);

  vector_tool->function       = VECTORS_CREATE_VECTOR;
  vector_tool->restriction    = PICMAN_ANCHOR_FEATURE_NONE;
  vector_tool->modifier_lock  = FALSE;
  vector_tool->last_x         = 0.0;
  vector_tool->last_y         = 0.0;
  vector_tool->undo_motion    = FALSE;
  vector_tool->have_undo      = FALSE;

  vector_tool->cur_anchor     = NULL;
  vector_tool->cur_anchor2    = NULL;
  vector_tool->cur_stroke     = NULL;
  vector_tool->cur_position   = 0.0;
  vector_tool->cur_vectors    = NULL;
  vector_tool->vectors        = NULL;

  vector_tool->sel_count      = 0;
  vector_tool->sel_anchor     = NULL;
  vector_tool->sel_stroke     = NULL;

  vector_tool->saved_mode     = PICMAN_VECTOR_MODE_DESIGN;
}


static void
picman_vector_tool_control (PicmanTool       *tool,
                          PicmanToolAction  action,
                          PicmanDisplay    *display)
{
  PicmanVectorTool *vector_tool = PICMAN_VECTOR_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      picman_vector_tool_set_vectors (vector_tool, NULL);
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static gboolean
picman_vector_tool_check_writable (PicmanVectorTool *vector_tool)
{
  if (picman_item_is_content_locked (PICMAN_ITEM (vector_tool->vectors)) ||
      picman_item_is_position_locked (PICMAN_ITEM (vector_tool->vectors)))
    {
      picman_tool_message_literal (PICMAN_TOOL (vector_tool),
                                 PICMAN_TOOL (vector_tool)->display,
                                 _("The active path is locked."));
      vector_tool->function = VECTORS_FINISHED;

      return FALSE;
    }

  return TRUE;
}

static void
picman_vector_tool_button_press (PicmanTool            *tool,
                               const PicmanCoords    *coords,
                               guint32              time,
                               GdkModifierType      state,
                               PicmanButtonPressType  press_type,
                               PicmanDisplay         *display)
{
  PicmanDrawTool      *draw_tool   = PICMAN_DRAW_TOOL (tool);
  PicmanVectorTool    *vector_tool = PICMAN_VECTOR_TOOL (tool);
  PicmanVectorOptions *options     = PICMAN_VECTOR_TOOL_GET_OPTIONS (tool);
  PicmanImage         *image       = picman_display_get_image (display);
  PicmanVectors       *vectors;

  /* do nothing if we are an FINISHED state */
  if (vector_tool->function == VECTORS_FINISHED)
    return;

  g_return_if_fail (vector_tool->vectors  != NULL                  ||
                    vector_tool->function == VECTORS_SELECT_VECTOR ||
                    vector_tool->function == VECTORS_CREATE_VECTOR);

  vector_tool->undo_motion = FALSE;

  /* save the current modifier state */

  vector_tool->saved_state = state;

  picman_draw_tool_pause (draw_tool);

  if (picman_draw_tool_is_active (draw_tool) && draw_tool->display != display)
    {
      picman_draw_tool_stop (draw_tool);
    }

  picman_tool_control_activate (tool->control);
  tool->display = display;

  /* select a vectors object */

  if (vector_tool->function == VECTORS_SELECT_VECTOR)
    {
      if (picman_draw_tool_on_vectors (draw_tool, display, coords,
                                     PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                     PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                     NULL, NULL, NULL, NULL, NULL, &vectors))
        {
          picman_vector_tool_set_vectors (vector_tool, vectors);
          picman_image_set_active_vectors (image, vectors);
        }

      vector_tool->function = VECTORS_FINISHED;
    }

  /* create a new vector from scratch */

  if (vector_tool->function == VECTORS_CREATE_VECTOR)
    {
      vectors = picman_vectors_new (image, _("Unnamed"));

      /* Undo step gets added implicitely */
      vector_tool->have_undo = TRUE;

      vector_tool->undo_motion = TRUE;

      picman_image_add_vectors (image, vectors,
                              PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);
      picman_image_flush (image);

      picman_vector_tool_set_vectors (vector_tool, vectors);

      vector_tool->function = VECTORS_CREATE_STROKE;
    }

  picman_vectors_freeze (vector_tool->vectors);

  /* create a new stroke */

  if (vector_tool->function == VECTORS_CREATE_STROKE &&
      picman_vector_tool_check_writable (vector_tool))
    {
      picman_vector_tool_undo_push (vector_tool, _("Add Stroke"));

      vector_tool->cur_stroke = picman_bezier_stroke_new ();
      picman_vectors_stroke_add (vector_tool->vectors, vector_tool->cur_stroke);
      g_object_unref (vector_tool->cur_stroke);

      vector_tool->undo_motion = TRUE;

      vector_tool->sel_stroke = vector_tool->cur_stroke;
      vector_tool->cur_anchor = NULL;
      vector_tool->sel_anchor = NULL;
      vector_tool->function = VECTORS_ADD_ANCHOR;
    }


  /* add an anchor to an existing stroke */

  if (vector_tool->function == VECTORS_ADD_ANCHOR &&
      picman_vector_tool_check_writable (vector_tool))
    {
      PicmanCoords position = PICMAN_COORDS_DEFAULT_VALUES;

      position.x = coords->x;
      position.y = coords->y;

      picman_vector_tool_undo_push (vector_tool, _("Add Anchor"));

      vector_tool->undo_motion = TRUE;

      vector_tool->cur_anchor =
                     picman_bezier_stroke_extend (vector_tool->sel_stroke,
                                                &position,
                                                vector_tool->sel_anchor,
                                                EXTEND_EDITABLE);

      vector_tool->restriction = PICMAN_ANCHOR_FEATURE_SYMMETRIC;

      if (! options->polygonal)
        vector_tool->function = VECTORS_MOVE_HANDLE;
      else
        vector_tool->function = VECTORS_MOVE_ANCHOR;

      vector_tool->cur_stroke = vector_tool->sel_stroke;
    }


  /* insertion of an anchor in a curve segment */

  if (vector_tool->function == VECTORS_INSERT_ANCHOR &&
      picman_vector_tool_check_writable (vector_tool))
    {
      picman_vector_tool_undo_push (vector_tool, _("Insert Anchor"));

      vector_tool->undo_motion = TRUE;

      vector_tool->cur_anchor =
                         picman_stroke_anchor_insert (vector_tool->cur_stroke,
                                                    vector_tool->cur_anchor,
                                                    vector_tool->cur_position);
      if (vector_tool->cur_anchor)
        {
          if (options->polygonal)
            {
              picman_stroke_anchor_convert (vector_tool->cur_stroke,
                                          vector_tool->cur_anchor,
                                          PICMAN_ANCHOR_FEATURE_EDGE);
            }

          vector_tool->function = VECTORS_MOVE_ANCHOR;
        }
      else
        {
          vector_tool->function = VECTORS_FINISHED;
        }
    }


  /* move a handle */

  if (vector_tool->function == VECTORS_MOVE_HANDLE &&
      picman_vector_tool_check_writable (vector_tool))
    {
      picman_vector_tool_undo_push (vector_tool, _("Drag Handle"));

      if (vector_tool->cur_anchor->type == PICMAN_ANCHOR_ANCHOR)
        {
          if (! vector_tool->cur_anchor->selected)
            {
              picman_vectors_anchor_select (vector_tool->vectors,
                                          vector_tool->cur_stroke,
                                          vector_tool->cur_anchor,
                                          TRUE, TRUE);
              vector_tool->undo_motion = TRUE;
            }

          picman_draw_tool_on_vectors_handle (PICMAN_DRAW_TOOL (tool), display,
                                            vector_tool->vectors, coords,
                                            PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                            PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                            PICMAN_ANCHOR_CONTROL, TRUE,
                                            &vector_tool->cur_anchor,
                                            &vector_tool->cur_stroke);
          if (! vector_tool->cur_anchor)
            vector_tool->function = VECTORS_FINISHED;
        }
    }


  /* move an anchor */

  if (vector_tool->function == VECTORS_MOVE_ANCHOR &&
      picman_vector_tool_check_writable (vector_tool))
    {
      picman_vector_tool_undo_push (vector_tool, _("Drag Anchor"));

      if (! vector_tool->cur_anchor->selected)
        {
          picman_vectors_anchor_select (vector_tool->vectors,
                                      vector_tool->cur_stroke,
                                      vector_tool->cur_anchor,
                                      TRUE, TRUE);
          vector_tool->undo_motion = TRUE;
        }
    }


  /* move multiple anchors */

  if (vector_tool->function == VECTORS_MOVE_ANCHORSET &&
      picman_vector_tool_check_writable (vector_tool))
    {
      picman_vector_tool_undo_push (vector_tool, _("Drag Anchors"));

      if (state & TOGGLE_MASK)
        {
          picman_vectors_anchor_select (vector_tool->vectors,
                                      vector_tool->cur_stroke,
                                      vector_tool->cur_anchor,
                                      !vector_tool->cur_anchor->selected,
                                      FALSE);

          vector_tool->undo_motion = TRUE;

          if (vector_tool->cur_anchor->selected == FALSE)
            vector_tool->function = VECTORS_FINISHED;
        }
    }


  /* move a curve segment directly */

  if (vector_tool->function == VECTORS_MOVE_CURVE &&
      picman_vector_tool_check_writable (vector_tool))
    {
      picman_vector_tool_undo_push (vector_tool, _("Drag Curve"));

      /* the magic numbers are taken from the "feel good" parameter
       * from picman_bezier_stroke_point_move_relative in picmanbezierstroke.c. */
      if (vector_tool->cur_position < 5.0 / 6.0)
        {
          picman_vectors_anchor_select (vector_tool->vectors,
                                      vector_tool->cur_stroke,
                                      vector_tool->cur_anchor, TRUE, TRUE);
          vector_tool->undo_motion = TRUE;
        }

      if (vector_tool->cur_position > 1.0 / 6.0)
        {
          picman_vectors_anchor_select (vector_tool->vectors,
                                      vector_tool->cur_stroke,
                                      vector_tool->cur_anchor2, TRUE,
                                      (vector_tool->cur_position >= 5.0 / 6.0));
          vector_tool->undo_motion = TRUE;
        }

    }


  /* connect two strokes */

  if (vector_tool->function == VECTORS_CONNECT_STROKES &&
      picman_vector_tool_check_writable (vector_tool))
    {
      picman_vector_tool_undo_push (vector_tool, _("Connect Strokes"));

      picman_stroke_connect_stroke (vector_tool->sel_stroke,
                                  vector_tool->sel_anchor,
                                  vector_tool->cur_stroke,
                                  vector_tool->cur_anchor);
      vector_tool->undo_motion = TRUE;

      if (vector_tool->cur_stroke != vector_tool->sel_stroke &&
          picman_stroke_is_empty (vector_tool->cur_stroke))
        {
          picman_vectors_stroke_remove (vector_tool->vectors,
                                      vector_tool->cur_stroke);
        }

      vector_tool->sel_anchor = vector_tool->cur_anchor;
      vector_tool->cur_stroke = vector_tool->sel_stroke;

      picman_vectors_anchor_select (vector_tool->vectors,
                                  vector_tool->sel_stroke,
                                  vector_tool->sel_anchor, TRUE, TRUE);

      vector_tool->function = VECTORS_FINISHED;
    }


  /* move a stroke or all strokes of a vectors object */

  if ((vector_tool->function == VECTORS_MOVE_STROKE ||
       vector_tool->function == VECTORS_MOVE_VECTORS) &&
      picman_vector_tool_check_writable (vector_tool))
    {
      picman_vector_tool_undo_push (vector_tool, _("Drag Path"));

      /* Work is being done in picman_vector_tool_motion ()... */
    }


  /* convert an anchor to something that looks like an edge */

  if (vector_tool->function == VECTORS_CONVERT_EDGE &&
      picman_vector_tool_check_writable (vector_tool))
    {
      picman_vector_tool_undo_push (vector_tool, _("Convert Edge"));

      picman_stroke_anchor_convert (vector_tool->cur_stroke,
                                  vector_tool->cur_anchor,
                                  PICMAN_ANCHOR_FEATURE_EDGE);
      vector_tool->undo_motion = TRUE;

      if (vector_tool->cur_anchor->type == PICMAN_ANCHOR_ANCHOR)
        {
          picman_vectors_anchor_select (vector_tool->vectors,
                                      vector_tool->cur_stroke,
                                      vector_tool->cur_anchor, TRUE, TRUE);

          vector_tool->function = VECTORS_MOVE_ANCHOR;
        }
      else
        {
          vector_tool->cur_stroke = NULL;
          vector_tool->cur_anchor = NULL;

          /* avoid doing anything stupid */
          vector_tool->function = VECTORS_FINISHED;
        }
    }


  /* removal of a node in a stroke */

  if (vector_tool->function == VECTORS_DELETE_ANCHOR &&
      picman_vector_tool_check_writable (vector_tool))
    {
      picman_vector_tool_undo_push (vector_tool, _("Delete Anchor"));

      picman_stroke_anchor_delete (vector_tool->cur_stroke,
                                 vector_tool->cur_anchor);
      vector_tool->undo_motion = TRUE;

      if (picman_stroke_is_empty (vector_tool->cur_stroke))
        picman_vectors_stroke_remove (vector_tool->vectors,
                                    vector_tool->cur_stroke);

      vector_tool->cur_stroke = NULL;
      vector_tool->cur_anchor = NULL;
      vector_tool->function = VECTORS_FINISHED;
    }


  /* deleting a segment (opening up a stroke) */

  if (vector_tool->function == VECTORS_DELETE_SEGMENT &&
      picman_vector_tool_check_writable (vector_tool))
    {
      PicmanStroke *new_stroke;

      picman_vector_tool_undo_push (vector_tool, _("Delete Segment"));

      new_stroke = picman_stroke_open (vector_tool->cur_stroke,
                                     vector_tool->cur_anchor);
      if (new_stroke)
        {
          picman_vectors_stroke_add (vector_tool->vectors, new_stroke);
          g_object_unref (new_stroke);
        }

      vector_tool->undo_motion = TRUE;
      vector_tool->cur_stroke = NULL;
      vector_tool->cur_anchor = NULL;
      vector_tool->function = VECTORS_FINISHED;
    }

  vector_tool->last_x = coords->x;
  vector_tool->last_y = coords->y;

  picman_vectors_thaw (vector_tool->vectors);

  if (! picman_draw_tool_is_active (draw_tool))
    {
      picman_draw_tool_start (draw_tool, display);
    }

  picman_draw_tool_resume (draw_tool);
}

static void
picman_vector_tool_button_release (PicmanTool              *tool,
                                 const PicmanCoords      *coords,
                                 guint32                time,
                                 GdkModifierType        state,
                                 PicmanButtonReleaseType  release_type,
                                 PicmanDisplay           *display)
{
  PicmanVectorTool *vector_tool = PICMAN_VECTOR_TOOL (tool);
  PicmanImage      *image       = picman_display_get_image (display);

  vector_tool->function = VECTORS_FINISHED;

  if (vector_tool->have_undo &&
      (! vector_tool->undo_motion ||
       (release_type == PICMAN_BUTTON_RELEASE_CANCEL)))
    {
      PicmanUndo            *undo;
      PicmanUndoAccumulator  accum = { 0, };

      undo = picman_undo_stack_pop_undo (picman_image_get_undo_stack (image),
                                       PICMAN_UNDO_MODE_UNDO, &accum);

      picman_image_undo_event (image, PICMAN_UNDO_EVENT_UNDO_EXPIRED, undo);

      picman_undo_free (undo, PICMAN_UNDO_MODE_UNDO);
      g_object_unref (undo);
    }

  vector_tool->have_undo = FALSE;
  vector_tool->undo_motion = FALSE;

  picman_tool_control_halt (tool->control);
  picman_image_flush (image);
}

static void
picman_vector_tool_motion (PicmanTool         *tool,
                         const PicmanCoords *coords,
                         guint32           time,
                         GdkModifierType   state,
                         PicmanDisplay      *display)
{
  PicmanVectorTool    *vector_tool = PICMAN_VECTOR_TOOL (tool);
  PicmanVectorOptions *options     = PICMAN_VECTOR_TOOL_GET_OPTIONS (tool);
  PicmanCoords         position    = PICMAN_COORDS_DEFAULT_VALUES;
  PicmanAnchor        *anchor;

  if (vector_tool->function == VECTORS_FINISHED)
    return;

  position.x = coords->x;
  position.y = coords->y;

  picman_vectors_freeze (vector_tool->vectors);

  if ((vector_tool->saved_state & TOGGLE_MASK) != (state & TOGGLE_MASK))
    vector_tool->modifier_lock = FALSE;

  if (!vector_tool->modifier_lock)
    {
      if (state & TOGGLE_MASK)
        {
          vector_tool->restriction = PICMAN_ANCHOR_FEATURE_SYMMETRIC;
        }
      else
        {
          vector_tool->restriction = PICMAN_ANCHOR_FEATURE_NONE;
        }
    }

  switch (vector_tool->function)
    {
    case VECTORS_MOVE_ANCHOR:
    case VECTORS_MOVE_HANDLE:
      anchor = vector_tool->cur_anchor;

      if (anchor)
        {
          picman_stroke_anchor_move_absolute (vector_tool->cur_stroke,
                                            vector_tool->cur_anchor,
                                            &position,
                                            vector_tool->restriction);
          vector_tool->undo_motion = TRUE;
        }
      break;

    case VECTORS_MOVE_CURVE:
      if (options->polygonal)
        {
          picman_vector_tool_move_selected_anchors (vector_tool,
                                                  coords->x - vector_tool->last_x,
                                                  coords->y - vector_tool->last_y);
          vector_tool->undo_motion = TRUE;
        }
      else
        {
          picman_stroke_point_move_absolute (vector_tool->cur_stroke,
                                           vector_tool->cur_anchor,
                                           vector_tool->cur_position,
                                           &position,
                                           vector_tool->restriction);
          vector_tool->undo_motion = TRUE;
        }
      break;

    case VECTORS_MOVE_ANCHORSET:
      picman_vector_tool_move_selected_anchors (vector_tool,
                                              coords->x - vector_tool->last_x,
                                              coords->y - vector_tool->last_y);
      vector_tool->undo_motion = TRUE;
      break;

    case VECTORS_MOVE_STROKE:
      if (vector_tool->cur_stroke)
        {
          picman_stroke_translate (vector_tool->cur_stroke,
                                 coords->x - vector_tool->last_x,
                                 coords->y - vector_tool->last_y);
          vector_tool->undo_motion = TRUE;
        }
      else if (vector_tool->sel_stroke)
        {
          picman_stroke_translate (vector_tool->sel_stroke,
                                 coords->x - vector_tool->last_x,
                                 coords->y - vector_tool->last_y);
          vector_tool->undo_motion = TRUE;
        }
      break;

    case VECTORS_MOVE_VECTORS:
      picman_item_translate (PICMAN_ITEM (vector_tool->vectors),
                           coords->x - vector_tool->last_x,
                           coords->y - vector_tool->last_y, FALSE);
      vector_tool->undo_motion = TRUE;
      break;

    default:
      break;
    }

  vector_tool->last_x = coords->x;
  vector_tool->last_y = coords->y;

  picman_vectors_thaw (vector_tool->vectors);
}

static gboolean
picman_vector_tool_key_press (PicmanTool     *tool,
                            GdkEventKey  *kevent,
                            PicmanDisplay  *display)
{
  PicmanVectorTool    *vector_tool = PICMAN_VECTOR_TOOL (tool);
  PicmanDrawTool      *draw_tool   = PICMAN_DRAW_TOOL (tool);
  PicmanVectorOptions *options     = PICMAN_VECTOR_TOOL_GET_OPTIONS (tool);
  PicmanDisplayShell  *shell;
  gdouble            xdist, ydist;
  gdouble            pixels = 1.0;

  if (! vector_tool->vectors)
    return FALSE;

  if (display != draw_tool->display)
    return FALSE;

  shell = picman_display_get_shell (draw_tool->display);

  if (kevent->state & GDK_SHIFT_MASK)
    pixels = 10.0;

  if (kevent->state & picman_get_toggle_behavior_mask ())
    pixels = 50.0;

  switch (kevent->keyval)
    {
    case GDK_KEY_Return:
    case GDK_KEY_KP_Enter:
    case GDK_KEY_ISO_Enter:
      picman_vector_tool_to_selection_extended (vector_tool, kevent->state);
      break;

    case GDK_KEY_BackSpace:
    case GDK_KEY_Delete:
      picman_vector_tool_delete_selected_anchors (vector_tool);
      break;

    case GDK_KEY_Left:
    case GDK_KEY_Right:
    case GDK_KEY_Up:
    case GDK_KEY_Down:
      xdist = FUNSCALEX (shell, pixels);
      ydist = FUNSCALEY (shell, pixels);

      picman_vector_tool_undo_push (vector_tool, _("Move Anchors"));

      picman_vectors_freeze (vector_tool->vectors);

      switch (kevent->keyval)
        {
        case GDK_KEY_Left:
          picman_vector_tool_move_selected_anchors (vector_tool, -xdist, 0);
          break;

        case GDK_KEY_Right:
          picman_vector_tool_move_selected_anchors (vector_tool, xdist, 0);
          break;

        case GDK_KEY_Up:
          picman_vector_tool_move_selected_anchors (vector_tool, 0, -ydist);
          break;

        case GDK_KEY_Down:
          picman_vector_tool_move_selected_anchors (vector_tool, 0, ydist);
          break;

        default:
          break;
        }

      picman_vectors_thaw (vector_tool->vectors);
      vector_tool->have_undo = FALSE;
      break;

    case GDK_KEY_Escape:
      if (options->edit_mode != PICMAN_VECTOR_MODE_DESIGN)
        g_object_set (options, "vectors-edit-mode",
                      PICMAN_VECTOR_MODE_DESIGN, NULL);
      break;

    default:
      return FALSE;
    }

  picman_image_flush (picman_display_get_image (display));

  return TRUE;
}

static void
picman_vector_tool_modifier_key (PicmanTool        *tool,
                               GdkModifierType  key,
                               gboolean         press,
                               GdkModifierType  state,
                               PicmanDisplay     *display)
{
  PicmanVectorTool    *vector_tool = PICMAN_VECTOR_TOOL (tool);
  PicmanVectorOptions *options     = PICMAN_VECTOR_TOOL_GET_OPTIONS (tool);

  if (key == TOGGLE_MASK)
    return;

  if (key == INSDEL_MASK || key == MOVE_MASK)
    {
      PicmanVectorMode button_mode = options->edit_mode;

      if (press)
        {
          if (key == (state & (INSDEL_MASK | MOVE_MASK)))
            {
              /*  first modifier pressed  */

              vector_tool->saved_mode = options->edit_mode;
            }
        }
      else
        {
          if (! (state & (INSDEL_MASK | MOVE_MASK)))
            {
              /*  last modifier released  */

              button_mode = vector_tool->saved_mode;
            }
        }

      if (state & MOVE_MASK)
        {
          button_mode = PICMAN_VECTOR_MODE_MOVE;
        }
      else if (state & INSDEL_MASK)
        {
          button_mode = PICMAN_VECTOR_MODE_EDIT;
        }

      if (button_mode != options->edit_mode)
        {
          g_object_set (options, "vectors-edit-mode", button_mode, NULL);
        }
    }
}

static void
picman_vector_tool_oper_update (PicmanTool         *tool,
                              const PicmanCoords *coords,
                              GdkModifierType   state,
                              gboolean          proximity,
                              PicmanDisplay      *display)
{
  PicmanVectorTool    *vector_tool = PICMAN_VECTOR_TOOL (tool);
  PicmanDrawTool      *draw_tool   = PICMAN_DRAW_TOOL (tool);
  PicmanVectorOptions *options     = PICMAN_VECTOR_TOOL_GET_OPTIONS (tool);
  PicmanAnchor        *anchor      = NULL;
  PicmanAnchor        *anchor2     = NULL;
  PicmanStroke        *stroke      = NULL;
  gdouble            position    = -1;
  gboolean           on_handle   = FALSE;
  gboolean           on_curve    = FALSE;
  gboolean           on_vectors  = FALSE;

  vector_tool->modifier_lock = FALSE;

  /* are we hovering the current vectors on the current display? */
  if (vector_tool->vectors && PICMAN_DRAW_TOOL (tool)->display == display)
    {
      on_handle = picman_draw_tool_on_vectors_handle (PICMAN_DRAW_TOOL (tool),
                                                    display,
                                                    vector_tool->vectors,
                                                    coords,
                                                    PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                                    PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                                    PICMAN_ANCHOR_ANCHOR,
                                                    vector_tool->sel_count > 2,
                                                    &anchor, &stroke);

      if (! on_handle)
        on_curve = picman_draw_tool_on_vectors_curve (PICMAN_DRAW_TOOL (tool),
                                                    display,
                                                    vector_tool->vectors,
                                                    coords,
                                                    PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                                    PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                                    NULL,
                                                    &position, &anchor,
                                                    &anchor2, &stroke);
    }

  if (on_handle || on_curve)
    {
      vector_tool->cur_vectors = NULL;
    }
  else
    {
      on_vectors = picman_draw_tool_on_vectors (draw_tool, display, coords,
                                              PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                              PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                              NULL, NULL, NULL, NULL, NULL,
                                              &(vector_tool->cur_vectors));
    }

  vector_tool->cur_position   = position;
  vector_tool->cur_anchor     = anchor;
  vector_tool->cur_anchor2    = anchor2;
  vector_tool->cur_stroke     = stroke;

  switch (options->edit_mode)
    {
    case PICMAN_VECTOR_MODE_DESIGN:
      if (! vector_tool->vectors || PICMAN_DRAW_TOOL (tool)->display != display)
        {
          if (on_vectors)
            {
              vector_tool->function = VECTORS_SELECT_VECTOR;
            }
          else
            {
              vector_tool->function = VECTORS_CREATE_VECTOR;
              vector_tool->restriction = PICMAN_ANCHOR_FEATURE_SYMMETRIC;
              vector_tool->modifier_lock = TRUE;
            }
        }
      else if (on_handle)
        {
          if (anchor->type == PICMAN_ANCHOR_ANCHOR)
            {
              if (state & TOGGLE_MASK)
                {
                  vector_tool->function = VECTORS_MOVE_ANCHORSET;
                }
              else
                {
                  if (vector_tool->sel_count >= 2 && anchor->selected)
                    vector_tool->function = VECTORS_MOVE_ANCHORSET;
                  else
                    vector_tool->function = VECTORS_MOVE_ANCHOR;
                }
            }
          else
            {
              vector_tool->function = VECTORS_MOVE_HANDLE;

              if (state & TOGGLE_MASK)
                vector_tool->restriction = PICMAN_ANCHOR_FEATURE_SYMMETRIC;
              else
                vector_tool->restriction = PICMAN_ANCHOR_FEATURE_NONE;
            }
        }
      else if (on_curve)
        {
          if (picman_stroke_point_is_movable (stroke, anchor, position))
            {
              vector_tool->function = VECTORS_MOVE_CURVE;

              if (state & TOGGLE_MASK)
                vector_tool->restriction = PICMAN_ANCHOR_FEATURE_SYMMETRIC;
              else
                vector_tool->restriction = PICMAN_ANCHOR_FEATURE_NONE;
            }
          else
            {
              vector_tool->function = VECTORS_FINISHED;
            }
        }
      else
        {
          if (vector_tool->sel_stroke && vector_tool->sel_anchor &&
              picman_stroke_is_extendable (vector_tool->sel_stroke,
                                         vector_tool->sel_anchor) &&
              !(state & TOGGLE_MASK))
            vector_tool->function = VECTORS_ADD_ANCHOR;
          else
            vector_tool->function = VECTORS_CREATE_STROKE;

          vector_tool->restriction = PICMAN_ANCHOR_FEATURE_SYMMETRIC;
          vector_tool->modifier_lock = TRUE;
        }

      break;

    case PICMAN_VECTOR_MODE_EDIT:
      if (! vector_tool->vectors || PICMAN_DRAW_TOOL (tool)->display != display)
        {
          if (on_vectors)
            {
              vector_tool->function = VECTORS_SELECT_VECTOR;
            }
          else
            {
              vector_tool->function = VECTORS_FINISHED;
            }
        }
      else if (on_handle)
        {
          if (anchor->type == PICMAN_ANCHOR_ANCHOR)
            {
              if (!(state & TOGGLE_MASK) && vector_tool->sel_anchor &&
                  vector_tool->sel_anchor != anchor &&
                  picman_stroke_is_extendable (vector_tool->sel_stroke,
                                             vector_tool->sel_anchor) &&
                  picman_stroke_is_extendable (stroke, anchor))
                {
                  vector_tool->function = VECTORS_CONNECT_STROKES;
                }
              else
                {
                  if (state & TOGGLE_MASK)
                    {
                      vector_tool->function = VECTORS_DELETE_ANCHOR;
                    }
                  else
                    {
                      if (options->polygonal)
                        vector_tool->function = VECTORS_MOVE_ANCHOR;
                      else
                        vector_tool->function = VECTORS_MOVE_HANDLE;
                    }
                }
            }
          else
            {
              if (state & TOGGLE_MASK)
                vector_tool->function = VECTORS_CONVERT_EDGE;
              else
                vector_tool->function = VECTORS_MOVE_HANDLE;
            }
        }
      else if (on_curve)
        {
          if (state & TOGGLE_MASK)
            {
              vector_tool->function = VECTORS_DELETE_SEGMENT;
            }
          else if (picman_stroke_anchor_is_insertable (stroke, anchor, position))
            {
              vector_tool->function = VECTORS_INSERT_ANCHOR;
            }
          else
            {
              vector_tool->function = VECTORS_FINISHED;
            }
        }
      else
        {
          vector_tool->function = VECTORS_FINISHED;
        }

      break;

    case PICMAN_VECTOR_MODE_MOVE:
      if (! vector_tool->vectors || PICMAN_DRAW_TOOL (tool)->display != display)
        {
          if (on_vectors)
            {
              vector_tool->function = VECTORS_SELECT_VECTOR;
            }
          else
            {
              vector_tool->function = VECTORS_FINISHED;
            }
        }
      else if (on_handle || on_curve)
        {
          if (state & TOGGLE_MASK)
            {
              vector_tool->function = VECTORS_MOVE_VECTORS;
            }
          else
            {
              vector_tool->function = VECTORS_MOVE_STROKE;
            }
        }
      else
        {
          if (on_vectors)
            {
              vector_tool->function = VECTORS_SELECT_VECTOR;
            }
          else
            {
              vector_tool->function = VECTORS_MOVE_VECTORS;
            }
        }
      break;
    }

  picman_vector_tool_status_update (tool, display, state, proximity);
}


static void
picman_vector_tool_status_update (PicmanTool        *tool,
                                PicmanDisplay     *display,
                                GdkModifierType  state,
                                gboolean         proximity)
{
  PicmanVectorTool    *vector_tool = PICMAN_VECTOR_TOOL (tool);
  PicmanVectorOptions *options     = PICMAN_VECTOR_TOOL_GET_OPTIONS (tool);

  picman_tool_pop_status (tool, display);

  if (proximity)
    {
      const gchar *status      = NULL;
      gboolean     free_status = FALSE;

      switch (vector_tool->function)
        {
        case VECTORS_SELECT_VECTOR:
          status = _("Click to pick path to edit");
          break;

        case VECTORS_CREATE_VECTOR:
          status = _("Click to create a new path");
          break;

        case VECTORS_CREATE_STROKE:
          status = _("Click to create a new component of the path");
          break;

        case VECTORS_ADD_ANCHOR:
          status = picman_suggest_modifiers (_("Click or Click-Drag to create "
                                             "a new anchor"),
                                           GDK_SHIFT_MASK & ~state,
                                           NULL, NULL, NULL);
          free_status = TRUE;
          break;

        case VECTORS_MOVE_ANCHOR:
          if (options->edit_mode != PICMAN_VECTOR_MODE_EDIT)
            {
              GdkModifierType toggle_mask = picman_get_toggle_behavior_mask ();

              status = picman_suggest_modifiers (_("Click-Drag to move the "
                                                 "anchor around"),
                                               toggle_mask & ~state,
                                               NULL, NULL, NULL);
              free_status = TRUE;
            }
          else
            status = _("Click-Drag to move the anchor around");
          break;

        case VECTORS_MOVE_ANCHORSET:
          status = _("Click-Drag to move the anchors around");
          break;

        case VECTORS_MOVE_HANDLE:
          if (vector_tool->restriction != PICMAN_ANCHOR_FEATURE_SYMMETRIC)
            {
              status = picman_suggest_modifiers (_("Click-Drag to move the "
                                                 "handle around"),
                                               GDK_SHIFT_MASK & ~state,
                                               NULL, NULL, NULL);
            }
          else
            {
              status = picman_suggest_modifiers (_("Click-Drag to move the "
                                                 "handles around symmetrically"),
                                               GDK_SHIFT_MASK & ~state,
                                               NULL, NULL, NULL);
            }
          free_status = TRUE;
          break;

        case VECTORS_MOVE_CURVE:
          if (PICMAN_VECTOR_TOOL_GET_OPTIONS (tool)->polygonal)
            status = picman_suggest_modifiers (_("Click-Drag to move the "
                                               "anchors around"),
                                             GDK_SHIFT_MASK & ~state,
                                             NULL, NULL, NULL);
          else
            status = picman_suggest_modifiers (_("Click-Drag to change the "
                                               "shape of the curve"),
                                             GDK_SHIFT_MASK & ~state,
                                             _("%s: symmetrical"), NULL, NULL);
          free_status = TRUE;
          break;

        case VECTORS_MOVE_STROKE:
          status = picman_suggest_modifiers (_("Click-Drag to move the "
                                             "component around"),
                                           GDK_SHIFT_MASK & ~state,
                                           NULL, NULL, NULL);
          free_status = TRUE;
          break;

        case VECTORS_MOVE_VECTORS:
          status = _("Click-Drag to move the path around");
          break;

        case VECTORS_INSERT_ANCHOR:
          status = picman_suggest_modifiers (_("Click-Drag to insert an anchor "
                                             "on the path"),
                                           GDK_SHIFT_MASK & ~state,
                                           NULL, NULL, NULL);
          free_status = TRUE;
          break;

        case VECTORS_DELETE_ANCHOR:
          status = _("Click to delete this anchor");
          break;

        case VECTORS_CONNECT_STROKES:
          status = _("Click to connect this anchor "
                     "with the selected endpoint");
          break;

        case VECTORS_DELETE_SEGMENT:
          status = _("Click to open up the path");
          break;

        case VECTORS_CONVERT_EDGE:
          status = _("Click to make this node angular");
          break;

        case VECTORS_FINISHED:
          status = NULL;
          break;
      }

      if (status)
        picman_tool_push_status (tool, display, "%s", status);

      if (free_status)
        g_free ((gchar *) status);
    }
}

static void
picman_vector_tool_cursor_update (PicmanTool         *tool,
                                const PicmanCoords *coords,
                                GdkModifierType   state,
                                PicmanDisplay      *display)
{
  PicmanVectorTool     *vector_tool = PICMAN_VECTOR_TOOL (tool);
  PicmanToolCursorType  tool_cursor = PICMAN_TOOL_CURSOR_PATHS;
  PicmanCursorModifier  modifier    = PICMAN_CURSOR_MODIFIER_NONE;

  switch (vector_tool->function)
    {
    case VECTORS_SELECT_VECTOR:
      tool_cursor = PICMAN_TOOL_CURSOR_HAND;
      break;

    case VECTORS_CREATE_VECTOR:
    case VECTORS_CREATE_STROKE:
      modifier = PICMAN_CURSOR_MODIFIER_CONTROL;
      break;

    case VECTORS_ADD_ANCHOR:
    case VECTORS_INSERT_ANCHOR:
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS_ANCHOR;
      modifier    = PICMAN_CURSOR_MODIFIER_PLUS;
      break;

    case VECTORS_DELETE_ANCHOR:
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS_ANCHOR;
      modifier    = PICMAN_CURSOR_MODIFIER_MINUS;
      break;

    case VECTORS_DELETE_SEGMENT:
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS_SEGMENT;
      modifier    = PICMAN_CURSOR_MODIFIER_MINUS;
      break;

    case VECTORS_MOVE_HANDLE:
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS_CONTROL;
      modifier    = PICMAN_CURSOR_MODIFIER_MOVE;
      break;

    case VECTORS_CONVERT_EDGE:
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS_CONTROL;
      modifier    = PICMAN_CURSOR_MODIFIER_MINUS;
      break;

    case VECTORS_MOVE_ANCHOR:
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS_ANCHOR;
      modifier    = PICMAN_CURSOR_MODIFIER_MOVE;
      break;

    case VECTORS_MOVE_CURVE:
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS_SEGMENT;
      modifier    = PICMAN_CURSOR_MODIFIER_MOVE;
      break;

    case VECTORS_MOVE_STROKE:
    case VECTORS_MOVE_VECTORS:
      modifier = PICMAN_CURSOR_MODIFIER_MOVE;
      break;

    case VECTORS_MOVE_ANCHORSET:
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS_ANCHOR;
      modifier    = PICMAN_CURSOR_MODIFIER_MOVE;
      break;

    case VECTORS_CONNECT_STROKES:
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS_SEGMENT;
      modifier    = PICMAN_CURSOR_MODIFIER_JOIN;
      break;

    default:
      modifier = PICMAN_CURSOR_MODIFIER_BAD;
      break;
    }

  picman_tool_control_set_tool_cursor     (tool->control, tool_cursor);
  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_vector_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanVectorTool *vector_tool = PICMAN_VECTOR_TOOL (draw_tool);
  PicmanStroke     *cur_stroke;
  PicmanVectors    *vectors;

  vectors = vector_tool->vectors;

  if (! vectors || ! picman_vectors_get_bezier (vectors))
    return;

  /* the stroke itself */
  if (! picman_item_get_visible (PICMAN_ITEM (vectors)))
    picman_draw_tool_add_path (draw_tool, picman_vectors_get_bezier (vectors), 0, 0);

  for (cur_stroke = picman_vectors_stroke_get_next (vectors, NULL);
       cur_stroke;
       cur_stroke = picman_vectors_stroke_get_next (vectors, cur_stroke))
    {
      GArray *coords;
      GList  *draw_anchors;
      GList  *list;

      /* anchor handles */
      draw_anchors = picman_stroke_get_draw_anchors (cur_stroke);

      for (list = draw_anchors; list; list = g_list_next (list))
        {
          PicmanAnchor *cur_anchor = PICMAN_ANCHOR (list->data);

          if (cur_anchor->type == PICMAN_ANCHOR_ANCHOR)
            {
              picman_draw_tool_add_handle (draw_tool,
                                         cur_anchor->selected ?
                                         PICMAN_HANDLE_CIRCLE :
                                         PICMAN_HANDLE_FILLED_CIRCLE,
                                         cur_anchor->position.x,
                                         cur_anchor->position.y,
                                         PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                         PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                         PICMAN_HANDLE_ANCHOR_CENTER);
            }
        }

      g_list_free (draw_anchors);

      if (vector_tool->sel_count <= 2)
        {
          /* the lines to the control handles */
          coords = picman_stroke_get_draw_lines (cur_stroke);

          if (coords)
            {
              if (coords->len % 2 == 0)
                {
                  gint i;

                  for (i = 0; i < coords->len; i += 2)
                    {
                      PicmanCanvasItem *item;

                      item = picman_draw_tool_add_line
                        (draw_tool,
                         g_array_index (coords, PicmanCoords, i).x,
                         g_array_index (coords, PicmanCoords, i).y,
                         g_array_index (coords, PicmanCoords, i + 1).x,
                         g_array_index (coords, PicmanCoords, i + 1).y);

                      picman_canvas_item_set_highlight (item, TRUE);
                    }
                }

              g_array_free (coords, TRUE);
            }

          /* control handles */
          draw_anchors = picman_stroke_get_draw_controls (cur_stroke);

          for (list = draw_anchors; list; list = g_list_next (list))
            {
              PicmanAnchor *cur_anchor = PICMAN_ANCHOR (list->data);

              picman_draw_tool_add_handle (draw_tool,
                                         PICMAN_HANDLE_SQUARE,
                                         cur_anchor->position.x,
                                         cur_anchor->position.y,
                                         PICMAN_TOOL_HANDLE_SIZE_CIRCLE - 3,
                                         PICMAN_TOOL_HANDLE_SIZE_CIRCLE - 3,
                                         PICMAN_HANDLE_ANCHOR_CENTER);
            }

          g_list_free (draw_anchors);
        }
    }
}

static void
picman_vector_tool_vectors_changed (PicmanImage      *image,
                                  PicmanVectorTool *vector_tool)
{
  picman_vector_tool_set_vectors (vector_tool,
                                picman_image_get_active_vectors (image));
}

static void
picman_vector_tool_vectors_removed (PicmanVectors    *vectors,
                                  PicmanVectorTool *vector_tool)
{
  picman_vector_tool_set_vectors (vector_tool, NULL);
}

static void
picman_vector_tool_vectors_visible (PicmanVectors    *vectors,
                                  PicmanVectorTool *vector_tool)
{
  PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (vector_tool);

  if (picman_draw_tool_is_active (draw_tool) && draw_tool->paused_count == 0)
    {
      PicmanStroke *stroke = NULL;

      while ((stroke = picman_vectors_stroke_get_next (vectors, stroke)))
        {
          GArray   *coords;
          gboolean  closed;

          coords = picman_stroke_interpolate (stroke, 1.0, &closed);

          if (coords)
            {
              if (coords->len)
                picman_draw_tool_add_strokes (draw_tool,
                                            &g_array_index (coords,
                                                            PicmanCoords, 0),
                                            coords->len, FALSE);

              g_array_free (coords, TRUE);
            }
        }
    }
}

static void
picman_vector_tool_vectors_freeze (PicmanVectors    *vectors,
                                 PicmanVectorTool *vector_tool)
{
  picman_draw_tool_pause (PICMAN_DRAW_TOOL (vector_tool));
}

static void
picman_vector_tool_vectors_thaw (PicmanVectors    *vectors,
                               PicmanVectorTool *vector_tool)
{
  /*  Ok, the vector might have changed externally (e.g. Undo) we need
   *  to validate our internal state.
   */
  picman_vector_tool_verify_state (vector_tool);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (vector_tool));
}

void
picman_vector_tool_set_vectors (PicmanVectorTool *vector_tool,
                              PicmanVectors    *vectors)
{
  PicmanDrawTool      *draw_tool;
  PicmanTool          *tool;
  PicmanItem          *item = NULL;
  PicmanVectorOptions *options;

  g_return_if_fail (PICMAN_IS_VECTOR_TOOL (vector_tool));
  g_return_if_fail (vectors == NULL || PICMAN_IS_VECTORS (vectors));

  draw_tool = PICMAN_DRAW_TOOL (vector_tool);
  tool      = PICMAN_TOOL (vector_tool);
  options   = PICMAN_VECTOR_TOOL_GET_OPTIONS (vector_tool);

  if (vectors)
    item = PICMAN_ITEM (vectors);

  if (vectors == vector_tool->vectors)
    return;

  picman_draw_tool_pause (draw_tool);

  if (picman_draw_tool_is_active (draw_tool) &&
      (! vectors ||
       picman_display_get_image (draw_tool->display) != picman_item_get_image (item)))
    {
      picman_draw_tool_stop (draw_tool);
    }

  if (vector_tool->vectors)
    {
      PicmanImage *old_image;

      old_image = picman_item_get_image (PICMAN_ITEM (vector_tool->vectors));

      g_signal_handlers_disconnect_by_func (old_image,
                                            picman_vector_tool_vectors_changed,
                                            vector_tool);
      g_signal_handlers_disconnect_by_func (vector_tool->vectors,
                                            picman_vector_tool_vectors_removed,
                                            vector_tool);
      g_signal_handlers_disconnect_by_func (vector_tool->vectors,
                                            picman_vector_tool_vectors_visible,
                                            vector_tool);
      g_signal_handlers_disconnect_by_func (vector_tool->vectors,
                                            picman_vector_tool_vectors_freeze,
                                            vector_tool);
      g_signal_handlers_disconnect_by_func (vector_tool->vectors,
                                            picman_vector_tool_vectors_thaw,
                                            vector_tool);
      g_object_unref (vector_tool->vectors);

      if (options->to_selection_button)
        {
          gtk_widget_set_sensitive (options->to_selection_button, FALSE);
          g_signal_handlers_disconnect_by_func (options->to_selection_button,
                                                picman_vector_tool_to_selection,
                                                tool);
          g_signal_handlers_disconnect_by_func (options->to_selection_button,
                                                picman_vector_tool_to_selection_extended,
                                                tool);
        }

      if (options->stroke_button)
        {
          gtk_widget_set_sensitive (options->stroke_button, FALSE);
          g_signal_handlers_disconnect_by_func (options->stroke_button,
                                                picman_vector_tool_stroke_vectors,
                                                tool);
        }
    }

  vector_tool->vectors  = vectors;
  vector_tool->function = VECTORS_FINISHED;
  picman_vector_tool_verify_state (vector_tool);

  if (! vector_tool->vectors)
    {
      tool->display = NULL;

      /* leave draw_tool->paused_count in a consistent state */
      picman_draw_tool_resume (draw_tool);

      vector_tool->function = VECTORS_CREATE_VECTOR;

      return;
    }

  g_object_ref (vectors);

  g_signal_connect_object (picman_item_get_image (item), "active-vectors-changed",
                           G_CALLBACK (picman_vector_tool_vectors_changed),
                           vector_tool, 0);
  g_signal_connect_object (vectors, "removed",
                           G_CALLBACK (picman_vector_tool_vectors_removed),
                           vector_tool, 0);
  g_signal_connect_object (vectors, "visibility-changed",
                           G_CALLBACK (picman_vector_tool_vectors_visible),
                           vector_tool, 0);
  g_signal_connect_object (vectors, "freeze",
                           G_CALLBACK (picman_vector_tool_vectors_freeze),
                           vector_tool, 0);
  g_signal_connect_object (vectors, "thaw",
                           G_CALLBACK (picman_vector_tool_vectors_thaw),
                           vector_tool, 0);

  if (options->to_selection_button)
    {
      g_signal_connect_swapped (options->to_selection_button, "clicked",
                                G_CALLBACK (picman_vector_tool_to_selection),
                                tool);
      g_signal_connect_swapped (options->to_selection_button, "extended-clicked",
                                G_CALLBACK (picman_vector_tool_to_selection_extended),
                                tool);
      gtk_widget_set_sensitive (options->to_selection_button, TRUE);
    }

  if (options->stroke_button)
    {
      g_signal_connect_swapped (options->stroke_button, "clicked",
                                G_CALLBACK (picman_vector_tool_stroke_vectors),
                                tool);
      gtk_widget_set_sensitive (options->stroke_button, TRUE);
    }

  if (! picman_draw_tool_is_active (draw_tool))
    {
      if (tool->display &&
          picman_display_get_image (tool->display) == picman_item_get_image (item))
        {
          picman_draw_tool_start (draw_tool, tool->display);
        }
      else
        {
          PicmanContext *context;
          PicmanDisplay *display;

          context = picman_get_user_context (tool->tool_info->picman);
          display = picman_context_get_display (context);

          if (! display ||
              picman_display_get_image (display) != picman_item_get_image (item))
            {
              GList *list;

              display = NULL;

              for (list = picman_get_display_iter (picman_item_get_image (item)->picman);
                   list;
                   list = g_list_next (list))
                {
                  display = list->data;

                  if (picman_display_get_image (display) == picman_item_get_image (item))
                    {
                      picman_context_set_display (context, display);
                      break;
                    }

                  display = NULL;
                }
            }

          tool->display = display;

          if (tool->display)
            picman_draw_tool_start (draw_tool, tool->display);
        }
    }

  picman_draw_tool_resume (draw_tool);

  if (options->edit_mode != PICMAN_VECTOR_MODE_DESIGN)
    g_object_set (options, "vectors-edit-mode",
                  PICMAN_VECTOR_MODE_DESIGN, NULL);
}

static void
picman_vector_tool_move_selected_anchors (PicmanVectorTool *vector_tool,
                                        gdouble         x,
                                        gdouble         y)
{
  PicmanAnchor *cur_anchor;
  PicmanStroke *cur_stroke = NULL;
  GList      *anchors;
  GList      *list;
  PicmanCoords  offset = { 0.0, };

  offset.x = x;
  offset.y = y;

  while ((cur_stroke = picman_vectors_stroke_get_next (vector_tool->vectors,
                                                     cur_stroke)))
    {
      /* anchors */
      anchors = picman_stroke_get_draw_anchors (cur_stroke);

      for (list = anchors; list; list = g_list_next (list))
        {
          cur_anchor = PICMAN_ANCHOR (list->data);

          if (cur_anchor->selected)
            picman_stroke_anchor_move_relative (cur_stroke,
                                              cur_anchor,
                                              &offset,
                                              PICMAN_ANCHOR_FEATURE_NONE);
        }

      g_list_free (anchors);
    }
}

static void
picman_vector_tool_delete_selected_anchors (PicmanVectorTool *vector_tool)
{
  PicmanAnchor *cur_anchor;
  PicmanStroke *cur_stroke = NULL;
  GList      *anchors;
  GList      *list;
  gboolean    have_undo = FALSE;

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (vector_tool));
  picman_vectors_freeze (vector_tool->vectors);

  while ((cur_stroke = picman_vectors_stroke_get_next (vector_tool->vectors,
                                                     cur_stroke)))
    {
      /* anchors */
      anchors = picman_stroke_get_draw_anchors (cur_stroke);

      for (list = anchors; list; list = g_list_next (list))
        {
          cur_anchor = PICMAN_ANCHOR (list->data);

          if (cur_anchor->selected)
            {
              if (! have_undo)
                {
                  picman_vector_tool_undo_push (vector_tool, _("Delete Anchors"));
                  have_undo = TRUE;
                }

              picman_stroke_anchor_delete (cur_stroke, cur_anchor);

              if (picman_stroke_is_empty (cur_stroke))
                {
                  picman_vectors_stroke_remove (vector_tool->vectors, cur_stroke);
                  cur_stroke = NULL;
                }
            }
        }

      g_list_free (anchors);
    }

  picman_vectors_thaw (vector_tool->vectors);
  picman_draw_tool_resume (PICMAN_DRAW_TOOL (vector_tool));
}

static void
picman_vector_tool_verify_state (PicmanVectorTool *vector_tool)
{
  PicmanStroke *cur_stroke       = NULL;
  gboolean    cur_anchor_valid = FALSE;
  gboolean    cur_stroke_valid = FALSE;

  vector_tool->sel_count  = 0;
  vector_tool->sel_anchor = NULL;
  vector_tool->sel_stroke = NULL;

  if (! vector_tool->vectors)
    {
      vector_tool->cur_position = -1;
      vector_tool->cur_anchor   = NULL;
      vector_tool->cur_stroke   = NULL;
      return;
    }

  while ((cur_stroke = picman_vectors_stroke_get_next (vector_tool->vectors,
                                                     cur_stroke)))
    {
      GList *anchors;
      GList *list;

      /* anchor handles */
      anchors = picman_stroke_get_draw_anchors (cur_stroke);

      if (cur_stroke == vector_tool->cur_stroke)
        cur_stroke_valid = TRUE;

      for (list = anchors; list; list = g_list_next (list))
        {
          PicmanAnchor *cur_anchor = list->data;

          if (cur_anchor == vector_tool->cur_anchor)
            cur_anchor_valid = TRUE;

          if (cur_anchor->type == PICMAN_ANCHOR_ANCHOR &&
              cur_anchor->selected)
            {
              vector_tool->sel_count++;
              if (vector_tool->sel_count == 1)
                {
                  vector_tool->sel_anchor = cur_anchor;
                  vector_tool->sel_stroke = cur_stroke;
                }
              else
                {
                  vector_tool->sel_anchor = NULL;
                  vector_tool->sel_stroke = NULL;
                }
            }
        }

      g_list_free (anchors);

      anchors = picman_stroke_get_draw_controls (cur_stroke);

      for (list = anchors; list; list = g_list_next (list))
        {
          PicmanAnchor *cur_anchor = list->data;

          if (cur_anchor == vector_tool->cur_anchor)
            cur_anchor_valid = TRUE;
        }

      g_list_free (anchors);
    }

  if (! cur_stroke_valid)
    vector_tool->cur_stroke = NULL;

  if (! cur_anchor_valid)
    vector_tool->cur_anchor = NULL;

}

static void
picman_vector_tool_undo_push (PicmanVectorTool *vector_tool,
                            const gchar    *desc)
{
  g_return_if_fail (vector_tool->vectors != NULL);

  /* don't push two undos */
  if (vector_tool->have_undo)
    return;

  picman_image_undo_push_vectors_mod (picman_item_get_image (PICMAN_ITEM (vector_tool->vectors)),
                                    desc, vector_tool->vectors);
  vector_tool->have_undo = TRUE;
}


static void
picman_vector_tool_to_selection (PicmanVectorTool *vector_tool)
{
  picman_vector_tool_to_selection_extended (vector_tool, 0);
}


static void
picman_vector_tool_to_selection_extended (PicmanVectorTool *vector_tool,
                                        gint            state)
{
  PicmanImage *image;

  if (! vector_tool->vectors)
    return;

  image = picman_item_get_image (PICMAN_ITEM (vector_tool->vectors));

  picman_item_to_selection (PICMAN_ITEM (vector_tool->vectors),
                          picman_modifiers_to_channel_op (state),
                          TRUE, FALSE, 0, 0);
  picman_image_flush (image);
}


static void
picman_vector_tool_stroke_vectors (PicmanVectorTool *vector_tool,
                                 GtkWidget      *button)
{
  PicmanImage    *image;
  PicmanDrawable *active_drawable;
  GtkWidget    *dialog;

  if (! vector_tool->vectors)
    return;

  image = picman_item_get_image (PICMAN_ITEM (vector_tool->vectors));

  active_drawable = picman_image_get_active_drawable (image);

  if (! active_drawable)
    {
      picman_tool_message (PICMAN_TOOL (vector_tool),
                         PICMAN_TOOL (vector_tool)->display,
                         _("There is no active layer or channel to stroke to"));
      return;
    }

  dialog = stroke_dialog_new (PICMAN_ITEM (vector_tool->vectors),
                              PICMAN_CONTEXT (PICMAN_TOOL_GET_OPTIONS (vector_tool)),
                              _("Stroke Path"),
                              PICMAN_STOCK_PATH_STROKE,
                              PICMAN_HELP_PATH_STROKE,
                              button);
  gtk_widget_show (dialog);
}
