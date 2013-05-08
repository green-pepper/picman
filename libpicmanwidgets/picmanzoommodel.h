/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanzoommodel.h
 * Copyright (C) 2005  David Odin <dindinx@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_ZOOM_MODEL_H__
#define __PICMAN_ZOOM_MODEL_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_ZOOM_MODEL            (picman_zoom_model_get_type ())
#define PICMAN_ZOOM_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ZOOM_MODEL, PicmanZoomModel))
#define PICMAN_ZOOM_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ZOOM_MODEL, PicmanZoomModelClass))
#define PICMAN_IS_ZOOM_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ZOOM_MODEL))
#define PICMAN_IS_ZOOM_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ZOOM_MODEL))
#define PICMAN_ZOOM_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ZOOM_MODEL, PicmanZoomModel))


typedef struct _PicmanZoomModelClass  PicmanZoomModelClass;

struct _PicmanZoomModel
{
  GObject   parent_instance;

  /*< private >*/
  gpointer  priv;
};

struct _PicmanZoomModelClass
{
  GObjectClass  parent_class;

  void (* zoomed) (PicmanZoomModel *model,
                   gdouble        old_factor,
                   gdouble        new_factor);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType           picman_zoom_model_get_type     (void) G_GNUC_CONST;

PicmanZoomModel * picman_zoom_model_new          (void);
void            picman_zoom_model_set_range    (PicmanZoomModel      *model,
                                              gdouble             min,
                                              gdouble             max);
void            picman_zoom_model_zoom         (PicmanZoomModel      *model,
                                              PicmanZoomType        zoom_type,
                                              gdouble             scale);
gdouble         picman_zoom_model_get_factor   (PicmanZoomModel      *model);
void            picman_zoom_model_get_fraction (PicmanZoomModel      *model,
                                              gint               *numerator,
                                              gint               *denominator);

GtkWidget     * picman_zoom_button_new         (PicmanZoomModel      *model,
                                              PicmanZoomType        zoom_type,
                                              GtkIconSize         icon_size);

gdouble         picman_zoom_model_zoom_step    (PicmanZoomType        zoom_type,
                                              gdouble             scale);

G_END_DECLS

#endif /* __PICMAN_ZOOM_MODEL_H__ */
