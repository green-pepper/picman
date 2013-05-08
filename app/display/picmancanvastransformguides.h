/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvastransformguides.h
 * Copyright (C) 2011 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CANVAS_TRANSFORM_GUIDES_H__
#define __PICMAN_CANVAS_TRANSFORM_GUIDES_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_TRANSFORM_GUIDES            (picman_canvas_transform_guides_get_type ())
#define PICMAN_CANVAS_TRANSFORM_GUIDES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_TRANSFORM_GUIDES, PicmanCanvasTransformGuides))
#define PICMAN_CANVAS_TRANSFORM_GUIDES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_TRANSFORM_GUIDES, PicmanCanvasTransformGuidesClass))
#define PICMAN_IS_CANVAS_TRANSFORM_GUIDES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_TRANSFORM_GUIDES))
#define PICMAN_IS_CANVAS_TRANSFORM_GUIDES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_TRANSFORM_GUIDES))
#define PICMAN_CANVAS_TRANSFORM_GUIDES_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_TRANSFORM_GUIDES, PicmanCanvasTransformGuidesClass))


typedef struct _PicmanCanvasTransformGuides      PicmanCanvasTransformGuides;
typedef struct _PicmanCanvasTransformGuidesClass PicmanCanvasTransformGuidesClass;

struct _PicmanCanvasTransformGuides
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasTransformGuidesClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_transform_guides_get_type (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_transform_guides_new      (PicmanDisplayShell  *shell,
                                                        const PicmanMatrix3 *transform,
                                                        gdouble            x1,
                                                        gdouble            y1,
                                                        gdouble            x2,
                                                        gdouble            y2,
                                                        PicmanGuidesType     type,
                                                        gint               n_guides);

void             picman_canvas_transform_guides_set      (PicmanCanvasItem    *guides,
                                                        const PicmanMatrix3 *transform,
                                                        PicmanGuidesType     type,
                                                        gint               n_guides);


#endif /* __PICMAN_CANVAS_TRANSFORM_GUIDES_H__ */
