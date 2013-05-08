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

#ifndef __PICMAN_VECTOR_TOOL_H__
#define __PICMAN_VECTOR_TOOL_H__


#include "picmandrawtool.h"


/*  possible vector functions  */
typedef enum
{
  VECTORS_SELECT_VECTOR,
  VECTORS_CREATE_VECTOR,
  VECTORS_CREATE_STROKE,
  VECTORS_ADD_ANCHOR,
  VECTORS_MOVE_ANCHOR,
  VECTORS_MOVE_ANCHORSET,
  VECTORS_MOVE_HANDLE,
  VECTORS_MOVE_CURVE,
  VECTORS_MOVE_STROKE,
  VECTORS_MOVE_VECTORS,
  VECTORS_INSERT_ANCHOR,
  VECTORS_DELETE_ANCHOR,
  VECTORS_CONNECT_STROKES,
  VECTORS_DELETE_SEGMENT,
  VECTORS_CONVERT_EDGE,
  VECTORS_FINISHED
} PicmanVectorFunction;


#define PICMAN_TYPE_VECTOR_TOOL            (picman_vector_tool_get_type ())
#define PICMAN_VECTOR_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VECTOR_TOOL, PicmanVectorTool))
#define PICMAN_VECTOR_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VECTOR_TOOL, PicmanVectorToolClass))
#define PICMAN_IS_VECTOR_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_VECTOR_TOOL))
#define PICMAN_IS_VECTOR_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VECTOR_TOOL))
#define PICMAN_VECTOR_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VECTOR_TOOL, PicmanVectorToolClass))

#define PICMAN_VECTOR_TOOL_GET_OPTIONS(t)  (PICMAN_VECTOR_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanVectorTool      PicmanVectorTool;
typedef struct _PicmanVectorToolClass PicmanVectorToolClass;

struct _PicmanVectorTool
{
  PicmanDrawTool          parent_instance;

  PicmanVectorFunction    function;       /* function we're performing         */
  PicmanAnchorFeatureType restriction;    /* movement restriction              */
  gboolean              modifier_lock;  /* can we toggle the Shift key?      */
  GdkModifierType       saved_state;    /* modifier state at button_press    */
  gdouble               last_x;         /* last x coordinate                 */
  gdouble               last_y;         /* last y coordinate                 */
  gboolean              undo_motion;    /* we need a motion to have an undo  */
  gboolean              have_undo;      /* did we push an undo at            */
                                        /* ..._button_press?                 */

  PicmanAnchor           *cur_anchor;     /* the current Anchor                */
  PicmanAnchor           *cur_anchor2;    /* secondary Anchor (end on_curve)   */
  PicmanStroke           *cur_stroke;     /* the current Stroke                */
  gdouble               cur_position;   /* the current Position on a segment */
  PicmanVectors          *cur_vectors;    /* the vectors the tool is hovering  */
                                        /* over (if different from ->vectors */
  PicmanVectors          *vectors;        /* the current Vector data           */

  gint                  sel_count;      /* number of selected anchors        */
  PicmanAnchor           *sel_anchor;     /* currently selected anchor, NULL   */
                                        /* if multiple anchors are selected  */
  PicmanStroke           *sel_stroke;     /* selected stroke                   */

  PicmanVectorMode        saved_mode;     /* used by modifier_key()            */
};

struct _PicmanVectorToolClass
{
  PicmanDrawToolClass  parent_class;
};


void    picman_vector_tool_register    (PicmanToolRegisterCallback  callback,
                                      gpointer                  data);

GType   picman_vector_tool_get_type    (void) G_GNUC_CONST;

void    picman_vector_tool_set_vectors (PicmanVectorTool           *vector_tool,
                                      PicmanVectors              *vectors);


#endif  /*  __PICMAN_VECTOR_TOOL_H__  */
