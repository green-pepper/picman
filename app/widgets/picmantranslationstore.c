/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantranslationstore.c
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

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"

#include "widgets-types.h"

#include "picmantranslationstore.h"

#include "picman-intl.h"


struct _PicmanTranslationStoreClass
{
  PicmanLanguageStoreClass  parent_class;
};

struct _PicmanTranslationStore
{
  PicmanLanguageStore  parent_instance;

  GHashTable        *map;
};


static void   picman_translation_store_constructed (GObject              *object);

static void   picman_translation_store_add         (PicmanLanguageStore    *store,
                                                  const gchar          *lang,
                                                  const gchar          *code);

static void   picman_translation_store_populate    (PicmanTranslationStore *store);


G_DEFINE_TYPE (PicmanTranslationStore, picman_translation_store,
               PICMAN_TYPE_LANGUAGE_STORE)

#define parent_class picman_translation_store_parent_class


static void
picman_translation_store_class_init (PicmanTranslationStoreClass *klass)
{
  GObjectClass           *object_class = G_OBJECT_CLASS (klass);
  PicmanLanguageStoreClass *store_class  = PICMAN_LANGUAGE_STORE_CLASS (klass);

  object_class->constructed = picman_translation_store_constructed;

  store_class->add          = picman_translation_store_add;
}

static void
picman_translation_store_init (PicmanTranslationStore *store)
{
  store->map = g_hash_table_new_full (g_str_hash, g_str_equal,
                                      (GDestroyNotify) g_free,
                                      (GDestroyNotify) g_free);
}

static void
picman_translation_store_constructed (GObject *object)
{
  PicmanTranslationStore *store = PICMAN_TRANSLATION_STORE (object);
  gchar                *label;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_translation_store_populate (store);

  /*  we don't need the map any longer  */
  g_hash_table_unref (store->map);
  store->map = NULL;

  /*  add special entries for system locale and for "C"  */
  PICMAN_LANGUAGE_STORE_CLASS (parent_class)->add (PICMAN_LANGUAGE_STORE (store),
                                                 _("System Language"),
                                                 NULL);
  label = g_strdup_printf ("%s [%s]", _("English"), "en_US");
  PICMAN_LANGUAGE_STORE_CLASS (parent_class)->add (PICMAN_LANGUAGE_STORE (store),
                                                 label, "en_US");
  g_free (label);
}

static const gchar *
picman_translation_store_map (PicmanTranslationStore *store,
                            const gchar          *locale)
{
  const gchar *lang;

  /*  A locale directory name is typically of the form language[_territory]  */
  lang = g_hash_table_lookup (store->map, locale);

  if (! lang)
    {
      /*  strip off the territory suffix  */
      const gchar *delimiter = strchr (locale, '_');

      if (delimiter)
        {
          gchar *copy;

          copy = g_strndup (locale, delimiter - locale);
          lang = g_hash_table_lookup (store->map, copy);
          g_free (copy);
        }
    }

  return lang;
}

static void
picman_translation_store_populate (PicmanTranslationStore *store)
{
  /*  FIXME: this should better be done asynchronously  */
  GDir        *dir = g_dir_open (picman_locale_directory (), 0, NULL);
  const gchar *dirname;

  if (! dir)
    return;

  while ((dirname = g_dir_read_name (dir)) != NULL)
    {
      gchar *filename = g_build_filename (picman_locale_directory (),
                                          dirname,
                                          "LC_MESSAGES",
                                          GETTEXT_PACKAGE ".mo",
                                          NULL);
      if (g_file_test (filename, G_FILE_TEST_EXISTS))
        {
          const gchar *lang = picman_translation_store_map (store, dirname);

          if (lang)
            {
              PicmanLanguageStore *language_store = PICMAN_LANGUAGE_STORE (store);
              gchar             *label;

              label = g_strdup_printf ("%s [%s]", lang, dirname);

              PICMAN_LANGUAGE_STORE_CLASS (parent_class)->add (language_store,
                                                             label, dirname);
              g_free (label);
            }
        }

      g_free (filename);
    }

  g_dir_close (dir);
}

static void
picman_translation_store_add (PicmanLanguageStore *store,
                            const gchar       *lang,
                            const gchar       *code)
{
  g_hash_table_replace (PICMAN_TRANSLATION_STORE (store)->map,
                        g_strdup (code),
                        g_strdup (lang));
}

GtkListStore *
picman_translation_store_new (void)
{
  return g_object_new (PICMAN_TYPE_TRANSLATION_STORE, NULL);
}
