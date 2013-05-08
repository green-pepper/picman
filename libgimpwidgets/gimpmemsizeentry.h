/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanmemsizeentry.h
 * Copyright (C) 2000-2003  Sven Neumann <sven@picman.org>
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

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_MEMSIZE_ENTRY_H__
#define __PICMAN_MEMSIZE_ENTRY_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_MEMSIZE_ENTRY            (picman_memsize_entry_get_type ())
#define PICMAN_MEMSIZE_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MEMSIZE_ENTRY, PicmanMemsizeEntry))
#define PICMAN_MEMSIZE_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MEMSIZE_ENTRY, PicmanMemsizeEntryClass))
#define PICMAN_IS_MEMSIZE_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MEMSIZE_ENTRY))
#define PICMAN_IS_MEMSIZE_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MEMSIZE_ENTRY))
#define PICMAN_MEMSIZE_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MEMSIZE_ENTRY, PicmanMemsizeEntryClass))


typedef struct _PicmanMemsizeEntryClass  PicmanMemsizeEntryClass;

struct _PicmanMemsizeEntry
{
  GtkBox         parent_instance;

  /*< private >*/
  guint64        value;
  guint64        lower;
  guint64        upper;

  guint          shift;

  GtkAdjustment *adjustment;
  GtkWidget     *spinbutton;
  GtkWidget     *menu;
};

struct _PicmanMemsizeEntryClass
{
  GtkBoxClass  parent_class;

  void (* value_changed)  (PicmanMemsizeEntry *entry);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_memsize_entry_get_type  (void) G_GNUC_CONST;

GtkWidget * picman_memsize_entry_new       (guint64           value,
                                          guint64           lower,
                                          guint64           upper);
void        picman_memsize_entry_set_value (PicmanMemsizeEntry *entry,
                                          guint64           value);
guint64     picman_memsize_entry_get_value (PicmanMemsizeEntry *entry);


G_END_DECLS

#endif /* __PICMAN_MEMSIZE_ENTRY_H__ */
