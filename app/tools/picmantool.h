/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2002 Spencer Kimball, Peter Mattis, and others
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

#ifndef __PICMAN_TOOL_H__
#define __PICMAN_TOOL_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_TOOL            (picman_tool_get_type ())
#define PICMAN_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TOOL, PicmanTool))
#define PICMAN_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TOOL, PicmanToolClass))
#define PICMAN_IS_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TOOL))
#define PICMAN_IS_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TOOL))
#define PICMAN_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TOOL, PicmanToolClass))

#define PICMAN_TOOL_GET_OPTIONS(t)  (picman_tool_get_options (PICMAN_TOOL (t)))


typedef struct _PicmanToolClass PicmanToolClass;

struct _PicmanTool
{
  PicmanObject       parent_instance;

  PicmanToolInfo    *tool_info;

  gint             ID;          /*  unique tool ID                         */

  PicmanToolControl *control;

  PicmanDisplay     *display;     /*  pointer to currently active display    */
  PicmanDrawable    *drawable;    /*  pointer to the tool's current drawable */

  /*  private state of picman_tool_set_focus_display() and
   *  picman_tool_set_[active_]modifier_state()
   */
  PicmanDisplay     *focus_display;
  GdkModifierType  modifier_state;
  GdkModifierType  button_press_state;
  GdkModifierType  active_modifier_state;

  /*  private state for click detection
   */
  gboolean         in_click_distance;
  gboolean         got_motion_event;
  PicmanCoords       button_press_coords;
  guint32          button_press_time;

  /*  private list of displays which have a status message from this tool
   */
  GList           *status_displays;

  /*  on-canvas progress  */
  PicmanCanvasItem  *progress;
  PicmanDisplay     *progress_display;
};

struct _PicmanToolClass
{
  PicmanObjectClass  parent_class;

  /*  virtual functions  */

  gboolean        (* has_display)         (PicmanTool              *tool,
                                           PicmanDisplay           *display);
  PicmanDisplay   * (* has_image)           (PicmanTool              *tool,
                                           PicmanImage             *image);

  gboolean        (* initialize)          (PicmanTool              *tool,
                                           PicmanDisplay           *display,
                                           GError               **error);
  void            (* control)             (PicmanTool              *tool,
                                           PicmanToolAction         action,
                                           PicmanDisplay           *display);

  void            (* button_press)        (PicmanTool              *tool,
                                           const PicmanCoords      *coords,
                                           guint32                time,
                                           GdkModifierType        state,
                                           PicmanButtonPressType    press_type,
                                           PicmanDisplay           *display);
  void            (* button_release)      (PicmanTool              *tool,
                                           const PicmanCoords      *coords,
                                           guint32                time,
                                           GdkModifierType        state,
                                           PicmanButtonReleaseType  release_type,
                                           PicmanDisplay           *display);
  void            (* motion)              (PicmanTool              *tool,
                                           const PicmanCoords      *coords,
                                           guint32                time,
                                           GdkModifierType        state,
                                           PicmanDisplay           *display);

  gboolean        (* key_press)           (PicmanTool              *tool,
                                           GdkEventKey           *kevent,
                                           PicmanDisplay           *display);
  gboolean        (* key_release)         (PicmanTool              *tool,
                                           GdkEventKey           *kevent,
                                           PicmanDisplay           *display);
  void            (* modifier_key)        (PicmanTool              *tool,
                                           GdkModifierType        key,
                                           gboolean               press,
                                           GdkModifierType        state,
                                           PicmanDisplay           *display);
  void            (* active_modifier_key) (PicmanTool              *tool,
                                           GdkModifierType        key,
                                           gboolean               press,
                                           GdkModifierType        state,
                                           PicmanDisplay           *display);

  void            (* oper_update)         (PicmanTool              *tool,
                                           const PicmanCoords      *coords,
                                           GdkModifierType        state,
                                           gboolean               proximity,
                                           PicmanDisplay           *display);
  void            (* cursor_update)       (PicmanTool              *tool,
                                           const PicmanCoords      *coords,
                                           GdkModifierType        state,
                                           PicmanDisplay           *display);

  PicmanUIManager * (* get_popup)           (PicmanTool              *tool,
                                           const PicmanCoords      *coords,
                                           GdkModifierType        state,
                                           PicmanDisplay           *display,
                                           const gchar          **ui_path);

  void            (* options_notify)      (PicmanTool              *tool,
                                           PicmanToolOptions       *options,
                                           const GParamSpec      *pspec);
};


GType             picman_tool_get_type            (void) G_GNUC_CONST;

PicmanToolOptions * picman_tool_get_options         (PicmanTool            *tool);

gboolean          picman_tool_has_display         (PicmanTool            *tool,
                                                 PicmanDisplay         *display);
PicmanDisplay     * picman_tool_has_image           (PicmanTool            *tool,
                                                 PicmanImage           *image);

gboolean          picman_tool_initialize          (PicmanTool            *tool,
                                                 PicmanDisplay         *display);
void              picman_tool_control             (PicmanTool            *tool,
                                                 PicmanToolAction       action,
                                                 PicmanDisplay         *display);

void              picman_tool_button_press        (PicmanTool            *tool,
                                                 const PicmanCoords    *coords,
                                                 guint32              time,
                                                 GdkModifierType      state,
                                                 PicmanButtonPressType  press_type,
                                                 PicmanDisplay         *display);
void              picman_tool_button_release      (PicmanTool            *tool,
                                                 const PicmanCoords    *coords,
                                                 guint32              time,
                                                 GdkModifierType      state,
                                                 PicmanDisplay         *display);
void              picman_tool_motion              (PicmanTool            *tool,
                                                 const PicmanCoords    *coords,
                                                 guint32              time,
                                                 GdkModifierType      state,
                                                 PicmanDisplay         *display);

gboolean          picman_tool_key_press           (PicmanTool            *tool,
                                                 GdkEventKey         *kevent,
                                                 PicmanDisplay         *display);
gboolean          picman_tool_key_release         (PicmanTool            *tool,
                                                 GdkEventKey         *kevent,
                                                 PicmanDisplay         *display);

void              picman_tool_set_focus_display   (PicmanTool            *tool,
                                                 PicmanDisplay         *display);
void              picman_tool_set_modifier_state  (PicmanTool            *tool,
                                                 GdkModifierType      state,
                                                 PicmanDisplay         *display);
void        picman_tool_set_active_modifier_state (PicmanTool            *tool,
                                                 GdkModifierType      state,
                                                 PicmanDisplay         *display);

void              picman_tool_oper_update         (PicmanTool            *tool,
                                                 const PicmanCoords    *coords,
                                                 GdkModifierType      state,
                                                 gboolean             proximity,
                                                 PicmanDisplay         *display);
void              picman_tool_cursor_update       (PicmanTool            *tool,
                                                 const PicmanCoords    *coords,
                                                 GdkModifierType      state,
                                                 PicmanDisplay         *display);

PicmanUIManager   * picman_tool_get_popup           (PicmanTool            *tool,
                                                 const PicmanCoords    *coords,
                                                 GdkModifierType      state,
                                                 PicmanDisplay         *display,
                                                 const gchar        **ui_path);

void              picman_tool_push_status         (PicmanTool            *tool,
                                                 PicmanDisplay         *display,
                                                 const gchar         *format,
                                                 ...) G_GNUC_PRINTF(3,4);
void              picman_tool_push_status_coords  (PicmanTool            *tool,
                                                 PicmanDisplay         *display,
                                                 PicmanCursorPrecision  precision,
                                                 const gchar         *title,
                                                 gdouble              x,
                                                 const gchar         *separator,
                                                 gdouble              y,
                                                 const gchar         *help);
void              picman_tool_push_status_length  (PicmanTool            *tool,
                                                 PicmanDisplay         *display,
                                                 const gchar         *title,
                                                 PicmanOrientationType  axis,
                                                 gdouble              value,
                                                 const gchar         *help);
void              picman_tool_replace_status      (PicmanTool            *tool,
                                                 PicmanDisplay         *display,
                                                 const gchar         *format,
                                                 ...) G_GNUC_PRINTF(3,4);
void              picman_tool_pop_status          (PicmanTool            *tool,
                                                 PicmanDisplay         *display);

void              picman_tool_message             (PicmanTool            *tool,
                                                 PicmanDisplay         *display,
                                                 const gchar         *format,
                                                 ...) G_GNUC_PRINTF(3,4);
void              picman_tool_message_literal     (PicmanTool            *tool,
                                                 PicmanDisplay         *display,
                                                 const gchar         *message);

void              picman_tool_set_cursor          (PicmanTool            *tool,
                                                 PicmanDisplay         *display,
                                                 PicmanCursorType       cursor,
                                                 PicmanToolCursorType   tool_cursor,
                                                 PicmanCursorModifier   modifier);


#endif  /*  __PICMAN_TOOL_H__  */
