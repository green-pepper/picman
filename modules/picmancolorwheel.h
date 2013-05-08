/* HSV color selector for GTK+
 *
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Simon Budig <Simon.Budig@unix-ag.org> (original code)
 *          Federico Mena-Quintero <federico@picman.org> (cleanup for GTK+)
 *          Jonathan Blandford <jrb@redhat.com> (cleanup for GTK+)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __PICMAN_COLOR_WHEEL_H__
#define __PICMAN_COLOR_WHEEL_H__

G_BEGIN_DECLS

#define PICMAN_TYPE_COLOR_WHEEL            (picman_color_wheel_get_type ())
#define PICMAN_COLOR_WHEEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_WHEEL, PicmanColorWheel))
#define PICMAN_COLOR_WHEEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_WHEEL, PicmanColorWheelClass))
#define PICMAN_IS_COLOR_WHEEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_WHEEL))
#define PICMAN_IS_COLOR_WHEEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_WHEEL))
#define PICMAN_COLOR_WHEEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_WHEEL, PicmanColorWheelClass))


typedef struct _PicmanColorWheel      PicmanColorWheel;
typedef struct _PicmanColorWheelClass PicmanColorWheelClass;

struct _PicmanColorWheel
{
  GtkWidget parent_instance;

  /* Private data */
  gpointer priv;
};

struct _PicmanColorWheelClass
{
  GtkWidgetClass parent_class;

  /* Notification signals */
  void (* changed) (PicmanColorWheel   *wheel);

  /* Keybindings */
  void (* move)    (PicmanColorWheel   *wheel,
                    GtkDirectionType  type);

  /* Padding for future expansion */
  void (*_picman_reserved1) (void);
  void (*_picman_reserved2) (void);
  void (*_picman_reserved3) (void);
  void (*_picman_reserved4) (void);
};


GType       picman_color_wheel_get_type          (void) G_GNUC_CONST;
GtkWidget * picman_color_wheel_new               (void);

void        picman_color_wheel_set_color         (PicmanColorWheel *wheel,
                                                double          h,
                                                double          s,
                                                double          v);
void        picman_color_wheel_get_color         (PicmanColorWheel *wheel,
                                                gdouble        *h,
                                                gdouble        *s,
                                                gdouble        *v);

void        picman_color_wheel_set_ring_fraction (PicmanColorWheel *wheel,
                                                gdouble         fraction);
gdouble     picman_color_wheel_get_ring_fraction (PicmanColorWheel *wheel);

gboolean    picman_color_wheel_is_adjusting      (PicmanColorWheel *wheel);

G_END_DECLS

#endif /* __PICMAN_COLOR_WHEEL_H__ */
