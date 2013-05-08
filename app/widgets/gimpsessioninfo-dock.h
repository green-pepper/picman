/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessioninfo-dock.h
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

#ifndef __PICMAN_SESSION_INFO_DOCK_H__
#define __PICMAN_SESSION_INFO_DOCK_H__


/**
 * PicmanSessionInfoDock:
 *
 * Contains information about a dock in the interface.
 */
struct _PicmanSessionInfoDock
{
  /* Type of dock, written to/read from sessionrc. E.g. 'picman-dock' or
   * 'picman-toolbox'
   */
  gchar             *dock_type;

  /* What side this dock is in in single-window mode. Either
   * PICMAN_ARRANGE_LEFT, PICMAN_ARRANGE_RIGHT or -1.
   */
  PicmanAlignmentType  side;

  /* GtkPaned position of this dock */
  gint               position;

  /*  list of PicmanSessionInfoBook  */
  GList             *books;
};

PicmanSessionInfoDock * picman_session_info_dock_new         (const gchar          *dock_type);
void                  picman_session_info_dock_free        (PicmanSessionInfoDock  *dock_info);
void                  picman_session_info_dock_serialize   (PicmanConfigWriter     *writer,
                                                          PicmanSessionInfoDock  *dock);
GTokenType            picman_session_info_dock_deserialize (GScanner             *scanner,
                                                          gint                  scope,
                                                          PicmanSessionInfoDock **info,
                                                          const gchar          *dock_type);
PicmanSessionInfoDock * picman_session_info_dock_from_widget (PicmanDock             *dock);
PicmanDock            * picman_session_info_dock_restore     (PicmanSessionInfoDock  *dock_info,
                                                          PicmanDialogFactory    *factory,
                                                          GdkScreen            *screen,
                                                          PicmanDockContainer    *dock_container);


#endif  /* __PICMAN_SESSION_INFO_DOCK_H__ */
