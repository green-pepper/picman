/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis, and others
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

#ifndef __PICMAN_TRANSFORM_TOOL_H__
#define __PICMAN_TRANSFORM_TOOL_H__


#include "picmandrawtool.h"


typedef enum
{
  TRANSFORM_CREATING,
  TRANSFORM_HANDLE_NONE,
  TRANSFORM_HANDLE_NW_P, /* perspective handles */
  TRANSFORM_HANDLE_NE_P,
  TRANSFORM_HANDLE_SW_P,
  TRANSFORM_HANDLE_SE_P,
  TRANSFORM_HANDLE_NW, /* north west */
  TRANSFORM_HANDLE_NE, /* north east */
  TRANSFORM_HANDLE_SW, /* south west */
  TRANSFORM_HANDLE_SE, /* south east */
  TRANSFORM_HANDLE_N,  /* north      */
  TRANSFORM_HANDLE_S,  /* south      */
  TRANSFORM_HANDLE_E,  /* east       */
  TRANSFORM_HANDLE_W,  /* west       */
  TRANSFORM_HANDLE_CENTER, /* for moving */
  TRANSFORM_HANDLE_PIVOT,  /* pivot for rotation and scaling */
  TRANSFORM_HANDLE_N_S,  /* shearing handles */
  TRANSFORM_HANDLE_S_S,
  TRANSFORM_HANDLE_E_S,
  TRANSFORM_HANDLE_W_S,
  TRANSFORM_HANDLE_ROTATION, /* rotation handle */

  TRANSFORM_HANDLE_NUM /* keep this last so *handles[] is the right size */
} TransformAction;

/* This is not the number of items in the enum above, but the max size of the
 * enums at the top of each transformation tool, stored in trans_info and related */
#define TRANS_INFO_SIZE 10

typedef gdouble TransInfo[TRANS_INFO_SIZE];


#define PICMAN_TYPE_TRANSFORM_TOOL            (picman_transform_tool_get_type ())
#define PICMAN_TRANSFORM_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TRANSFORM_TOOL, PicmanTransformTool))
#define PICMAN_TRANSFORM_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TRANSFORM_TOOL, PicmanTransformToolClass))
#define PICMAN_IS_TRANSFORM_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TRANSFORM_TOOL))
#define PICMAN_IS_TRANSFORM_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TRANSFORM_TOOL))
#define PICMAN_TRANSFORM_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TRANSFORM_TOOL, PicmanTransformToolClass))

#define PICMAN_TRANSFORM_TOOL_GET_OPTIONS(t)  (PICMAN_TRANSFORM_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanTransformToolClass PicmanTransformToolClass;

struct _PicmanTransformTool
{
  PicmanDrawTool    parent_instance;

  gdouble         curx;            /*  current x coord                   */
  gdouble         cury;            /*  current y coord                   */

  gdouble         lastx;           /*  last x coord                      */
  gdouble         lasty;           /*  last y coord                      */

  gdouble         previousx;       /*  previous x coord                  */
  gdouble         previousy;       /*  previous y coord                  */

  gdouble         mousex;          /*  x coord where mouse was clicked   */
  gdouble         mousey;          /*  y coord where mouse was clicked   */

  gint            x1, y1;          /*  upper left hand coordinate        */
  gint            x2, y2;          /*  lower right hand coords           */
  gdouble         cx, cy;          /*  center point (for moving)         */
  gdouble         px, py;          /*  pivot point (for rotation/scaling)*/
  gdouble         aspect;          /*  original aspect ratio             */

  gdouble         tx1, ty1;        /*  transformed handle coords         */
  gdouble         tx2, ty2;
  gdouble         tx3, ty3;
  gdouble         tx4, ty4;
  gdouble         tcx, tcy;
  gdouble         tpx, tpy;

  PicmanMatrix3     transform;       /*  transformation matrix             */
  TransInfo       trans_info;      /*  transformation info               */
  TransInfo      *old_trans_info;  /*  for resetting everything          */
  TransInfo      *prev_trans_info; /*  the current finished state        */
  GList          *undo_list;       /*  list of all states,
                                       head is current == prev_trans_info,
                                       tail is original == old_trans_info*/
  GList          *redo_list;       /*  list of all undone states,
                                       NULL when nothing undone */

  TransformAction function;        /*  current tool activity             */

  gboolean        use_grid;        /*  does the tool use the grid        */
  gboolean        use_handles;     /*  uses the corner handles           */
  gboolean        use_center;      /*  uses the center handle            */
  gboolean        use_mid_handles; /*  use handles at midpoints of edges */
  gboolean        use_pivot;       /*  use pivot point                   */

  PicmanCanvasItem *handles[TRANSFORM_HANDLE_NUM];

  const gchar    *progress_text;

  GtkWidget      *dialog;
};

struct _PicmanTransformToolClass
{
  PicmanDrawToolClass  parent_class;

  /*  virtual functions  */
  void            (* dialog)        (PicmanTransformTool *tool);
  void            (* dialog_update) (PicmanTransformTool *tool);
  void            (* prepare)       (PicmanTransformTool *tool);
  void            (* motion)        (PicmanTransformTool *tool);
  void            (* recalc_matrix) (PicmanTransformTool *tool);
  gchar         * (* get_undo_desc) (PicmanTransformTool *tool);
  TransformAction (* pick_function) (PicmanTransformTool *tool,
                                     const PicmanCoords  *coords,
                                     GdkModifierType    state,
                                     PicmanDisplay       *display);
  void            (* cursor_update) (PicmanTransformTool  *tr_tool,
                                     PicmanCursorType     *cursor,
                                     PicmanCursorModifier *modifier);
  void            (* draw_gui)      (PicmanTransformTool *tool,
                                     gint               handle_w,
                                     gint               handle_h);
  GeglBuffer    * (* transform)     (PicmanTransformTool *tool,
                                     PicmanItem          *item,
                                     GeglBuffer        *orig_buffer,
                                     gint               orig_offset_x,
                                     gint               orig_offset_y,
                                     gint              *new_offset_x,
                                     gint              *new_offset_y);
};


GType   picman_transform_tool_get_type      (void) G_GNUC_CONST;

void    picman_transform_tool_recalc_matrix (PicmanTransformTool *tr_tool);
void    picman_transform_tool_push_internal_undo (PicmanTransformTool *tr_tool);


#endif  /*  __PICMAN_TRANSFORM_TOOL_H__  */
