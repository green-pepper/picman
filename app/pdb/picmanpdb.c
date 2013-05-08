/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

#include <gegl.h>
#include <gobject/gvaluecollector.h>

#include "libpicmanbase/picmanbase.h"

#include "pdb-types.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmancontext.h"
#include "core/picmanmarshal.h"
#include "core/picmanprogress.h"

#include "picmanpdb.h"
#include "picmanpdberror.h"
#include "picmanprocedure.h"

#include "picman-intl.h"


enum
{
  REGISTER_PROCEDURE,
  UNREGISTER_PROCEDURE,
  LAST_SIGNAL
};


static void     picman_pdb_finalize                  (GObject       *object);
static gint64   picman_pdb_get_memsize               (PicmanObject    *object,
                                                    gint64        *gui_size);
static void     picman_pdb_real_register_procedure   (PicmanPDB       *pdb,
                                                    PicmanProcedure *procedure);
static void     picman_pdb_real_unregister_procedure (PicmanPDB       *pdb,
                                                    PicmanProcedure *procedure);
static void     picman_pdb_entry_free                (gpointer       key,
                                                    gpointer       value,
                                                    gpointer       user_data);
static gint64   picman_pdb_entry_get_memsize         (GList         *procedures,
                                                    gint64        *gui_size);


G_DEFINE_TYPE (PicmanPDB, picman_pdb, PICMAN_TYPE_OBJECT)

#define parent_class picman_pdb_parent_class

static guint picman_pdb_signals[LAST_SIGNAL] = { 0 };


static void
picman_pdb_class_init (PicmanPDBClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  picman_pdb_signals[REGISTER_PROCEDURE] =
    g_signal_new ("register-procedure",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPDBClass, register_procedure),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_PROCEDURE);

  picman_pdb_signals[UNREGISTER_PROCEDURE] =
    g_signal_new ("unregister-procedure",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPDBClass, unregister_procedure),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_PROCEDURE);

  object_class->finalize         = picman_pdb_finalize;

  picman_object_class->get_memsize = picman_pdb_get_memsize;

  klass->register_procedure      = picman_pdb_real_register_procedure;
  klass->unregister_procedure    = picman_pdb_real_unregister_procedure;
}

static void
picman_pdb_init (PicmanPDB *pdb)
{
  pdb->procedures        = g_hash_table_new (g_str_hash, g_str_equal);
  pdb->compat_proc_names = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
picman_pdb_finalize (GObject *object)
{
  PicmanPDB *pdb = PICMAN_PDB (object);

  if (pdb->procedures)
    {
      g_hash_table_foreach (pdb->procedures, picman_pdb_entry_free, NULL);
      g_hash_table_destroy (pdb->procedures);
      pdb->procedures = NULL;
    }

  if (pdb->compat_proc_names)
    {
      g_hash_table_destroy (pdb->compat_proc_names);
      pdb->compat_proc_names = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_pdb_get_memsize (PicmanObject *object,
                      gint64     *gui_size)
{
  PicmanPDB *pdb     = PICMAN_PDB (object);
  gint64   memsize = 0;

  memsize += picman_g_hash_table_get_memsize_foreach (pdb->procedures,
                                                    (PicmanMemsizeFunc)
                                                    picman_pdb_entry_get_memsize,
                                                    gui_size);
  memsize += picman_g_hash_table_get_memsize (pdb->compat_proc_names, 0);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_pdb_real_register_procedure (PicmanPDB       *pdb,
                                  PicmanProcedure *procedure)
{
  const gchar *name;
  GList       *list;

  name = picman_object_get_name (procedure);

  list = g_hash_table_lookup (pdb->procedures, name);

  g_hash_table_replace (pdb->procedures, (gpointer) name,
                        g_list_prepend (list, g_object_ref (procedure)));
}

static void
picman_pdb_real_unregister_procedure (PicmanPDB       *pdb,
                                    PicmanProcedure *procedure)
{
  const gchar *name;
  GList       *list;

  name = picman_object_get_name (procedure);

  list = g_hash_table_lookup (pdb->procedures, name);

  if (list)
    {
      list = g_list_remove (list, procedure);

      if (list)
        {
          name = picman_object_get_name (list->data);
          g_hash_table_replace (pdb->procedures, (gpointer) name, list);
        }
      else
        {
          g_hash_table_remove (pdb->procedures, name);
        }

      g_object_unref (procedure);
    }
}


/*  public functions  */

PicmanPDB *
picman_pdb_new (Picman *picman)
{
  PicmanPDB *pdb;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  pdb = g_object_new (PICMAN_TYPE_PDB,
                      "name", "pdb",
                      NULL);

  pdb->picman = picman;

  return pdb;
}

void
picman_pdb_register_procedure (PicmanPDB       *pdb,
                             PicmanProcedure *procedure)
{
  g_return_if_fail (PICMAN_IS_PDB (pdb));
  g_return_if_fail (PICMAN_IS_PROCEDURE (procedure));

  if (! procedure->deprecated ||
      pdb->picman->pdb_compat_mode != PICMAN_PDB_COMPAT_OFF)
    {
      g_signal_emit (pdb, picman_pdb_signals[REGISTER_PROCEDURE], 0,
                     procedure);
    }
}

void
picman_pdb_unregister_procedure (PicmanPDB       *pdb,
                               PicmanProcedure *procedure)
{
  g_return_if_fail (PICMAN_IS_PDB (pdb));
  g_return_if_fail (PICMAN_IS_PROCEDURE (procedure));

  g_signal_emit (pdb, picman_pdb_signals[UNREGISTER_PROCEDURE], 0,
                 procedure);
}

PicmanProcedure *
picman_pdb_lookup_procedure (PicmanPDB     *pdb,
                           const gchar *name)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_PDB (pdb), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  list = g_hash_table_lookup (pdb->procedures, name);

  if (list)
    return list->data;

  return NULL;
}

void
picman_pdb_register_compat_proc_name (PicmanPDB     *pdb,
                                    const gchar *old_name,
                                    const gchar *new_name)
{
  g_return_if_fail (PICMAN_IS_PDB (pdb));
  g_return_if_fail (old_name != NULL);
  g_return_if_fail (new_name != NULL);

  g_hash_table_insert (pdb->compat_proc_names,
                       (gpointer) old_name,
                       (gpointer) new_name);
}

const gchar *
picman_pdb_lookup_compat_proc_name (PicmanPDB     *pdb,
                                  const gchar *old_name)
{
  g_return_val_if_fail (PICMAN_IS_PDB (pdb), NULL);
  g_return_val_if_fail (old_name != NULL, NULL);

  return g_hash_table_lookup (pdb->compat_proc_names, old_name);
}

PicmanValueArray *
picman_pdb_execute_procedure_by_name_args (PicmanPDB         *pdb,
                                         PicmanContext     *context,
                                         PicmanProgress    *progress,
                                         GError         **error,
                                         const gchar     *name,
                                         PicmanValueArray  *args)
{
  PicmanValueArray *return_vals = NULL;
  GList          *list;

  g_return_val_if_fail (PICMAN_IS_PDB (pdb), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (name != NULL, NULL);

  list = g_hash_table_lookup (pdb->procedures, name);

  if (list == NULL)
    {
      GError *pdb_error = g_error_new (PICMAN_PDB_ERROR,
                                       PICMAN_PDB_ERROR_PROCEDURE_NOT_FOUND,
                                       _("Procedure '%s' not found"), name);

      return_vals = picman_procedure_get_return_values (NULL, FALSE, pdb_error);
      g_propagate_error (error, pdb_error);

      return return_vals;
    }

  g_return_val_if_fail (args != NULL, NULL);

  for (; list; list = g_list_next (list))
    {
      PicmanProcedure *procedure = list->data;

      g_return_val_if_fail (PICMAN_IS_PROCEDURE (procedure), NULL);

      return_vals = picman_procedure_execute (procedure,
                                            pdb->picman, context, progress,
                                            args, error);

      if (g_value_get_enum (picman_value_array_index (return_vals, 0)) ==
          PICMAN_PDB_PASS_THROUGH)
        {
          /*  If the return value is PICMAN_PDB_PASS_THROUGH and there is
           *  a next procedure in the list, destroy the return values
           *  and run the next procedure.
           */
          if (g_list_next (list))
            {
              picman_value_array_unref (return_vals);
              g_clear_error (error);
            }
        }
      else
        {
          /*  No PICMAN_PDB_PASS_THROUGH, break out of the list of
           *  procedures and return the current return values.
           */
          break;
        }
    }

  return return_vals;
}

PicmanValueArray *
picman_pdb_execute_procedure_by_name (PicmanPDB       *pdb,
                                    PicmanContext   *context,
                                    PicmanProgress  *progress,
                                    GError       **error,
                                    const gchar   *name,
                                    ...)
{
  PicmanProcedure  *procedure;
  PicmanValueArray *args;
  PicmanValueArray *return_vals;
  va_list         va_args;
  gint            i;

  g_return_val_if_fail (PICMAN_IS_PDB (pdb), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (name != NULL, NULL);

  procedure = picman_pdb_lookup_procedure (pdb, name);

  if (! procedure)
    {
      GError *pdb_error = g_error_new (PICMAN_PDB_ERROR,
                                       PICMAN_PDB_ERROR_PROCEDURE_NOT_FOUND,
                                       _("Procedure '%s' not found"), name);

      return_vals = picman_procedure_get_return_values (NULL, FALSE, pdb_error);
      g_propagate_error (error, pdb_error);

      return return_vals;
    }

  args = picman_procedure_get_arguments (procedure);

  va_start (va_args, name);

  for (i = 0; i < procedure->num_args; i++)
    {
      GValue *value;
      GType   arg_type;
      gchar  *error_msg = NULL;

      arg_type = va_arg (va_args, GType);

      if (arg_type == G_TYPE_NONE)
        break;

      value = picman_value_array_index (args, i);

      if (arg_type != G_VALUE_TYPE (value))
        {
          GError      *pdb_error;
          const gchar *expected = g_type_name (G_VALUE_TYPE (value));
          const gchar *got      = g_type_name (arg_type);

          picman_value_array_unref (args);

          pdb_error = g_error_new (PICMAN_PDB_ERROR,
                                   PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                                   _("Procedure '%s' has been called with a "
                                     "wrong type for argument #%d. "
                                     "Expected %s, got %s."),
                                   picman_object_get_name (procedure),
                                   i + 1, expected, got);

          return_vals = picman_procedure_get_return_values (procedure,
                                                          FALSE, pdb_error);
          g_propagate_error (error, pdb_error);

          va_end (va_args);

          return return_vals;
        }

      G_VALUE_COLLECT (value, va_args, G_VALUE_NOCOPY_CONTENTS, &error_msg);

      if (error_msg)
        {
          GError *pdb_error = g_error_new_literal (PICMAN_PDB_ERROR,
                                                   PICMAN_PDB_ERROR_INTERNAL_ERROR,
                                                   error_msg);
          g_warning ("%s: %s", G_STRFUNC, error_msg);
          g_free (error_msg);

          picman_value_array_unref (args);

          return_vals = picman_procedure_get_return_values (procedure,
                                                          FALSE, pdb_error);
          g_propagate_error (error, pdb_error);

          va_end (va_args);

          return return_vals;
        }
    }

  va_end (va_args);

  return_vals = picman_pdb_execute_procedure_by_name_args (pdb, context,
                                                         progress, error,
                                                         name, args);

  picman_value_array_unref (args);

  return return_vals;
}

/**
 * picman_pdb_get_deprecated_procedures:
 * @pdb:
 *
 * Returns: A new #GList with the deprecated procedures. Free with
 *          g_list_free().
 **/
GList *
picman_pdb_get_deprecated_procedures (PicmanPDB *pdb)
{
  GList *result = NULL;
  GList *procs;
  GList *iter;

  g_return_val_if_fail (PICMAN_IS_PDB (pdb), NULL);

  procs = g_hash_table_get_values (pdb->procedures);

  for (iter = procs;
       iter;
       iter = g_list_next (iter))
    {
      GList *proc_list = iter->data;

      /* Only care about the first procedure in the list */
      PicmanProcedure *procedure = PICMAN_PROCEDURE (proc_list->data);

      if (procedure->deprecated)
        result = g_list_prepend (result, procedure);
    }

  result = g_list_sort (result, (GCompareFunc) picman_procedure_name_compare);

  g_list_free (procs);

  return result;
}


/*  private functions  */

static void
picman_pdb_entry_free (gpointer key,
                     gpointer value,
                     gpointer user_data)
{
  if (value)
    g_list_free_full (value, (GDestroyNotify) g_object_unref);
}

static gint64
picman_pdb_entry_get_memsize (GList  *procedures,
                            gint64 *gui_size)
{
  return picman_g_list_get_memsize_foreach (procedures,
                                          (PicmanMemsizeFunc)
                                          picman_object_get_memsize,
                                          gui_size);
}
