/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_DOCUMENT_LIST_H__
#define __PICMAN_DOCUMENT_LIST_H__

#include "core/picmanlist.h"


#define PICMAN_TYPE_DOCUMENT_LIST           (picman_document_list_get_type ())
#define PICMAN_DOCUMENT_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DOCUMENT_LIST, PicmanDocumentList))
#define PICMAN_DOCUMENT_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DOCUMENT_LIST, PicmanDocumentListClass))
#define PICMAN_IS_DOCUMENT_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DOCUMENT_LIST))
#define PICMAN_IS_DOCUMENT_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DOCUMENT_LIST))


typedef struct _PicmanDocumentListClass PicmanDocumentListClass;

struct _PicmanDocumentList
{
  PicmanList  parent_instance;

  Picman     *picman;
};

struct _PicmanDocumentListClass
{
  PicmanListClass  parent_class;
};


GType           picman_document_list_get_type (void) G_GNUC_CONST;
PicmanContainer * picman_document_list_new      (Picman             *picman);

PicmanImagefile * picman_document_list_add_uri  (PicmanDocumentList *document_list,
                                             const gchar      *uri,
                                             const gchar      *mime_type);


#endif  /*  __PICMAN_DOCUMENT_LIST_H__  */
