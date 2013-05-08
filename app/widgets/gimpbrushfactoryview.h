/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrushfactoryview.h
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_BRUSH_FACTORY_VIEW_H__
#define __PICMAN_BRUSH_FACTORY_VIEW_H__

#include "picmandatafactoryview.h"


#define PICMAN_TYPE_BRUSH_FACTORY_VIEW            (picman_brush_factory_view_get_type ())
#define PICMAN_BRUSH_FACTORY_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BRUSH_FACTORY_VIEW, PicmanBrushFactoryView))
#define PICMAN_BRUSH_FACTORY_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BRUSH_FACTORY_VIEW, PicmanBrushFactoryViewClass))
#define PICMAN_IS_BRUSH_FACTORY_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BRUSH_FACTORY_VIEW))
#define PICMAN_IS_BRUSH_FACTORY_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BRUSH_FACTORY_VIEW))
#define PICMAN_BRUSH_FACTORY_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BRUSH_FACTORY_VIEW, PicmanBrushFactoryViewClass))


typedef struct _PicmanBrushFactoryViewClass  PicmanBrushFactoryViewClass;

struct _PicmanBrushFactoryView
{
  PicmanDataFactoryView  parent_instance;

  GtkWidget           *spacing_scale;
  GtkAdjustment       *spacing_adjustment;

  gboolean             change_brush_spacing;
  GQuark               spacing_changed_handler_id;
};

struct _PicmanBrushFactoryViewClass
{
  PicmanDataFactoryViewClass  parent_class;
};


GType       picman_brush_factory_view_get_type (void) G_GNUC_CONST;

GtkWidget * picman_brush_factory_view_new      (PicmanViewType     view_type,
                                              PicmanDataFactory *factory,
                                              PicmanContext     *context,
                                              gboolean         change_brush_spacing,
                                              gint             view_size,
                                              gint             view_border_width,
                                              PicmanMenuFactory *menu_factory);


#endif  /*  __PICMAN_BRUSH_FACTORY_VIEW_H__  */
