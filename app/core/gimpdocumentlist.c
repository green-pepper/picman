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

#include "config.h"

#include <gegl.h>

#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "config/picmancoreconfig.h"

#include "picman.h"
#include "picmandocumentlist.h"
#include "picmanimagefile.h"


G_DEFINE_TYPE (PicmanDocumentList, picman_document_list, PICMAN_TYPE_LIST)


static void
picman_document_list_class_init (PicmanDocumentListClass *klass)
{
}

static void
picman_document_list_init (PicmanDocumentList *list)
{
}

PicmanContainer *
picman_document_list_new (Picman *picman)
{
  PicmanDocumentList *document_list;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  document_list = g_object_new (PICMAN_TYPE_DOCUMENT_LIST,
                                "name",          "document-list",
                                "children-type", PICMAN_TYPE_IMAGEFILE,
                                "policy",        PICMAN_CONTAINER_POLICY_STRONG,
                                NULL);

  document_list->picman = picman;

  return PICMAN_CONTAINER (document_list);
}

PicmanImagefile *
picman_document_list_add_uri (PicmanDocumentList *document_list,
                            const gchar      *uri,
                            const gchar      *mime_type)
{
  Picman          *picman;
  PicmanImagefile *imagefile;
  PicmanContainer *container;

  g_return_val_if_fail (PICMAN_IS_DOCUMENT_LIST (document_list), NULL);
  g_return_val_if_fail (uri != NULL, NULL);

  picman = document_list->picman;

  container = PICMAN_CONTAINER (document_list);

  imagefile = (PicmanImagefile *) picman_container_get_child_by_name (container,
                                                                  uri);

  if (imagefile)
    {
      picman_container_reorder (container, PICMAN_OBJECT (imagefile), 0);
    }
  else
    {
      imagefile = picman_imagefile_new (picman, uri);
      picman_container_add (container, PICMAN_OBJECT (imagefile));
      g_object_unref (imagefile);
    }

  picman_imagefile_set_mime_type (imagefile, mime_type);

  if (picman->config->save_document_history)
    picman_recent_list_add_uri (picman, uri, mime_type);

  return imagefile;
}
