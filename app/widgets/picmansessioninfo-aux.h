/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessioninfo-aux.h
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

#ifndef __PICMAN_SESSION_INFO_AUX_H__
#define __PICMAN_SESSION_INFO_AUX_H__


/**
 * PicmanSessionInfoAux:
 *
 * Contains arbitrary data in the session management system, used for
 * example by dockables to manage dockable-specific data.
 */
struct _PicmanSessionInfoAux
{
  gchar *name;
  gchar *value;
};


PicmanSessionInfoAux *
             picman_session_info_aux_new            (const gchar         *name,
                                                   const gchar         *value);
void         picman_session_info_aux_free           (PicmanSessionInfoAux  *aux);

GList      * picman_session_info_aux_new_from_props (GObject             *object,
                                                   ...) G_GNUC_NULL_TERMINATED;
void         picman_session_info_aux_set_props      (GObject             *object,
                                                   GList               *aux,
                                                   ...) G_GNUC_NULL_TERMINATED;

void         picman_session_info_aux_serialize      (PicmanConfigWriter    *writer,
                                                   GList               *aux_info);
GTokenType   picman_session_info_aux_deserialize    (GScanner            *scanner,
                                                   GList              **aux_list);


#endif  /* __PICMAN_SESSION_INFO_AUX_H__ */
