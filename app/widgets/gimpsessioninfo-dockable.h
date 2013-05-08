/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessioninfo-dockable.h
 * Copyright (C) 2001-2007 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_SESSION_INFO_DOCKABLE_H__
#define __PICMAN_SESSION_INFO_DOCKABLE_H__


/**
 * PicmanSessionInfoDockable:
 *
 * Contains information about a dockable in the interface.
 */
struct _PicmanSessionInfoDockable
{
  gchar        *identifier;
  gboolean      locked;
  PicmanTabStyle  tab_style;
  gint          view_size;

  /*  dialog specific list of PicmanSessionInfoAux  */
  GList        *aux_info;
};


PicmanSessionInfoDockable *
               picman_session_info_dockable_new         (void);
void           picman_session_info_dockable_free        (PicmanSessionInfoDockable  *info);

void           picman_session_info_dockable_serialize   (PicmanConfigWriter         *writer,
                                                       PicmanSessionInfoDockable  *dockable);
GTokenType     picman_session_info_dockable_deserialize (GScanner                 *scanner,
                                                       gint                      scope,
                                                       PicmanSessionInfoDockable **dockable);

PicmanSessionInfoDockable *
               picman_session_info_dockable_from_widget (PicmanDockable             *dockable);

PicmanDockable * picman_session_info_dockable_restore     (PicmanSessionInfoDockable  *info,
                                                       PicmanDock                 *dock);


#endif  /* __PICMAN_SESSION_INFO_DOCKABLE_H__ */
