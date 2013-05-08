/* PicmanXmpModelEntry.h - custom entry widget linked to the xmp model
 *
 * Copyright (C) 2009, Róman Joost <romanofski@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_XMP_MODEL_ENTRY_H__
#define __PICMAN_XMP_MODEL_ENTRY_H__

G_BEGIN_DECLS

#define PICMAN_TYPE_XMP_MODEL_ENTRY               (picman_xmp_model_entry_get_type ())
#define PICMAN_XMP_MODEL_ENTRY(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_XMP_MODEL_ENTRY, PicmanXmpModelEntry))
#define PICMAN_XMP_MODEL_ENTRY_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_XMP_MODEL_ENTRY, XMPModelClass))
#define PICMAN_IS_XMP_MODEL_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_XMP_MODEL_ENTRY))
#define PICMAN_IS_XMP_MODEL_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_XMP_MODEL_ENTRY))
#define PICMAN_XMP_MODEL_ENTRY_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_XMP_MODEL_ENTRY, XMPModelClass))


typedef struct _PicmanXmpModelEntry       PicmanXmpModelEntry;
typedef struct _PicmanXmpModelEntryClass  PicmanXmpModelEntryClass;


struct _PicmanXmpModelEntryClass
{
  GtkEntryClass parent_class;

  void          (*picman_xmp_model_set_text) (PicmanXmpModelEntry *entry,
                                            const gchar       *value);

  const gchar * (*picman_xmp_model_get_text) (PicmanXmpModelEntry *entry);
};

struct _PicmanXmpModelEntry
{
  GtkEntry   parent_instance;
  gpointer   priv;
};


GType         picman_xmp_model_entry_get_type (void) G_GNUC_CONST;

GtkWidget   * picman_xmp_model_entry_new      (const gchar       *schema_uri,
                                             const gchar       *property,
                                             XMPModel          *xmp_model);

void          picman_xmp_model_set_text       (PicmanXmpModelEntry *entry,
                                             const gchar       *value);

const gchar * picman_xmp_model_get_text       (PicmanXmpModelEntry *entry);

G_END_DECLS

#endif /* __PICMAN_XMP_MODEL_ENTRY_H__ */
