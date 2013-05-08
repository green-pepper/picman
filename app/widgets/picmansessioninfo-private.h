/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessioninfo-private.h
 * Copyright (C) 2001-2008 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_SESSION_INFO_PRIVATE_H__
#define __PICMAN_SESSION_INFO_PRIVATE_H__


struct _PicmanSessionInfoPrivate
{
  /*  the dialog factory entry for object we have session info for
   *  note that pure "dock" entries don't have any factory entry
   */
  PicmanDialogFactoryEntry *factory_entry;

  gint                    x;
  gint                    y;
  gint                    width;
  gint                    height;
  gboolean                right_align;
  gboolean                bottom_align;

  /*  only valid while restoring and saving the session  */
  gboolean                open;
  gint                    screen;

  /*  dialog specific list of PicmanSessionInfoAux  */
  GList                  *aux_info;

  GtkWidget              *widget;

  /*  list of PicmanSessionInfoDock  */
  GList                  *docks;
};


#endif /* __PICMAN_SESSION_INFO_PRIVATE_H__ */
