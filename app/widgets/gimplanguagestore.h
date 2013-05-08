/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanlanguagestore.h
 * Copyright (C) 2008, 2009  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_LANGUAGE_STORE_H__
#define __PICMAN_LANGUAGE_STORE_H__


enum
{
  PICMAN_LANGUAGE_STORE_LABEL,
  PICMAN_LANGUAGE_STORE_CODE
};


#define PICMAN_TYPE_LANGUAGE_STORE            (picman_language_store_get_type ())
#define PICMAN_LANGUAGE_STORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_LANGUAGE_STORE, PicmanLanguageStore))
#define PICMAN_LANGUAGE_STORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_LANGUAGE_STORE, PicmanLanguageStoreClass))
#define PICMAN_IS_LANGUAGE_STORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_LANGUAGE_STORE))
#define PICMAN_IS_LANGUAGE_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_LANGUAGE_STORE))
#define PICMAN_LANGUAGE_STORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_LANGUAGE_STORE, PicmanLanguageStoreClass))


typedef struct _PicmanLanguageStoreClass  PicmanLanguageStoreClass;

struct _PicmanLanguageStoreClass
{
  GtkListStoreClass  parent_class;

  void (* add) (PicmanLanguageStore *store,
                const gchar       *label,
                const gchar       *code);
};

struct _PicmanLanguageStore
{
  GtkListStore     parent_instance;
};


GType          picman_language_store_get_type (void) G_GNUC_CONST;

GtkListStore * picman_language_store_new      (void);

gboolean       picman_language_store_lookup   (PicmanLanguageStore *store,
                                             const gchar       *code,
                                             GtkTreeIter       *iter);

/*  used from picmanlanguagestore-parser.c  */
void           picman_language_store_add      (PicmanLanguageStore *store,
                                             const gchar       *label,
                                             const gchar       *code);


#endif  /* __PICMAN_LANGUAGE_STORE_H__ */
