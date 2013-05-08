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
#include <sys/types.h>

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"

#include "pdb-types.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmanchannel.h"
#include "core/picmanlayer.h"
#include "core/picmanparamspecs.h"
#include "core/picmanprogress.h"

#include "vectors/picmanvectors.h"

#include "picmanpdbcontext.h"
#include "picmanpdberror.h"
#include "picmanprocedure.h"

#include "picman-intl.h"


static void             picman_procedure_finalize      (GObject         *object);

static gint64           picman_procedure_get_memsize   (PicmanObject      *object,
                                                      gint64          *gui_size);

static PicmanValueArray * picman_procedure_real_execute  (PicmanProcedure   *procedure,
                                                      Picman            *picman,
                                                      PicmanContext     *context,
                                                      PicmanProgress    *progress,
                                                      PicmanValueArray  *args,
                                                      GError         **error);
static void       picman_procedure_real_execute_async  (PicmanProcedure   *procedure,
                                                      Picman            *picman,
                                                      PicmanContext     *context,
                                                      PicmanProgress    *progress,
                                                      PicmanValueArray  *args,
                                                      PicmanObject      *display);

static void             picman_procedure_free_strings  (PicmanProcedure   *procedure);
static gboolean         picman_procedure_validate_args (PicmanProcedure   *procedure,
                                                      GParamSpec     **param_specs,
                                                      gint             n_param_specs,
                                                      PicmanValueArray  *args,
                                                      gboolean         return_vals,
                                                      GError         **error);


G_DEFINE_TYPE (PicmanProcedure, picman_procedure, PICMAN_TYPE_OBJECT)

#define parent_class picman_procedure_parent_class


static void
picman_procedure_class_init (PicmanProcedureClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  object_class->finalize         = picman_procedure_finalize;

  picman_object_class->get_memsize = picman_procedure_get_memsize;

  klass->execute                 = picman_procedure_real_execute;
  klass->execute_async           = picman_procedure_real_execute_async;
}

static void
picman_procedure_init (PicmanProcedure *procedure)
{
  procedure->proc_type = PICMAN_INTERNAL;
}

static void
picman_procedure_finalize (GObject *object)
{
  PicmanProcedure *procedure = PICMAN_PROCEDURE (object);
  gint           i;

  picman_procedure_free_strings (procedure);

  if (procedure->args)
    {
      for (i = 0; i < procedure->num_args; i++)
        g_param_spec_unref (procedure->args[i]);

      g_free (procedure->args);
      procedure->args = NULL;
    }

  if (procedure->values)
    {
      for (i = 0; i < procedure->num_values; i++)
        g_param_spec_unref (procedure->values[i]);

      g_free (procedure->values);
      procedure->values = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_procedure_get_memsize (PicmanObject *object,
                            gint64     *gui_size)
{
  PicmanProcedure *procedure = PICMAN_PROCEDURE (object);
  gint64         memsize   = 0;
  gint           i;

  if (! procedure->static_strings)
    {
      memsize += picman_string_get_memsize (procedure->original_name);
      memsize += picman_string_get_memsize (procedure->blurb);
      memsize += picman_string_get_memsize (procedure->help);
      memsize += picman_string_get_memsize (procedure->author);
      memsize += picman_string_get_memsize (procedure->copyright);
      memsize += picman_string_get_memsize (procedure->date);
      memsize += picman_string_get_memsize (procedure->deprecated);
    }

  memsize += procedure->num_args * sizeof (GParamSpec *);

  for (i = 0; i < procedure->num_args; i++)
    memsize += picman_g_param_spec_get_memsize (procedure->args[i]);

  memsize += procedure->num_values * sizeof (GParamSpec *);

  for (i = 0; i < procedure->num_values; i++)
    memsize += picman_g_param_spec_get_memsize (procedure->values[i]);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static PicmanValueArray *
picman_procedure_real_execute (PicmanProcedure   *procedure,
                             Picman            *picman,
                             PicmanContext     *context,
                             PicmanProgress    *progress,
                             PicmanValueArray  *args,
                             GError         **error)
{
  g_return_val_if_fail (picman_value_array_length (args) >=
                        procedure->num_args, NULL);

  return procedure->marshal_func (procedure, picman,
                                  context, progress,
                                  args, error);
}

static void
picman_procedure_real_execute_async (PicmanProcedure  *procedure,
                                   Picman           *picman,
                                   PicmanContext    *context,
                                   PicmanProgress   *progress,
                                   PicmanValueArray *args,
                                   PicmanObject     *display)
{
  PicmanValueArray *return_vals;
  GError         *error = NULL;

  g_return_if_fail (picman_value_array_length (args) >= procedure->num_args);

  return_vals = PICMAN_PROCEDURE_GET_CLASS (procedure)->execute (procedure,
                                                               picman,
                                                               context,
                                                               progress,
                                                               args,
                                                               &error);

  picman_value_array_unref (return_vals);

  if (error)
    {
      picman_message_literal (picman, G_OBJECT (progress), PICMAN_MESSAGE_ERROR,
			    error->message);
      g_error_free (error);
    }
}


/*  public functions  */

PicmanProcedure  *
picman_procedure_new (PicmanMarshalFunc marshal_func)
{
  PicmanProcedure *procedure;

  g_return_val_if_fail (marshal_func != NULL, NULL);

  procedure = g_object_new (PICMAN_TYPE_PROCEDURE, NULL);

  procedure->marshal_func = marshal_func;

  return procedure;
}

void
picman_procedure_set_strings (PicmanProcedure *procedure,
                            const gchar   *original_name,
                            const gchar   *blurb,
                            const gchar   *help,
                            const gchar   *author,
                            const gchar   *copyright,
                            const gchar   *date,
                            const gchar   *deprecated)
{
  g_return_if_fail (PICMAN_IS_PROCEDURE (procedure));

  picman_procedure_free_strings (procedure);

  procedure->original_name = g_strdup (original_name);
  procedure->blurb         = g_strdup (blurb);
  procedure->help          = g_strdup (help);
  procedure->author        = g_strdup (author);
  procedure->copyright     = g_strdup (copyright);
  procedure->date          = g_strdup (date);
  procedure->deprecated    = g_strdup (deprecated);

  procedure->static_strings = FALSE;
}

void
picman_procedure_set_static_strings (PicmanProcedure *procedure,
                                   const gchar   *original_name,
                                   const gchar   *blurb,
                                   const gchar   *help,
                                   const gchar   *author,
                                   const gchar   *copyright,
                                   const gchar   *date,
                                   const gchar   *deprecated)
{
  g_return_if_fail (PICMAN_IS_PROCEDURE (procedure));

  picman_procedure_free_strings (procedure);

  procedure->original_name = (gchar *) original_name;
  procedure->blurb         = (gchar *) blurb;
  procedure->help          = (gchar *) help;
  procedure->author        = (gchar *) author;
  procedure->copyright     = (gchar *) copyright;
  procedure->date          = (gchar *) date;
  procedure->deprecated    = (gchar *) deprecated;

  procedure->static_strings = TRUE;
}

void
picman_procedure_take_strings (PicmanProcedure *procedure,
                             gchar         *original_name,
                             gchar         *blurb,
                             gchar         *help,
                             gchar         *author,
                             gchar         *copyright,
                             gchar         *date,
                             gchar         *deprecated)
{
  g_return_if_fail (PICMAN_IS_PROCEDURE (procedure));

  picman_procedure_free_strings (procedure);

  procedure->original_name = original_name;
  procedure->blurb         = blurb;
  procedure->help          = help;
  procedure->author        = author;
  procedure->copyright     = copyright;
  procedure->date          = date;
  procedure->deprecated    = deprecated;

  procedure->static_strings = FALSE;
}

PicmanValueArray *
picman_procedure_execute (PicmanProcedure   *procedure,
                        Picman            *picman,
                        PicmanContext     *context,
                        PicmanProgress    *progress,
                        PicmanValueArray  *args,
                        GError         **error)
{
  PicmanValueArray *return_vals;
  GError         *pdb_error = NULL;

  g_return_val_if_fail (PICMAN_IS_PROCEDURE (procedure), NULL);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (args != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! picman_procedure_validate_args (procedure,
                                      procedure->args, procedure->num_args,
                                      args, FALSE, &pdb_error))
    {
      return_vals = picman_procedure_get_return_values (procedure, FALSE,
                                                      pdb_error);
      g_propagate_error (error, pdb_error);

      return return_vals;
    }

  if (PICMAN_IS_PDB_CONTEXT (context))
    context = g_object_ref (context);
  else
    context = picman_pdb_context_new (picman, context, TRUE);

  /*  call the procedure  */
  return_vals = PICMAN_PROCEDURE_GET_CLASS (procedure)->execute (procedure,
                                                               picman,
                                                               context,
                                                               progress,
                                                               args,
                                                               error);

  g_object_unref (context);

  if (return_vals)
    {
      switch (g_value_get_enum (picman_value_array_index (return_vals, 0)))
        {
        case PICMAN_PDB_CALLING_ERROR:
        case PICMAN_PDB_EXECUTION_ERROR:
          /*  If the error has not already been set, construct one
           *  from the error message that is optionally passed with
           *  the return values.
           */
          if (error && *error == NULL)
            {
              if (picman_value_array_length (return_vals) > 1 &&
                  G_VALUE_HOLDS_STRING (picman_value_array_index (return_vals, 1)))
                {
                  GValue *value = picman_value_array_index (return_vals, 1);

                  g_set_error_literal (error, PICMAN_PDB_ERROR,
                                       PICMAN_PDB_ERROR_FAILED,
				       g_value_get_string (value));
                }
            }
          break;

        default:
          break;
        }
    }
  else
    {
      g_warning ("%s: no return values, shouldn't happen", G_STRFUNC);

      pdb_error = g_error_new (PICMAN_PDB_ERROR,
                               PICMAN_PDB_ERROR_INVALID_RETURN_VALUE,
                               _("Procedure '%s' returned no return values"),
                               picman_object_get_name (procedure));

      return_vals = picman_procedure_get_return_values (procedure, FALSE,
                                                      pdb_error);
      if (error && *error == NULL)
        g_propagate_error (error, pdb_error);
      else
        g_error_free (pdb_error);

    }

  return return_vals;
}

void
picman_procedure_execute_async (PicmanProcedure  *procedure,
                              Picman           *picman,
                              PicmanContext    *context,
                              PicmanProgress   *progress,
                              PicmanValueArray *args,
                              PicmanObject     *display,
                              GError        **error)
{
  g_return_if_fail (PICMAN_IS_PROCEDURE (procedure));
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (args != NULL);
  g_return_if_fail (display == NULL || PICMAN_IS_OBJECT (display));
  g_return_if_fail (error == NULL || *error == NULL);

  if (picman_procedure_validate_args (procedure,
                                    procedure->args, procedure->num_args,
                                    args, FALSE, error))
    {
      if (PICMAN_IS_PDB_CONTEXT (context))
        context = g_object_ref (context);
      else
        context = picman_pdb_context_new (picman, context, TRUE);

      PICMAN_PROCEDURE_GET_CLASS (procedure)->execute_async (procedure, picman,
                                                           context, progress,
                                                           args, display);

      g_object_unref (context);
    }
}

PicmanValueArray *
picman_procedure_get_arguments (PicmanProcedure *procedure)
{
  PicmanValueArray *args;
  GValue          value = { 0, };
  gint            i;

  g_return_val_if_fail (PICMAN_IS_PROCEDURE (procedure), NULL);

  args = picman_value_array_new (procedure->num_args);

  for (i = 0; i < procedure->num_args; i++)
    {
      g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (procedure->args[i]));
      picman_value_array_append (args, &value);
      g_value_unset (&value);
    }

  return args;
}

PicmanValueArray *
picman_procedure_get_return_values (PicmanProcedure *procedure,
                                  gboolean       success,
                                  const GError  *error)
{
  PicmanValueArray *args;
  GValue          value = { 0, };
  gint            i;

  g_return_val_if_fail (success == FALSE || PICMAN_IS_PROCEDURE (procedure),
                        NULL);

  if (success)
    {
      args = picman_value_array_new (procedure->num_values + 1);

      g_value_init (&value, PICMAN_TYPE_PDB_STATUS_TYPE);
      g_value_set_enum (&value, PICMAN_PDB_SUCCESS);
      picman_value_array_append (args, &value);
      g_value_unset (&value);

      for (i = 0; i < procedure->num_values; i++)
        {
          g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (procedure->values[i]));
          picman_value_array_append (args, &value);
          g_value_unset (&value);
        }
    }
  else
    {
      args = picman_value_array_new ((error && error->message) ? 2 : 1);

      g_value_init (&value, PICMAN_TYPE_PDB_STATUS_TYPE);

      /*  errors in the PICMAN_PDB_ERROR domain are calling errors  */
      if (error && error->domain == PICMAN_PDB_ERROR)
        {
          switch ((PicmanPdbErrorCode) error->code)
            {
            case PICMAN_PDB_ERROR_FAILED:
            case PICMAN_PDB_ERROR_PROCEDURE_NOT_FOUND:
            case PICMAN_PDB_ERROR_INVALID_ARGUMENT:
            case PICMAN_PDB_ERROR_INVALID_RETURN_VALUE:
            case PICMAN_PDB_ERROR_INTERNAL_ERROR:
              g_value_set_enum (&value, PICMAN_PDB_CALLING_ERROR);
              break;

            case PICMAN_PDB_ERROR_CANCELLED:
              g_value_set_enum (&value, PICMAN_PDB_CANCEL);
              break;

            default:
              g_assert_not_reached ();
            }
        }
      else
        {
          g_value_set_enum (&value, PICMAN_PDB_EXECUTION_ERROR);
        }

      picman_value_array_append (args, &value);
      g_value_unset (&value);

      if (error && error->message)
        {
          g_value_init (&value, G_TYPE_STRING);
          g_value_set_string (&value, error->message);
          picman_value_array_append (args, &value);
          g_value_unset (&value);
        }
    }

  return args;
}

void
picman_procedure_add_argument (PicmanProcedure *procedure,
                             GParamSpec    *pspec)
{
  g_return_if_fail (PICMAN_IS_PROCEDURE (procedure));
  g_return_if_fail (G_IS_PARAM_SPEC (pspec));

  procedure->args = g_renew (GParamSpec *, procedure->args,
                             procedure->num_args + 1);

  procedure->args[procedure->num_args] = pspec;

  g_param_spec_ref_sink (pspec);

  procedure->num_args++;
}

void
picman_procedure_add_return_value (PicmanProcedure *procedure,
                                 GParamSpec    *pspec)
{
  g_return_if_fail (PICMAN_IS_PROCEDURE (procedure));
  g_return_if_fail (G_IS_PARAM_SPEC (pspec));

  procedure->values = g_renew (GParamSpec *, procedure->values,
                               procedure->num_values + 1);

  procedure->values[procedure->num_values] = pspec;

  g_param_spec_ref_sink (pspec);

  procedure->num_values++;
}

/**
 * picman_procedure_create_override:
 * @procedure:
 * @new_marshal_func:
 *
 * Creates a new PicmanProcedure that can be used to override the
 * existing @procedure.
 *
 * Returns: The new #PicmanProcedure.
 **/
PicmanProcedure *
picman_procedure_create_override (PicmanProcedure   *procedure,
                                PicmanMarshalFunc  new_marshal_func)
{
  PicmanProcedure *new_procedure = NULL;
  const gchar   *name          = NULL;
  int            i             = 0;

  new_procedure = picman_procedure_new (new_marshal_func);
  name          = picman_object_get_name (procedure);

  picman_object_set_static_name (PICMAN_OBJECT (new_procedure), name);

  for (i = 0; i < procedure->num_args; i++)
    picman_procedure_add_argument (new_procedure, procedure->args[i]);

  for (i = 0; i < procedure->num_values; i++)
    picman_procedure_add_return_value (new_procedure, procedure->values[i]);

  return new_procedure;
}

gint
picman_procedure_name_compare (PicmanProcedure *proc1,
                             PicmanProcedure *proc2)
{
  /* Assume there always is a name, don't bother with NULL checks */
  return strcmp (proc1->original_name,
                 proc2->original_name);
}

/*  private functions  */

static void
picman_procedure_free_strings (PicmanProcedure *procedure)
{
  if (! procedure->static_strings)
    {
      g_free (procedure->original_name);
      g_free (procedure->blurb);
      g_free (procedure->help);
      g_free (procedure->author);
      g_free (procedure->copyright);
      g_free (procedure->date);
      g_free (procedure->deprecated);
    }

  procedure->original_name = NULL;
  procedure->blurb         = NULL;
  procedure->help          = NULL;
  procedure->author        = NULL;
  procedure->copyright     = NULL;
  procedure->date          = NULL;
  procedure->deprecated    = NULL;

  procedure->static_strings = FALSE;
}

static gboolean
picman_procedure_validate_args (PicmanProcedure  *procedure,
                              GParamSpec    **param_specs,
                              gint            n_param_specs,
                              PicmanValueArray *args,
                              gboolean        return_vals,
                              GError        **error)
{
  gint i;

  for (i = 0; i < MIN (picman_value_array_length (args), n_param_specs); i++)
    {
      GValue     *arg       = picman_value_array_index (args, i);
      GParamSpec *pspec     = param_specs[i];
      GType       arg_type  = G_VALUE_TYPE (arg);
      GType       spec_type = G_PARAM_SPEC_VALUE_TYPE (pspec);

      if (arg_type != spec_type)
        {
          if (return_vals)
            {
              g_set_error (error,
                           PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_RETURN_VALUE,
                           _("Procedure '%s' returned a wrong value type "
                             "for return value '%s' (#%d). "
                             "Expected %s, got %s."),
                           picman_object_get_name (procedure),
                           g_param_spec_get_name (pspec),
                           i + 1, g_type_name (spec_type),
                           g_type_name (arg_type));
            }
          else
            {
              g_set_error (error,
                           PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                           _("Procedure '%s' has been called with a "
                             "wrong value type for argument '%s' (#%d). "
                             "Expected %s, got %s."),
                           picman_object_get_name (procedure),
                           g_param_spec_get_name (pspec),
                           i + 1, g_type_name (spec_type),
                           g_type_name (arg_type));
            }

          return FALSE;
        }
      else if (! (pspec->flags & PICMAN_PARAM_NO_VALIDATE))
        {
          GValue string_value = { 0, };

          g_value_init (&string_value, G_TYPE_STRING);

          if (g_value_type_transformable (arg_type, G_TYPE_STRING))
            g_value_transform (arg, &string_value);
          else
            g_value_set_static_string (&string_value,
                                       "<not transformable to string>");

          if (g_param_value_validate (pspec, arg))
            {
              if (PICMAN_IS_PARAM_SPEC_DRAWABLE_ID (pspec) &&
                  g_value_get_int (arg) == -1)
                {
                  if (return_vals)
                    {
                      g_set_error (error,
                                   PICMAN_PDB_ERROR,
                                   PICMAN_PDB_ERROR_INVALID_RETURN_VALUE,
                                   _("Procedure '%s' returned an "
                                     "invalid ID for argument '%s'. "
                                     "Most likely a plug-in is trying "
                                     "to work on a layer that doesn't "
                                     "exist any longer."),
                                   picman_object_get_name (procedure),
                                   g_param_spec_get_name (pspec));
                    }
                  else
                    {
                      g_set_error (error,
                                   PICMAN_PDB_ERROR,
                                   PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                                   _("Procedure '%s' has been called with an "
                                     "invalid ID for argument '%s'. "
                                     "Most likely a plug-in is trying "
                                     "to work on a layer that doesn't "
                                     "exist any longer."),
                                   picman_object_get_name (procedure),
                                   g_param_spec_get_name (pspec));
                    }
                }
              else if (PICMAN_IS_PARAM_SPEC_IMAGE_ID (pspec) &&
                       g_value_get_int (arg) == -1)
                {
                  if (return_vals)
                    {
                      g_set_error (error,
                                   PICMAN_PDB_ERROR,
                                   PICMAN_PDB_ERROR_INVALID_RETURN_VALUE,
                                   _("Procedure '%s' returned an "
                                     "invalid ID for argument '%s'. "
                                     "Most likely a plug-in is trying "
                                     "to work on an image that doesn't "
                                     "exist any longer."),
                                   picman_object_get_name (procedure),
                                   g_param_spec_get_name (pspec));
                    }
                  else
                    {
                      g_set_error (error,
                                   PICMAN_PDB_ERROR,
                                   PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                                   _("Procedure '%s' has been called with an "
                                     "invalid ID for argument '%s'. "
                                     "Most likely a plug-in is trying "
                                     "to work on an image that doesn't "
                                     "exist any longer."),
                                   picman_object_get_name (procedure),
                                   g_param_spec_get_name (pspec));
                    }
                }
              else
                {
                  const gchar *value = g_value_get_string (&string_value);

                  if (value == NULL)
                    value = "(null)";

                  if (return_vals)
                    {
                      g_set_error (error,
                                   PICMAN_PDB_ERROR,
                                   PICMAN_PDB_ERROR_INVALID_RETURN_VALUE,
                                   _("Procedure '%s' returned "
                                     "'%s' as return value '%s' "
                                     "(#%d, type %s). "
                                     "This value is out of range."),
                                   picman_object_get_name (procedure),
                                   value,
                                   g_param_spec_get_name (pspec),
                                   i + 1, g_type_name (spec_type));
                    }
                  else
                    {
                      g_set_error (error,
                                   PICMAN_PDB_ERROR,
                                   PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                                   _("Procedure '%s' has been called with "
                                     "value '%s' for argument '%s' "
                                     "(#%d, type %s). "
                                     "This value is out of range."),
                                   picman_object_get_name (procedure),
                                   value,
                                   g_param_spec_get_name (pspec),
                                   i + 1, g_type_name (spec_type));
                    }
                }

              g_value_unset (&string_value);

              return FALSE;
            }

          g_value_unset (&string_value);
        }
    }

  return TRUE;
}
