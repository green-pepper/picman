/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessioninfo-book.h
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

#ifndef __PICMAN_SESSION_INFO_BOOK_H__
#define __PICMAN_SESSION_INFO_BOOK_H__


/**
 * PicmanSessionInfoBook:
 *
 * Contains information about a book (a GtkNotebook of dockables) in
 * the interface.
 */
struct _PicmanSessionInfoBook
{
  gint   position;
  gint   current_page;

  /*  list of PicmanSessionInfoDockable  */
  GList *dockables;
};


PicmanSessionInfoBook *
             picman_session_info_book_new         (void);
void         picman_session_info_book_free        (PicmanSessionInfoBook  *info);

void         picman_session_info_book_serialize   (PicmanConfigWriter     *writer,
                                                 PicmanSessionInfoBook  *book);
GTokenType   picman_session_info_book_deserialize (GScanner             *scanner,
                                                 gint                  scope,
                                                 PicmanSessionInfoBook **book);

PicmanSessionInfoBook *
             picman_session_info_book_from_widget (PicmanDockbook         *dockbook);

PicmanDockbook * picman_session_info_book_restore   (PicmanSessionInfoBook  *info,
                                                 PicmanDock             *dock);


#endif  /* __PICMAN_SESSION_INFO_BOOK_H__ */
