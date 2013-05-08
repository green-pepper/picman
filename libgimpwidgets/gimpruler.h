/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#ifndef __PICMAN_RULER_H__
#define __PICMAN_RULER_H__

G_BEGIN_DECLS

#define PICMAN_TYPE_RULER            (picman_ruler_get_type ())
#define PICMAN_RULER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_RULER, PicmanRuler))
#define PICMAN_RULER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_RULER, PicmanRulerClass))
#define PICMAN_IS_RULER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_RULER))
#define PICMAN_IS_RULER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_RULER))
#define PICMAN_RULER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_RULER, PicmanRulerClass))


typedef struct _PicmanRulerClass   PicmanRulerClass;

struct _PicmanRuler
{
  GtkWidget  parent_instance;
};

struct _PicmanRulerClass
{
  GtkWidgetClass  parent_class;

  /* Padding for future expansion */
  void (*_picman_reserved1) (void);
  void (*_picman_reserved2) (void);
  void (*_picman_reserved3) (void);
  void (*_picman_reserved4) (void);
};


GType       picman_ruler_get_type            (void) G_GNUC_CONST;

GtkWidget * picman_ruler_new                 (GtkOrientation  orientation);

void        picman_ruler_add_track_widget    (PicmanRuler      *ruler,
                                            GtkWidget      *widget);
void        picman_ruler_remove_track_widget (PicmanRuler      *ruler,
                                            GtkWidget      *widget);

void        picman_ruler_set_unit            (PicmanRuler      *ruler,
                                            PicmanUnit        unit);
PicmanUnit    picman_ruler_get_unit            (PicmanRuler      *ruler);
void        picman_ruler_set_position        (PicmanRuler      *ruler,
                                            gdouble         position);
gdouble     picman_ruler_get_position        (PicmanRuler      *ruler);
void        picman_ruler_set_range           (PicmanRuler      *ruler,
                                            gdouble         lower,
                                            gdouble         upper,
                                            gdouble         max_size);
void        picman_ruler_get_range           (PicmanRuler      *ruler,
                                            gdouble        *lower,
                                            gdouble        *upper,
                                            gdouble        *max_size);

G_END_DECLS

#endif /* __PICMAN_RULER_H__ */
