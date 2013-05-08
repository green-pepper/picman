/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanThrobberAction
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

#ifndef __PICMAN_THROBBER_ACTION_H__
#define __PICMAN_THROBBER_ACTION_H__

G_BEGIN_DECLS

#define PICMAN_TYPE_THROBBER_ACTION            (picman_throbber_action_get_type ())
#define PICMAN_THROBBER_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_THROBBER_ACTION, PicmanThrobberAction))
#define PICMAN_THROBBER_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_THROBBER_ACTION, PicmanThrobberActionClass))
#define PICMAN_IS_THROBBER_ACTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_THROBBER_ACTION))
#define PICMAN_IS_THROBBER_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), PICMAN_TYPE_THROBBER_ACTION))
#define PICMAN_THROBBER_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PICMAN_TYPE_THROBBER_ACTION, PicmanThrobberActionClass))


typedef GtkAction       PicmanThrobberAction;
typedef GtkActionClass  PicmanThrobberActionClass;


GType       picman_throbber_action_get_type (void) G_GNUC_CONST;

GtkAction * picman_throbber_action_new      (const gchar *name,
                                           const gchar *label,
                                           const gchar *tooltip,
                                           const gchar *stock_id);


G_END_DECLS

#endif /* __PICMAN_THROBBER_ACTION_H__ */
