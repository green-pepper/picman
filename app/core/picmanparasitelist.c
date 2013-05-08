/* parasitelist.c: Copyright 1998 Jay Cox <jaycox@picman.org>
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gegl.h>

#ifdef G_OS_WIN32
#include <io.h>
#endif

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman-utils.h"
#include "picmanmarshal.h"
#include "picmanparasitelist.h"


enum
{
  ADD,
  REMOVE,
  LAST_SIGNAL
};


static void     picman_parasite_list_finalize          (GObject     *object);
static gint64   picman_parasite_list_get_memsize       (PicmanObject  *object,
                                                      gint64      *gui_size);

static void     picman_parasite_list_config_iface_init (gpointer     iface,
                                                      gpointer     iface_data);
static gboolean picman_parasite_list_serialize    (PicmanConfig       *list,
                                                 PicmanConfigWriter *writer,
                                                 gpointer          data);
static gboolean picman_parasite_list_deserialize  (PicmanConfig       *list,
                                                 GScanner         *scanner,
                                                 gint              nest_level,
                                                 gpointer          data);

static void     parasite_serialize           (const gchar      *key,
                                              PicmanParasite     *parasite,
                                              PicmanConfigWriter *writer);
static void     parasite_copy                (const gchar      *key,
                                              PicmanParasite     *parasite,
                                              PicmanParasiteList *list);
static gboolean parasite_free                (const gchar      *key,
                                              PicmanParasite     *parasite,
                                              gpointer          unused);
static void     parasite_count_if_persistent (const gchar      *key,
                                              PicmanParasite     *parasite,
                                              gint             *count);


G_DEFINE_TYPE_WITH_CODE (PicmanParasiteList, picman_parasite_list, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_parasite_list_config_iface_init))

#define parent_class picman_parasite_list_parent_class

static guint        parasite_list_signals[LAST_SIGNAL] = { 0 };
static const gchar  parasite_symbol[]                  = "parasite";


static void
picman_parasite_list_class_init (PicmanParasiteListClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  parasite_list_signals[ADD] =
    g_signal_new ("add",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanParasiteListClass, add),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  parasite_list_signals[REMOVE] =
    g_signal_new ("remove",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanParasiteListClass, remove),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  object_class->finalize         = picman_parasite_list_finalize;

  picman_object_class->get_memsize = picman_parasite_list_get_memsize;

  klass->add                     = NULL;
  klass->remove                  = NULL;
}

static void
picman_parasite_list_config_iface_init (gpointer  iface,
                                      gpointer  iface_data)
{
  PicmanConfigInterface *config_iface = (PicmanConfigInterface *) iface;

  config_iface->serialize   = picman_parasite_list_serialize;
  config_iface->deserialize = picman_parasite_list_deserialize;
}

static void
picman_parasite_list_init (PicmanParasiteList *list)
{
  list->table = NULL;
}

static void
picman_parasite_list_finalize (GObject *object)
{
  PicmanParasiteList *list = PICMAN_PARASITE_LIST (object);

  if (list->table)
    {
      g_hash_table_foreach_remove (list->table, (GHRFunc) parasite_free, NULL);
      g_hash_table_destroy (list->table);
      list->table = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_parasite_list_get_memsize (PicmanObject *object,
                                gint64     *gui_size)
{
  PicmanParasiteList *list    = PICMAN_PARASITE_LIST (object);
  gint64            memsize = 0;

  memsize += picman_g_hash_table_get_memsize_foreach (list->table,
                                                    (PicmanMemsizeFunc)
                                                    picman_parasite_get_memsize,
                                                    gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_parasite_list_serialize (PicmanConfig       *list,
                              PicmanConfigWriter *writer,
                              gpointer          data)
{
  if (PICMAN_PARASITE_LIST (list)->table)
    g_hash_table_foreach (PICMAN_PARASITE_LIST (list)->table,
                          (GHFunc) parasite_serialize,
                          writer);

  return TRUE;
}

static gboolean
picman_parasite_list_deserialize (PicmanConfig *list,
                                GScanner   *scanner,
                                gint        nest_level,
                                gpointer    data)
{
  GTokenType token;

  g_scanner_scope_add_symbol (scanner, 0,
                              parasite_symbol, (gpointer) parasite_symbol);

  token = G_TOKEN_LEFT_PAREN;

  while (g_scanner_peek_next_token (scanner) == token)
    {
      token = g_scanner_get_next_token (scanner);

      switch (token)
        {
        case G_TOKEN_LEFT_PAREN:
          token = G_TOKEN_SYMBOL;
          break;

        case G_TOKEN_SYMBOL:
          if (scanner->value.v_symbol == parasite_symbol)
            {
              gchar        *parasite_name      = NULL;
              gint          parasite_flags     = 0;
              guint8       *parasite_data      = NULL;
              gint          parasite_data_size = 0;
              PicmanParasite *parasite;

              token = G_TOKEN_STRING;

              if (g_scanner_peek_next_token (scanner) != token)
                break;

              if (! picman_scanner_parse_string (scanner, &parasite_name))
                break;

              token = G_TOKEN_INT;

              if (g_scanner_peek_next_token (scanner) != token)
                goto cleanup;

              if (! picman_scanner_parse_int (scanner, &parasite_flags))
                goto cleanup;

              token = G_TOKEN_INT;

              if (g_scanner_peek_next_token (scanner) != token)
                {
                  /*  old format -- plain string  */

                  gchar *str;

                  if (g_scanner_peek_next_token (scanner) != G_TOKEN_STRING)
                    goto cleanup;

                  if (! picman_scanner_parse_string (scanner, &str))
                    goto cleanup;

                  parasite_data_size = strlen (str);
                  parasite_data      = (guint8 *) str;
                }
              else
                {
                  /*  new format -- properly encoded binary data  */

                  if (! picman_scanner_parse_int (scanner, &parasite_data_size))
                    goto cleanup;

                  token = G_TOKEN_STRING;

                  if (g_scanner_peek_next_token (scanner) != token)
                    goto cleanup;

                  if (! picman_scanner_parse_data (scanner, parasite_data_size,
                                                 &parasite_data))
                    goto cleanup;
                }

              parasite = picman_parasite_new (parasite_name,
                                            parasite_flags,
                                            parasite_data_size,
                                            parasite_data);
              picman_parasite_list_add (PICMAN_PARASITE_LIST (list),
                                      parasite);  /* adds a copy */
              picman_parasite_free (parasite);

              token = G_TOKEN_RIGHT_PAREN;

              g_free (parasite_data);
            cleanup:
              g_free (parasite_name);
            }
          break;

        case G_TOKEN_RIGHT_PAREN:
          token = G_TOKEN_LEFT_PAREN;
          break;

        default: /* do nothing */
          break;
        }
    }

  return picman_config_deserialize_return (scanner, token, nest_level);
}

PicmanParasiteList *
picman_parasite_list_new (void)
{
  PicmanParasiteList *list;

  list = g_object_new (PICMAN_TYPE_PARASITE_LIST, NULL);

  return list;
}

PicmanParasiteList *
picman_parasite_list_copy (const PicmanParasiteList *list)
{
  PicmanParasiteList *newlist;

  g_return_val_if_fail (PICMAN_IS_PARASITE_LIST (list), NULL);

  newlist = picman_parasite_list_new ();

  if (list->table)
    g_hash_table_foreach (list->table, (GHFunc) parasite_copy, newlist);

  return newlist;
}

void
picman_parasite_list_add (PicmanParasiteList   *list,
                        const PicmanParasite *parasite)
{
  PicmanParasite *copy;

  g_return_if_fail (PICMAN_IS_PARASITE_LIST (list));
  g_return_if_fail (parasite != NULL);
  g_return_if_fail (parasite->name != NULL);

  if (list->table == NULL)
    list->table = g_hash_table_new (g_str_hash, g_str_equal);

  picman_parasite_list_remove (list, parasite->name);
  copy = picman_parasite_copy (parasite);
  g_hash_table_insert (list->table, copy->name, copy);

  g_signal_emit (list, parasite_list_signals[ADD], 0, copy);
}

void
picman_parasite_list_remove (PicmanParasiteList *list,
                           const gchar      *name)
{
  g_return_if_fail (PICMAN_IS_PARASITE_LIST (list));

  if (list->table)
    {
      PicmanParasite *parasite;

      parasite = (PicmanParasite *) picman_parasite_list_find (list, name);

      if (parasite)
        {
          g_hash_table_remove (list->table, name);

          g_signal_emit (list, parasite_list_signals[REMOVE], 0, parasite);

          picman_parasite_free (parasite);
        }
    }
}

gint
picman_parasite_list_length (PicmanParasiteList *list)
{
  g_return_val_if_fail (PICMAN_IS_PARASITE_LIST (list), 0);

  if (! list->table)
    return 0;

  return g_hash_table_size (list->table);
}

gint
picman_parasite_list_persistent_length (PicmanParasiteList *list)
{
  gint len = 0;

  g_return_val_if_fail (PICMAN_IS_PARASITE_LIST (list), 0);

  if (! list->table)
    return 0;

  picman_parasite_list_foreach (list,
                              (GHFunc) parasite_count_if_persistent, &len);

  return len;
}

void
picman_parasite_list_foreach (PicmanParasiteList *list,
                            GHFunc            function,
                            gpointer          user_data)
{
  g_return_if_fail (PICMAN_IS_PARASITE_LIST (list));

  if (! list->table)
    return;

  g_hash_table_foreach (list->table, function, user_data);
}

const PicmanParasite *
picman_parasite_list_find (PicmanParasiteList *list,
                         const gchar      *name)
{
  g_return_val_if_fail (PICMAN_IS_PARASITE_LIST (list), NULL);

  if (list->table)
    return (PicmanParasite *) g_hash_table_lookup (list->table, name);

  return NULL;
}


static void
parasite_serialize (const gchar      *key,
                    PicmanParasite     *parasite,
                    PicmanConfigWriter *writer)
{
  if (! picman_parasite_is_persistent (parasite))
    return;

  picman_config_writer_open (writer, parasite_symbol);

  picman_config_writer_printf (writer, "\"%s\" %lu %lu",
                             picman_parasite_name (parasite),
                             picman_parasite_flags (parasite),
                             picman_parasite_data_size (parasite));

  picman_config_writer_data (writer,
                           picman_parasite_data_size (parasite),
                           picman_parasite_data (parasite));

  picman_config_writer_close (writer);
  picman_config_writer_linefeed (writer);
}

static void
parasite_copy (const gchar      *key,
               PicmanParasite     *parasite,
               PicmanParasiteList *list)
{
  picman_parasite_list_add (list, parasite);
}

static gboolean
parasite_free (const gchar  *key,
               PicmanParasite *parasite,
               gpointer     unused)
{
  picman_parasite_free (parasite);

  return TRUE;
}

static void
parasite_count_if_persistent (const gchar  *key,
                              PicmanParasite *parasite,
                              gint         *count)
{
  if (picman_parasite_is_persistent (parasite))
    *count = *count + 1;
}
