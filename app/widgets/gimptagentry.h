/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantagentry.h
 * Copyright (C) 2008 Aurimas Juška <aurisj@svn.gnome.org>
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

#ifndef __PICMAN_TAG_ENTRY_H__
#define __PICMAN_TAG_ENTRY_H__


#define PICMAN_TYPE_TAG_ENTRY            (picman_tag_entry_get_type ())
#define PICMAN_TAG_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TAG_ENTRY, PicmanTagEntry))
#define PICMAN_TAG_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TAG_ENTRY, PicmanTagEntryClass))
#define PICMAN_IS_TAG_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TAG_ENTRY))
#define PICMAN_IS_TAG_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TAG_ENTRY))
#define PICMAN_TAG_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TAG_ENTRY, PicmanTagEntryClass))


typedef struct _PicmanTagEntryClass  PicmanTagEntryClass;

struct _PicmanTagEntry
{
  GtkEntry             parent_instance;

  PicmanTaggedContainer *container;

  /* mask describes the meaning of each char in PicmanTagEntry.
   * It is maintained automatically on insert-text and delete-text
   * events. If manual mask modification is desired, then
   * suppress_mask_update must be increased before calling any
   * function changing entry contents.
   * Meaning of mask chars:
   * u - undefined / unknown (just typed unparsed text)
   * t - tag
   * s - separator
   * w - whitespace.
   */
  GString             *mask;
  GList               *selected_items;
  GList               *common_tags;
  GList               *recent_list;
  gint                 tab_completion_index;
  gint                 internal_operation;
  gint                 suppress_mask_update;
  gint                 suppress_tag_query;
  PicmanTagEntryMode     mode;
  gboolean             description_shown;
  gboolean             has_invalid_tags;
  guint                tag_query_idle_id;
};

struct _PicmanTagEntryClass
{
  GtkEntryClass   parent_class;
};


GType          picman_tag_entry_get_type           (void) G_GNUC_CONST;

GtkWidget    * picman_tag_entry_new                (PicmanTaggedContainer *container,
                                                  PicmanTagEntryMode     mode);

void           picman_tag_entry_set_selected_items (PicmanTagEntry        *entry,
                                                  GList               *items);
gchar       ** picman_tag_entry_parse_tags         (PicmanTagEntry        *entry);
void           picman_tag_entry_set_tag_string     (PicmanTagEntry        *entry,
                                                  const gchar         *tag_string);

const gchar  * picman_tag_entry_get_separator      (void);


#endif  /*  __PICMAN_TAG_ENTRY_H__  */
