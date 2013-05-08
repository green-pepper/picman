/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanview-popup.h
 * Copyright (C) 2003-2006 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_VIEW_POPUP_H__
#define __PICMAN_VIEW_POPUP_H__


gboolean   picman_view_popup_show (GtkWidget      *widget,
                                 GdkEventButton *bevent,
                                 PicmanContext    *context,
                                 PicmanViewable   *viewable,
                                 gint            view_width,
                                 gint            view_height,
                                 gboolean        dot_for_dot);


#endif /* __PICMAN_VIEW_POPUP_H__ */
