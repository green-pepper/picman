/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextTool
 * Copyright (C) 2002-2010  Sven Neumann <sven@picman.org>
 *                          Daniel Eddeland <danedde@svn.gnome.org>
 *                          Michael Natterer <mitch@picman.org>
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
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-pick-layer.h"
#include "core/picmanimage-undo.h"
#include "core/picmanimage-undo-push.h"
#include "core/picmanlayer-floating-sel.h"
#include "core/picmanmarshal.h"
#include "core/picmantoolinfo.h"
#include "core/picmanundostack.h"

#include "text/picmantext.h"
#include "text/picmantext-vectors.h"
#include "text/picmantextlayer.h"
#include "text/picmantextlayout.h"
#include "text/picmantextundo.h"

#include "vectors/picmanvectors-warp.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmenufactory.h"
#include "widgets/picmantextbuffer.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanviewabledialog.h"

#include "display/picmancanvasgroup.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "picmanrectangletool.h"
#include "picmantextoptions.h"
#include "picmantexttool.h"
#include "picmantexttool-editor.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


#define TEXT_UNDO_TIMEOUT 3


/*  local function prototypes  */

static void picman_text_tool_rectangle_tool_iface_init (PicmanRectangleToolInterface *iface);

static void      picman_text_tool_constructed     (GObject           *object);
static void      picman_text_tool_finalize        (GObject           *object);

static void      picman_text_tool_control         (PicmanTool          *tool,
                                                 PicmanToolAction     action,
                                                 PicmanDisplay       *display);
static void      picman_text_tool_button_press    (PicmanTool          *tool,
                                                 const PicmanCoords  *coords,
                                                 guint32            time,
                                                 GdkModifierType    state,
                                                 PicmanButtonPressType  press_type,
                                                 PicmanDisplay       *display);
static void      picman_text_tool_button_release  (PicmanTool          *tool,
                                                 const PicmanCoords  *coords,
                                                 guint32            time,
                                                 GdkModifierType    state,
                                                 PicmanButtonReleaseType release_type,
                                                 PicmanDisplay       *display);
static void      picman_text_tool_motion          (PicmanTool          *tool,
                                                 const PicmanCoords  *coords,
                                                 guint32            time,
                                                 GdkModifierType    state,
                                                 PicmanDisplay       *display);
static gboolean  picman_text_tool_key_press       (PicmanTool          *tool,
                                                 GdkEventKey       *kevent,
                                                 PicmanDisplay       *display);
static gboolean  picman_text_tool_key_release     (PicmanTool          *tool,
                                                 GdkEventKey       *kevent,
                                                 PicmanDisplay       *display);
static void      picman_text_tool_oper_update     (PicmanTool          *tool,
                                                 const PicmanCoords  *coords,
                                                 GdkModifierType    state,
                                                 gboolean           proximity,
                                                 PicmanDisplay       *display);
static void      picman_text_tool_cursor_update   (PicmanTool          *tool,
                                                 const PicmanCoords  *coords,
                                                 GdkModifierType    state,
                                                 PicmanDisplay       *display);
static PicmanUIManager * picman_text_tool_get_popup (PicmanTool          *tool,
                                                 const PicmanCoords  *coords,
                                                 GdkModifierType    state,
                                                 PicmanDisplay       *display,
                                                 const gchar      **ui_path);

static void      picman_text_tool_draw            (PicmanDrawTool      *draw_tool);
static void      picman_text_tool_draw_selection  (PicmanDrawTool      *draw_tool);

static void      picman_text_tool_frame_item      (PicmanTextTool      *text_tool);

static gboolean  picman_text_tool_rectangle_change_complete
                                                (PicmanRectangleTool *rect_tool);

static void      picman_text_tool_connect         (PicmanTextTool      *text_tool,
                                                 PicmanTextLayer     *layer,
                                                 PicmanText          *text);

static void      picman_text_tool_layer_notify    (PicmanTextLayer     *layer,
                                                 const GParamSpec  *pspec,
                                                 PicmanTextTool      *text_tool);
static void      picman_text_tool_proxy_notify    (PicmanText          *text,
                                                 const GParamSpec  *pspec,
                                                 PicmanTextTool      *text_tool);

static void      picman_text_tool_text_notify     (PicmanText          *text,
                                                 const GParamSpec  *pspec,
                                                 PicmanTextTool      *text_tool);
static void      picman_text_tool_text_changed    (PicmanText          *text,
                                                 PicmanTextTool      *text_tool);

static gboolean  picman_text_tool_apply           (PicmanTextTool      *text_tool,
                                                 gboolean           push_undo);

static void      picman_text_tool_create_layer    (PicmanTextTool      *text_tool,
                                                 PicmanText          *text);

static void      picman_text_tool_layer_changed   (PicmanImage         *image,
                                                 PicmanTextTool      *text_tool);
static void      picman_text_tool_set_image       (PicmanTextTool      *text_tool,
                                                 PicmanImage         *image);
static gboolean  picman_text_tool_set_drawable    (PicmanTextTool      *text_tool,
                                                 PicmanDrawable      *drawable,
                                                 gboolean           confirm);

static void      picman_text_tool_block_drawing   (PicmanTextTool      *text_tool);
static void      picman_text_tool_unblock_drawing (PicmanTextTool      *text_tool);

static void    picman_text_tool_buffer_begin_edit (PicmanTextBuffer    *buffer,
                                                 PicmanTextTool      *text_tool);
static void    picman_text_tool_buffer_end_edit   (PicmanTextBuffer    *buffer,
                                                 PicmanTextTool      *text_tool);


G_DEFINE_TYPE_WITH_CODE (PicmanTextTool, picman_text_tool,
                         PICMAN_TYPE_DRAW_TOOL,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_RECTANGLE_TOOL,
                                                picman_text_tool_rectangle_tool_iface_init))

#define parent_class picman_text_tool_parent_class


void
picman_text_tool_register (PicmanToolRegisterCallback  callback,
                         gpointer                  data)
{
  (* callback) (PICMAN_TYPE_TEXT_TOOL,
                PICMAN_TYPE_TEXT_OPTIONS,
                picman_text_options_gui,
                PICMAN_CONTEXT_FOREGROUND_MASK |
                PICMAN_CONTEXT_FONT_MASK       |
                PICMAN_CONTEXT_PALETTE_MASK /* for the color popup's palette tab */,
                "picman-text-tool",
                _("Text"),
                _("Text Tool: Create or edit text layers"),
                N_("Te_xt"), "T",
                NULL, PICMAN_HELP_TOOL_TEXT,
                PICMAN_STOCK_TOOL_TEXT,
                data);
}

static void
picman_text_tool_class_init (PicmanTextToolClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  object_class->constructed    = picman_text_tool_constructed;
  object_class->finalize       = picman_text_tool_finalize;
  object_class->set_property   = picman_rectangle_tool_set_property;
  object_class->get_property   = picman_rectangle_tool_get_property;

  tool_class->control          = picman_text_tool_control;
  tool_class->button_press     = picman_text_tool_button_press;
  tool_class->motion           = picman_text_tool_motion;
  tool_class->button_release   = picman_text_tool_button_release;
  tool_class->key_press        = picman_text_tool_key_press;
  tool_class->key_release      = picman_text_tool_key_release;
  tool_class->oper_update      = picman_text_tool_oper_update;
  tool_class->cursor_update    = picman_text_tool_cursor_update;
  tool_class->get_popup        = picman_text_tool_get_popup;

  draw_tool_class->draw        = picman_text_tool_draw;

  picman_rectangle_tool_install_properties (object_class);
}

static void
picman_text_tool_rectangle_tool_iface_init (PicmanRectangleToolInterface *iface)
{
  iface->execute                   = NULL;
  iface->cancel                    = NULL;
  iface->rectangle_change_complete = picman_text_tool_rectangle_change_complete;
}

static void
picman_text_tool_init (PicmanTextTool *text_tool)
{
  PicmanTool *tool = PICMAN_TOOL (text_tool);

  text_tool->proxy   = NULL;
  text_tool->pending = NULL;
  text_tool->idle_id = 0;

  text_tool->text    = NULL;
  text_tool->layer   = NULL;
  text_tool->image   = NULL;
  text_tool->layout  = NULL;

  text_tool->buffer = picman_text_buffer_new ();

  g_signal_connect (text_tool->buffer, "begin-user-action",
                    G_CALLBACK (picman_text_tool_buffer_begin_edit),
                    text_tool);
  g_signal_connect (text_tool->buffer, "end-user-action",
                    G_CALLBACK (picman_text_tool_buffer_end_edit),
                    text_tool);

  text_tool->handle_rectangle_change_complete = TRUE;

  picman_text_tool_editor_init (text_tool);

  picman_tool_control_set_scroll_lock          (tool->control, TRUE);
  picman_tool_control_set_handle_empty_image   (tool->control, TRUE);
  picman_tool_control_set_wants_click          (tool->control, TRUE);
  picman_tool_control_set_wants_double_click   (tool->control, TRUE);
  picman_tool_control_set_wants_triple_click   (tool->control, TRUE);
  picman_tool_control_set_wants_all_key_events (tool->control, TRUE);
  picman_tool_control_set_precision            (tool->control,
                                              PICMAN_CURSOR_PRECISION_PIXEL_BORDER);
  picman_tool_control_set_tool_cursor          (tool->control,
                                              PICMAN_TOOL_CURSOR_TEXT);
  picman_tool_control_set_action_object_1      (tool->control,
                                              "context/context-font-select-set");
}

static void
picman_text_tool_constructed (GObject *object)
{
  PicmanTextTool    *text_tool = PICMAN_TEXT_TOOL (object);
  PicmanTextOptions *options   = PICMAN_TEXT_TOOL_GET_OPTIONS (text_tool);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_rectangle_tool_constructor (object);

  text_tool->proxy = g_object_new (PICMAN_TYPE_TEXT, NULL);

  picman_text_options_connect_text (options, text_tool->proxy);

  g_signal_connect_object (text_tool->proxy, "notify",
                           G_CALLBACK (picman_text_tool_proxy_notify),
                           text_tool, 0);

  g_object_set (options,
                "highlight", FALSE,
                NULL);
}

static void
picman_text_tool_finalize (GObject *object)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (object);

  if (text_tool->proxy)
    {
      g_object_unref (text_tool->proxy);
      text_tool->proxy = NULL;
    }

  if (text_tool->buffer)
    {
      g_object_unref (text_tool->buffer);
      text_tool->buffer = NULL;
    }

  picman_text_tool_editor_finalize (text_tool);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_text_tool_control (PicmanTool       *tool,
                        PicmanToolAction  action,
                        PicmanDisplay    *display)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      picman_text_tool_editor_halt (text_tool);
      picman_text_tool_set_drawable (text_tool, NULL, FALSE);
      break;
    }

  picman_rectangle_tool_control (tool, action, display);

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_text_tool_button_press (PicmanTool            *tool,
                             const PicmanCoords    *coords,
                             guint32              time,
                             GdkModifierType      state,
                             PicmanButtonPressType  press_type,
                             PicmanDisplay         *display)
{
  PicmanTextTool      *text_tool = PICMAN_TEXT_TOOL (tool);
  PicmanRectangleTool *rect_tool = PICMAN_RECTANGLE_TOOL (tool);
  PicmanImage         *image     = picman_display_get_image (display);
  PicmanText          *text      = text_tool->text;

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  if (tool->display && tool->display != display)
    picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);

  if (press_type == PICMAN_BUTTON_PRESS_NORMAL)
    {
      picman_tool_control_activate (tool->control);

      picman_text_tool_reset_im_context (text_tool);

      text_tool->selecting = FALSE;

      if (picman_rectangle_tool_point_in_rectangle (rect_tool,
                                                  coords->x,
                                                  coords->y) &&
          ! text_tool->moving)
        {
          picman_rectangle_tool_set_function (rect_tool, PICMAN_RECTANGLE_TOOL_DEAD);
        }
      else
        {
          picman_rectangle_tool_button_press (tool, coords, time, state, display);
        }

      /*  bail out now if the user user clicked on a handle of an
       *  existing rectangle, but not inside an existing framed layer
       */
      if (picman_rectangle_tool_get_function (rect_tool) !=
          PICMAN_RECTANGLE_TOOL_CREATING)
        {
          if (text_tool->layer)
            {
              PicmanItem *item = PICMAN_ITEM (text_tool->layer);
              gdouble   x    = coords->x - picman_item_get_offset_x (item);
              gdouble   y    = coords->y - picman_item_get_offset_y (item);

              if (x < 0 || x >= picman_item_get_width  (item) ||
                  y < 0 || y >= picman_item_get_height (item))
                {
                  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
                  return;
                }
            }
          else
            {
              picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
              return;
            }
        }

      /* if the the click is not related to the currently edited text
       * layer in any way, try to pick a text layer
       */
      if (! text_tool->moving &&
          picman_rectangle_tool_get_function (rect_tool) ==
          PICMAN_RECTANGLE_TOOL_CREATING)
        {
          PicmanTextLayer *text_layer;

          text_layer = picman_image_pick_text_layer (image, coords->x, coords->y);

          if (text_layer && text_layer != text_tool->layer)
            {
              if (text_tool->image == image)
                g_signal_handlers_block_by_func (image,
                                                 picman_text_tool_layer_changed,
                                                 text_tool);

              picman_image_set_active_layer (image, PICMAN_LAYER (text_layer));

              if (text_tool->image == image)
                g_signal_handlers_unblock_by_func (image,
                                                   picman_text_tool_layer_changed,
                                                   text_tool);
            }
        }
    }

  if (picman_image_coords_in_active_pickable (image, coords, FALSE, FALSE))
    {
      PicmanDrawable *drawable = picman_image_get_active_drawable (image);
      PicmanItem     *item     = PICMAN_ITEM (drawable);
      gdouble       x        = coords->x - picman_item_get_offset_x (item);
      gdouble       y        = coords->y - picman_item_get_offset_y (item);

      /*  did the user click on a text layer?  */
      if (picman_text_tool_set_drawable (text_tool, drawable, TRUE))
        {
          if (press_type == PICMAN_BUTTON_PRESS_NORMAL)
            {
              /*  if we clicked on a text layer while the tool was idle
               *  (didn't show a rectangle), frame the layer and switch to
               *  selecting instead of drawing a new rectangle
               */
              if (picman_rectangle_tool_get_function (rect_tool) ==
                  PICMAN_RECTANGLE_TOOL_CREATING)
                {
                  picman_rectangle_tool_set_function (rect_tool,
                                                    PICMAN_RECTANGLE_TOOL_DEAD);

                  picman_text_tool_frame_item (text_tool);
                }

              if (text_tool->text && text_tool->text != text)
                {
                  picman_text_tool_editor_start (text_tool);
                }
            }

          if (text_tool->text && ! text_tool->moving)
            {
              text_tool->selecting = TRUE;

              picman_text_tool_editor_button_press (text_tool, x, y, press_type);
            }
          else
            {
              text_tool->selecting = FALSE;
            }

          picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));

          return;
        }
    }

  if (press_type == PICMAN_BUTTON_PRESS_NORMAL)
    {
      /*  create a new text layer  */
      text_tool->text_box_fixed = FALSE;

      picman_text_tool_connect (text_tool, NULL, NULL);
      picman_text_tool_editor_start (text_tool);
    }

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_text_tool_button_release (PicmanTool              *tool,
                               const PicmanCoords      *coords,
                               guint32                time,
                               GdkModifierType        state,
                               PicmanButtonReleaseType  release_type,
                               PicmanDisplay           *display)
{
  PicmanRectangleTool *rect_tool = PICMAN_RECTANGLE_TOOL (tool);
  PicmanTextTool      *text_tool = PICMAN_TEXT_TOOL (tool);

  picman_tool_control_halt (tool->control);

  if (text_tool->selecting)
    {
      /*  we are in a selection process (user has initially clicked on
       *  an existing text layer), so finish the selection process and
       *  ignore rectangle-change-complete.
       */

      /*  need to block "end-user-action" on the text buffer, because
       *  GtkTextBuffer considers copying text to the clipboard an
       *  undo-relevant user action, which is clearly a bug, but what
       *  can we do...
       */
      g_signal_handlers_block_by_func (text_tool->buffer,
                                       picman_text_tool_buffer_begin_edit,
                                       text_tool);
      g_signal_handlers_block_by_func (text_tool->buffer,
                                       picman_text_tool_buffer_end_edit,
                                       text_tool);

      picman_text_tool_editor_button_release (text_tool);

      g_signal_handlers_unblock_by_func (text_tool->buffer,
                                         picman_text_tool_buffer_end_edit,
                                         text_tool);
      g_signal_handlers_unblock_by_func (text_tool->buffer,
                                         picman_text_tool_buffer_begin_edit,
                                         text_tool);

      text_tool->selecting = FALSE;

      text_tool->handle_rectangle_change_complete = FALSE;

      /*  there is no cancelling of selections yet  */
      if (release_type == PICMAN_BUTTON_RELEASE_CANCEL)
        release_type = PICMAN_BUTTON_RELEASE_NORMAL;
    }
  else if (picman_rectangle_tool_get_function (rect_tool) ==
           PICMAN_RECTANGLE_TOOL_DEAD)
    {
      /*  the user clicked in dead space (like between the corner and
       *  edge handles, so completely ignore that.
       */

      text_tool->handle_rectangle_change_complete = FALSE;
    }
  else if (release_type == PICMAN_BUTTON_RELEASE_CANCEL)
    {
      /*  user has clicked outside of any text layer in order to
       *  create a new text, but cancelled the operation.
       */

      picman_text_tool_editor_halt (text_tool);

      text_tool->handle_rectangle_change_complete = FALSE;
    }
  else
    {
      gint x1, y1;
      gint x2, y2;

      /*  otherwise the user has clicked outside of any text layer in
       *  order to create a new text, fall through and let
       *  rectangle-change-complete do its job of setting the new text
       *  layer's size.
       */

      g_object_get (text_tool,
                    "x1", &x1,
                    "y1", &y1,
                    "x2", &x2,
                    "y2", &y2,
                    NULL);

      if (release_type == PICMAN_BUTTON_RELEASE_CLICK ||
          (x2 - x1) < 3                             ||
          (y2 - y1) < 3)
        {
          /*  unless the rectangle is unreasonably small to hold any
           *  real text (the user has eitherjust clicked or just made
           *  a rectangle of a few pixels), so set the text box to
           *  dynamic and ignore rectangle-change-complete.
           */

          g_object_set (text_tool->proxy,
                        "box-mode", PICMAN_TEXT_BOX_DYNAMIC,
                        NULL);

          text_tool->handle_rectangle_change_complete = FALSE;
        }
    }

  picman_rectangle_tool_button_release (tool, coords, time, state,
                                      release_type, display);

  text_tool->handle_rectangle_change_complete = TRUE;
}

static void
picman_text_tool_motion (PicmanTool         *tool,
                       const PicmanCoords *coords,
                       guint32           time,
                       GdkModifierType   state,
                       PicmanDisplay      *display)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (tool);

  if (! text_tool->selecting)
    {
      picman_rectangle_tool_motion (tool, coords, time, state, display);
    }
  else
    {
      PicmanItem *item = PICMAN_ITEM (text_tool->layer);
      gdouble   x    = coords->x - picman_item_get_offset_x (item);
      gdouble   y    = coords->y - picman_item_get_offset_y (item);

      picman_text_tool_editor_motion (text_tool, x, y);
    }
}

static gboolean
picman_text_tool_key_press (PicmanTool    *tool,
                          GdkEventKey *kevent,
                          PicmanDisplay *display)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (tool);

  if (display == tool->display)
    return picman_text_tool_editor_key_press (text_tool, kevent);

  return FALSE;
}

static gboolean
picman_text_tool_key_release (PicmanTool    *tool,
                            GdkEventKey *kevent,
                            PicmanDisplay *display)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (tool);

  if (display == tool->display)
    return picman_text_tool_editor_key_release (text_tool, kevent);

  return FALSE;
}

static void
picman_text_tool_oper_update (PicmanTool         *tool,
                            const PicmanCoords *coords,
                            GdkModifierType   state,
                            gboolean          proximity,
                            PicmanDisplay      *display)
{
  PicmanTextTool      *text_tool = PICMAN_TEXT_TOOL (tool);
  PicmanRectangleTool *rect_tool = PICMAN_RECTANGLE_TOOL (tool);

  picman_rectangle_tool_oper_update (tool, coords, state, proximity, display);

  text_tool->moving = (picman_rectangle_tool_get_function (rect_tool) ==
                       PICMAN_RECTANGLE_TOOL_MOVING &&
                       (state & GDK_MOD1_MASK));
}

static void
picman_text_tool_cursor_update (PicmanTool         *tool,
                              const PicmanCoords *coords,
                              GdkModifierType   state,
                              PicmanDisplay      *display)
{
  if (tool->display == display)
    {
      PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (tool);

      if (picman_rectangle_tool_point_in_rectangle (PICMAN_RECTANGLE_TOOL (tool),
                                                  coords->x,
                                                  coords->y) &&
          ! text_tool->moving)
        {
          picman_tool_control_set_cursor          (tool->control, GDK_XTERM);
          picman_tool_control_set_cursor_modifier (tool->control,
                                                 PICMAN_CURSOR_MODIFIER_NONE);
        }
      else
        {
          picman_rectangle_tool_cursor_update (tool, coords, state, display);
        }
    }
  else
    {
      picman_tool_control_set_cursor          (tool->control, GDK_XTERM);
      picman_tool_control_set_cursor_modifier (tool->control,
                                             PICMAN_CURSOR_MODIFIER_NONE);
    }

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static PicmanUIManager *
picman_text_tool_get_popup (PicmanTool         *tool,
                          const PicmanCoords *coords,
                          GdkModifierType   state,
                          PicmanDisplay      *display,
                          const gchar     **ui_path)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (tool);

  if (picman_rectangle_tool_point_in_rectangle (PICMAN_RECTANGLE_TOOL (text_tool),
                                              coords->x,
                                              coords->y))
    {
      if (! text_tool->ui_manager)
        {
          PicmanDialogFactory *dialog_factory;
          GtkWidget         *im_menu;
          GList             *children;

          dialog_factory = picman_dialog_factory_get_singleton ();

          text_tool->ui_manager =
            picman_menu_factory_manager_new (picman_dialog_factory_get_menu_factory (dialog_factory),
                                           "<TextTool>",
                                           text_tool, FALSE);

          im_menu = gtk_ui_manager_get_widget (GTK_UI_MANAGER (text_tool->ui_manager),
                                               "/text-tool-popup/text-tool-input-methods-menu");

          if (GTK_IS_MENU_ITEM (im_menu))
            im_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (im_menu));

          /*  hide the generated "empty" item  */
          children = gtk_container_get_children (GTK_CONTAINER (im_menu));
          while (children)
            {
              gtk_widget_hide (children->data);
              children = g_list_remove (children, children->data);
            }

          gtk_im_multicontext_append_menuitems (GTK_IM_MULTICONTEXT (text_tool->im_context),
                                                GTK_MENU_SHELL (im_menu));
        }

      picman_ui_manager_update (text_tool->ui_manager, text_tool);

      *ui_path = "/text-tool-popup";

      return text_tool->ui_manager;
    }

  return NULL;
}

static void
picman_text_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (draw_tool);

  g_object_set (text_tool,
                "narrow-mode", TRUE,
                NULL);

  picman_rectangle_tool_draw (draw_tool, NULL);

  if (! text_tool->text  ||
      ! text_tool->layer ||
      ! text_tool->layer->text)
    return;

  picman_text_tool_ensure_layout (text_tool);

  if (gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (text_tool->buffer)))
    {
      /* If the text buffer has a selection, highlight the selected letters */

      picman_text_tool_draw_selection (draw_tool);
    }
  else
    {
      /* If the text buffer has no selection, draw the text cursor */

      PicmanCanvasItem *item;
      PangoRectangle  cursor_rect;
      gint            off_x, off_y;
      gboolean        overwrite;

      picman_text_tool_editor_get_cursor_rect (text_tool,
                                             text_tool->overwrite_mode,
                                             &cursor_rect);

      picman_item_get_offset (PICMAN_ITEM (text_tool->layer), &off_x, &off_y);
      cursor_rect.x += off_x;
      cursor_rect.y += off_y;

      overwrite = text_tool->overwrite_mode && cursor_rect.width != 0;

      item = picman_draw_tool_add_text_cursor (draw_tool, &cursor_rect,
                                             overwrite);
      picman_canvas_item_set_highlight (item, TRUE);
    }
}

static void
picman_text_tool_draw_selection (PicmanDrawTool *draw_tool)
{
  PicmanTextTool    *text_tool = PICMAN_TEXT_TOOL (draw_tool);
  GtkTextBuffer   *buffer    = GTK_TEXT_BUFFER (text_tool->buffer);
  PicmanCanvasGroup *fill_group;
  PangoLayout     *layout;
  gint             offset_x;
  gint             offset_y;
  gint             off_x, off_y;
  PangoLayoutIter *iter;
  GtkTextIter      sel_start, sel_end;
  gint             min, max;
  gint             i;

  fill_group = picman_draw_tool_add_fill_group (draw_tool);
  picman_canvas_item_set_highlight (PICMAN_CANVAS_ITEM (fill_group), TRUE);

  gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end);

  min = picman_text_buffer_get_iter_index (text_tool->buffer, &sel_start, TRUE);
  max = picman_text_buffer_get_iter_index (text_tool->buffer, &sel_end, TRUE);

  layout = picman_text_layout_get_pango_layout (text_tool->layout);

  picman_text_layout_get_offsets (text_tool->layout, &offset_x, &offset_y);

  picman_item_get_offset (PICMAN_ITEM (text_tool->layer), &off_x, &off_y);
  offset_x += off_x;
  offset_y += off_y;

  iter = pango_layout_get_iter (layout);

  picman_draw_tool_push_group (draw_tool, fill_group);

  do
    {
      if (! pango_layout_iter_get_run (iter))
        continue;

      i = pango_layout_iter_get_index (iter);

      if (i >= min && i < max)
        {
          PangoRectangle rect;
          gint           ytop, ybottom;

          pango_layout_iter_get_char_extents (iter, &rect);
          pango_layout_iter_get_line_yrange (iter, &ytop, &ybottom);

          rect.y      = ytop;
          rect.height = ybottom - ytop;

          pango_extents_to_pixels (&rect, NULL);

          picman_text_layout_transform_rect (text_tool->layout, &rect);

          rect.x += offset_x;
          rect.y += offset_y;

          picman_draw_tool_add_rectangle (draw_tool, TRUE,
                                        rect.x, rect.y,
                                        rect.width, rect.height);
        }
    }
  while (pango_layout_iter_next_char (iter));

  picman_draw_tool_pop_group (draw_tool);

  pango_layout_iter_free (iter);
}

static void
picman_text_tool_frame_item (PicmanTextTool *text_tool)
{
  g_return_if_fail (PICMAN_IS_LAYER (text_tool->layer));

  text_tool->handle_rectangle_change_complete = FALSE;

  picman_rectangle_tool_frame_item (PICMAN_RECTANGLE_TOOL (text_tool),
                                  PICMAN_ITEM (text_tool->layer));

  text_tool->handle_rectangle_change_complete = TRUE;
}

static gboolean
picman_text_tool_rectangle_change_complete (PicmanRectangleTool *rect_tool)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (rect_tool);

  picman_text_tool_editor_position (text_tool);

  if (text_tool->handle_rectangle_change_complete)
    {
      PicmanItem *item = PICMAN_ITEM (text_tool->layer);
      gint      x1, y1;
      gint      x2, y2;

      if (! item)
        {
          /* we can't set properties for the text layer, because it
           * isn't created until some text has been inserted, so we
           * need to make a special note that will remind us what to
           * do when we actually create the layer
           */
          text_tool->text_box_fixed = TRUE;

          return TRUE;
        }

      g_object_get (rect_tool,
                    "x1", &x1,
                    "y1", &y1,
                    "x2", &x2,
                    "y2", &y2,
                    NULL);

      if (x1        != picman_item_get_offset_x (item) ||
          y1        != picman_item_get_offset_y (item) ||
          (x2 - x1) != picman_item_get_width  (item)   ||
          (y2 - y1) != picman_item_get_height (item))
        {
          PicmanUnit  box_unit = text_tool->proxy->box_unit;
          gdouble   xres, yres;
          gboolean  push_undo = TRUE;
          PicmanUndo *undo;

          picman_image_get_resolution (text_tool->image, &xres, &yres);

          g_object_set (text_tool->proxy,
                        "box-mode",   PICMAN_TEXT_BOX_FIXED,
                        "box-width",  picman_pixels_to_units (x2 - x1,
                                                            box_unit, xres),
                        "box-height", picman_pixels_to_units (y2 - y1,
                                                            box_unit, yres),
                        NULL);

          undo = picman_image_undo_can_compress (text_tool->image,
                                               PICMAN_TYPE_UNDO_STACK,
                                               PICMAN_UNDO_GROUP_TEXT);

          if (undo &&
              picman_undo_get_age (undo) <= TEXT_UNDO_TIMEOUT &&
              g_object_get_data (G_OBJECT (undo), "reshape-text-layer") == (gpointer) item)
            push_undo = FALSE;

          if (push_undo)
            {
              picman_image_undo_group_start (text_tool->image, PICMAN_UNDO_GROUP_TEXT,
                                           _("Reshape Text Layer"));

              undo = picman_image_undo_can_compress (text_tool->image, PICMAN_TYPE_UNDO_STACK,
                                                   PICMAN_UNDO_GROUP_TEXT);

              if (undo)
                g_object_set_data (G_OBJECT (undo), "reshape-text-layer",
                                   (gpointer) item);
            }

          picman_item_translate (item,
                               x1 - picman_item_get_offset_x (item),
                               y1 - picman_item_get_offset_y (item),
                               push_undo);
          picman_text_tool_apply (text_tool, push_undo);

          if (push_undo)
            picman_image_undo_group_end (text_tool->image);
        }
    }

  return TRUE;
}

static void
picman_text_tool_connect (PicmanTextTool  *text_tool,
                        PicmanTextLayer *layer,
                        PicmanText      *text)
{
  PicmanTool *tool = PICMAN_TOOL (text_tool);

  g_return_if_fail (text == NULL || (layer != NULL && layer->text == text));

  if (text_tool->text != text)
    {
      PicmanTextOptions *options = PICMAN_TEXT_TOOL_GET_OPTIONS (tool);

      g_signal_handlers_block_by_func (text_tool->buffer,
                                       picman_text_tool_buffer_begin_edit,
                                       text_tool);
      g_signal_handlers_block_by_func (text_tool->buffer,
                                       picman_text_tool_buffer_end_edit,
                                       text_tool);

      if (text_tool->text)
        {
          g_signal_handlers_disconnect_by_func (text_tool->text,
                                                picman_text_tool_text_notify,
                                                text_tool);
          g_signal_handlers_disconnect_by_func (text_tool->text,
                                                picman_text_tool_text_changed,
                                                text_tool);

          if (text_tool->pending)
            picman_text_tool_apply (text_tool, TRUE);

          g_object_unref (text_tool->text);
          text_tool->text = NULL;

          g_object_set (text_tool->proxy,
                        "text",   NULL,
                        "markup", NULL,
                        NULL);
          picman_text_buffer_set_text (text_tool->buffer, NULL);

          picman_text_tool_clear_layout (text_tool);
        }

      picman_context_define_property (PICMAN_CONTEXT (options),
                                    PICMAN_CONTEXT_PROP_FOREGROUND,
                                    text != NULL);

      if (text)
        {
          if (text->unit != text_tool->proxy->unit)
            picman_size_entry_set_unit (PICMAN_SIZE_ENTRY (options->size_entry),
                                      text->unit);

          picman_config_sync (G_OBJECT (text), G_OBJECT (text_tool->proxy), 0);

          if (text->markup)
            picman_text_buffer_set_markup (text_tool->buffer, text->markup);
          else
            picman_text_buffer_set_text (text_tool->buffer, text->text);

          picman_text_tool_clear_layout (text_tool);

          text_tool->text = g_object_ref (text);

          g_signal_connect (text, "notify",
                            G_CALLBACK (picman_text_tool_text_notify),
                            text_tool);
          g_signal_connect (text, "changed",
                            G_CALLBACK (picman_text_tool_text_changed),
                            text_tool);
        }

      g_signal_handlers_unblock_by_func (text_tool->buffer,
                                         picman_text_tool_buffer_end_edit,
                                         text_tool);
      g_signal_handlers_unblock_by_func (text_tool->buffer,
                                         picman_text_tool_buffer_begin_edit,
                                         text_tool);
    }

  if (text_tool->layer != layer)
    {
      if (text_tool->layer)
        g_signal_handlers_disconnect_by_func (text_tool->layer,
                                              picman_text_tool_layer_notify,
                                              text_tool);

      text_tool->layer = layer;

      if (layer)
        g_signal_connect_object (text_tool->layer, "notify",
                                 G_CALLBACK (picman_text_tool_layer_notify),
                                 text_tool, 0);
    }
}

static void
picman_text_tool_layer_notify (PicmanTextLayer    *layer,
                             const GParamSpec *pspec,
                             PicmanTextTool     *text_tool)
{
  PicmanTool *tool = PICMAN_TOOL (text_tool);

  if (! strcmp (pspec->name, "modified"))
    {
      if (layer->modified)
        picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, tool->display);
    }
  else if (! strcmp (pspec->name, "text"))
    {
      if (! layer->text)
        picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, tool->display);
    }
}

static gboolean
picman_text_tool_apply_idle (gpointer text_tool)
{
  return picman_text_tool_apply (text_tool, TRUE);
}

static void
picman_text_tool_proxy_notify (PicmanText         *text,
                             const GParamSpec *pspec,
                             PicmanTextTool     *text_tool)
{
  if (! text_tool->text)
    return;

  if ((pspec->flags & G_PARAM_READWRITE) == G_PARAM_READWRITE &&
      pspec->owner_type == PICMAN_TYPE_TEXT)
    {
      picman_text_tool_block_drawing (text_tool);

      text_tool->pending = g_list_append (text_tool->pending, (gpointer) pspec);

      if (text_tool->idle_id)
        g_source_remove (text_tool->idle_id);

      text_tool->idle_id =
        g_idle_add_full (G_PRIORITY_LOW,
                         picman_text_tool_apply_idle, text_tool,
                         NULL);
    }
}

static void
picman_text_tool_text_notify (PicmanText         *text,
                            const GParamSpec *pspec,
                            PicmanTextTool     *text_tool)
{
  g_return_if_fail (text == text_tool->text);

  picman_text_tool_block_drawing (text_tool);

  if ((pspec->flags & G_PARAM_READWRITE) == G_PARAM_READWRITE)
    {
      GValue value = { 0, };

      g_value_init (&value, pspec->value_type);

      g_object_get_property (G_OBJECT (text), pspec->name, &value);

      g_signal_handlers_block_by_func (text_tool->proxy,
                                       picman_text_tool_proxy_notify,
                                       text_tool);

      g_object_set_property (G_OBJECT (text_tool->proxy), pspec->name, &value);

      g_signal_handlers_unblock_by_func (text_tool->proxy,
                                         picman_text_tool_proxy_notify,
                                         text_tool);

      g_value_unset (&value);
    }

  /* if the text has changed, (probably because of an undo), we put
   * the new text into the text buffer
   */
  if (strcmp (pspec->name, "text")   == 0 ||
      strcmp (pspec->name, "markup") == 0)
    {
      g_signal_handlers_block_by_func (text_tool->buffer,
                                       picman_text_tool_buffer_begin_edit,
                                       text_tool);
      g_signal_handlers_block_by_func (text_tool->buffer,
                                       picman_text_tool_buffer_end_edit,
                                       text_tool);
      if (text->markup)
        picman_text_buffer_set_markup (text_tool->buffer, text->markup);
      else
        picman_text_buffer_set_text (text_tool->buffer, text->text);

      g_signal_handlers_unblock_by_func (text_tool->buffer,
                                         picman_text_tool_buffer_end_edit,
                                         text_tool);
      g_signal_handlers_unblock_by_func (text_tool->buffer,
                                         picman_text_tool_buffer_begin_edit,
                                         text_tool);
    }
}

static void
picman_text_tool_text_changed (PicmanText     *text,
                             PicmanTextTool *text_tool)
{
  /* we need to redraw the rectangle in any case because whatever
   * changes to the text can change its size
   */
  picman_text_tool_frame_item (text_tool);

  picman_text_tool_unblock_drawing (text_tool);
}

static gboolean
picman_text_tool_apply (PicmanTextTool *text_tool,
                      gboolean      push_undo)
{
  const GParamSpec *pspec = NULL;
  PicmanImage        *image;
  PicmanTextLayer    *layer;
  GObject          *src;
  GObject          *dest;
  GList            *list;
  gboolean          undo_group = FALSE;

  if (text_tool->idle_id)
    {
      g_source_remove (text_tool->idle_id);
      text_tool->idle_id = 0;
    }

  g_return_val_if_fail (text_tool->text != NULL, FALSE);
  g_return_val_if_fail (text_tool->layer != NULL, FALSE);

  layer = text_tool->layer;
  image = picman_item_get_image (PICMAN_ITEM (layer));

  g_return_val_if_fail (layer->text == text_tool->text, FALSE);

  /*  Walk over the list of changes and figure out if we are changing
   *  a single property or need to push a full text undo.
   */
  for (list = text_tool->pending;
       list && list->next && list->next->data == list->data;
       list = list->next)
    /* do nothing */;

  if (g_list_length (list) == 1)
    pspec = list->data;

  /*  If we are changing a single property, we don't need to push
   *  an undo if all of the following is true:
   *   - the redo stack is empty
   *   - the last item on the undo stack is a text undo
   *   - the last undo changed the same text property on the same layer
   *   - the last undo happened less than TEXT_UNDO_TIMEOUT seconds ago
   */
  if (pspec)
    {
      PicmanUndo *undo = picman_image_undo_can_compress (image, PICMAN_TYPE_TEXT_UNDO,
                                                     PICMAN_UNDO_TEXT_LAYER);

      if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (layer))
        {
          PicmanTextUndo *text_undo = PICMAN_TEXT_UNDO (undo);

          if (text_undo->pspec == pspec)
            {
              if (picman_undo_get_age (undo) < TEXT_UNDO_TIMEOUT)
                {
                  PicmanTool    *tool = PICMAN_TOOL (text_tool);
                  PicmanContext *context;

                  context = PICMAN_CONTEXT (picman_tool_get_options (tool));

                  push_undo = FALSE;
                  picman_undo_reset_age (undo);
                  picman_undo_refresh_preview (undo, context);
                }
            }
        }
    }

  if (push_undo)
    {
      if (layer->modified)
        {
          undo_group = TRUE;
          picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TEXT, NULL);

          picman_image_undo_push_text_layer_modified (image, NULL, layer);

          /*  see comment in picman_text_layer_set()  */
          picman_image_undo_push_drawable_mod (image, NULL,
                                             PICMAN_DRAWABLE (layer), TRUE);
        }

      picman_image_undo_push_text_layer (image, NULL, layer, pspec);
    }

  src  = G_OBJECT (text_tool->proxy);
  dest = G_OBJECT (text_tool->text);

  g_signal_handlers_block_by_func (dest,
                                   picman_text_tool_text_notify,
                                   text_tool);
  g_signal_handlers_block_by_func (dest,
                                   picman_text_tool_text_changed,
                                   text_tool);

  g_object_freeze_notify (dest);

  for (; list; list = g_list_next (list))
    {
      GValue value = { 0, };

      /*  look ahead and compress changes  */
      if (list->next && list->next->data == list->data)
        continue;

      pspec = list->data;

      g_value_init (&value, pspec->value_type);

      g_object_get_property (src,  pspec->name, &value);
      g_object_set_property (dest, pspec->name, &value);

      g_value_unset (&value);
    }

  g_list_free (text_tool->pending);
  text_tool->pending = NULL;

  g_object_thaw_notify (dest);

  g_signal_handlers_unblock_by_func (dest,
                                     picman_text_tool_text_notify,
                                     text_tool);
  g_signal_handlers_unblock_by_func (dest,
                                     picman_text_tool_text_changed,
                                     text_tool);

  if (push_undo)
    {
      g_object_set (layer, "modified", FALSE, NULL);

      if (undo_group)
        picman_image_undo_group_end (image);
    }

  picman_text_tool_frame_item (text_tool);

  picman_image_flush (image);

  picman_text_tool_unblock_drawing (text_tool);

  return FALSE;
}

static void
picman_text_tool_create_layer (PicmanTextTool *text_tool,
                             PicmanText     *text)
{
  PicmanRectangleTool *rect_tool = PICMAN_RECTANGLE_TOOL (text_tool);
  PicmanTool          *tool      = PICMAN_TOOL (text_tool);
  PicmanImage         *image     = picman_display_get_image (tool->display);
  PicmanLayer         *layer;
  gint               x1, y1;
  gint               x2, y2;

  picman_text_tool_block_drawing (text_tool);

  if (text)
    {
      text = picman_config_duplicate (PICMAN_CONFIG (text));
    }
  else
    {
      gchar *string;

      if (picman_text_buffer_has_markup (text_tool->buffer))
        {
          string = picman_text_buffer_get_markup (text_tool->buffer);

          g_object_set (text_tool->proxy,
                        "markup",   string,
                        "box-mode", PICMAN_TEXT_BOX_DYNAMIC,
                        NULL);
        }
      else
        {
          string = picman_text_buffer_get_text (text_tool->buffer);

          g_object_set (text_tool->proxy,
                        "text",     string,
                        "box-mode", PICMAN_TEXT_BOX_DYNAMIC,
                        NULL);
        }

      g_free (string);

      text = picman_config_duplicate (PICMAN_CONFIG (text_tool->proxy));
    }

  layer = picman_text_layer_new (image, text);

  g_object_unref (text);

  if (! layer)
    {
      picman_text_tool_unblock_drawing (text_tool);
      return;
    }

  picman_text_tool_connect (text_tool, PICMAN_TEXT_LAYER (layer), text);

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TEXT,
                               _("Add Text Layer"));

  if (picman_image_get_floating_selection (image))
    {
      g_signal_handlers_block_by_func (image,
                                       picman_text_tool_layer_changed,
                                       text_tool);

      floating_sel_anchor (picman_image_get_floating_selection (image));

      g_signal_handlers_unblock_by_func (image,
                                         picman_text_tool_layer_changed,
                                         text_tool);
    }

  g_object_get (rect_tool,
                "x1", &x1,
                "y1", &y1,
                "x2", &x2,
                "y2", &y2,
                NULL);

  picman_item_set_offset (PICMAN_ITEM (layer), x1, y1);

  picman_image_add_layer (image, layer,
                        PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  if (text_tool->text_box_fixed)
    {
      PicmanUnit box_unit = text_tool->proxy->box_unit;
      gdouble  xres, yres;

      picman_image_get_resolution (image, &xres, &yres);

      g_object_set (text_tool->proxy,
                    "box-mode",   PICMAN_TEXT_BOX_FIXED,
                    "box-width",  picman_pixels_to_units (x2 - x1,
                                                        box_unit, xres),
                    "box-height", picman_pixels_to_units (y2 - y1,
                                                        box_unit, yres),
                    NULL);

      picman_text_tool_apply (text_tool, TRUE); /* unblocks drawing */
    }
  else
    {
      picman_text_tool_frame_item (text_tool);

      picman_text_tool_unblock_drawing (text_tool);
    }

  picman_image_undo_group_end (image);

  picman_image_flush (image);

  picman_text_tool_set_drawable (text_tool, PICMAN_DRAWABLE (layer), FALSE);
}

#define  RESPONSE_NEW 1

static void
picman_text_tool_confirm_response (GtkWidget    *widget,
                                 gint          response_id,
                                 PicmanTextTool *text_tool)
{
  PicmanTextLayer *layer = text_tool->layer;

  gtk_widget_destroy (widget);

  if (layer && layer->text)
    {
      switch (response_id)
        {
        case RESPONSE_NEW:
          picman_text_tool_create_layer (text_tool, layer->text);
          break;

        case GTK_RESPONSE_ACCEPT:
          picman_text_tool_connect (text_tool, layer, layer->text);

          /*  cause the text layer to be rerendered  */
          g_object_notify (G_OBJECT (text_tool->proxy), "markup");

          picman_text_tool_editor_start (text_tool);
          break;

        default:
          break;
        }
    }
}

static void
picman_text_tool_confirm_dialog (PicmanTextTool *text_tool)
{
  PicmanTool         *tool  = PICMAN_TOOL (text_tool);
  PicmanDisplayShell *shell = picman_display_get_shell (tool->display);
  GtkWidget        *dialog;
  GtkWidget        *vbox;
  GtkWidget        *label;

  g_return_if_fail (text_tool->layer != NULL);

  if (text_tool->confirm_dialog)
    {
      gtk_window_present (GTK_WINDOW (text_tool->confirm_dialog));
      return;
    }

  dialog = picman_viewable_dialog_new (PICMAN_VIEWABLE (text_tool->layer),
                                     PICMAN_CONTEXT (picman_tool_get_options (tool)),
                                     _("Confirm Text Editing"),
                                     "picman-text-tool-confirm",
                                     PICMAN_STOCK_TEXT_LAYER,
                                     _("Confirm Text Editing"),
                                     GTK_WIDGET (shell),
                                     picman_standard_help_func, NULL,

                                     _("Create _New Layer"), RESPONSE_NEW,
                                     GTK_STOCK_CANCEL,       GTK_RESPONSE_CANCEL,
                                     GTK_STOCK_EDIT,         GTK_RESPONSE_ACCEPT,

                                     NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           RESPONSE_NEW,
                                           GTK_RESPONSE_ACCEPT,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (picman_text_tool_confirm_response),
                    text_tool);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  label = gtk_label_new (_("The layer you selected is a text layer but "
                           "it has been modified using other tools. "
                           "Editing the layer with the text tool will "
                           "discard these modifications."
                           "\n\n"
                           "You can edit the layer or create a new "
                           "text layer from its text attributes."));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  gtk_widget_show (dialog);

  text_tool->confirm_dialog = dialog;
  g_signal_connect_swapped (dialog, "destroy",
                            G_CALLBACK (g_nullify_pointer),
                            &text_tool->confirm_dialog);
}

static void
picman_text_tool_layer_changed (PicmanImage    *image,
                              PicmanTextTool *text_tool)
{
  PicmanLayer *layer = picman_image_get_active_layer (image);

  if (layer == PICMAN_LAYER (text_tool->layer))
    return;

  /* all this stuff doesn't work quite yet, but it's better than before
   */

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (text_tool));

  picman_text_tool_editor_halt (text_tool);
  picman_text_tool_clear_layout (text_tool);

  if (picman_draw_tool_is_active (PICMAN_DRAW_TOOL (text_tool)))
    picman_draw_tool_stop (PICMAN_DRAW_TOOL (text_tool));

  if (picman_text_tool_set_drawable (text_tool, PICMAN_DRAWABLE (layer), FALSE) &&
      PICMAN_LAYER (text_tool->layer) == layer)
    {
      picman_draw_tool_start (PICMAN_DRAW_TOOL (text_tool),
                            PICMAN_TOOL (text_tool)->display);

      picman_text_tool_frame_item (text_tool);
      picman_text_tool_editor_start (text_tool);
      picman_text_tool_editor_position (text_tool);
    }
  else
    {
      picman_tool_control (PICMAN_TOOL (text_tool), PICMAN_TOOL_ACTION_HALT,
                         PICMAN_TOOL (text_tool)->display);
    }

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (text_tool));
}

static void
picman_text_tool_set_image (PicmanTextTool *text_tool,
                          PicmanImage    *image)
{
  if (text_tool->image == image)
    return;

  if (text_tool->image)
    {
      g_signal_handlers_disconnect_by_func (text_tool->image,
                                            picman_text_tool_layer_changed,
                                            text_tool);

      g_object_remove_weak_pointer (G_OBJECT (text_tool->image),
                                    (gpointer) &text_tool->image);
    }

  text_tool->image = image;

  if (image)
    {
      PicmanTextOptions *options = PICMAN_TEXT_TOOL_GET_OPTIONS (text_tool);
      gdouble          xres;
      gdouble          yres;

      g_object_add_weak_pointer (G_OBJECT (text_tool->image),
                                 (gpointer) &text_tool->image);

      g_signal_connect_object (text_tool->image, "active-layer-changed",
                               G_CALLBACK (picman_text_tool_layer_changed),
                               text_tool, 0);

      picman_image_get_resolution (image, &xres, &yres);
      picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (options->size_entry), 0,
                                      yres, FALSE);
    }
}

static gboolean
picman_text_tool_set_drawable (PicmanTextTool *text_tool,
                             PicmanDrawable *drawable,
                             gboolean      confirm)
{
  PicmanImage *image = NULL;

  if (text_tool->confirm_dialog)
    gtk_widget_destroy (text_tool->confirm_dialog);

  if (drawable)
    image = picman_item_get_image (PICMAN_ITEM (drawable));

  picman_text_tool_set_image (text_tool, image);

  if (PICMAN_IS_TEXT_LAYER (drawable) && PICMAN_TEXT_LAYER (drawable)->text)
    {
      PicmanTextLayer *layer = PICMAN_TEXT_LAYER (drawable);

      if (layer == text_tool->layer && layer->text == text_tool->text)
        return TRUE;

      if (layer->modified)
        {
          if (confirm)
            {
              picman_text_tool_connect (text_tool, layer, NULL);
              picman_text_tool_confirm_dialog (text_tool);
              return TRUE;
            }
        }
      else
        {
          picman_text_tool_connect (text_tool, layer, layer->text);
          return TRUE;
        }
    }

  picman_text_tool_connect (text_tool, NULL, NULL);

  return FALSE;
}

static void
picman_text_tool_block_drawing (PicmanTextTool *text_tool)
{
  if (! text_tool->drawing_blocked)
    {
      picman_draw_tool_pause (PICMAN_DRAW_TOOL (text_tool));

      picman_text_tool_clear_layout (text_tool);

      text_tool->drawing_blocked = TRUE;
    }
}

static void
picman_text_tool_unblock_drawing (PicmanTextTool *text_tool)
{
  g_return_if_fail (text_tool->drawing_blocked == TRUE);

  text_tool->drawing_blocked = FALSE;

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (text_tool));
}

static void
picman_text_tool_buffer_begin_edit (PicmanTextBuffer *buffer,
                                  PicmanTextTool   *text_tool)
{
  picman_text_tool_block_drawing (text_tool);
}

static void
picman_text_tool_buffer_end_edit (PicmanTextBuffer *buffer,
                                PicmanTextTool   *text_tool)
{
  if (text_tool->text)
    {
      gchar *string;

      if (picman_text_buffer_has_markup (buffer))
        {
          string = picman_text_buffer_get_markup (buffer);

          g_object_set (text_tool->proxy,
                        "markup", string,
                        NULL);
        }
      else
        {
          string = picman_text_buffer_get_text (buffer);

          g_object_set (text_tool->proxy,
                        "text", string,
                        NULL);
        }

      g_free (string);
    }
  else
    {
      picman_text_tool_create_layer (text_tool, NULL);
    }
}


/*  public functions  */

void
picman_text_tool_clear_layout (PicmanTextTool *text_tool)
{
  if (text_tool->layout)
    {
      g_object_unref (text_tool->layout);
      text_tool->layout = NULL;
    }
}

gboolean
picman_text_tool_ensure_layout (PicmanTextTool *text_tool)
{
  if (! text_tool->layout && text_tool->text)
    {
      PicmanImage *image = picman_item_get_image (PICMAN_ITEM (text_tool->layer));
      gdouble    xres;
      gdouble    yres;

      picman_image_get_resolution (image, &xres, &yres);

      text_tool->layout = picman_text_layout_new (text_tool->layer->text,
                                                xres, yres);
    }

  return text_tool->layout != NULL;
}

void
picman_text_tool_set_layer (PicmanTextTool *text_tool,
                          PicmanLayer    *layer)
{
  g_return_if_fail (PICMAN_IS_TEXT_TOOL (text_tool));
  g_return_if_fail (layer == NULL || PICMAN_IS_LAYER (layer));

  if (picman_text_tool_set_drawable (text_tool, PICMAN_DRAWABLE (layer), TRUE))
    {
      PicmanTool    *tool = PICMAN_TOOL (text_tool);
      PicmanItem    *item = PICMAN_ITEM (layer);
      PicmanContext *context;
      PicmanDisplay *display;

      context = picman_get_user_context (tool->tool_info->picman);
      display = picman_context_get_display (context);

      if (! display ||
          picman_display_get_image (display) != picman_item_get_image (item))
        {
          GList *list;

          display = NULL;

          for (list = picman_get_display_iter (tool->tool_info->picman);
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
        {
          PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tool);

          tool->drawable = PICMAN_DRAWABLE (layer);

          if (picman_draw_tool_is_active (draw_tool))
            picman_draw_tool_stop (draw_tool);

          picman_draw_tool_start (draw_tool, display);

          picman_text_tool_frame_item (text_tool);

          picman_text_tool_editor_start (text_tool);
        }
    }
}

gboolean
picman_text_tool_get_has_text_selection (PicmanTextTool *text_tool)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (text_tool->buffer);

  return gtk_text_buffer_get_has_selection (buffer);
}

void
picman_text_tool_delete_selection (PicmanTextTool *text_tool)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (text_tool->buffer);

  if (gtk_text_buffer_get_has_selection (buffer))
    {
      gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
    }
}

void
picman_text_tool_cut_clipboard (PicmanTextTool *text_tool)
{
  PicmanDisplayShell *shell;
  GtkClipboard     *clipboard;

  g_return_if_fail (PICMAN_IS_TEXT_TOOL (text_tool));

  shell = picman_display_get_shell (PICMAN_TOOL (text_tool)->display);

  clipboard = gtk_widget_get_clipboard (GTK_WIDGET (shell),
                                        GDK_SELECTION_CLIPBOARD);

  gtk_text_buffer_cut_clipboard (GTK_TEXT_BUFFER (text_tool->buffer),
                                 clipboard, TRUE);
}

void
picman_text_tool_copy_clipboard (PicmanTextTool *text_tool)
{
  PicmanDisplayShell *shell;
  GtkClipboard     *clipboard;

  g_return_if_fail (PICMAN_IS_TEXT_TOOL (text_tool));

  shell = picman_display_get_shell (PICMAN_TOOL (text_tool)->display);

  clipboard = gtk_widget_get_clipboard (GTK_WIDGET (shell),
                                        GDK_SELECTION_CLIPBOARD);

  /*  need to block "end-user-action" on the text buffer, because
   *  GtkTextBuffer considers copying text to the clipboard an
   *  undo-relevant user action, which is clearly a bug, but what
   *  can we do...
   */
  g_signal_handlers_block_by_func (text_tool->buffer,
                                   picman_text_tool_buffer_begin_edit,
                                   text_tool);
  g_signal_handlers_block_by_func (text_tool->buffer,
                                   picman_text_tool_buffer_end_edit,
                                   text_tool);

  gtk_text_buffer_copy_clipboard (GTK_TEXT_BUFFER (text_tool->buffer),
                                  clipboard);

  g_signal_handlers_unblock_by_func (text_tool->buffer,
                                     picman_text_tool_buffer_end_edit,
                                     text_tool);
  g_signal_handlers_unblock_by_func (text_tool->buffer,
                                     picman_text_tool_buffer_begin_edit,
                                     text_tool);
}

void
picman_text_tool_paste_clipboard (PicmanTextTool *text_tool)
{
  PicmanDisplayShell *shell;
  GtkClipboard     *clipboard;

  g_return_if_fail (PICMAN_IS_TEXT_TOOL (text_tool));

  shell = picman_display_get_shell (PICMAN_TOOL (text_tool)->display);

  clipboard = gtk_widget_get_clipboard (GTK_WIDGET (shell),
                                        GDK_SELECTION_CLIPBOARD);

  gtk_text_buffer_paste_clipboard (GTK_TEXT_BUFFER (text_tool->buffer),
                                   clipboard, NULL, TRUE);
}

void
picman_text_tool_create_vectors (PicmanTextTool *text_tool)
{
  PicmanVectors *vectors;

  g_return_if_fail (PICMAN_IS_TEXT_TOOL (text_tool));

  if (! text_tool->text || ! text_tool->image)
    return;

  vectors = picman_text_vectors_new (text_tool->image, text_tool->text);

  if (text_tool->layer)
    {
      gint x, y;

      picman_item_get_offset (PICMAN_ITEM (text_tool->layer), &x, &y);
      picman_item_translate (PICMAN_ITEM (vectors), x, y, FALSE);
    }

  picman_image_add_vectors (text_tool->image, vectors,
                          PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  picman_image_flush (text_tool->image);
}

void
picman_text_tool_create_vectors_warped (PicmanTextTool *text_tool)
{
  PicmanVectors *vectors0;
  PicmanVectors *vectors;
  gdouble      box_height;

  g_return_if_fail (PICMAN_IS_TEXT_TOOL (text_tool));

  if (! text_tool->text || ! text_tool->image || ! text_tool->layer)
    return;

  box_height = picman_item_get_height (PICMAN_ITEM (text_tool->layer));

  vectors0 = picman_image_get_active_vectors (text_tool->image);
  if (! vectors0)
    return;

  vectors = picman_text_vectors_new (text_tool->image, text_tool->text);

  picman_vectors_warp_vectors (vectors0, vectors, 0.5 * box_height);

  picman_item_set_visible (PICMAN_ITEM (vectors), TRUE, FALSE);

  picman_image_add_vectors (text_tool->image, vectors,
                          PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  picman_image_flush (text_tool->image);
}
