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

#if 0

#ifndef __PICMAN_ISCISSORS_TOOL_H__
#define __PICMAN_ISCISSORS_TOOL_H__


#include "picmanselectiontool.h"


/*  The possible states...  */
typedef enum
{
  NO_ACTION,
  SEED_PLACEMENT,
  SEED_ADJUSTMENT,
  WAITING
} IscissorsState;

/*  For oper_update & cursor_update  */
typedef enum
{
  ISCISSORS_OP_NONE,
  ISCISSORS_OP_SELECT,
  ISCISSORS_OP_MOVE_POINT,
  ISCISSORS_OP_ADD_POINT,
  ISCISSORS_OP_CONNECT,
  ISCISSORS_OP_IMPOSSIBLE
} IscissorsOps;

typedef struct _ICurve ICurve;


#define PICMAN_TYPE_ISCISSORS_TOOL            (picman_iscissors_tool_get_type ())
#define PICMAN_ISCISSORS_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ISCISSORS_TOOL, PicmanIscissorsTool))
#define PICMAN_ISCISSORS_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ISCISSORS_TOOL, PicmanIscissorsToolClass))
#define PICMAN_IS_ISCISSORS_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ISCISSORS_TOOL))
#define PICMAN_IS_ISCISSORS_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ISCISSORS_TOOL))
#define PICMAN_ISCISSORS_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ISCISSORS_TOOL, PicmanIscissorsToolClass))

#define PICMAN_ISCISSORS_TOOL_GET_OPTIONS(t)  (PICMAN_ISCISSORS_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanIscissorsTool      PicmanIscissorsTool;
typedef struct _PicmanIscissorsToolClass PicmanIscissorsToolClass;

struct _PicmanIscissorsTool
{
  PicmanSelectionTool  parent_instance;

  IscissorsOps    op;

  gint            x, y;         /*  upper left hand coordinate            */
  gint            ix, iy;       /*  initial coordinates                   */
  gint            nx, ny;       /*  new coordinates                       */

  PicmanTempBuf    *dp_buf;       /*  dynamic programming buffer            */

  ICurve         *livewire;     /*  livewire boundary curve               */

  ICurve         *curve1;       /*  1st curve connected to current point  */
  ICurve         *curve2;       /*  2nd curve connected to current point  */

  GQueue         *curves;       /*  the list of curves                    */

  gboolean        first_point;  /*  is this the first point?              */
  gboolean        connected;    /*  is the region closed?                 */

  IscissorsState  state;        /*  state of iscissors                    */

  /* XXX might be useful */
  PicmanChannel    *mask;         /*  selection mask                        */
  TileManager    *gradient_map; /*  lazily filled gradient map            */
};

struct _PicmanIscissorsToolClass
{
  PicmanSelectionToolClass parent_class;
};


void    picman_iscissors_tool_register (PicmanToolRegisterCallback  callback,
                                      gpointer                  data);

GType   picman_iscissors_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_ISCISSORS_TOOL_H__  */

#endif
