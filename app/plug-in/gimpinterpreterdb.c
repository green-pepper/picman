/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmaninterpreterdb.c
 * (C) 2005 Manish Singh <yosh@picman.org>
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

/*
 * The binfmt_misc bits are derived from linux/fs/binfmt_misc.c
 * Copyright (C) 1997 Richard Günther
 */

/*
 * The sh-bang code is derived from linux/fs/binfmt_script.c
 * Copyright (C) 1996  Martin von Löwis
 * original #!-checking implemented by tytso.
 */

#include "config.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <fcntl.h>

#include <glib-object.h>
#include <glib/gstdio.h>

#ifdef G_OS_WIN32
#include <io.h>
#endif

#ifndef _O_BINARY
#define _O_BINARY 0
#endif

#include "libpicmanbase/picmanbase.h"

#include "plug-in-types.h"

#include "picmaninterpreterdb.h"

#include "picman-intl.h"


#define BUFSIZE 4096


typedef struct _PicmanInterpreterMagic PicmanInterpreterMagic;

struct _PicmanInterpreterMagic
{
  gulong    offset;
  gchar    *magic;
  gchar    *mask;
  guint     size;
  gchar    *program;
};


static void     picman_interpreter_db_finalize         (GObject                 *object);

static void     picman_interpreter_db_load_interp_file (const PicmanDatafileData  *file_data,
                                                      gpointer                 user_data);

static void     picman_interpreter_db_add_program      (PicmanInterpreterDB       *db,
                                                      const PicmanDatafileData  *file_data,
                                                      gchar                   *buffer);
static void     picman_interpreter_db_add_binfmt_misc  (PicmanInterpreterDB       *db,
                                                      const PicmanDatafileData  *file_data,
                                                      gchar                   *buffer);

static gboolean picman_interpreter_db_add_extension    (PicmanInterpreterDB       *db,
                                                      gchar                  **tokens);
static gboolean picman_interpreter_db_add_magic        (PicmanInterpreterDB       *db,
                                                      gchar                  **tokens);

static void     picman_interpreter_db_clear_magics     (PicmanInterpreterDB       *db);

static void     picman_interpreter_db_resolve_programs (PicmanInterpreterDB       *db);


G_DEFINE_TYPE (PicmanInterpreterDB, picman_interpreter_db, G_TYPE_OBJECT)

#define parent_class picman_interpreter_db_parent_class


static void
picman_interpreter_db_class_init (PicmanInterpreterDBClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = picman_interpreter_db_finalize;
}

static void
picman_interpreter_db_init (PicmanInterpreterDB *db)
{
  db->programs        = NULL;

  db->magics          = NULL;
  db->magic_names     = NULL;

  db->extensions      = NULL;
  db->extension_names = NULL;
}

static void
picman_interpreter_db_finalize (GObject *object)
{
  PicmanInterpreterDB *db = PICMAN_INTERPRETER_DB (object);

  picman_interpreter_db_clear (db);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

PicmanInterpreterDB *
picman_interpreter_db_new (void)
{
  return g_object_new (PICMAN_TYPE_INTERPRETER_DB, NULL);
}

void
picman_interpreter_db_load (PicmanInterpreterDB *db,
                          const gchar       *interp_path)
{
  g_return_if_fail (PICMAN_IS_INTERPRETER_DB (db));

  picman_interpreter_db_clear (db);

  db->programs = g_hash_table_new_full (g_str_hash, g_str_equal,
                                        g_free, g_free);

  db->extensions = g_hash_table_new_full (g_str_hash, g_str_equal,
                                          g_free, g_free);

  db->magic_names = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           g_free, NULL);

  db->extension_names = g_hash_table_new_full (g_str_hash, g_str_equal,
                                               g_free, NULL);

  picman_datafiles_read_directories (interp_path,
                                   G_FILE_TEST_EXISTS,
                                   picman_interpreter_db_load_interp_file,
                                   db);

  picman_interpreter_db_resolve_programs (db);
}

void
picman_interpreter_db_clear (PicmanInterpreterDB *db)
{
  g_return_if_fail (PICMAN_IS_INTERPRETER_DB (db));

  if (db->magic_names)
    {
      g_hash_table_destroy (db->magic_names);
      db->magic_names = NULL;
    }

  if (db->extension_names)
    {
      g_hash_table_destroy (db->extension_names);
      db->extension_names = NULL;
    }

  if (db->programs)
    {
      g_hash_table_destroy (db->programs);
      db->programs = NULL;
    }

  if (db->extensions)
    {
      g_hash_table_destroy (db->extensions);
      db->extensions = NULL;
    }

  picman_interpreter_db_clear_magics (db);
}

static void
picman_interpreter_db_load_interp_file (const PicmanDatafileData *file_data,
                                      gpointer                user_data)
{
  PicmanInterpreterDB *db;
  FILE              *interp_file;
  gchar              buffer[4096];
  gsize              len;

  db = PICMAN_INTERPRETER_DB (user_data);

  interp_file = g_fopen (file_data->filename, "r");
  if (! interp_file)
    return;

  while (fgets (buffer, sizeof (buffer), interp_file))
    {
      /* Skip comments */
      if (buffer[0] == '#')
        continue;

      len = strlen (buffer) - 1;

      /* Skip too long lines */
      if (buffer[len] != '\n')
        continue;

      buffer[len] = '\0';

      if (g_ascii_isalnum (buffer[0]) || (buffer[0] == '/'))
        picman_interpreter_db_add_program (db, file_data, buffer);
      else if (! g_ascii_isspace (buffer[0]) && (buffer[0] != '\0'))
        picman_interpreter_db_add_binfmt_misc (db, file_data, buffer);
    }

  fclose (interp_file);
}

static void
picman_interpreter_db_add_program (PicmanInterpreterDB      *db,
                                 const PicmanDatafileData *file_data,
                                 gchar                  *buffer)
{
  gchar *name;
  gchar *program;
  gchar *p;

  p = strchr (buffer, '=');
  if (! p)
    return;

  *p = '\0';

  name = buffer;
  program = p + 1;

  if (! g_file_test (program, G_FILE_TEST_IS_EXECUTABLE))
    {
      g_message (_("Bad interpreter referenced in interpreter file %s: %s"),
                 picman_filename_to_utf8 (file_data->filename),
                 picman_filename_to_utf8 (program));
      return;
    }

  if (! g_hash_table_lookup (db->programs, name))
    g_hash_table_insert (db->programs, g_strdup (name), g_strdup (program));
}

static void
picman_interpreter_db_add_binfmt_misc (PicmanInterpreterDB      *db,
                                     const PicmanDatafileData *file_data,
                                     gchar                  *buffer)
{
  gchar **tokens = NULL;
  gchar  *name, *type, *program;
  gsize   count;
  gchar   del[2];

  count = strlen (buffer);

  if ((count < 10) || (count > 255))
    goto bail;

  del[0] = *buffer;
  del[1] = '\0';

  memset (buffer + count, del[0], 8);

  tokens = g_strsplit (buffer + 1, del, -1);

  name    = tokens[0];
  type    = tokens[1];
  program = tokens[5];

  if ((name[0] == '\0') || (program[0] == '\0') ||
      (type[0] == '\0') || (type[1] != '\0'))
    goto bail;

  switch (type[0])
    {
    case 'E':
      if (! picman_interpreter_db_add_extension (db, tokens))
        goto bail;
      break;

    case 'M':
      if (! picman_interpreter_db_add_magic (db, tokens))
        goto bail;
      break;

    default:
      goto bail;
    }

  goto out;

bail:
  g_message (_("Bad binary format string in interpreter file %s"),
             picman_filename_to_utf8 (file_data->filename));

out:
  g_strfreev (tokens);
}

static gboolean
picman_interpreter_db_add_extension (PicmanInterpreterDB  *db,
                                   gchar             **tokens)
{
  const gchar *name      = tokens[0];
  const gchar *extension = tokens[3];
  const gchar *program   = tokens[5];

  if (! g_hash_table_lookup (db->extension_names, name))
    {
      gchar *prog;

      if (extension[0] == '\0' || extension[0] == '/')
        return FALSE;

      prog = g_strdup (program);

      g_hash_table_insert (db->extensions, g_strdup (extension), prog);
      g_hash_table_insert (db->extension_names, g_strdup (name), prog);
    }

  return TRUE;
}

static gboolean
scanarg (const gchar *s)
{
  gchar c;

  while ((c = *s++) != '\0')
    {
      if (c == '\\' && *s == 'x')
        {
          s++;

          if (! g_ascii_isxdigit (*s++))
            return FALSE;

          if (! g_ascii_isxdigit (*s++))
            return FALSE;
        }
   }

  return TRUE;
}

static guint
unquote (gchar *from)
{
  gchar *s = from;
  gchar *p = from;
  gchar  c;

  while ((c = *s++) != '\0')
    {
      if (c == '\\' && *s == 'x')
        {
          s++;
          *p = g_ascii_xdigit_value (*s++) << 4;
          *p++ |= g_ascii_xdigit_value (*s++);
          continue;
        }

      *p++ = c;
    }

  return p - from;
}

static gboolean
picman_interpreter_db_add_magic (PicmanInterpreterDB  *db,
                               gchar             **tokens)
{
  PicmanInterpreterMagic *interp_magic;
  gchar                *name, *num, *magic, *mask, *program;
  gulong                offset;
  guint                 size;

  name    = tokens[0];
  num     = tokens[2];
  magic   = tokens[3];
  mask    = tokens[4];
  program = tokens[5];

  if (! g_hash_table_lookup (db->magic_names, name))
    {
      if (num[0] != '\0')
        {
          offset = strtoul (num, &num, 10);

          if (num[0] != '\0')
            return FALSE;

          if (offset > (BUFSIZE / 4))
            return FALSE;
        }
      else
        {
          offset = 0;
        }

      if (! scanarg (magic))
        return FALSE;

      if (! scanarg (mask))
        return FALSE;

      size = unquote (magic);

      if ((size + offset) > (BUFSIZE / 2))
        return FALSE;

      if (mask[0] == '\0')
        mask = NULL;
      else if (unquote (mask) != size)
        return FALSE;

      interp_magic = g_slice_new (PicmanInterpreterMagic);

      interp_magic->offset  = offset;
      interp_magic->magic   = g_memdup (magic, size);
      interp_magic->mask    = g_memdup (mask, size);
      interp_magic->size    = size;
      interp_magic->program = g_strdup (program);

      db->magics = g_slist_append (db->magics, interp_magic);

      g_hash_table_insert (db->magic_names, g_strdup (name), interp_magic);
    }

  return TRUE;
}

static void
picman_interpreter_db_clear_magics (PicmanInterpreterDB *db)
{
  PicmanInterpreterMagic *magic;
  GSList               *list, *last;

  list = db->magics;
  db->magics = NULL;

  while (list)
    {
      magic = list->data;

      g_free (magic->magic);
      g_free (magic->mask);
      g_free (magic->program);

      g_slice_free (PicmanInterpreterMagic, magic);

      last = list;
      list = list->next;

      g_slist_free_1 (last);
    }
}

#ifdef INTERP_DEBUG
static void
print_kv (gpointer key,
          gpointer value,
          gpointer user_data)
{
  g_print ("%s: %s\n", (gchar *) key, (gchar *) value);
}

static gchar *
quote (gchar *s,
       guint  size)
{
  GString *d;
  guint    i;

  if (s == NULL)
    return "(null)";

  d = g_string_sized_new (size * 4);

  for (i = 0; i < size; i++)
    g_string_append_printf (d, "\\x%02x", ((guint) s[i]) & 0xff);

  return g_string_free (d, FALSE);
}
#endif

static gboolean
resolve_program (gpointer key,
                 gpointer value,
                 gpointer user_data)
{
  PicmanInterpreterDB *db = user_data;
  gchar             *program;

  program = g_hash_table_lookup (db->programs, value);

  if (program != NULL)
    {
      g_free (value);
      value = g_strdup (program);
    }

  g_hash_table_insert (db->extensions, key, value);

  return TRUE;
}

static void
picman_interpreter_db_resolve_programs (PicmanInterpreterDB *db)
{
  GSList     *list;
  GHashTable *extensions;

  for (list = db->magics; list; list = list->next)
    {
      PicmanInterpreterMagic *magic = list->data;
      const gchar          *program;

      program = g_hash_table_lookup (db->programs, magic->program);

      if (program != NULL)
        {
          g_free (magic->program);
          magic->program = g_strdup (program);
        }
    }

  extensions = db->extensions;
  db->extensions = g_hash_table_new_full (g_str_hash, g_str_equal,
                                          g_free, g_free);

  g_hash_table_foreach_steal (extensions, resolve_program, db);

  g_hash_table_destroy (extensions);

#ifdef INTERP_DEBUG
  g_print ("Programs:\n");
  g_hash_table_foreach (db->programs, print_kv, NULL);

  g_print ("\nExtensions:\n");
  g_hash_table_foreach (db->extensions, print_kv, NULL);

  g_print ("\nMagics:\n");

  list = db->magics;

  while (list)
    {
      PicmanInterpreterMagic *magic;

      magic = list->data;
      g_print ("program: %s, offset: %lu, magic: %s, mask: %s\n",
               magic->program, magic->offset,
               quote (magic->magic, magic->size),
               quote (magic->mask, magic->size));

      list = list->next;
    }

  g_print ("\n");
#endif
}

static gchar *
resolve_extension (PicmanInterpreterDB *db,
                   const gchar       *program_path)
{
  gchar       *filename;
  gchar       *p;
  const gchar *program;

  filename = g_path_get_basename (program_path);

  p = strrchr (filename, '.');
  if (! p)
    {
      g_free (filename);
      return NULL;
    }

  program = g_hash_table_lookup (db->extensions, p + 1);

  g_free (filename);

  return g_strdup (program);
}

static gchar *
resolve_sh_bang (PicmanInterpreterDB  *db,
                 const gchar        *program_path,
                 gchar              *buffer,
                 gssize              len,
                 gchar             **interp_arg)
{
  gchar *cp;
  gchar *name;
  gchar *program;

  cp = strchr (buffer, '\n');
  if (! cp)
    cp = buffer + len - 1;

  *cp = '\0';

  while (cp > buffer)
    {
      cp--;
      if ((*cp == ' ') || (*cp == '\t') || (*cp == '\r'))
        *cp = '\0';
      else
        break;
    }

  for (cp = buffer + 2; (*cp == ' ') || (*cp == '\t'); cp++);

  if (*cp == '\0')
    return NULL;

  name = cp;

  for ( ; *cp && (*cp != ' ') && (*cp != '\t'); cp++)
    /* nothing */ ;

  while ((*cp == ' ') || (*cp == '\t'))
    *cp++ = '\0';

  if (*cp)
    {
      if (strcmp ("/usr/bin/env", name) == 0)
        {
          program = g_hash_table_lookup (db->programs, cp);
          if (program)
            return g_strdup (program);
        }

      *interp_arg = g_strdup (cp);
    }

  program = g_hash_table_lookup (db->programs, name);
  if (! program)
    program = name;

  return g_strdup (program);
}

static gchar *
resolve_magic (PicmanInterpreterDB *db,
               const gchar       *program_path,
               gchar             *buffer)
{
  GSList                *list;
  PicmanInterpreterMagic  *magic;
  gchar                 *s;
  guint                  i;

  list = db->magics;

  while (list)
    {
      magic = list->data;

      s = buffer + magic->offset;

      if (magic->mask)
        {
          for (i = 0; i < magic->size; i++)
            if ((*s++ ^ magic->magic[i]) & magic->mask[i])
              break;
        }
      else
        {
          for (i = 0; i < magic->size; i++)
            if ((*s++ ^ magic->magic[i]))
              break;
        }

      if (i == magic->size)
        return g_strdup (magic->program);

      list = list->next;
    }

  return resolve_extension (db, program_path);
}

gchar *
picman_interpreter_db_resolve (PicmanInterpreterDB  *db,
                             const gchar        *program_path,
                             gchar             **interp_arg)
{
  gint    fd;
  gssize  len;
  gchar   buffer[BUFSIZE];

  g_return_val_if_fail (PICMAN_IS_INTERPRETER_DB (db), NULL);
  g_return_val_if_fail (program_path != NULL, NULL);
  g_return_val_if_fail (interp_arg != NULL, NULL);

  *interp_arg = NULL;

  fd = g_open (program_path, O_RDONLY | _O_BINARY, 0);

  if (fd == -1)
    return resolve_extension (db, program_path);

  memset (buffer, 0, sizeof (buffer));
  len = read (fd, buffer, sizeof (buffer));
  close (fd);

  if (len <= 0)
    return resolve_extension (db, program_path);

  if (len > 3 && buffer[0] == '#' && buffer[1] == '!')
    return resolve_sh_bang (db, program_path, buffer, len, interp_arg);

  return resolve_magic (db, program_path, buffer);
}

static void
collect_extensions (const gchar *ext,
                    const gchar *program G_GNUC_UNUSED,
                    GString     *str)
{
  if (str->len)
    g_string_append_c (str, G_SEARCHPATH_SEPARATOR);

  g_string_append_c (str, '.');
  g_string_append (str, ext);
}

/**
 * picman_interpreter_db_get_extensions:
 * @db:
 *
 * Return value: a newly allocated string with all registered file
 *               extensions separated by %G_SEARCHPATH_SEPARATOR;
 *               or %NULL if no extensions are registered
 **/
gchar *
picman_interpreter_db_get_extensions (PicmanInterpreterDB *db)
{
  GString *str;

  g_return_val_if_fail (PICMAN_IS_INTERPRETER_DB (db), NULL);

  if (g_hash_table_size (db->extensions) == 0)
    return NULL;

  str = g_string_new (NULL);

  g_hash_table_foreach (db->extensions, (GHFunc) collect_extensions, str);

  return g_string_free (str, FALSE);
}
