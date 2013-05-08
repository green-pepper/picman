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

#ifndef __PICMAN_PERSPECTIVE_CLONE_TOOL_H__
#define __PICMAN_PERSPECTIVE_CLONE_TOOL_H__


#include "picmanbrushtool.h"
#include "picmantransformtool.h"  /* for TransInfo */


#define PICMAN_TYPE_PERSPECTIVE_CLONE_TOOL            (picman_perspective_clone_tool_get_type ())
#define PICMAN_PERSPECTIVE_CLONE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PERSPECTIVE_CLONE_TOOL, PicmanPerspectiveCloneTool))
#define PICMAN_PERSPECTIVE_CLONE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PERSPECTIVE_CLONE_TOOL, PicmanPerspectiveCloneToolClass))
#define PICMAN_IS_PERSPECTIVE_CLONE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PERSPECTIVE_CLONE_TOOL))
#define PICMAN_IS_PERSPECTIVE_CLONE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PERSPECTIVE_CLONE_TOOL))
#define PICMAN_PERSPECTIVE_CLONE_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PERSPECTIVE_CLONE_TOOL, PicmanPerspectiveCloneToolClass))

#define PICMAN_PERSPECTIVE_CLONE_TOOL_GET_OPTIONS(t)  (PICMAN_PERSPECTIVE_CLONE_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanPerspectiveCloneTool      PicmanPerspectiveCloneTool;
typedef struct _PicmanPerspectiveCloneToolClass PicmanPerspectiveCloneToolClass;

struct _PicmanPerspectiveCloneTool
{
  PicmanBrushTool   parent_instance;

  PicmanDisplay    *src_display;

  gint            src_x;
  gint            src_y;

  gdouble         curx;           /*  current x coord                  */
  gdouble         cury;           /*  current y coord                  */

  gdouble         lastx;          /*  last x coord                     */
  gdouble         lasty;          /*  last y coord                     */

  PicmanMatrix3     transform;      /*  transformation matrix            */
  TransInfo       trans_info;     /*  transformation info              */
  TransInfo       old_trans_info; /*  for cancelling a drag operation  */

  gint            x1, y1;         /*  upper left hand coordinate       */
  gint            x2, y2;         /*  lower right hand coords          */

  gdouble         tx1, ty1;       /*  transformed coords               */
  gdouble         tx2, ty2;
  gdouble         tx3, ty3;
  gdouble         tx4, ty4;
  gdouble         tcx, tcy;

  TransformAction function;       /*  current tool activity            */
};

struct _PicmanPerspectiveCloneToolClass
{
  PicmanBrushToolClass  parent_class;
};


void    picman_perspective_clone_tool_register (PicmanToolRegisterCallback  callback,
                                              gpointer                  data);

GType   picman_perspective_clone_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_PERSPECTIVE_CLONE_TOOL_H__  */
