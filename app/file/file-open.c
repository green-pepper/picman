/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995, 1996, 1997 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997 Josh MacDonald
 *
 * file-open.c
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
#define R_OK 4
#endif

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core/core-types.h"

#include "config/picmancoreconfig.h"

#include "gegl/picman-babl.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmandocumentlist.h"
#include "core/picmanimage.h"
#include "core/picmanimage-merge.h"
#include "core/picmanimage-undo.h"
#include "core/picmanimagefile.h"
#include "core/picmanlayer.h"
#include "core/picmanparamspecs.h"
#include "core/picmanprogress.h"

#include "pdb/picmanpdb.h"

#include "plug-in/picmanpluginmanager.h"
#include "plug-in/picmanpluginprocedure.h"
#include "plug-in/picmanpluginerror.h"
#include "plug-in/plug-in-icc-profile.h"

#include "file-open.h"
#include "file-procedure.h"
#include "file-utils.h"
#include "picman-file.h"

#include "picman-intl.h"


static void     file_open_sanitize_image       (PicmanImage                 *image,
                                                gboolean                   as_new);
static void     file_open_convert_items        (PicmanImage                 *dest_image,
                                                const gchar               *basename,
                                                GList                     *items);
static void     file_open_handle_color_profile (PicmanImage                 *image,
                                                PicmanContext               *context,
                                                PicmanProgress              *progress,
                                                PicmanRunMode                run_mode);
static GList *  file_open_get_layers           (const PicmanImage           *image,
                                                gboolean                   merge_visible,
                                                gint                      *n_visible);
static gboolean file_open_file_proc_is_import  (const PicmanPlugInProcedure *file_proc);


/*  public functions  */

PicmanImage *
file_open_image (Picman                *picman,
                 PicmanContext         *context,
                 PicmanProgress        *progress,
                 const gchar         *uri,
                 const gchar         *entered_filename,
                 gboolean             as_new,
                 PicmanPlugInProcedure *file_proc,
                 PicmanRunMode          run_mode,
                 PicmanPDBStatusType   *status,
                 const gchar        **mime_type,
                 GError             **error)
{
  PicmanValueArray *return_vals;
  gchar          *filename;
  PicmanImage      *image = NULL;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (status != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  *status = PICMAN_PDB_EXECUTION_ERROR;

  if (! file_proc)
    file_proc = file_procedure_find (picman->plug_in_manager->load_procs, uri,
                                     error);

  if (! file_proc)
    return NULL;

  filename = file_utils_filename_from_uri (uri);

  if (filename)
    {
      /* check if we are opening a file */
      if (g_file_test (filename, G_FILE_TEST_EXISTS))
        {
          if (! g_file_test (filename, G_FILE_TEST_IS_REGULAR))
            {
              g_free (filename);
              g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
				   _("Not a regular file"));
              return NULL;
            }

          if (g_access (filename, R_OK) != 0)
            {
              g_free (filename);
              g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_ACCES,
				   g_strerror (errno));
              return NULL;
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

  return_vals =
    picman_pdb_execute_procedure_by_name (picman->pdb,
                                        context, progress, error,
                                        picman_object_get_name (file_proc),
                                        PICMAN_TYPE_INT32, run_mode,
                                        G_TYPE_STRING,   filename,
                                        G_TYPE_STRING,   entered_filename,
                                        G_TYPE_NONE);

  g_free (filename);

  *status = g_value_get_enum (picman_value_array_index (return_vals, 0));

  if (*status == PICMAN_PDB_SUCCESS)
    {
      image = picman_value_get_image (picman_value_array_index (return_vals, 1),
                                    picman);

      if (image)
        {
          file_open_sanitize_image (image, as_new);

          /* Only set the load procedure if it hasn't already been set. */
          if (! picman_image_get_load_proc (image))
            picman_image_set_load_proc (image, file_proc);

          file_proc = picman_image_get_load_proc (image);

          if (mime_type)
            *mime_type = file_proc->mime_type;
        }
      else
        {
          if (error && ! *error)
            g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                         _("%s plug-in returned SUCCESS but did not "
                           "return an image"),
                         picman_plug_in_procedure_get_label (file_proc));

          *status = PICMAN_PDB_EXECUTION_ERROR;
        }
    }
  else if (*status != PICMAN_PDB_CANCEL)
    {
      if (error && ! *error)
        g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                     _("%s plug-In could not open image"),
                     picman_plug_in_procedure_get_label (file_proc));
    }

  picman_value_array_unref (return_vals);

  if (image)
    {
      file_open_handle_color_profile (image, context, progress, run_mode);

      if (file_open_file_proc_is_import (file_proc))
        {
          /* Remember the import source */
          picman_image_set_imported_uri (image, uri);

          /* We shall treat this file as an Untitled file */
          picman_image_set_uri (image, NULL);
        }
    }

  return image;
}

/**
 * file_open_thumbnail:
 * @picman:
 * @context:
 * @progress:
 * @uri:          the URI of the image file
 * @size:         requested size of the thumbnail
 * @mime_type:    return location for image MIME type
 * @image_width:  return location for image width
 * @image_height: return location for image height
 * @format:       return location for image format (set to NULL if unknown)
 * @num_layers:   return location for number of layers
 *                (set to -1 if the number of layers is not known)
 * @error:
 *
 * Attempts to load a thumbnail by using a registered thumbnail loader.
 *
 * Return value: the thumbnail image
 */
PicmanImage *
file_open_thumbnail (Picman           *picman,
                     PicmanContext    *context,
                     PicmanProgress   *progress,
                     const gchar    *uri,
                     gint            size,
                     const gchar   **mime_type,
                     gint           *image_width,
                     gint           *image_height,
                     const Babl    **format,
                     gint           *num_layers,
                     GError        **error)
{
  PicmanPlugInProcedure *file_proc;
  PicmanProcedure       *procedure;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (mime_type != NULL, NULL);
  g_return_val_if_fail (image_width != NULL, NULL);
  g_return_val_if_fail (image_height != NULL, NULL);
  g_return_val_if_fail (format != NULL, NULL);
  g_return_val_if_fail (num_layers != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  *image_width  = 0;
  *image_height = 0;
  *format       = NULL;
  *num_layers   = -1;

  file_proc = file_procedure_find (picman->plug_in_manager->load_procs, uri,
                                   NULL);

  if (! file_proc || ! file_proc->thumb_loader)
    return NULL;

  procedure = picman_pdb_lookup_procedure (picman->pdb, file_proc->thumb_loader);

  if (procedure && procedure->num_args >= 2 && procedure->num_values >= 1)
    {
      PicmanPDBStatusType  status;
      PicmanValueArray    *return_vals;
      gchar             *filename;
      PicmanImage         *image = NULL;

      filename = file_utils_filename_from_uri (uri);

      if (! filename)
        filename = g_strdup (uri);

      return_vals =
        picman_pdb_execute_procedure_by_name (picman->pdb,
                                            context, progress, error,
                                            picman_object_get_name (procedure),
                                            G_TYPE_STRING,   filename,
                                            PICMAN_TYPE_INT32, size,
                                            G_TYPE_NONE);

      g_free (filename);

      status = g_value_get_enum (picman_value_array_index (return_vals, 0));

      if (status == PICMAN_PDB_SUCCESS &&
          PICMAN_VALUE_HOLDS_IMAGE_ID (picman_value_array_index (return_vals, 1)))
        {
          image = picman_value_get_image (picman_value_array_index (return_vals, 1),
                                        picman);

          if (picman_value_array_length (return_vals) >= 3 &&
              G_VALUE_HOLDS_INT (picman_value_array_index (return_vals, 2)) &&
              G_VALUE_HOLDS_INT (picman_value_array_index (return_vals, 3)))
            {
              *image_width =
                MAX (0, g_value_get_int (picman_value_array_index (return_vals, 2)));

              *image_height =
                MAX (0, g_value_get_int (picman_value_array_index (return_vals, 3)));

              if (picman_value_array_length (return_vals) >= 5 &&
                  G_VALUE_HOLDS_INT (picman_value_array_index (return_vals, 4)))
                {
                  gint value = g_value_get_int (picman_value_array_index (return_vals, 4));

                  switch (value)
                    {
                    case PICMAN_RGB_IMAGE:
                      *format = picman_babl_format (PICMAN_RGB, PICMAN_PRECISION_U8,
                                                  FALSE);
                      break;

                    case PICMAN_RGBA_IMAGE:
                      *format = picman_babl_format (PICMAN_RGB, PICMAN_PRECISION_U8,
                                                  TRUE);
                      break;

                    case PICMAN_GRAY_IMAGE:
                      *format = picman_babl_format (PICMAN_GRAY, PICMAN_PRECISION_U8,
                                                  FALSE);
                      break;

                    case PICMAN_GRAYA_IMAGE:
                      *format = picman_babl_format (PICMAN_GRAY, PICMAN_PRECISION_U8,
                                                  TRUE);
                      break;

                    case PICMAN_INDEXED_IMAGE:
                    case PICMAN_INDEXEDA_IMAGE:
                      {
                        const Babl *rgb;
                        const Babl *rgba;

                        babl_new_palette ("-picman-indexed-format-dummy",
                                          &rgb, &rgba);

                        if (value == PICMAN_INDEXED_IMAGE)
                          *format = rgb;
                        else
                          *format = rgba;
                      }
                      break;

                    default:
                      break;
                    }
                }

              if (picman_value_array_length (return_vals) >= 6 &&
                  G_VALUE_HOLDS_INT (picman_value_array_index (return_vals, 5)))
                {
                  *num_layers =
                    MAX (0, g_value_get_int (picman_value_array_index (return_vals, 5)));
                }
            }

          if (image)
            {
              file_open_sanitize_image (image, FALSE);

              *mime_type = file_proc->mime_type;

#ifdef PICMAN_UNSTABLE
              g_printerr ("opened thumbnail at %d x %d\n",
                          picman_image_get_width  (image),
                          picman_image_get_height (image));
#endif
            }
        }

      picman_value_array_unref (return_vals);

      return image;
    }

  return NULL;
}

PicmanImage *
file_open_with_display (Picman               *picman,
                        PicmanContext        *context,
                        PicmanProgress       *progress,
                        const gchar        *uri,
                        gboolean            as_new,
                        PicmanPDBStatusType  *status,
                        GError            **error)
{
  return file_open_with_proc_and_display (picman, context, progress,
                                          uri, uri, as_new, NULL,
                                          status, error);
}

PicmanImage *
file_open_with_proc_and_display (Picman                *picman,
                                 PicmanContext         *context,
                                 PicmanProgress        *progress,
                                 const gchar         *uri,
                                 const gchar         *entered_filename,
                                 gboolean             as_new,
                                 PicmanPlugInProcedure *file_proc,
                                 PicmanPDBStatusType   *status,
                                 GError             **error)
{
  PicmanImage   *image;
  const gchar *mime_type = NULL;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (status != NULL, NULL);

  image = file_open_image (picman, context, progress,
                           uri,
                           entered_filename,
                           as_new,
                           file_proc,
                           PICMAN_RUN_INTERACTIVE,
                           status,
                           &mime_type,
                           error);

  if (image)
    {
      /* If the file was imported we want to set the layer name to the
       * file name. For now, assume that multi-layered imported images
       * have named the layers already, so only rename the layer of
       * single-layered imported files. Note that this will also
       * rename already named layers from e.g. single-layered PSD
       * files. To solve this properly, we would need new file plug-in
       * API.
       */
      if (! file_proc)
        file_proc = picman_image_get_load_proc (image);

      if (file_open_file_proc_is_import (file_proc) &&
          picman_image_get_n_layers (image) == 1)
        {
          PicmanObject *layer    = picman_image_get_layer_iter (image)->data;
          gchar      *basename = file_utils_uri_display_basename (uri);

          picman_item_rename (PICMAN_ITEM (layer), basename, NULL);
          picman_image_undo_free (image);
          picman_image_clean_all (image);

          g_free (basename);
        }

      if (picman_create_display (image->picman, image, PICMAN_UNIT_PIXEL, 1.0))
        {
          /*  the display owns the image now  */
          g_object_unref (image);
        }

      if (! as_new)
        {
          PicmanDocumentList *documents = PICMAN_DOCUMENT_LIST (picman->documents);
          PicmanImagefile    *imagefile;
          const gchar      *any_uri;

          imagefile = picman_document_list_add_uri (documents, uri, mime_type);

          /*  can only create a thumbnail if the passed uri and the
           *  resulting image's uri match. Use any_uri() here so we
           *  create thumbnails for both XCF and imported images.
           */
          any_uri = picman_image_get_any_uri (image);

          if (any_uri && ! strcmp (uri, any_uri))
            {
              /*  no need to save a thumbnail if there's a good one already  */
              if (! picman_imagefile_check_thumbnail (imagefile))
                {
                  picman_imagefile_save_thumbnail (imagefile, mime_type, image);
                }
            }
        }

      /*  announce that we opened this image  */
      picman_image_opened (image->picman, uri);
    }

  return image;
}

GList *
file_open_layers (Picman                *picman,
                  PicmanContext         *context,
                  PicmanProgress        *progress,
                  PicmanImage           *dest_image,
                  gboolean             merge_visible,
                  const gchar         *uri,
                  PicmanRunMode          run_mode,
                  PicmanPlugInProcedure *file_proc,
                  PicmanPDBStatusType   *status,
                  GError             **error)
{
  PicmanImage   *new_image;
  GList       *layers    = NULL;
  const gchar *mime_type = NULL;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (PICMAN_IS_IMAGE (dest_image), NULL);
  g_return_val_if_fail (uri != NULL, NULL);
  g_return_val_if_fail (status != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  new_image = file_open_image (picman, context, progress,
                               uri, uri, FALSE,
                               file_proc,
                               run_mode,
                               status, &mime_type, error);

  if (new_image)
    {
      gint n_visible = 0;

      picman_image_undo_disable (new_image);

      layers = file_open_get_layers (new_image, merge_visible, &n_visible);

      if (merge_visible && n_visible > 1)
        {
          PicmanLayer *layer;

          g_list_free (layers);

          layer = picman_image_merge_visible_layers (new_image, context,
                                                   PICMAN_CLIP_TO_IMAGE,
                                                   FALSE, FALSE);

          layers = g_list_prepend (NULL, layer);
        }

      if (layers)
        {
          gchar *basename = file_utils_uri_display_basename (uri);

          file_open_convert_items (dest_image, basename, layers);
          g_free (basename);

          picman_document_list_add_uri (PICMAN_DOCUMENT_LIST (picman->documents),
                                      uri, mime_type);
        }
      else
        {
          g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			       _("Image doesn't contain any layers"));
          *status = PICMAN_PDB_EXECUTION_ERROR;
        }

      g_object_unref (new_image);
    }

  return g_list_reverse (layers);
}


/*  This function is called for filenames passed on the command-line
 *  or from the D-Bus service.
 */
gboolean
file_open_from_command_line (Picman        *picman,
                             const gchar *filename,
                             gboolean     as_new)
{
  GError   *error   = NULL;
  gchar    *uri;
  gboolean  success = FALSE;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);

  /* we accept URI or filename */
  uri = file_utils_any_to_uri (picman, filename, &error);

  if (uri)
    {
      PicmanImage         *image;
      PicmanObject        *display = picman_get_empty_display (picman);
      PicmanPDBStatusType  status;

      image = file_open_with_display (picman,
                                      picman_get_user_context (picman),
                                      PICMAN_PROGRESS (display),
                                      uri, as_new,
                                      &status, &error);

      if (image)
        {
          success = TRUE;

          g_object_set_data_full (G_OBJECT (picman), PICMAN_FILE_OPEN_LAST_URI_KEY,
                                  uri, (GDestroyNotify) g_free);
        }
      else if (status != PICMAN_PDB_CANCEL)
        {
          gchar *filename = file_utils_uri_display_name (uri);

          picman_message (picman, G_OBJECT (display), PICMAN_MESSAGE_ERROR,
                        _("Opening '%s' failed: %s"),
                        filename, error->message);
          g_clear_error (&error);

          g_free (filename);
          g_free (uri);
        }
    }
  else
    {
      g_printerr ("conversion filename -> uri failed: %s\n",
                  error->message);
      g_clear_error (&error);
    }

  return success;
}


/*  private functions  */

static void
file_open_sanitize_image (PicmanImage *image,
                          gboolean   as_new)
{
  if (as_new)
    picman_image_set_uri (image, NULL);

  /* clear all undo steps */
  picman_image_undo_free (image);

  /* make sure that undo is enabled */
  while (! picman_image_undo_is_enabled (image))
    picman_image_undo_thaw (image);

  /* Set the image to clean. Note that export dirtiness is not set to
   * clean here; we can only consider export clean after the first
   * export
   */
  picman_image_clean_all (image);

#if 0
  /* XXX this is not needed any longer, remove it when sure */

  /* make sure the entire projection is properly constructed, because
   * load plug-ins are not required to call picman_drawable_update() or
   * anything.
   */
  picman_image_invalidate (image,
                         0, 0,
                         picman_image_get_width  (image),
                         picman_image_get_height (image));
  picman_image_flush (image);

  /* same for drawable previews */
  picman_image_invalidate_previews (image);
#endif
}

/* Converts items from one image to another */
static void
file_open_convert_items (PicmanImage   *dest_image,
                         const gchar *basename,
                         GList       *items)
{
  GList *list;

  for (list = items; list; list = g_list_next (list))
    {
      PicmanItem *src = list->data;
      PicmanItem *item;

      item = picman_item_convert (src, dest_image, G_TYPE_FROM_INSTANCE (src));

      if (g_list_length (items) == 1)
        {
          picman_object_set_name (PICMAN_OBJECT (item), basename);
        }
      else
        {
          picman_object_set_name (PICMAN_OBJECT (item),
                                picman_object_get_name (src));
        }

      list->data = item;
    }
}

static void
file_open_profile_apply_rgb (PicmanImage    *image,
                             PicmanContext  *context,
                             PicmanProgress *progress,
                             PicmanRunMode   run_mode)
{
  PicmanColorConfig *config = image->picman->config->color_management;
  GError          *error  = NULL;

  if (picman_image_get_base_type (image) == PICMAN_GRAY)
    return;

  if (config->mode == PICMAN_COLOR_MANAGEMENT_OFF)
    return;

  if (! plug_in_icc_profile_apply_rgb (image, context, progress, run_mode,
                                       &error))
    {
      if (error->domain == PICMAN_PLUG_IN_ERROR &&
          error->code   == PICMAN_PLUG_IN_NOT_FOUND)
        {
          gchar *msg = g_strdup_printf ("%s\n\n%s",
                                        error->message,
                                        _("Color management has been disabled. "
                                          "It can be enabled again in the "
                                          "Preferences dialog."));

          g_object_set (config, "mode", PICMAN_COLOR_MANAGEMENT_OFF, NULL);

          picman_message_literal (image->picman, G_OBJECT (progress),
				PICMAN_MESSAGE_WARNING, msg);
          g_free (msg);
        }
      else
        {
          picman_message_literal (image->picman, G_OBJECT (progress),
				PICMAN_MESSAGE_ERROR, error->message);
        }

      g_error_free (error);
    }
}

static void
file_open_handle_color_profile (PicmanImage    *image,
                                PicmanContext  *context,
                                PicmanProgress *progress,
                                PicmanRunMode   run_mode)
{
  if (picman_image_parasite_find (image, "icc-profile"))
    {
      picman_image_undo_disable (image);

      switch (image->picman->config->color_profile_policy)
        {
        case PICMAN_COLOR_PROFILE_POLICY_ASK:
          if (run_mode == PICMAN_RUN_INTERACTIVE)
            file_open_profile_apply_rgb (image, context, progress,
                                         PICMAN_RUN_INTERACTIVE);
          break;

        case PICMAN_COLOR_PROFILE_POLICY_KEEP:
          break;

        case PICMAN_COLOR_PROFILE_POLICY_CONVERT:
          file_open_profile_apply_rgb (image, context, progress,
                                       PICMAN_RUN_NONINTERACTIVE);
          break;
        }

      picman_image_clean_all (image);
      picman_image_undo_enable (image);
    }
}

static GList *
file_open_get_layers (const PicmanImage *image,
                      gboolean         merge_visible,
                      gint            *n_visible)
{
  GList *iter   = NULL;
  GList *layers = NULL;

  for (iter = picman_image_get_layer_iter (image);
       iter;
       iter = g_list_next (iter))
    {
      PicmanItem *item = iter->data;

      if (! merge_visible)
        layers = g_list_prepend (layers, item);

      if (picman_item_get_visible (item))
        {
          if (n_visible)
            (*n_visible)++;

          if (! layers)
            layers = g_list_prepend (layers, item);
        }
    }

  return layers;
}

static gboolean
file_open_file_proc_is_import (const PicmanPlugInProcedure *file_proc)
{
  return !(file_proc &&
           file_proc->mime_type &&
           strcmp (file_proc->mime_type, "image/xcf") == 0);
}
