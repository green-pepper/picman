/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2003 Spencer Kimball and Peter Mattis
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

#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "pdb-types.h"

#include "core/picmanparamspecs-desc.h"

#include "picmanpdb.h"
#include "picmanpdb-query.h"
#include "picmanpdberror.h"
#include "picman-pdb-compat.h"
#include "picmanprocedure.h"

#include "picman-intl.h"


#define PDB_REGEX_FLAGS    (G_REGEX_CASELESS | G_REGEX_OPTIMIZE)

#define COMPAT_BLURB       "This procedure is deprecated! Use '%s' instead."


typedef struct _PDBDump PDBDump;

struct _PDBDump
{
  PicmanPDB  *pdb;
  FILE     *file;

  gboolean  dumping_compat;
};

typedef struct _PDBQuery PDBQuery;

struct _PDBQuery
{
  PicmanPDB  *pdb;

  GRegex   *name_regex;
  GRegex   *blurb_regex;
  GRegex   *help_regex;
  GRegex   *author_regex;
  GRegex   *copyright_regex;
  GRegex   *date_regex;
  GRegex   *proc_type_regex;

  gchar   **list_of_procs;
  gint      num_procs;
  gboolean  querying_compat;
};

typedef struct _PDBStrings PDBStrings;

struct _PDBStrings
{
  gboolean  compat;

  gchar    *blurb;
  gchar    *help;
  gchar    *author;
  gchar    *copyright;
  gchar    *date;
};


/*  local function prototypes  */

static void   picman_pdb_query_entry  (gpointer       key,
                                     gpointer       value,
                                     gpointer       user_data);
static void   picman_pdb_print_entry  (gpointer       key,
                                     gpointer       value,
                                     gpointer       user_data);
static void   picman_pdb_get_strings  (PDBStrings    *strings,
                                     PicmanProcedure *procedure,
                                     gboolean       compat);
static void   picman_pdb_free_strings (PDBStrings    *strings);


/*  public functions  */

gboolean
picman_pdb_dump (PicmanPDB     *pdb,
               const gchar *filename)
{
  PDBDump pdb_dump;

  g_return_val_if_fail (PICMAN_IS_PDB (pdb), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);

  pdb_dump.pdb  = pdb;
  pdb_dump.file = g_fopen (filename, "w");

  if (! pdb_dump.file)
    return FALSE;

  pdb_dump.dumping_compat = FALSE;

  g_hash_table_foreach (pdb->procedures,
                        picman_pdb_print_entry,
                        &pdb_dump);

  pdb_dump.dumping_compat = TRUE;

  g_hash_table_foreach (pdb->compat_proc_names,
                        picman_pdb_print_entry,
                        &pdb_dump);

  fclose (pdb_dump.file);

  return TRUE;
}

gboolean
picman_pdb_query (PicmanPDB       *pdb,
                const gchar   *name,
                const gchar   *blurb,
                const gchar   *help,
                const gchar   *author,
                const gchar   *copyright,
                const gchar   *date,
                const gchar   *proc_type,
                gint          *num_procs,
                gchar       ***procs,
                GError       **error)
{
  PDBQuery pdb_query = { 0, };
  gboolean success   = FALSE;

  g_return_val_if_fail (PICMAN_IS_PDB (pdb), FALSE);
  g_return_val_if_fail (name != NULL, FALSE);
  g_return_val_if_fail (blurb != NULL, FALSE);
  g_return_val_if_fail (help != NULL, FALSE);
  g_return_val_if_fail (author != NULL, FALSE);
  g_return_val_if_fail (copyright != NULL, FALSE);
  g_return_val_if_fail (date != NULL, FALSE);
  g_return_val_if_fail (proc_type != NULL, FALSE);
  g_return_val_if_fail (num_procs != NULL, FALSE);
  g_return_val_if_fail (procs != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  *num_procs = 0;
  *procs     = NULL;

  pdb_query.name_regex = g_regex_new (name, PDB_REGEX_FLAGS, 0, error);
  if (! pdb_query.name_regex)
    goto cleanup;

  pdb_query.blurb_regex = g_regex_new (blurb, PDB_REGEX_FLAGS, 0, error);
  if (! pdb_query.blurb_regex)
    goto cleanup;

  pdb_query.help_regex = g_regex_new (help, PDB_REGEX_FLAGS, 0, error);
  if (! pdb_query.help_regex)
    goto cleanup;

  pdb_query.author_regex = g_regex_new (author, PDB_REGEX_FLAGS, 0, error);
  if (! pdb_query.author_regex)
    goto cleanup;

  pdb_query.copyright_regex = g_regex_new (copyright, PDB_REGEX_FLAGS, 0, error);
  if (! pdb_query.copyright_regex)
    goto cleanup;

  pdb_query.date_regex = g_regex_new (date, PDB_REGEX_FLAGS, 0, error);
  if (! pdb_query.date_regex)
    goto cleanup;

  pdb_query.proc_type_regex = g_regex_new (proc_type, PDB_REGEX_FLAGS, 0, error);
  if (! pdb_query.proc_type_regex)
    goto cleanup;

  success = TRUE;

  pdb_query.pdb             = pdb;
  pdb_query.list_of_procs   = NULL;
  pdb_query.num_procs       = 0;
  pdb_query.querying_compat = FALSE;

  g_hash_table_foreach (pdb->procedures,
                        picman_pdb_query_entry, &pdb_query);

  pdb_query.querying_compat = TRUE;

  g_hash_table_foreach (pdb->compat_proc_names,
                        picman_pdb_query_entry, &pdb_query);

 cleanup:

  if (pdb_query.proc_type_regex)
    g_regex_unref (pdb_query.proc_type_regex);

  if (pdb_query.date_regex)
    g_regex_unref (pdb_query.date_regex);

  if (pdb_query.copyright_regex)
    g_regex_unref (pdb_query.copyright_regex);

  if (pdb_query.author_regex)
    g_regex_unref (pdb_query.author_regex);

  if (pdb_query.help_regex)
    g_regex_unref (pdb_query.help_regex);

  if (pdb_query.blurb_regex)
    g_regex_unref (pdb_query.blurb_regex);

  if (pdb_query.name_regex)
    g_regex_unref (pdb_query.name_regex);

  if (success)
    {
      *num_procs = pdb_query.num_procs;
      *procs     = pdb_query.list_of_procs;
    }

  return success;
}

gboolean
picman_pdb_proc_info (PicmanPDB          *pdb,
                    const gchar      *proc_name,
                    gchar           **blurb,
                    gchar           **help,
                    gchar           **author,
                    gchar           **copyright,
                    gchar           **date,
                    PicmanPDBProcType  *proc_type,
                    gint             *num_args,
                    gint             *num_values,
                    GError          **error)
{
  PicmanProcedure *procedure;
  PDBStrings     strings;

  g_return_val_if_fail (PICMAN_IS_PDB (pdb), FALSE);
  g_return_val_if_fail (proc_name != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  procedure = picman_pdb_lookup_procedure (pdb, proc_name);

  if (procedure)
    {
      picman_pdb_get_strings (&strings, procedure, FALSE);
    }
  else
    {
      const gchar *compat_name;

      compat_name = picman_pdb_lookup_compat_proc_name (pdb, proc_name);

      if (compat_name)
        {
          procedure = picman_pdb_lookup_procedure (pdb, compat_name);

          if (procedure)
            picman_pdb_get_strings (&strings, procedure, TRUE);
        }
    }

  if (procedure)
    {
      *blurb      = strings.compat ? strings.blurb : g_strdup (strings.blurb);
      *help       = strings.compat ? strings.help : g_strdup (strings.help);
      *author     = strings.compat ? strings.author : g_strdup (strings.author);
      *copyright  = strings.compat ? strings.copyright : g_strdup (strings.copyright);
      *date       = strings.compat ? strings.date : g_strdup (strings.date);
      *proc_type  = procedure->proc_type;
      *num_args   = procedure->num_args;
      *num_values = procedure->num_values;

      return TRUE;
    }

  g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_PROCEDURE_NOT_FOUND,
               _("Procedure '%s' not found"), proc_name);

  return FALSE;
}


/*  private functions  */

static gboolean
match_string (GRegex      *regex,
              const gchar *string)
{
  if (! string)
    string = "";

  return g_regex_match (regex, string, 0, NULL);
}

static void
picman_pdb_query_entry (gpointer key,
                      gpointer value,
                      gpointer user_data)
{
  PDBQuery      *pdb_query = user_data;
  GList         *list;
  PicmanProcedure *procedure;
  const gchar   *proc_name;
  PDBStrings     strings;
  GEnumClass    *enum_class;
  PicmanEnumDesc  *type_desc;

  proc_name = key;

  if (pdb_query->querying_compat)
    list = g_hash_table_lookup (pdb_query->pdb->procedures, value);
  else
    list = value;

  if (! list)
    return;

  procedure = list->data;

  picman_pdb_get_strings (&strings, procedure, pdb_query->querying_compat);

  enum_class = g_type_class_ref (PICMAN_TYPE_PDB_PROC_TYPE);
  type_desc = picman_enum_get_desc (enum_class, procedure->proc_type);
  g_type_class_unref  (enum_class);

  if (match_string (pdb_query->name_regex,      proc_name)         &&
      match_string (pdb_query->blurb_regex,     strings.blurb)     &&
      match_string (pdb_query->help_regex,      strings.help)      &&
      match_string (pdb_query->author_regex,    strings.author)    &&
      match_string (pdb_query->copyright_regex, strings.copyright) &&
      match_string (pdb_query->date_regex,      strings.date)      &&
      match_string (pdb_query->proc_type_regex, type_desc->value_desc))
    {
      pdb_query->num_procs++;
      pdb_query->list_of_procs = g_renew (gchar *, pdb_query->list_of_procs,
                                          pdb_query->num_procs);
      pdb_query->list_of_procs[pdb_query->num_procs - 1] = g_strdup (proc_name);
    }

  picman_pdb_free_strings (&strings);
}

/* #define DEBUG_OUTPUT 1 */

static gboolean
output_string (FILE        *file,
               const gchar *string)
{
#ifndef DEBUG_OUTPUT
  if (fprintf (file, "\"") < 0)
    return FALSE;
#endif

  if (string)
    while (*string)
      {
        switch (*string)
          {
          case '\\' : if (fprintf (file, "\\\\") < 0) return FALSE; break;
          case '\"' : if (fprintf (file, "\\\"") < 0) return FALSE; break;
          case '{'  : if (fprintf (file, "@{")   < 0) return FALSE; break;
          case '@'  : if (fprintf (file, "@@")   < 0) return FALSE; break;
          case '}'  : if (fprintf (file, "@}")   < 0) return FALSE; break;

          default:
            if (fprintf (file, "%c", *string) < 0)
              return FALSE;
          }
        string++;
      }

#ifndef DEBUG_OUTPUT
  if (fprintf (file, "\"\n") < 0)
    return FALSE;
#endif

  return TRUE;
}

static void
picman_pdb_print_entry (gpointer key,
                      gpointer value,
                      gpointer user_data)
{
  PDBDump     *pdb_dump = user_data;
  FILE        *file     = pdb_dump->file;
  const gchar *proc_name;
  GList       *list;
  GEnumClass  *arg_class;
  GEnumClass  *proc_class;
  GString     *buf;
  gint         num = 0;

  proc_name = key;

  if (pdb_dump->dumping_compat)
    list = g_hash_table_lookup (pdb_dump->pdb->procedures, value);
  else
    list = value;

  arg_class  = g_type_class_ref (PICMAN_TYPE_PDB_ARG_TYPE);
  proc_class = g_type_class_ref (PICMAN_TYPE_PDB_PROC_TYPE);

  buf = g_string_new ("");

  for (; list; list = list->next)
    {
      PicmanProcedure *procedure = list->data;
      PDBStrings     strings;
      GEnumValue    *arg_value;
      PicmanEnumDesc  *type_desc;
      gint           i;

      num++;

      picman_pdb_get_strings (&strings, procedure, pdb_dump->dumping_compat);

#ifdef DEBUG_OUTPUT
      fprintf (file, "(");
#else
      fprintf (file, "(register-procedure ");
#endif

      if (num != 1)
        {
          g_string_printf (buf, "%s <%d>", proc_name, num);
          output_string (file, buf->str);
        }
      else
        {
          output_string (file, proc_name);
        }

      type_desc = picman_enum_get_desc (proc_class, procedure->proc_type);

#ifdef DEBUG_OUTPUT

      fprintf (file, " (");

      for (i = 0; i < procedure->num_args; i++)
        {
          GParamSpec     *pspec = procedure->args[i];
          PicmanPDBArgType  arg_type;

          arg_type = picman_pdb_compat_arg_type_from_gtype (pspec->value_type);

          arg_value = g_enum_get_value (arg_class, arg_type);

          if (i > 0)
            fprintf (file, " ");

          output_string (file, arg_value->value_name);
        }

      fprintf (file, ") (");

      for (i = 0; i < procedure->num_values; i++)
        {
          GParamSpec     *pspec = procedure->values[i];
          PicmanPDBArgType  arg_type;

          arg_type = picman_pdb_compat_arg_type_from_gtype (pspec->value_type);

          arg_value = g_enum_get_value (arg_class, arg_type);

          if (i > 0)
            fprintf (file, " ");

          output_string (file, arg_value->value_name);
        }
      fprintf (file, "))\n");

#else /* ! DEBUG_OUTPUT */

      fprintf (file, "  ");
      output_string (file, strings.blurb);
      fprintf (file, "  ");
      output_string (file, strings.help);
      fprintf (file, "  ");
      output_string (file, strings.author);
      fprintf (file, "  ");
      output_string (file, strings.copyright);
      fprintf (file, "  ");
      output_string (file, strings.date);
      fprintf (file, "  ");
      output_string (file, type_desc->value_desc);

      fprintf (file, "  (");
      for (i = 0; i < procedure->num_args; i++)
        {
          GParamSpec     *pspec = procedure->args[i];
          PicmanPDBArgType  arg_type;
          gchar          *desc  = picman_param_spec_get_desc (pspec);

          fprintf (file, "\n    (\n");

          arg_type = picman_pdb_compat_arg_type_from_gtype (pspec->value_type);

          arg_value = g_enum_get_value (arg_class, arg_type);

          fprintf (file, "      ");
          output_string (file, g_param_spec_get_name (pspec));
          fprintf (file, "      ");
          output_string (file, arg_value->value_name);
          fprintf (file, "      ");
          output_string (file, desc);
          g_free (desc);

          fprintf (file, "    )");
        }
      fprintf (file, "\n  )\n");

      fprintf (file, "  (");
      for (i = 0; i < procedure->num_values; i++)
        {
          GParamSpec     *pspec = procedure->values[i];
          PicmanPDBArgType  arg_type;
          gchar          *desc  = picman_param_spec_get_desc (pspec);

          fprintf (file, "\n    (\n");

          arg_type = picman_pdb_compat_arg_type_from_gtype (pspec->value_type);

          arg_value = g_enum_get_value (arg_class, arg_type);

          fprintf (file, "      ");
          output_string (file, g_param_spec_get_name (pspec));
          fprintf (file, "      ");
          output_string (file, arg_value->value_name);
          fprintf (file, "      ");
          output_string (file, desc);
          g_free (desc);

          fprintf (file, "    )");
        }
      fprintf (file, "\n  )");
      fprintf (file, "\n)\n");

#endif /* DEBUG_OUTPUT */

      picman_pdb_free_strings (&strings);
    }

  g_string_free (buf, TRUE);

  g_type_class_unref (arg_class);
  g_type_class_unref (proc_class);
}

static void
picman_pdb_get_strings (PDBStrings    *strings,
                      PicmanProcedure *procedure,
                      gboolean       compat)
{
  strings->compat = compat;

  if (compat)
    {
      strings->blurb     = g_strdup_printf (COMPAT_BLURB,
                                            picman_object_get_name (procedure));
      strings->help      = g_strdup (strings->blurb);
      strings->author    = NULL;
      strings->copyright = NULL;
      strings->date      = NULL;
    }
  else
    {
      strings->blurb     = procedure->blurb;
      strings->help      = procedure->help;
      strings->author    = procedure->author;
      strings->copyright = procedure->copyright;
      strings->date      = procedure->date;
    }
}

static void
picman_pdb_free_strings (PDBStrings *strings)
{
  if (strings->compat)
    {
      g_free (strings->blurb);
      g_free (strings->help);
    }
}
