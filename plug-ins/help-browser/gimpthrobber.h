/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanThrobber
 * Copyright (C) 2005  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_THROBBER_H__
#define __PICMAN_THROBBER_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_THROBBER            (picman_throbber_get_type ())
#define PICMAN_THROBBER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_THROBBER, PicmanThrobber))
#define PICMAN_THROBBER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_THROBBER, PicmanThrobberClass))
#define PICMAN_IS_THROBBER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_THROBBER))
#define PICMAN_IS_THROBBER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_THROBBER))
#define PICMAN_THROBBER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PICMAN_TYPE_THROBBER, PicmanThrobberClass))


typedef struct _PicmanThrobber        PicmanThrobber;
typedef struct _PicmanThrobberClass   PicmanThrobberClass;
typedef struct _PicmanThrobberPrivate PicmanThrobberPrivate;

struct _PicmanThrobber
{
  GtkToolItem          parent;

  /*< private >*/
  PicmanThrobberPrivate *priv;
};

struct _PicmanThrobberClass
{
  GtkToolItemClass parent_class;

  /* signal */
  void  (* clicked) (PicmanThrobber *button);
};

GType         picman_throbber_get_type      (void) G_GNUC_CONST;

GtkToolItem * picman_throbber_new           (const gchar  *stock_id);
void          picman_throbber_set_stock_id  (PicmanThrobber *button,
                                           const gchar  *stock_id);
const gchar * picman_throbber_get_stock_id  (PicmanThrobber *button);
void          picman_throbber_set_image     (PicmanThrobber *button,
                                           GtkWidget    *image);
GtkWidget   * picman_throbber_get_image     (PicmanThrobber *button);


G_END_DECLS

#endif /* __PICMAN_THROBBER_H__ */
