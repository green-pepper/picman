/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantagcache.c
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

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "config/picmanxmlparser.h"

#include "picman-utils.h"
#include "picmancontext.h"
#include "picmandata.h"
#include "picmanlist.h"
#include "picmantag.h"
#include "picmantagcache.h"
#include "picmantagged.h"

#include "picman-intl.h"


#define PICMAN_TAG_CACHE_FILE  "tags.xml"

/* #define DEBUG_PICMAN_TAG_CACHE  1 */


enum
{
  PROP_0,
  PROP_PICMAN
};


typedef struct
{
  GQuark  identifier;
  GQuark  checksum;
  GList  *tags;
  guint   referenced : 1;
} PicmanTagCacheRecord;

typedef struct
{
  GArray             *records;
  PicmanTagCacheRecord  current_record;
} PicmanTagCacheParseData;

struct _PicmanTagCachePriv
{
  GArray *records;
  GList  *containers;
};


static void          picman_tag_cache_finalize           (GObject                *object);

static gint64        picman_tag_cache_get_memsize        (PicmanObject             *object,
                                                        gint64                 *gui_size);
static void          picman_tag_cache_object_initialize  (PicmanTagged             *tagged,
                                                        PicmanTagCache           *cache);
static void          picman_tag_cache_add_object         (PicmanTagCache           *cache,
                                                        PicmanTagged             *tagged);

static void          picman_tag_cache_load_start_element (GMarkupParseContext    *context,
                                                        const gchar            *element_name,
                                                        const gchar           **attribute_names,
                                                        const gchar           **attribute_values,
                                                        gpointer                user_data,
                                                        GError                **error);
static void          picman_tag_cache_load_end_element   (GMarkupParseContext    *context,
                                                        const gchar            *element_name,
                                                        gpointer                user_data,
                                                        GError                **error);
static void          picman_tag_cache_load_text          (GMarkupParseContext    *context,
                                                        const gchar            *text,
                                                        gsize                   text_len,
                                                        gpointer                user_data,
                                                        GError                **error);
static  void         picman_tag_cache_load_error         (GMarkupParseContext    *context,
                                                        GError                 *error,
                                                        gpointer                user_data);
static const gchar * picman_tag_cache_attribute_name_to_value
                                                       (const gchar           **attribute_names,
                                                        const gchar           **attribute_values,
                                                        const gchar            *name);

static GQuark        picman_tag_cache_get_error_domain   (void);


G_DEFINE_TYPE (PicmanTagCache, picman_tag_cache, PICMAN_TYPE_OBJECT)

#define parent_class picman_tag_cache_parent_class


static void
picman_tag_cache_class_init (PicmanTagCacheClass *klass)
{
  GObjectClass      *object_class         = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class    = PICMAN_OBJECT_CLASS (klass);
  PicmanTagCacheClass *picman_tag_cache_class = PICMAN_TAG_CACHE_CLASS (klass);

  object_class->finalize         = picman_tag_cache_finalize;

  picman_object_class->get_memsize = picman_tag_cache_get_memsize;

  g_type_class_add_private (picman_tag_cache_class,
                            sizeof (PicmanTagCachePriv));
}

static void
picman_tag_cache_init (PicmanTagCache *cache)
{
  cache->priv = G_TYPE_INSTANCE_GET_PRIVATE (cache,
                                             PICMAN_TYPE_TAG_CACHE,
                                             PicmanTagCachePriv);

  cache->priv->records    = g_array_new (FALSE, FALSE,
                                         sizeof (PicmanTagCacheRecord));
  cache->priv->containers = NULL;
}

static void
picman_tag_cache_finalize (GObject *object)
{
  PicmanTagCache *cache = PICMAN_TAG_CACHE (object);

  if (cache->priv->records)
    {
      gint i;

      for (i = 0; i < cache->priv->records->len; i++)
        {
          PicmanTagCacheRecord *rec = &g_array_index (cache->priv->records,
                                                    PicmanTagCacheRecord, i);

          g_list_free_full (rec->tags, (GDestroyNotify) g_object_unref);
        }

      g_array_free (cache->priv->records, TRUE);
      cache->priv->records = NULL;
    }

  if (cache->priv->containers)
    {
      g_list_free (cache->priv->containers);
      cache->priv->containers = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_tag_cache_get_memsize (PicmanObject *object,
                            gint64     *gui_size)
{
  PicmanTagCache *cache   = PICMAN_TAG_CACHE (object);
  gint64        memsize = 0;

  memsize += picman_g_list_get_memsize (cache->priv->containers, 0);
  memsize += cache->priv->records->len * sizeof (PicmanTagCacheRecord);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

/**
 * picman_tag_cache_new:
 *
 * Return value: creates new PicmanTagCache object.
 **/
PicmanTagCache *
picman_tag_cache_new (void)
{
  return g_object_new (PICMAN_TYPE_TAG_CACHE, NULL);
}

static void
picman_tag_cache_container_add_callback (PicmanTagCache  *cache,
                                       PicmanTagged    *tagged,
                                       PicmanContainer *not_used)
{
  picman_tag_cache_add_object (cache, tagged);
}

/**
 * picman_tag_cache_add_container:
 * @cache:      a PicmanTagCache object.
 * @container:  container containing PicmanTagged objects.
 *
 * Adds container of PicmanTagged objects to tag cache. Before calling this
 * function tag cache must be loaded using picman_tag_cache_load(). When tag
 * cache is saved to file, tags are collected from objects in priv->containers.
 **/
void
picman_tag_cache_add_container (PicmanTagCache  *cache,
                              PicmanContainer *container)
{
  g_return_if_fail (PICMAN_IS_TAG_CACHE (cache));
  g_return_if_fail (PICMAN_IS_CONTAINER (container));

  cache->priv->containers = g_list_append (cache->priv->containers, container);
  picman_container_foreach (container, (GFunc) picman_tag_cache_object_initialize,
                          cache);

  g_signal_connect_swapped (container, "add",
                            G_CALLBACK (picman_tag_cache_container_add_callback),
                            cache);
}

static void
picman_tag_cache_add_object (PicmanTagCache *cache,
                           PicmanTagged   *tagged)
{
  gchar  *identifier;
  GQuark  identifier_quark = 0;
  gchar  *checksum;
  GQuark  checksum_quark = 0;
  GList  *list;
  gint    i;

  identifier = picman_tagged_get_identifier (tagged);

  if (identifier)
    {
      identifier_quark = g_quark_try_string (identifier);
      g_free (identifier);
    }

  if (identifier_quark)
    {
      for (i = 0; i < cache->priv->records->len; i++)
        {
          PicmanTagCacheRecord *rec = &g_array_index (cache->priv->records,
                                                    PicmanTagCacheRecord, i);

          if (rec->identifier == identifier_quark)
            {
              for (list = rec->tags; list; list = g_list_next (list))
                {
                  picman_tagged_add_tag (tagged, PICMAN_TAG (list->data));
                }

              rec->referenced = TRUE;
              return;
            }
        }
    }

  checksum = picman_tagged_get_checksum (tagged);

  if (checksum)
    {
      checksum_quark = g_quark_try_string (checksum);
      g_free (checksum);
    }

  if (checksum_quark)
    {
      for (i = 0; i < cache->priv->records->len; i++)
        {
          PicmanTagCacheRecord *rec = &g_array_index (cache->priv->records,
                                                    PicmanTagCacheRecord, i);

          if (rec->checksum == checksum_quark)
            {
#if DEBUG_PICMAN_TAG_CACHE
              g_printerr ("remapping identifier: %s ==> %s\n",
                          rec->identifier ? g_quark_to_string (rec->identifier) : "(NULL)",
                          identifier_quark ? g_quark_to_string (identifier_quark) : "(NULL)");
#endif

              rec->identifier = identifier_quark;

              for (list = rec->tags; list; list = g_list_next (list))
                {
                  picman_tagged_add_tag (tagged, PICMAN_TAG (list->data));
                }

              rec->referenced = TRUE;
              return;
            }
        }
    }

}

static void
picman_tag_cache_object_initialize (PicmanTagged   *tagged,
                                  PicmanTagCache *cache)
{
  picman_tag_cache_add_object (cache, tagged);
}

static void
picman_tag_cache_tagged_to_cache_record_foreach (PicmanTagged  *tagged,
                                               GList      **cache_records)
{
  gchar *identifier = picman_tagged_get_identifier (tagged);

  if (identifier)
    {
      PicmanTagCacheRecord *cache_rec = g_new (PicmanTagCacheRecord, 1);
      gchar              *checksum;

      checksum = picman_tagged_get_checksum (tagged);

      cache_rec->identifier = g_quark_from_string (identifier);
      cache_rec->checksum   = g_quark_from_string (checksum);
      cache_rec->tags       = g_list_copy (picman_tagged_get_tags (tagged));

      g_free (checksum);

      *cache_records = g_list_prepend (*cache_records, cache_rec);
    }

  g_free (identifier);
}

/**
 * picman_tag_cache_save:
 * @cache:      a PicmanTagCache object.
 *
 * Saves tag cache to cache file.
 **/
void
picman_tag_cache_save (PicmanTagCache *cache)
{
  GString *buf;
  GList   *saved_records;
  GList   *iterator;
  gchar   *filename;
  GError  *error = NULL;
  gint     i;

  g_return_if_fail (PICMAN_IS_TAG_CACHE (cache));

  saved_records = NULL;
  for (i = 0; i < cache->priv->records->len; i++)
    {
      PicmanTagCacheRecord *current_record = &g_array_index (cache->priv->records,
                                                           PicmanTagCacheRecord, i);

      if (! current_record->referenced && current_record->tags)
        {
          /* keep tagged objects which have tags assigned
           * but were not loaded.
           */
          PicmanTagCacheRecord *record_copy = g_new (PicmanTagCacheRecord, 1);

          record_copy->identifier = current_record->identifier;
          record_copy->checksum   = current_record->checksum;
          record_copy->tags       = g_list_copy (current_record->tags);

          saved_records = g_list_prepend (saved_records, record_copy);
        }
    }

  for (iterator = cache->priv->containers;
       iterator;
       iterator = g_list_next (iterator))
    {
      picman_container_foreach (PICMAN_CONTAINER (iterator->data),
                              (GFunc) picman_tag_cache_tagged_to_cache_record_foreach,
                              &saved_records);
    }

  saved_records = g_list_reverse (saved_records);

  buf = g_string_new ("");
  g_string_append (buf, "<?xml version='1.0' encoding='UTF-8'?>\n");
  g_string_append (buf, "<tags>\n");

  for (iterator = saved_records; iterator; iterator = g_list_next (iterator))
    {
      PicmanTagCacheRecord *cache_rec = iterator->data;
      GList              *tag_iterator;
      gchar              *identifier_string;
      gchar              *tag_string;

      identifier_string = g_markup_escape_text (g_quark_to_string (cache_rec->identifier), -1);
      g_string_append_printf (buf, "\n  <resource identifier=\"%s\" checksum=\"%s\">\n",
                              identifier_string,
                              g_quark_to_string (cache_rec->checksum));
      g_free (identifier_string);

      for (tag_iterator = cache_rec->tags;
           tag_iterator;
           tag_iterator = g_list_next (tag_iterator))
        {
          PicmanTag *tag = PICMAN_TAG (tag_iterator->data);

          if (! picman_tag_get_internal (tag))
            {
              tag_string = g_markup_escape_text (picman_tag_get_name (tag), -1);
              g_string_append_printf (buf, "    <tag>%s</tag>\n", tag_string);
              g_free (tag_string);
            }
        }

      g_string_append (buf, "  </resource>\n");
    }

  g_string_append (buf, "</tags>\n");

  filename = g_build_filename (picman_directory (), PICMAN_TAG_CACHE_FILE, NULL);

  if (! g_file_set_contents (filename, buf->str, buf->len, &error))
    {
      g_printerr ("Error while saving tag cache: %s\n", error->message);
      g_error_free (error);
    }

  g_free (filename);

  g_string_free (buf, TRUE);

  for (iterator = saved_records;
       iterator;
       iterator = g_list_next (iterator))
    {
      PicmanTagCacheRecord *cache_rec = iterator->data;

      g_list_free (cache_rec->tags);
      g_free (cache_rec);
    }

  g_list_free (saved_records);
}

/**
 * picman_tag_cache_load:
 * @cache:      a PicmanTagCache object.
 *
 * Loads tag cache from file.
 **/
void
picman_tag_cache_load (PicmanTagCache *cache)
{
  gchar                 *filename;
  GError                *error = NULL;
  GMarkupParser          markup_parser;
  PicmanXmlParser         *xml_parser;
  PicmanTagCacheParseData  parse_data;

  g_return_if_fail (PICMAN_IS_TAG_CACHE (cache));

  /* clear any previous priv->records */
  cache->priv->records = g_array_set_size (cache->priv->records, 0);

  filename = g_build_filename (picman_directory (), PICMAN_TAG_CACHE_FILE, NULL);

  parse_data.records = g_array_new (FALSE, FALSE, sizeof (PicmanTagCacheRecord));
  memset (&parse_data.current_record, 0, sizeof (PicmanTagCacheRecord));

  markup_parser.start_element = picman_tag_cache_load_start_element;
  markup_parser.end_element   = picman_tag_cache_load_end_element;
  markup_parser.text          = picman_tag_cache_load_text;
  markup_parser.passthrough   = NULL;
  markup_parser.error         = picman_tag_cache_load_error;

  xml_parser = picman_xml_parser_new (&markup_parser, &parse_data);

  if (picman_xml_parser_parse_file (xml_parser, filename, &error))
    {
      cache->priv->records = g_array_append_vals (cache->priv->records,
                                                  parse_data.records->data,
                                                  parse_data.records->len);
    }
  else
    {
      g_printerr ("Failed to parse tag cache: %s\n",
                  error ? error->message : NULL);
    }

  g_free (filename);
  picman_xml_parser_free (xml_parser);
  g_array_free (parse_data.records, TRUE);
}

static  void
picman_tag_cache_load_start_element (GMarkupParseContext *context,
                                   const gchar         *element_name,
                                   const gchar        **attribute_names,
                                   const gchar        **attribute_values,
                                   gpointer             user_data,
                                   GError             **error)
{
  PicmanTagCacheParseData *parse_data = user_data;

  if (! strcmp (element_name, "resource"))
    {
      const gchar *identifier;
      const gchar *checksum;

      identifier = picman_tag_cache_attribute_name_to_value (attribute_names,
                                                           attribute_values,
                                                           "identifier");
      checksum   = picman_tag_cache_attribute_name_to_value (attribute_names,
                                                           attribute_values,
                                                           "checksum");

      if (! identifier)
        {
          g_set_error (error,
                       picman_tag_cache_get_error_domain (),
                       1001,
                       "Resource tag does not contain required attribute identifier.");
          return;
        }

      memset (&parse_data->current_record, 0, sizeof (PicmanTagCacheRecord));

      parse_data->current_record.identifier = g_quark_from_string (identifier);
      parse_data->current_record.checksum   = g_quark_from_string (checksum);
    }
}

static void
picman_tag_cache_load_end_element (GMarkupParseContext *context,
                                 const gchar         *element_name,
                                 gpointer             user_data,
                                 GError             **error)
{
  PicmanTagCacheParseData *parse_data = user_data;

  if (strcmp (element_name, "resource") == 0)
    {
      parse_data->records = g_array_append_val (parse_data->records,
                                                parse_data->current_record);
      memset (&parse_data->current_record, 0, sizeof (PicmanTagCacheRecord));
    }
}

static void
picman_tag_cache_load_text (GMarkupParseContext  *context,
                          const gchar          *text,
                          gsize                 text_len,
                          gpointer              user_data,
                          GError              **error)
{
  PicmanTagCacheParseData *parse_data = user_data;
  const gchar           *current_element;
  gchar                  buffer[2048];
  PicmanTag               *tag;

  current_element = g_markup_parse_context_get_element (context);

  if (g_strcmp0 (current_element, "tag") == 0)
    {
      if (text_len >= sizeof (buffer))
        {
          g_set_error (error, picman_tag_cache_get_error_domain (), 1002,
                       "Tag value is too long.");
          return;
        }

      memcpy (buffer, text, text_len);
      buffer[text_len] = '\0';

      tag = picman_tag_new (buffer);
      if (tag)
        {
          parse_data->current_record.tags = g_list_append (parse_data->current_record.tags,
                                                           tag);
        }
      else
        {
          g_warning ("dropping invalid tag '%s' from '%s'\n", buffer,
                     g_quark_to_string (parse_data->current_record.identifier));
        }
    }
}

static  void
picman_tag_cache_load_error (GMarkupParseContext *context,
                           GError              *error,
                           gpointer             user_data)
{
  printf ("Tag cache parse error: %s\n", error->message);
}

static const gchar*
picman_tag_cache_attribute_name_to_value (const gchar **attribute_names,
                                        const gchar **attribute_values,
                                        const gchar  *name)
{
  while (*attribute_names)
    {
      if (! strcmp (*attribute_names, name))
        {
          return *attribute_values;
        }

      attribute_names++;
      attribute_values++;
    }

  return NULL;
}

static GQuark
picman_tag_cache_get_error_domain (void)
{
  return g_quark_from_static_string ("picman-tag-cache-error-quark");
}
