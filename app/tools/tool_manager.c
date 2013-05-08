/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanlist.h"
#include "core/picmanimage.h"
#include "core/picmantoolinfo.h"
#include "core/picmantoolpreset.h"

#include "paint/picmanpaintoptions.h"

#include "display/picmandisplay.h"

#include "picmantool.h"
#include "picmantoolcontrol.h"
#include "tool_manager.h"


typedef struct _PicmanToolManager PicmanToolManager;

struct _PicmanToolManager
{
  PicmanTool         *active_tool;
  PicmanPaintOptions *shared_paint_options;
  GSList           *tool_stack;

  GQuark            image_clean_handler_id;
  GQuark            image_dirty_handler_id;
};


/*  local function prototypes  */

static PicmanToolManager * tool_manager_get     (Picman            *picman);
static void              tool_manager_set     (Picman            *picman,
                                               PicmanToolManager *tool_manager);
static void   tool_manager_tool_changed       (PicmanContext     *user_context,
                                               PicmanToolInfo    *tool_info,
                                               PicmanToolManager *tool_manager);
static void   tool_manager_preset_changed     (PicmanContext     *user_context,
                                               PicmanToolPreset  *preset,
                                               PicmanToolManager *tool_manager);
static void   tool_manager_image_clean_dirty  (PicmanImage       *image,
                                               PicmanDirtyMask    dirty_mask,
                                               PicmanToolManager *tool_manager);

static void   tool_manager_connect_options    (PicmanToolManager *tool_manager,
                                               PicmanContext     *user_context,
                                               PicmanToolInfo    *tool_info);
static void   tool_manager_disconnect_options (PicmanToolManager *tool_manager,
                                               PicmanContext     *user_context,
                                               PicmanToolInfo    *tool_info);


/*  public functions  */

void
tool_manager_init (Picman *picman)
{
  PicmanToolManager *tool_manager;
  PicmanContext     *user_context;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = g_slice_new0 (PicmanToolManager);

  tool_manager->active_tool            = NULL;
  tool_manager->tool_stack             = NULL;
  tool_manager->image_clean_handler_id = 0;
  tool_manager->image_dirty_handler_id = 0;

  tool_manager_set (picman, tool_manager);

  tool_manager->image_clean_handler_id =
    picman_container_add_handler (picman->images, "clean",
                                G_CALLBACK (tool_manager_image_clean_dirty),
                                tool_manager);

  tool_manager->image_dirty_handler_id =
    picman_container_add_handler (picman->images, "dirty",
                                G_CALLBACK (tool_manager_image_clean_dirty),
                                tool_manager);

  user_context = picman_get_user_context (picman);

  tool_manager->shared_paint_options = g_object_new (PICMAN_TYPE_PAINT_OPTIONS,
                                                     "picman", picman,
                                                     "name", "tmp",
                                                     NULL);

  g_signal_connect (user_context, "tool-changed",
                    G_CALLBACK (tool_manager_tool_changed),
                    tool_manager);
  g_signal_connect (user_context, "tool-preset-changed",
                    G_CALLBACK (tool_manager_preset_changed),
                    tool_manager);
}

void
tool_manager_exit (Picman *picman)
{
  PicmanToolManager *tool_manager;
  PicmanContext     *user_context;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);
  tool_manager_set (picman, NULL);

  user_context = picman_get_user_context (picman);

  g_signal_handlers_disconnect_by_func (user_context,
                                        tool_manager_tool_changed,
                                        tool_manager);
  g_signal_handlers_disconnect_by_func (user_context,
                                        tool_manager_preset_changed,
                                        tool_manager);

  picman_container_remove_handler (picman->images,
                                 tool_manager->image_clean_handler_id);
  picman_container_remove_handler (picman->images,
                                 tool_manager->image_dirty_handler_id);

  if (tool_manager->active_tool)
    g_object_unref (tool_manager->active_tool);

  if (tool_manager->shared_paint_options)
    g_object_unref (tool_manager->shared_paint_options);

  g_slice_free (PicmanToolManager, tool_manager);
}

PicmanTool *
tool_manager_get_active (Picman *picman)
{
  PicmanToolManager *tool_manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  tool_manager = tool_manager_get (picman);

  return tool_manager->active_tool;
}

void
tool_manager_select_tool (Picman     *picman,
                          PicmanTool *tool)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_TOOL (tool));

  tool_manager = tool_manager_get (picman);

  /*  reset the previously selected tool, but only if it is not only
   *  temporarily pushed to the tool stack
   */
  if (tool_manager->active_tool &&
      ! (tool_manager->tool_stack &&
         tool_manager->active_tool == tool_manager->tool_stack->data))
    {
      PicmanTool    *active_tool = tool_manager->active_tool;
      PicmanDisplay *display;

      /*  NULL image returns any display (if there is any)  */
      display = picman_tool_has_image (active_tool, NULL);

      tool_manager_control_active (picman, PICMAN_TOOL_ACTION_HALT, display);
      tool_manager_focus_display_active (picman, NULL);

      g_object_unref (tool_manager->active_tool);
    }

  tool_manager->active_tool = g_object_ref (tool);
}

void
tool_manager_push_tool (Picman     *picman,
                        PicmanTool *tool)
{
  PicmanToolManager *tool_manager;
  PicmanDisplay     *focus_display = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_TOOL (tool));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      focus_display = tool_manager->active_tool->focus_display;

      tool_manager->tool_stack = g_slist_prepend (tool_manager->tool_stack,
                                                  tool_manager->active_tool);

      g_object_ref (tool_manager->tool_stack->data);
    }

  tool_manager_select_tool (picman, tool);

  if (focus_display)
    tool_manager_focus_display_active (picman, focus_display);
}

void
tool_manager_pop_tool (Picman *picman)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->tool_stack)
    {
      PicmanTool *tool = tool_manager->tool_stack->data;

      tool_manager->tool_stack = g_slist_remove (tool_manager->tool_stack,
                                                 tool);

      tool_manager_select_tool (picman, tool);

      g_object_unref (tool);
    }
}

gboolean
tool_manager_initialize_active (Picman        *picman,
                                PicmanDisplay *display)
{
  PicmanToolManager *tool_manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), FALSE);

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      PicmanTool *tool = tool_manager->active_tool;

      if (picman_tool_initialize (tool, display))
        {
          PicmanImage *image = picman_display_get_image (display);

          tool->drawable = picman_image_get_active_drawable (image);

          return TRUE;
        }
    }

  return FALSE;
}

void
tool_manager_control_active (Picman           *picman,
                             PicmanToolAction  action,
                             PicmanDisplay    *display)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      PicmanTool *tool = tool_manager->active_tool;

      if (display && picman_tool_has_display (tool, display))
        {
          picman_tool_control (tool, action, display);
        }
      else if (action == PICMAN_TOOL_ACTION_HALT)
        {
          if (picman_tool_control_is_active (tool->control))
            picman_tool_control_halt (tool->control);
        }
    }
}

void
tool_manager_button_press_active (Picman                *picman,
                                  const PicmanCoords    *coords,
                                  guint32              time,
                                  GdkModifierType      state,
                                  PicmanButtonPressType  press_type,
                                  PicmanDisplay         *display)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      picman_tool_button_press (tool_manager->active_tool,
                              coords, time, state, press_type,
                              display);
    }
}

void
tool_manager_button_release_active (Picman             *picman,
                                    const PicmanCoords *coords,
                                    guint32           time,
                                    GdkModifierType   state,
                                    PicmanDisplay      *display)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      picman_tool_button_release (tool_manager->active_tool,
                                coords, time, state,
                                display);
    }
}

void
tool_manager_motion_active (Picman             *picman,
                            const PicmanCoords *coords,
                            guint32           time,
                            GdkModifierType   state,
                            PicmanDisplay      *display)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      picman_tool_motion (tool_manager->active_tool,
                        coords, time, state,
                        display);
    }
}

gboolean
tool_manager_key_press_active (Picman        *picman,
                               GdkEventKey *kevent,
                               PicmanDisplay *display)
{
  PicmanToolManager *tool_manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      return picman_tool_key_press (tool_manager->active_tool,
                                  kevent,
                                  display);
    }

  return FALSE;
}

gboolean
tool_manager_key_release_active (Picman        *picman,
                                 GdkEventKey *kevent,
                                 PicmanDisplay *display)
{
  PicmanToolManager *tool_manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      return picman_tool_key_release (tool_manager->active_tool,
                                    kevent,
                                    display);
    }

  return FALSE;
}

void
tool_manager_focus_display_active (Picman        *picman,
                                   PicmanDisplay *display)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      picman_tool_set_focus_display (tool_manager->active_tool,
                                   display);
    }
}

void
tool_manager_modifier_state_active (Picman            *picman,
                                    GdkModifierType  state,
                                    PicmanDisplay     *display)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      picman_tool_set_modifier_state (tool_manager->active_tool,
                                    state,
                                    display);
    }
}

void
tool_manager_active_modifier_state_active (Picman            *picman,
                                           GdkModifierType  state,
                                           PicmanDisplay     *display)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      picman_tool_set_active_modifier_state (tool_manager->active_tool,
                                           state,
                                           display);
    }
}

void
tool_manager_oper_update_active (Picman             *picman,
                                 const PicmanCoords *coords,
                                 GdkModifierType   state,
                                 gboolean          proximity,
                                 PicmanDisplay      *display)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      picman_tool_oper_update (tool_manager->active_tool,
                             coords, state, proximity,
                             display);
    }
}

void
tool_manager_cursor_update_active (Picman             *picman,
                                   const PicmanCoords *coords,
                                   GdkModifierType   state,
                                   PicmanDisplay      *display)
{
  PicmanToolManager *tool_manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      picman_tool_cursor_update (tool_manager->active_tool,
                               coords, state,
                               display);
    }
}

PicmanUIManager *
tool_manager_get_popup_active (Picman             *picman,
                               const PicmanCoords *coords,
                               GdkModifierType   state,
                               PicmanDisplay      *display,
                               const gchar     **ui_path)
{
  PicmanToolManager *tool_manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  tool_manager = tool_manager_get (picman);

  if (tool_manager->active_tool)
    {
      return picman_tool_get_popup (tool_manager->active_tool,
                                  coords, state,
                                  display,
                                  ui_path);
    }

  return NULL;
}


/*  private functions  */

static GQuark tool_manager_quark = 0;

static PicmanToolManager *
tool_manager_get (Picman *picman)
{
  if (! tool_manager_quark)
    tool_manager_quark = g_quark_from_static_string ("picman-tool-manager");

  return g_object_get_qdata (G_OBJECT (picman), tool_manager_quark);
}

static void
tool_manager_set (Picman            *picman,
                  PicmanToolManager *tool_manager)
{
  if (! tool_manager_quark)
    tool_manager_quark = g_quark_from_static_string ("picman-tool-manager");

  g_object_set_qdata (G_OBJECT (picman), tool_manager_quark, tool_manager);
}

static void
tool_manager_tool_changed (PicmanContext     *user_context,
                           PicmanToolInfo    *tool_info,
                           PicmanToolManager *tool_manager)
{
  PicmanTool *new_tool = NULL;

  if (! tool_info)
    return;

  /* FIXME: picman_busy HACK */
  if (user_context->picman->busy)
    {
      /*  there may be contexts waiting for the user_context's "tool-changed"
       *  signal, so stop emitting it.
       */
      g_signal_stop_emission_by_name (user_context, "tool-changed");

      if (G_TYPE_FROM_INSTANCE (tool_manager->active_tool) !=
          tool_info->tool_type)
        {
          g_signal_handlers_block_by_func (user_context,
                                           tool_manager_tool_changed,
                                           tool_manager);

          /*  explicitly set the current tool  */
          picman_context_set_tool (user_context,
                                 tool_manager->active_tool->tool_info);

          g_signal_handlers_unblock_by_func (user_context,
                                             tool_manager_tool_changed,
                                             tool_manager);
        }

      return;
    }

  if (g_type_is_a (tool_info->tool_type, PICMAN_TYPE_TOOL))
    {
      new_tool = g_object_new (tool_info->tool_type,
                               "tool-info", tool_info,
                               NULL);
    }
  else
    {
      g_warning ("%s: tool_info->tool_type is no PicmanTool subclass",
                 G_STRFUNC);
      return;
    }

  /*  disconnect the old tool's context  */
  if (tool_manager->active_tool &&
      tool_manager->active_tool->tool_info)
    {
      tool_manager_disconnect_options (tool_manager, user_context,
                                       tool_manager->active_tool->tool_info);
    }

  /*  connect the new tool's context  */
  tool_manager_connect_options (tool_manager, user_context, tool_info);

  tool_manager_select_tool (user_context->picman, new_tool);

  g_object_unref (new_tool);
}

static void
tool_manager_preset_changed (PicmanContext     *user_context,
                             PicmanToolPreset  *preset,
                             PicmanToolManager *tool_manager)
{
  PicmanToolInfo *preset_tool;
  gchar        *options_name;
  gboolean      tool_change = FALSE;

  if (! preset || user_context->picman->busy)
    return;

  preset_tool = picman_context_get_tool (PICMAN_CONTEXT (preset->tool_options));

  if (preset_tool != picman_context_get_tool (user_context))
    tool_change = TRUE;

  if (! tool_change)
    tool_manager_disconnect_options (tool_manager, user_context, preset_tool);

  /*  save the name, we don't want to overwrite it  */
  options_name = g_strdup (picman_object_get_name (preset_tool->tool_options));

  picman_config_copy (PICMAN_CONFIG (preset->tool_options),
                    PICMAN_CONFIG (preset_tool->tool_options), 0);

  /*  restore the saved name  */
  picman_object_take_name (PICMAN_OBJECT (preset_tool->tool_options), options_name);

  if (tool_change)
    picman_context_set_tool (user_context, preset_tool);
  else
    tool_manager_connect_options (tool_manager, user_context, preset_tool);

  picman_context_copy_properties (PICMAN_CONTEXT (preset->tool_options),
                                user_context,
                                picman_tool_preset_get_prop_mask (preset));

  if (PICMAN_IS_PAINT_OPTIONS (preset->tool_options))
    {
      PicmanCoreConfig  *config = user_context->picman->config;
      PicmanToolOptions *src    = preset->tool_options;
      PicmanToolOptions *dest   = tool_manager->active_tool->tool_info->tool_options;

      /* if connect_options() did overwrite the brush options and the
       * preset contains a brush, use the brush options from the
       * preset
       */
      if (config->global_brush && preset->use_brush)
        picman_paint_options_copy_brush_props (PICMAN_PAINT_OPTIONS (src),
                                             PICMAN_PAINT_OPTIONS (dest));

      if (config->global_dynamics && preset->use_dynamics)
        picman_paint_options_copy_dynamics_props (PICMAN_PAINT_OPTIONS (src),
                                                PICMAN_PAINT_OPTIONS (dest));

      if (config->global_gradient && preset->use_gradient)
        picman_paint_options_copy_gradient_props (PICMAN_PAINT_OPTIONS (src),
                                                PICMAN_PAINT_OPTIONS (dest));
    }
}

static void
tool_manager_image_clean_dirty (PicmanImage       *image,
                                PicmanDirtyMask    dirty_mask,
                                PicmanToolManager *tool_manager)
{
  PicmanTool *tool = tool_manager->active_tool;

  if (tool &&
      ! picman_tool_control_get_preserve (tool->control) &&
      (picman_tool_control_get_dirty_mask (tool->control) & dirty_mask))
    {
      PicmanDisplay *display = picman_tool_has_image (tool, image);

      if (display)
        tool_manager_control_active (image->picman, PICMAN_TOOL_ACTION_HALT,
                                     display);
    }
}

static void
tool_manager_connect_options (PicmanToolManager *tool_manager,
                              PicmanContext     *user_context,
                              PicmanToolInfo    *tool_info)
{
  if (tool_info->context_props)
    {
      PicmanCoreConfig      *config       = user_context->picman->config;
      PicmanContextPropMask  global_props = 0;

      /*  FG and BG are always shared between all tools  */
      global_props |= PICMAN_CONTEXT_FOREGROUND_MASK;
      global_props |= PICMAN_CONTEXT_BACKGROUND_MASK;

      if (config->global_brush)
        global_props |= PICMAN_CONTEXT_BRUSH_MASK;
      if (config->global_dynamics)
        global_props |= PICMAN_CONTEXT_DYNAMICS_MASK;
      if (config->global_pattern)
        global_props |= PICMAN_CONTEXT_PATTERN_MASK;
      if (config->global_palette)
        global_props |= PICMAN_CONTEXT_PALETTE_MASK;
      if (config->global_gradient)
        global_props |= PICMAN_CONTEXT_GRADIENT_MASK;
      if (config->global_font)
        global_props |= PICMAN_CONTEXT_FONT_MASK;

      picman_context_copy_properties (PICMAN_CONTEXT (tool_info->tool_options),
                                    user_context,
                                    tool_info->context_props & ~global_props);
      picman_context_set_parent (PICMAN_CONTEXT (tool_info->tool_options),
                               user_context);

      if (PICMAN_IS_PAINT_OPTIONS (tool_info->tool_options))
        {
          if (config->global_brush)
            picman_paint_options_copy_brush_props (tool_manager->shared_paint_options,
                                                 PICMAN_PAINT_OPTIONS (tool_info->tool_options));

          if (config->global_dynamics)
            picman_paint_options_copy_dynamics_props (tool_manager->shared_paint_options,
                                                    PICMAN_PAINT_OPTIONS (tool_info->tool_options));

          if (config->global_gradient)
            picman_paint_options_copy_gradient_props (tool_manager->shared_paint_options,
                                                    PICMAN_PAINT_OPTIONS (tool_info->tool_options));
        }
    }
}

static void
tool_manager_disconnect_options (PicmanToolManager *tool_manager,
                                 PicmanContext     *user_context,
                                 PicmanToolInfo    *tool_info)
{
  if (tool_info->context_props)
    {
      if (PICMAN_IS_PAINT_OPTIONS (tool_info->tool_options))
        {
          /* Storing is unconditional, because the user may turn on
           * brush sharing mid use
           */
          picman_paint_options_copy_brush_props (PICMAN_PAINT_OPTIONS (tool_info->tool_options),
                                               tool_manager->shared_paint_options);

          picman_paint_options_copy_dynamics_props (PICMAN_PAINT_OPTIONS (tool_info->tool_options),
                                                  tool_manager->shared_paint_options);

          picman_paint_options_copy_gradient_props (PICMAN_PAINT_OPTIONS (tool_info->tool_options),
                                                  tool_manager->shared_paint_options);
        }

      picman_context_set_parent (PICMAN_CONTEXT (tool_info->tool_options), NULL);
    }
}
