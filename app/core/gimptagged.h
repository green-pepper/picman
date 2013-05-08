/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmantagged.h
 * Copyright (C) 2008  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_TAGGED_H__
#define __PICMAN_TAGGED_H__


#define PICMAN_TYPE_TAGGED               (picman_tagged_interface_get_type ())
#define PICMAN_IS_TAGGED(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TAGGED))
#define PICMAN_TAGGED(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TAGGED, PicmanTagged))
#define PICMAN_TAGGED_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_TAGGED, PicmanTaggedInterface))


typedef struct _PicmanTaggedInterface PicmanTaggedInterface;

struct _PicmanTaggedInterface
{
  GTypeInterface base_iface;

  /*  signals            */
  void       (* tag_added)      (PicmanTagged *tagged,
                                 PicmanTag    *tag);
  void       (* tag_removed)    (PicmanTagged *tagged,
                                 PicmanTag    *tag);

  /*  virtual functions  */
  gboolean   (* add_tag)        (PicmanTagged *tagged,
                                 PicmanTag    *tag);
  gboolean   (* remove_tag)     (PicmanTagged *tagged,
                                 PicmanTag    *tag);
  GList    * (* get_tags)       (PicmanTagged *tagged);
  gchar    * (* get_identifier) (PicmanTagged *tagged);
  gchar    * (* get_checksum)   (PicmanTagged *tagged);
};


GType      picman_tagged_interface_get_type (void) G_GNUC_CONST;

void       picman_tagged_add_tag            (PicmanTagged *tagged,
                                           PicmanTag    *tag);
void       picman_tagged_remove_tag         (PicmanTagged *tagged,
                                           PicmanTag    *tag);

void       picman_tagged_set_tags           (PicmanTagged *tagged,
                                           GList      *tags);
GList    * picman_tagged_get_tags           (PicmanTagged *tagged);

gchar    * picman_tagged_get_identifier     (PicmanTagged *tagged);
gchar    * picman_tagged_get_checksum       (PicmanTagged *tagged);

gboolean   picman_tagged_has_tag            (PicmanTagged *tagged,
                                           PicmanTag    *tag);


#endif  /* __PICMAN_TAGGED_H__ */
