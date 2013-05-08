/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrendererbrush.h
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_VIEW_RENDERER_BRUSH_H__
#define __PICMAN_VIEW_RENDERER_BRUSH_H__

#include "picmanviewrenderer.h"

#define PICMAN_TYPE_VIEW_RENDERER_BRUSH            (picman_view_renderer_brush_get_type ())
#define PICMAN_VIEW_RENDERER_BRUSH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VIEW_RENDERER_BRUSH, PicmanViewRendererBrush))
#define PICMAN_VIEW_RENDERER_BRUSH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VIEW_RENDERER_BRUSH, PicmanViewRendererBrushClass))
#define PICMAN_IS_VIEW_RENDERER_BRUSH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, PICMAN_TYPE_VIEW_RENDERER_BRUSH))
#define PICMAN_IS_VIEW_RENDERER_BRUSH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VIEW_RENDERER_BRUSH))
#define PICMAN_VIEW_RENDERER_BRUSH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VIEW_RENDERER_BRUSH, PicmanViewRendererBrushClass))


typedef struct _PicmanViewRendererBrushClass  PicmanViewRendererBrushClass;

struct _PicmanViewRendererBrush
{
  PicmanViewRenderer parent_instance;

  guint            pipe_timeout_id;
  gint             pipe_animation_index;
};

struct _PicmanViewRendererBrushClass
{
  PicmanViewRendererClass  parent_class;
};


GType   picman_view_renderer_brush_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_VIEW_RENDERER_BRUSH_H__ */
