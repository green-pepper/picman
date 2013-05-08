/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanDBusService
 * Copyright (C) 2007, 2008 Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_DBUS_SERVICE_H__
#define __PICMAN_DBUS_SERVICE_H__

G_BEGIN_DECLS


#define PICMAN_DBUS_SERVICE_NAME       "org.picman.PICMAN.UI"
#define PICMAN_DBUS_SERVICE_PATH       "/org/picman/PICMAN/UI"
#define PICMAN_DBUS_SERVICE_INTERFACE  "org.picman.PICMAN.UI"


#define PICMAN_TYPE_DBUS_SERVICE            (picman_dbus_service_get_type ())
#define PICMAN_DBUS_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DBUS_SERVICE, PicmanDBusService))
#define PICMAN_DBUS_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DBUS_SERVICE, PicmanDBusServiceClass))
#define PICMAN_IS_DBUS_SERVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DBUS_SERVICE))
#define PICMAN_IS_DBUS_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DBUS_SERVICE))
#define PICMAN_DBUS_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DBUS_SERVICE, PicmanDBusServiceClass))


typedef struct _PicmanDBusService      PicmanDBusService;
typedef struct _PicmanDBusServiceClass PicmanDBusServiceClass;

struct _PicmanDBusService
{
  GObject  parent_instance;

  Picman    *picman;
  GQueue  *queue;
  GSource *source;
};

struct _PicmanDBusServiceClass
{
  GObjectClass  parent_class;

  /*  signals  */
  void (* opened) (PicmanDBusService *service,
		   const gchar     *uri);
};


GType     picman_dbus_service_get_type    (void) G_GNUC_CONST;

GObject * picman_dbus_service_new         (Picman            *picman);

gboolean  picman_dbus_service_open        (PicmanDBusService  *service,
                                         const gchar      *uri,
                                         gboolean         *success,
                                         GError          **dbus_error);
gboolean  picman_dbus_service_open_as_new (PicmanDBusService  *service,
                                         const gchar      *uri,
                                         gboolean         *success,
                                         GError          **dbus_error);
gboolean  picman_dbus_service_activate    (PicmanDBusService  *service,
                                         GError          **dbus_error);


G_END_DECLS

#endif /* __PICMAN_DBUS_SERVICE_H__ */
