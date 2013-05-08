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

#ifndef  __PICMAN_RECTANGLE_TOOL_H__
#define  __PICMAN_RECTANGLE_TOOL_H__


typedef enum
{
  PICMAN_RECTANGLE_TOOL_PROP_0,
  PICMAN_RECTANGLE_TOOL_PROP_X1,
  PICMAN_RECTANGLE_TOOL_PROP_Y1,
  PICMAN_RECTANGLE_TOOL_PROP_X2,
  PICMAN_RECTANGLE_TOOL_PROP_Y2,
  PICMAN_RECTANGLE_TOOL_PROP_CONSTRAINT,
  PICMAN_RECTANGLE_TOOL_PROP_PRECISION,
  PICMAN_RECTANGLE_TOOL_PROP_NARROW_MODE,
  PICMAN_RECTANGLE_TOOL_PROP_LAST = PICMAN_RECTANGLE_TOOL_PROP_NARROW_MODE
} PicmanRectangleToolProp;


typedef enum
{
  PICMAN_RECTANGLE_TOOL_INACTIVE,
  PICMAN_RECTANGLE_TOOL_DEAD,
  PICMAN_RECTANGLE_TOOL_CREATING,
  PICMAN_RECTANGLE_TOOL_MOVING,
  PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_LEFT,
  PICMAN_RECTANGLE_TOOL_RESIZING_UPPER_RIGHT,
  PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_LEFT,
  PICMAN_RECTANGLE_TOOL_RESIZING_LOWER_RIGHT,
  PICMAN_RECTANGLE_TOOL_RESIZING_LEFT,
  PICMAN_RECTANGLE_TOOL_RESIZING_RIGHT,
  PICMAN_RECTANGLE_TOOL_RESIZING_TOP,
  PICMAN_RECTANGLE_TOOL_RESIZING_BOTTOM,
  PICMAN_RECTANGLE_TOOL_AUTO_SHRINK,
  PICMAN_RECTANGLE_TOOL_EXECUTING
} PicmanRectangleFunction;


#define PICMAN_TYPE_RECTANGLE_TOOL               (picman_rectangle_tool_interface_get_type ())
#define PICMAN_IS_RECTANGLE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_RECTANGLE_TOOL))
#define PICMAN_RECTANGLE_TOOL(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_RECTANGLE_TOOL, PicmanRectangleTool))
#define PICMAN_RECTANGLE_TOOL_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_RECTANGLE_TOOL, PicmanRectangleToolInterface))

#define PICMAN_RECTANGLE_TOOL_GET_OPTIONS(t)     (PICMAN_RECTANGLE_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanRectangleTool          PicmanRectangleTool;
typedef struct _PicmanRectangleToolInterface PicmanRectangleToolInterface;

struct _PicmanRectangleToolInterface
{
  GTypeInterface base_iface;

  /*  virtual functions  */
  gboolean (* execute)           (PicmanRectangleTool *rect_tool,
                                  gint               x,
                                  gint               y,
                                  gint               w,
                                  gint               h);
  void     (* cancel)            (PicmanRectangleTool *rect_tool);

  /*  signals  */
  gboolean (* rectangle_change_complete) (PicmanRectangleTool *rect_tool);
};


GType       picman_rectangle_tool_interface_get_type  (void) G_GNUC_CONST;

void        picman_rectangle_tool_constructor         (GObject                 *object);

void        picman_rectangle_tool_init                (PicmanRectangleTool       *rect_tool);
void        picman_rectangle_tool_control             (PicmanTool                *tool,
                                                     PicmanToolAction           action,
                                                     PicmanDisplay             *display);
void        picman_rectangle_tool_button_press        (PicmanTool                *tool,
                                                     const PicmanCoords        *coords,
                                                     guint32                  time,
                                                     GdkModifierType          state,
                                                     PicmanDisplay             *display);
void        picman_rectangle_tool_button_release      (PicmanTool                *tool,
                                                     const PicmanCoords        *coords,
                                                     guint32                  time,
                                                     GdkModifierType          state,
                                                     PicmanButtonReleaseType    release_type,
                                                     PicmanDisplay             *display);
void        picman_rectangle_tool_motion              (PicmanTool                *tool,
                                                     const PicmanCoords        *coords,
                                                     guint32                  time,
                                                     GdkModifierType          state,
                                                     PicmanDisplay             *display);
gboolean    picman_rectangle_tool_key_press           (PicmanTool                *tool,
                                                     GdkEventKey             *kevent,
                                                     PicmanDisplay             *display);
void        picman_rectangle_tool_active_modifier_key (PicmanTool                *tool,
                                                     GdkModifierType          key,
                                                     gboolean                 press,
                                                     GdkModifierType          state,
                                                     PicmanDisplay             *display);
void        picman_rectangle_tool_oper_update         (PicmanTool                *tool,
                                                     const PicmanCoords        *coords,
                                                     GdkModifierType          state,
                                                     gboolean                 proximity,
                                                     PicmanDisplay             *display);
void        picman_rectangle_tool_cursor_update       (PicmanTool                *tool,
                                                     const PicmanCoords        *coords,
                                                     GdkModifierType          state,
                                                     PicmanDisplay             *display);
void        picman_rectangle_tool_draw                (PicmanDrawTool            *draw,
                                                     PicmanCanvasGroup         *stroke_group);
gboolean    picman_rectangle_tool_execute             (PicmanRectangleTool       *rect_tool);
void        picman_rectangle_tool_cancel              (PicmanRectangleTool       *rect_tool);
void        picman_rectangle_tool_set_constraint      (PicmanRectangleTool       *rectangle,
                                                     PicmanRectangleConstraint  constraint);
PicmanRectangleConstraint picman_rectangle_tool_get_constraint
                                                    (PicmanRectangleTool       *rectangle);
PicmanRectangleFunction picman_rectangle_tool_get_function (PicmanRectangleTool    *rectangle);
void        picman_rectangle_tool_set_function        (PicmanRectangleTool       *rectangle,
                                                     PicmanRectangleFunction    function);
void        picman_rectangle_tool_pending_size_set    (PicmanRectangleTool       *rectangle,
                                                     GObject                 *object,
                                                     const gchar             *width_property,
                                                     const gchar             *height_property);
void        picman_rectangle_tool_constraint_size_set (PicmanRectangleTool       *rectangle,
                                                     GObject                 *object,
                                                     const gchar             *width_property,
                                                     const gchar             *height_property);
gboolean    picman_rectangle_tool_rectangle_is_new    (PicmanRectangleTool       *rect_tool);
gboolean    picman_rectangle_tool_point_in_rectangle  (PicmanRectangleTool       *rect_tool,
                                                     gdouble                  x,
                                                     gdouble                  y);
void        picman_rectangle_tool_frame_item          (PicmanRectangleTool       *rect_tool,
                                                     PicmanItem                *item);


/*  convenience functions  */

void        picman_rectangle_tool_install_properties  (GObjectClass *klass);
void        picman_rectangle_tool_set_property        (GObject      *object,
                                                     guint         property_id,
                                                     const GValue *value,
                                                     GParamSpec   *pspec);
void        picman_rectangle_tool_get_property        (GObject      *object,
                                                     guint         property_id,
                                                     GValue       *value,
                                                     GParamSpec   *pspec);


#endif  /*  __PICMAN_RECTANGLE_TOOL_H__  */
