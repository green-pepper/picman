/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfontlist.h
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
 *                    Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_FONT_LIST_H__
#define __PICMAN_FONT_LIST_H__


#include "core/picmanlist.h"


#define PICMAN_TYPE_FONT_LIST            (picman_font_list_get_type ())
#define PICMAN_FONT_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FONT_LIST, PicmanFontList))
#define PICMAN_FONT_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FONT_LIST, PicmanFontListClass))
#define PICMAN_IS_FONT_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FONT_LIST))
#define PICMAN_IS_FONT_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FONT_LIST))
#define PICMAN_FONT_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FONT_LIST, PicmanFontListClass))


typedef struct _PicmanFontListClass PicmanFontListClass;

struct _PicmanFontList
{
  PicmanList  parent_instance;

  gdouble   xresolution;
  gdouble   yresolution;
};

struct _PicmanFontListClass
{
  PicmanListClass  parent_class;
};


GType           picman_font_list_get_type (void) G_GNUC_CONST;

PicmanContainer * picman_font_list_new      (gdouble       xresolution,
                                         gdouble       yresolution);
void            picman_font_list_restore  (PicmanFontList *list);


#endif  /*  __PICMAN_FONT_LIST_H__  */
