/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasguide.h
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CANVAS_GUIDE_H__
#define __PICMAN_CANVAS_GUIDE_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_GUIDE            (picman_canvas_guide_get_type ())
#define PICMAN_CANVAS_GUIDE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_GUIDE, PicmanCanvasGuide))
#define PICMAN_CANVAS_GUIDE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_GUIDE, PicmanCanvasGuideClass))
#define PICMAN_IS_CANVAS_GUIDE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_GUIDE))
#define PICMAN_IS_CANVAS_GUIDE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_GUIDE))
#define PICMAN_CANVAS_GUIDE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_GUIDE, PicmanCanvasGuideClass))


typedef struct _PicmanCanvasGuide      PicmanCanvasGuide;
typedef struct _PicmanCanvasGuideClass PicmanCanvasGuideClass;

struct _PicmanCanvasGuide
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasGuideClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_guide_get_type (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_guide_new      (PicmanDisplayShell    *shell,
                                             PicmanOrientationType  orientation,
                                             gint                 position,
                                             gboolean             guide_style);

void             picman_canvas_guide_set      (PicmanCanvasItem      *guide,
                                             PicmanOrientationType  orientation,
                                             gint                 position);


#endif /* __PICMAN_CANVAS_GUIDE_H__ */
