/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantag.h
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

#ifndef __PICMAN_TAG_H__
#define __PICMAN_TAG_H__


#include <glib-object.h>


#define PICMAN_TYPE_TAG            (picman_tag_get_type ())
#define PICMAN_TAG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TAG, PicmanTag))
#define PICMAN_TAG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TAG, PicmanTagClass))
#define PICMAN_IS_TAG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TAG))
#define PICMAN_IS_TAG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TAG))
#define PICMAN_TAG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TAG, PicmanTagClass))


typedef struct _PicmanTagClass    PicmanTagClass;

struct _PicmanTag
{
  GObject parent_instance;

  GQuark  tag;
  GQuark  collate_key;

  gboolean internal; /* Tags that are not serialized to disk */
};

struct _PicmanTagClass
{
  GObjectClass parent_class;
};

GType         picman_tag_get_type            (void) G_GNUC_CONST;

PicmanTag     * picman_tag_new                 (const gchar    *tag_string);
PicmanTag     * picman_tag_try_new             (const gchar    *tag_string);

const gchar * picman_tag_get_name            (PicmanTag        *tag);
guint         picman_tag_get_hash            (PicmanTag        *tag);

gboolean      picman_tag_get_internal        (PicmanTag        *tag);
void          picman_tag_set_internal        (PicmanTag        *tag,
                                            gboolean        internal);

gboolean      picman_tag_equals              (const PicmanTag  *tag,
                                            const PicmanTag  *other);
gint          picman_tag_compare_func        (const void     *p1,
                                            const void     *p2);
gint          picman_tag_compare_with_string (PicmanTag        *tag,
                                            const gchar    *tag_string);
gboolean      picman_tag_has_prefix          (PicmanTag        *tag,
                                            const gchar    *prefix_string);
gchar       * picman_tag_string_make_valid   (const gchar    *tag_string);
gboolean      picman_tag_is_tag_separator    (gunichar        c);

void          picman_tag_or_null_ref         (PicmanTag        *tag_or_null);
void          picman_tag_or_null_unref       (PicmanTag        *tag_or_null);


#endif /* __PICMAN_TAG_H__ */
