/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_USER_INSTALL_H__
#define __PICMAN_USER_INSTALL_H__


typedef struct _PicmanUserInstall PicmanUserInstall;

typedef void  (* PicmanUserInstallLogFunc) (const gchar *message,
                                          gboolean     error,
                                          gpointer     user_data);


PicmanUserInstall * picman_user_install_new  (gboolean          verbose);
gboolean          picman_user_install_run  (PicmanUserInstall  *install);
void              picman_user_install_free (PicmanUserInstall  *install);

void   picman_user_install_set_log_handler (PicmanUserInstall        *install,
                                          PicmanUserInstallLogFunc  log,
                                          gpointer                user_data);


#endif /* __USER_INSTALL_H__ */
