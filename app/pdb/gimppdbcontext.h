/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmanpdbcontext.h
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

#ifndef __PICMAN_PDB_CONTEXT_H__
#define __PICMAN_PDB_CONTEXT_H__


#include "core/picmancontext.h"


#define PICMAN_TYPE_PDB_CONTEXT            (picman_pdb_context_get_type ())
#define PICMAN_PDB_CONTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PDB_CONTEXT, PicmanPDBContext))
#define PICMAN_PDB_CONTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PDB_CONTEXT, PicmanPDBContextClass))
#define PICMAN_IS_PDB_CONTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PDB_CONTEXT))
#define PICMAN_IS_PDB_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PDB_CONTEXT))
#define PICMAN_PDB_CONTEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PDB_CONTEXT, PicmanPDBContextClass))


typedef struct _PicmanPDBContext      PicmanPDBContext;
typedef struct _PicmanPDBContextClass PicmanPDBContextClass;

struct _PicmanPDBContext
{
  PicmanContext             parent_instance;

  gboolean                antialias;
  gboolean                feather;
  gdouble                 feather_radius_x;
  gdouble                 feather_radius_y;
  gboolean                sample_merged;
  PicmanSelectCriterion     sample_criterion;
  gdouble                 sample_threshold;
  gboolean                sample_transparent;

  PicmanInterpolationType   interpolation;
  PicmanTransformDirection  transform_direction;
  PicmanTransformResize     transform_resize;
  gint                    transform_recursion;

  PicmanContainer          *paint_options_list;
};

struct _PicmanPDBContextClass
{
  PicmanContextClass  parent_class;
};


GType              picman_pdb_context_get_type          (void) G_GNUC_CONST;

PicmanContext      * picman_pdb_context_new               (Picman           *picman,
                                                       PicmanContext    *parent,
                                                       gboolean        set_parent);

PicmanPaintOptions * picman_pdb_context_get_paint_options (PicmanPDBContext *context,
                                                       const gchar    *name);
GList            * picman_pdb_context_get_brush_options (PicmanPDBContext *context);


#endif  /*  __PICMAN_PDB_CONTEXT_H__  */
