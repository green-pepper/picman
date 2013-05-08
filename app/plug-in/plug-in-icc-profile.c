/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * plug-in-icc-profile.c
 * Copyright (C) 2006  Sven Neumann <sven@picman.org>
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

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"

#include "core/core-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanparamspecs.h"
#include "core/picmanprogress.h"

#include "pdb/picmanpdb.h"
#include "pdb/picmanprocedure.h"

#include "picmanpluginerror.h"
#include "plug-in-icc-profile.h"

#include "picman-intl.h"


#define ICC_PROFILE_APPLY_RGB_PROC  "plug-in-icc-profile-apply-rgb"
#define ICC_PROFILE_INFO_PROC       "plug-in-icc-profile-info"
#define ICC_PROFILE_FILE_INFO_PROC  "plug-in-icc-profile-file-info"


static void
plug_in_icc_profile_info_return (PicmanValueArray  *return_vals,
                                 gchar          **name,
                                 gchar          **desc,
                                 gchar          **info);


gboolean
plug_in_icc_profile_apply_rgb (PicmanImage     *image,
                               PicmanContext   *context,
                               PicmanProgress  *progress,
                               PicmanRunMode    run_mode,
                               GError       **error)
{
  Picman          *picman;
  PicmanProcedure *procedure;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), FALSE);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  picman = image->picman;

  if (picman_image_get_base_type (image) == PICMAN_GRAY)
    {
      g_set_error (error, PICMAN_PLUG_IN_ERROR, PICMAN_PLUG_IN_EXECUTION_FAILED,
                   _("Can't apply color profile to grayscale image (%s)"),
                   ICC_PROFILE_APPLY_RGB_PROC);
      return FALSE;
    }

  procedure = picman_pdb_lookup_procedure (picman->pdb, ICC_PROFILE_APPLY_RGB_PROC);

  if (procedure &&
      procedure->num_args >= 2 &&
      PICMAN_IS_PARAM_SPEC_INT32 (procedure->args[0]) &&
      PICMAN_IS_PARAM_SPEC_IMAGE_ID (procedure->args[1]))
    {
      PicmanValueArray         *return_vals;
      PicmanPDBStatusType       status;
      PicmanColorProfilePolicy  policy = PICMAN_COLOR_PROFILE_POLICY_ASK;
      gboolean                success;

      return_vals =
        picman_pdb_execute_procedure_by_name (picman->pdb, context, progress, error,
                                            ICC_PROFILE_APPLY_RGB_PROC,
                                            PICMAN_TYPE_INT32, run_mode,
                                            PICMAN_TYPE_IMAGE_ID,
                                            picman_image_get_ID (image),
                                            G_TYPE_NONE);

      status = g_value_get_enum (picman_value_array_index (return_vals, 0));

      switch (status)
        {
        case PICMAN_PDB_SUCCESS:
          policy = PICMAN_COLOR_PROFILE_POLICY_CONVERT;
          success = TRUE;
          break;

        case PICMAN_PDB_CANCEL:
          policy = PICMAN_COLOR_PROFILE_POLICY_KEEP;
          success = TRUE;
          break;

        default:
          if (error && *error == NULL)
            g_set_error (error,
                         PICMAN_PLUG_IN_ERROR, PICMAN_PLUG_IN_EXECUTION_FAILED,
                         _("Error running '%s'"), ICC_PROFILE_APPLY_RGB_PROC);
          success = FALSE;
          break;
        }

      if (success && picman_value_array_length (return_vals) > 1)
        {
          GValue *value = picman_value_array_index (return_vals, 1);

          if (PICMAN_VALUE_HOLDS_INT32 (value) && g_value_get_int (value))
            {
              g_object_set (G_OBJECT (picman->config),
                            "color-profile-policy", policy,
                            NULL);
            }
        }

      picman_value_array_unref (return_vals);

      return success;
    }

  g_set_error (error,
               PICMAN_PLUG_IN_ERROR, PICMAN_PLUG_IN_NOT_FOUND,
               _("Plug-In missing (%s)"), ICC_PROFILE_APPLY_RGB_PROC);

  return FALSE;
}

gboolean
plug_in_icc_profile_info (PicmanImage     *image,
                          PicmanContext   *context,
                          PicmanProgress  *progress,
                          gchar        **name,
                          gchar        **desc,
                          gchar        **info,
                          GError       **error)
{
  Picman          *picman;
  PicmanProcedure *procedure;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), FALSE);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  picman = image->picman;

  procedure = picman_pdb_lookup_procedure (picman->pdb, ICC_PROFILE_INFO_PROC);

  if (procedure &&
      procedure->num_args >= 1 &&
      PICMAN_IS_PARAM_SPEC_IMAGE_ID (procedure->args[0]))
    {
      PicmanValueArray    *return_vals;
      PicmanPDBStatusType  status;

      return_vals =
        picman_pdb_execute_procedure_by_name (picman->pdb, context, progress, error,
                                            ICC_PROFILE_INFO_PROC,
                                            PICMAN_TYPE_IMAGE_ID,
                                            picman_image_get_ID (image),
                                            G_TYPE_NONE);

      status = g_value_get_enum (picman_value_array_index (return_vals, 0));

      switch (status)
        {
        case PICMAN_PDB_SUCCESS:
          plug_in_icc_profile_info_return (return_vals, name, desc, info);
          break;

        default:
          if (error && *error == NULL)
            g_set_error (error, PICMAN_PLUG_IN_ERROR, PICMAN_PLUG_IN_FAILED,
                         _("Error running '%s'"), ICC_PROFILE_INFO_PROC);
          break;
        }

      picman_value_array_unref (return_vals);

      return (status == PICMAN_PDB_SUCCESS);
    }

  g_set_error (error, PICMAN_PLUG_IN_ERROR, PICMAN_PLUG_IN_FAILED,
               _("Plug-In missing (%s)"), ICC_PROFILE_INFO_PROC);

  return FALSE;
}

gboolean
plug_in_icc_profile_file_info (Picman          *picman,
                               PicmanContext   *context,
                               PicmanProgress  *progress,
                               const gchar   *filename,
                               gchar        **name,
                               gchar        **desc,
                               gchar        **info,
                               GError       **error)
{
  PicmanProcedure *procedure;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), FALSE);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  procedure = picman_pdb_lookup_procedure (picman->pdb, ICC_PROFILE_FILE_INFO_PROC);

  if (procedure &&
      procedure->num_args >= 1 &&
      PICMAN_IS_PARAM_SPEC_STRING (procedure->args[0]))
    {
      PicmanValueArray    *return_vals;
      PicmanPDBStatusType  status;

      return_vals =
        picman_pdb_execute_procedure_by_name (picman->pdb, context, progress, error,
                                            ICC_PROFILE_FILE_INFO_PROC,
                                            G_TYPE_STRING, filename,
                                            G_TYPE_NONE);

      status = g_value_get_enum (picman_value_array_index (return_vals, 0));

      switch (status)
        {
        case PICMAN_PDB_SUCCESS:
          plug_in_icc_profile_info_return (return_vals, name, desc, info);
          break;

        default:
          if (error && *error == NULL)
            g_set_error (error, PICMAN_PLUG_IN_ERROR, PICMAN_PLUG_IN_FAILED,
                         _("Error running '%s'"), ICC_PROFILE_FILE_INFO_PROC);
          break;
        }

      picman_value_array_unref (return_vals);

      return (status == PICMAN_PDB_SUCCESS);
    }

  g_set_error (error, PICMAN_PLUG_IN_ERROR, PICMAN_PLUG_IN_FAILED,
               _("Plug-In missing (%s)"), ICC_PROFILE_FILE_INFO_PROC);

  return FALSE;
}

static void
plug_in_icc_profile_info_return (PicmanValueArray  *return_vals,
                                 gchar          **name,
                                 gchar          **desc,
                                 gchar          **info)
{
  if (name)
    {
      GValue *value = picman_value_array_index (return_vals, 1);

      *name = G_VALUE_HOLDS_STRING (value) ? g_value_dup_string (value) : NULL;
    }

  if (desc)
    {
      GValue *value = picman_value_array_index (return_vals, 2);

      *desc = G_VALUE_HOLDS_STRING (value) ? g_value_dup_string (value) : NULL;
    }

  if (info)
    {
      GValue *value = picman_value_array_index (return_vals, 3);

      *info = G_VALUE_HOLDS_STRING (value) ? g_value_dup_string (value) : NULL;
    }
}
