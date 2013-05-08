/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanfileentry.h
 * Copyright (C) 1999-2004 Michael Natterer <mitch@picman.org>
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

#ifndef PICMAN_DISABLE_DEPRECATED

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_FILE_ENTRY_H__
#define __PICMAN_FILE_ENTRY_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_FILE_ENTRY            (picman_file_entry_get_type ())
#define PICMAN_FILE_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FILE_ENTRY, PicmanFileEntry))
#define PICMAN_FILE_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FILE_ENTRY, PicmanFileEntryClass))
#define PICMAN_IS_FILE_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, PICMAN_TYPE_FILE_ENTRY))
#define PICMAN_IS_FILE_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FILE_ENTRY))
#define PICMAN_FILE_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FILE_ENTRY, PicmanFileEntryClass))


typedef struct _PicmanFileEntryClass  PicmanFileEntryClass;

struct _PicmanFileEntry
{
  GtkBox     parent_instance;

  GtkWidget *file_exists;
  GtkWidget *entry;
  GtkWidget *browse_button;

  GtkWidget *file_dialog;

  gchar     *title;
  gboolean   dir_only;
  gboolean   check_valid;
};

struct _PicmanFileEntryClass
{
  GtkBoxClass  parent_class;

  void (* filename_changed) (PicmanFileEntry *entry);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_file_entry_get_type     (void) G_GNUC_CONST;

GtkWidget * picman_file_entry_new          (const gchar   *title,
                                          const gchar   *filename,
                                          gboolean       dir_only,
                                          gboolean       check_valid);

gchar     * picman_file_entry_get_filename (PicmanFileEntry *entry);
void        picman_file_entry_set_filename (PicmanFileEntry *entry,
                                          const gchar   *filename);


G_END_DECLS

#endif /* __PICMAN_FILE_ENTRY_H__ */

#endif /* PICMAN_DISABLE_DEPRECATED */
