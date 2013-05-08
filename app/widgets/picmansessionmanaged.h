/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessionmanaged.h
 * Copyright (C) 2011 Martin Nordholts <martinn@src.gnome.org>
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

#ifndef __PICMAN_SESSION_MANAGED_H__
#define __PICMAN_SESSION_MANAGED_H__


#define PICMAN_TYPE_SESSION_MANAGED               (picman_session_managed_interface_get_type ())
#define PICMAN_SESSION_MANAGED(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SESSION_MANAGED, PicmanSessionManaged))
#define PICMAN_IS_SESSION_MANAGED(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SESSION_MANAGED))
#define PICMAN_SESSION_MANAGED_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_SESSION_MANAGED, PicmanSessionManagedInterface))


typedef struct _PicmanSessionManagedInterface PicmanSessionManagedInterface;

struct _PicmanSessionManagedInterface
{
  GTypeInterface base_iface;

  /*  virtual functions  */
  GList           * (* get_aux_info)      (PicmanSessionManaged *session_managed);
  void              (* set_aux_info)      (PicmanSessionManaged *session_managed,
                                           GList              *aux_info);
};


GType              picman_session_managed_interface_get_type  (void) G_GNUC_CONST;
GList            * picman_session_managed_get_aux_info        (PicmanSessionManaged *session_managed);
void               picman_session_managed_set_aux_info        (PicmanSessionManaged *session_managed,
                                                             GList              *aux_info);


#endif  /*  __PICMAN_SESSION_MANAGED_H__  */
