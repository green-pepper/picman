/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanscalebutton.h
 * Copyright (C) 2008 Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_SCALE_BUTTON_H__
#define __PICMAN_SCALE_BUTTON_H__


#define PICMAN_TYPE_SCALE_BUTTON            (picman_scale_button_get_type ())
#define PICMAN_SCALE_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SCALE_BUTTON, PicmanScaleButton))
#define PICMAN_SCALE_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SCALE_BUTTON, PicmanScaleButtonClass))
#define PICMAN_IS_SCALE_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SCALE_BUTTON))
#define PICMAN_IS_SCALE_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), PICMAN_TYPE_SCALE_BUTTON))
#define PICMAN_SCALE_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PICMAN_TYPE_SCALE_BUTTON, PicmanScaleButtonClass))


typedef struct _PicmanScaleButtonClass PicmanScaleButtonClass;

struct _PicmanScaleButton
{
  GtkScaleButton      parent_instance;
};

struct _PicmanScaleButtonClass
{
  GtkScaleButtonClass parent_class;
};


GType       picman_scale_button_get_type (void) G_GNUC_CONST;

GtkWidget * picman_scale_button_new      (gdouble value,
                                        gdouble min,
                                        gdouble max);


#endif  /* __PICMAN_SCALE_BUTTON_H__ */
