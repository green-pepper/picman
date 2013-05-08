/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanspinscale.h
 * Copyright (C) 2010  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_SPIN_SCALE_H__
#define __PICMAN_SPIN_SCALE_H__


#define PICMAN_TYPE_SPIN_SCALE            (picman_spin_scale_get_type ())
#define PICMAN_SPIN_SCALE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SPIN_SCALE, PicmanSpinScale))
#define PICMAN_SPIN_SCALE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SPIN_SCALE, PicmanSpinScaleClass))
#define PICMAN_IS_SPIN_SCALE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SPIN_SCALE))
#define PICMAN_IS_SPIN_SCALE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SPIN_SCALE))
#define PICMAN_SPIN_SCALE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SPIN_SCALE, PicmanSpinScaleClass))


typedef struct _PicmanSpinScale      PicmanSpinScale;
typedef struct _PicmanSpinScaleClass PicmanSpinScaleClass;

struct _PicmanSpinScale
{
  GtkSpinButton  parent_instance;
};

struct _PicmanSpinScaleClass
{
  GtkSpinButtonClass  parent_class;
};


GType       picman_spin_scale_get_type           (void) G_GNUC_CONST;

GtkWidget * picman_spin_scale_new                (GtkAdjustment *adjustment,
                                                const gchar   *label,
                                                gint           digits);

void        picman_spin_scale_set_scale_limits   (PicmanSpinScale *scale,
                                                gdouble        lower,
                                                gdouble        upper);
void        picman_spin_scale_unset_scale_limits (PicmanSpinScale *scale);
gboolean    picman_spin_scale_get_scale_limits   (PicmanSpinScale *scale,
                                                gdouble       *lower,
                                                gdouble       *upper);

void        picman_spin_scale_set_gamma          (PicmanSpinScale *scale,
                                                gdouble        gamma);
gdouble     picman_spin_scale_get_gamma          (PicmanSpinScale *scale);


#endif  /*  __PICMAN_SPIN_SCALE_H__  */
