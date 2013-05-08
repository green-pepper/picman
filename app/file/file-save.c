/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995, 1996, 1997 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997 Josh MacDonald
 *
 * file-save.c
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gegl.h>
#include <glib/gstdio.h>

#ifdef G_OS_WIN32
#include <io.h>
#define W_OK 2
#endif

#include "libpicmanbase/picmanbase.h"

#include "core/core-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmandocumentlist.h"
#include "core/picmandrawable.h"
#include "core/picmanimage.h"
#include "core/picmanimagefile.h"
#include "core/picmanparamspecs.h"
#include "core/picmanprogress.h"

#include "pdb/picmanpdb.h"

#include "plug-in/picmanpluginprocedure.h"

#include "file-save.h"
#include "file-utils.h"
#include "picman-file.h"

#include "picman-intl.h"


/*  public functions  */

PicmanPDBStatusType
file_save (Picman                *picman,
           PicmanImage           *image,
           PicmanProgress        *progress,
           const gchar         *uri,
           PicmanPlugInProcedure *file_proc,
           PicmanRunMode          run_mode,
           gboolean             change_saved_state,
           gboolean             export_backward,
           gboolean             export_forward,
           GError             **error)
{
  PicmanDrawable      *drawable;
  PicmanValueArray    *return_vals;
  PicmanPDBStatusType  status;
  gchar             *filename;
  gint32             image_ID;
  gint32             drawable_ID;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), PICMAN_PDB_CALLING_ERROR);
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), PICMAN_PDB_CALLING_ERROR);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress),
                        PICMAN_PDB_CALLING_ERROR);
  g_return_val_if_fail (uri != NULL, PICMAN_PDB_CALLING_ERROR);
  g_return_val_if_fail (PICMAN_IS_PLUG_IN_PROCEDURE (file_proc),
                        PICMAN_PDB_CALLING_ERROR);
  g_return_val_if_fail ((export_backward && export_forward) == FALSE,
                        PICMAN_PDB_CALLING_ERROR);
  g_return_val_if_fail (error == NULL || *error == NULL,
                        PICMAN_PDB_CALLING_ERROR);

  drawable = picman_image_get_active_drawable (image);

  if (! drawable)
    return PICMAN_PDB_EXECUTION_ERROR;

  filename = file_utils_filename_from_uri (uri);

  if (filename)
    {
      /* check if we are saving to a file */
      if (g_file_test (filename, G_FILE_TEST_EXISTS))
        {
          if (! g_file_test (filename, G_FILE_TEST_IS_REGULAR))
            {
              g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
				   _("Not a regular file"));
              status = PICMAN_PDB_EXECUTION_ERROR;
              goto out;
            }

          if (g_access (filename, W_OK) != 0)
            {
              g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_ACCES,
                                   g_strerror (errno));
              status = PICMAN_PDB_EXECUTION_ERROR;
              goto out;
            }
        }

      if (file_proc->handles_uri)
        {
          g_free (filename);
          filename = g_strdup (uri);
        }
    }
  else
    {
      filename = g_strdup (uri);
    }

  /* ref the image, so it can't get deleted during save */
  g_object_ref (image);

  image_ID    = picman_image_get_ID (image);
  drawable_ID = picman_item_get_ID (PICMAN_ITEM (drawable));

  return_vals =
    picman_pdb_execute_procedure_by_name (image->picman->pdb,
                                        picman_get_user_context (picman),
                                        progress, error,
                                        picman_object_get_name (file_proc),
                                        PICMAN_TYPE_INT32,       run_mode,
                                        PICMAN_TYPE_IMAGE_ID,    image_ID,
                                        PICMAN_TYPE_DRAWABLE_ID, drawable_ID,
                                        G_TYPE_STRING,         filename,
                                        G_TYPE_STRING,         uri,
                                        G_TYPE_NONE);

  status = g_value_get_enum (picman_value_array_index (return_vals, 0));

  picman_value_array_unref (return_vals);

  if (status == PICMAN_PDB_SUCCESS)
    {
      PicmanDocumentList *documents;
      PicmanImagefile    *imagefile;

      if (change_saved_state)
        {
          picman_image_set_uri (image, uri);
          picman_image_set_save_proc (image, file_proc);

          /* Forget the import source when we save. We interpret a
           * save as that the user is not interested in being able
           * to quickly export back to the original any longer
           */
          picman_image_set_imported_uri (image, NULL);

          picman_image_clean_all (image);
        }
      else if (export_backward)
        {
          /* We exported the image back to its imported source,
           * change nothing about export/import flags, only set
           * the export state to clean
           */
          picman_image_export_clean_all (image);

          picman_object_name_changed (PICMAN_OBJECT (image));
        }
      else if (export_forward)
        {
          /* Remember the last entered Export URI for the image. We
           * only need to do this explicitly when exporting. It
           * happens implicitly when saving since the PicmanObject name
           * of a PicmanImage is the last-save URI
           */
          picman_image_set_exported_uri (image, uri);

          /* An image can not be considered both exported and imported
           * at the same time, so stop consider it as imported now
           * that we consider it exported.
           */
          picman_image_set_imported_uri (image, NULL);

          picman_image_export_clean_all (image);
        }

      if (export_backward || export_forward)
        picman_image_exported (image, uri);
      else
        picman_image_saved (image, uri);

      documents = PICMAN_DOCUMENT_LIST (image->picman->documents);
      imagefile = picman_document_list_add_uri (documents,
                                              uri,
                                              file_proc->mime_type);

      /* only save a thumbnail if we are saving as XCF, see bug #25272 */
      if (PICMAN_PROCEDURE (file_proc)->proc_type == PICMAN_INTERNAL)
        picman_imagefile_save_thumbnail (imagefile, file_proc->mime_type, image);
    }
  else if (status != PICMAN_PDB_CANCEL)
    {
      if (error && *error == NULL)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("%s plug-in could not save image"),
                       picman_plug_in_procedure_get_label (file_proc));
        }
    }

  picman_image_flush (image);

  g_object_unref (image);

 out:
  g_free (filename);

  return status;
}
