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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <gegl.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"

#include "core/core-types.h"

#include "core/picman.h"
#include "core/picmanimage.h"
#include "core/picmanparamspecs.h"
#include "core/picmanprogress.h"

#include "plug-in/picmanpluginmanager.h"
#include "plug-in/picmanpluginprocedure.h"

#include "xcf.h"
#include "xcf-private.h"
#include "xcf-load.h"
#include "xcf-read.h"
#include "xcf-save.h"

#include "picman-intl.h"


typedef PicmanImage * PicmanXcfLoaderFunc (Picman     *picman,
                                       XcfInfo  *info,
                                       GError  **error);


static PicmanValueArray * xcf_load_invoker (PicmanProcedure         *procedure,
                                          Picman                  *picman,
                                          PicmanContext           *context,
                                          PicmanProgress          *progress,
                                          const PicmanValueArray  *args,
                                          GError               **error);
static PicmanValueArray * xcf_save_invoker (PicmanProcedure         *procedure,
                                          Picman                  *picman,
                                          PicmanContext           *context,
                                          PicmanProgress          *progress,
                                          const PicmanValueArray  *args,
                                          GError               **error);


static PicmanXcfLoaderFunc * const xcf_loaders[] =
{
  xcf_load_image,   /* version 0 */
  xcf_load_image,   /* version 1 */
  xcf_load_image,   /* version 2 */
  xcf_load_image,   /* version 3 */
  xcf_load_image    /* version 4 */
};


void
xcf_init (Picman *picman)
{
  PicmanPlugInProcedure *proc;
  PicmanProcedure       *procedure;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  /* So this is sort of a hack, but its better than it was before.  To
   * do this right there would be a file load-save handler type and
   * the whole interface would change but there isn't, and currently
   * the plug-in structure contains all the load-save info, so it
   * makes sense to use that for the XCF load/save handlers, even
   * though they are internal.  The only thing it requires is using a
   * PlugInProcDef struct.  -josh
   */

  /*  picman-xcf-save  */
  procedure = picman_plug_in_procedure_new (PICMAN_PLUGIN, "picman-xcf-save");
  procedure->proc_type    = PICMAN_INTERNAL;
  procedure->marshal_func = xcf_save_invoker;

  proc = PICMAN_PLUG_IN_PROCEDURE (procedure);
  proc->menu_label = g_strdup (N_("PICMAN XCF image"));
  picman_plug_in_procedure_set_icon (proc, PICMAN_ICON_TYPE_STOCK_ID,
                                   (const guint8 *) "picman-wilber",
                                   strlen ("picman-wilber") + 1);
  picman_plug_in_procedure_set_image_types (proc, "RGB*, GRAY*, INDEXED*");
  picman_plug_in_procedure_set_file_proc (proc, "xcf", "", NULL);
  picman_plug_in_procedure_set_mime_type (proc, "image/xcf");

  picman_object_set_static_name (PICMAN_OBJECT (procedure), "picman-xcf-save");
  picman_procedure_set_static_strings (procedure,
                                     "picman-xcf-save",
                                     "Saves file in the .xcf file format",
                                     "The XCF file format has been designed "
                                     "specifically for loading and saving "
                                     "tiled and layered images in PICMAN. "
                                     "This procedure will save the specified "
                                     "image in the xcf file format.",
                                     "Spencer Kimball & Peter Mattis",
                                     "Spencer Kimball & Peter Mattis",
                                     "1995-1996",
                                     NULL);

  picman_procedure_add_argument (procedure,
                               picman_param_spec_int32 ("dummy-param",
                                                      "Dummy Param",
                                                      "Dummy parameter",
                                                      G_MININT32, G_MAXINT32, 0,
                                                      PICMAN_PARAM_READWRITE));
  picman_procedure_add_argument (procedure,
                               picman_param_spec_image_id ("image",
                                                         "Image",
                                                         "Input image",
                                                         picman, FALSE,
                                                         PICMAN_PARAM_READWRITE));
  picman_procedure_add_argument (procedure,
                               picman_param_spec_drawable_id ("drawable",
                                                            "Drawable",
                                                            "Active drawable of input image",
                                                            picman, TRUE,
                                                            PICMAN_PARAM_READWRITE));
  picman_procedure_add_argument (procedure,
                               picman_param_spec_string ("filename",
                                                       "Filename",
                                                       "The name of the file "
                                                       "to save the image in, "
                                                       "in the on-disk "
                                                       "character set and "
                                                       "encoding",
                                                       TRUE, FALSE, TRUE,
                                                       NULL,
                                                       PICMAN_PARAM_READWRITE));
  picman_procedure_add_argument (procedure,
                               picman_param_spec_string ("raw-filename",
                                                       "Raw filename",
                                                       "The basename of the "
                                                       "file, in UTF-8",
                                                       FALSE, FALSE, TRUE,
                                                       NULL,
                                                       PICMAN_PARAM_READWRITE));
  picman_plug_in_manager_add_procedure (picman->plug_in_manager, proc);
  g_object_unref (procedure);

  /*  picman-xcf-load  */
  procedure = picman_plug_in_procedure_new (PICMAN_PLUGIN, "picman-xcf-load");
  procedure->proc_type    = PICMAN_INTERNAL;
  procedure->marshal_func = xcf_load_invoker;

  proc = PICMAN_PLUG_IN_PROCEDURE (procedure);
  proc->menu_label = g_strdup (N_("PICMAN XCF image"));
  picman_plug_in_procedure_set_icon (proc, PICMAN_ICON_TYPE_STOCK_ID,
                                   (const guint8 *) "picman-wilber",
                                   strlen ("picman-wilber") + 1);
  picman_plug_in_procedure_set_image_types (proc, NULL);
  picman_plug_in_procedure_set_file_proc (proc, "xcf", "",
                                        "0,string,picman\\040xcf\\040");
  picman_plug_in_procedure_set_mime_type (proc, "image/xcf");

  picman_object_set_static_name (PICMAN_OBJECT (procedure), "picman-xcf-load");
  picman_procedure_set_static_strings (procedure,
                                     "picman-xcf-load",
                                     "Loads file saved in the .xcf file format",
                                     "The XCF file format has been designed "
                                     "specifically for loading and saving "
                                     "tiled and layered images in PICMAN. "
                                     "This procedure will load the specified "
                                     "file.",
                                     "Spencer Kimball & Peter Mattis",
                                     "Spencer Kimball & Peter Mattis",
                                     "1995-1996",
                                     NULL);

  picman_procedure_add_argument (procedure,
                               picman_param_spec_int32 ("dummy-param",
                                                      "Dummy Param",
                                                      "Dummy parameter",
                                                      G_MININT32, G_MAXINT32, 0,
                                                      PICMAN_PARAM_READWRITE));
  picman_procedure_add_argument (procedure,
                               picman_param_spec_string ("filename",
                                                       "Filename",
                                                       "The name of the file "
                                                       "to load, in the "
                                                       "on-disk character "
                                                       "set and encoding",
                                                       TRUE, FALSE, TRUE,
                                                       NULL,
                                                       PICMAN_PARAM_READWRITE));
  picman_procedure_add_argument (procedure,
                               picman_param_spec_string ("raw-filename",
                                                       "Raw filename",
                                                       "The basename of the "
                                                       "file, in UTF-8",
                                                       FALSE, FALSE, TRUE,
                                                       NULL,
                                                       PICMAN_PARAM_READWRITE));

  picman_procedure_add_return_value (procedure,
                                   picman_param_spec_image_id ("image",
                                                             "Image",
                                                             "Output image",
                                                             picman, FALSE,
                                                             PICMAN_PARAM_READWRITE));
  picman_plug_in_manager_add_procedure (picman->plug_in_manager, proc);
  g_object_unref (procedure);
}

void
xcf_exit (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
}

static PicmanValueArray *
xcf_load_invoker (PicmanProcedure         *procedure,
                  Picman                  *picman,
                  PicmanContext           *context,
                  PicmanProgress          *progress,
                  const PicmanValueArray  *args,
                  GError               **error)
{
  XcfInfo         info;
  PicmanValueArray *return_vals;
  PicmanImage      *image   = NULL;
  const gchar    *filename;
  gboolean        success = FALSE;
  gchar           id[14];

  picman_set_busy (picman);

  filename = g_value_get_string (picman_value_array_index (args, 1));

  info.fp = g_fopen (filename, "rb");

  if (info.fp)
    {
      info.picman                  = picman;
      info.progress              = progress;
      info.cp                    = 0;
      info.filename              = filename;
      info.tattoo_state          = 0;
      info.active_layer          = NULL;
      info.active_channel        = NULL;
      info.floating_sel_drawable = NULL;
      info.floating_sel          = NULL;
      info.floating_sel_offset   = 0;
      info.swap_num              = 0;
      info.ref_count             = NULL;
      info.compression           = COMPRESS_NONE;

      if (progress)
        {
          gchar *name = g_filename_display_name (filename);
          gchar *msg  = g_strdup_printf (_("Opening '%s'"), name);

          picman_progress_start (progress, msg, FALSE);

          g_free (msg);
          g_free (name);
        }

      success = TRUE;

      info.cp += xcf_read_int8 (info.fp, (guint8 *) id, 14);

      if (! g_str_has_prefix (id, "picman xcf "))
        {
          success = FALSE;
        }
      else if (strcmp (id + 9, "file") == 0)
        {
          info.file_version = 0;
        }
      else if (id[9] == 'v')
        {
          info.file_version = atoi (id + 10);
        }
      else
        {
          success = FALSE;
        }

      if (success)
        {
          if (info.file_version >= 0 &&
              info.file_version < G_N_ELEMENTS (xcf_loaders))
            {
              image = (*(xcf_loaders[info.file_version])) (picman, &info, error);

              if (! image)
                success = FALSE;
            }
          else
            {
              g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                           _("XCF error: unsupported XCF file version %d "
                             "encountered"), info.file_version);
              success = FALSE;
            }
        }

      fclose (info.fp);

      if (progress)
        picman_progress_end (progress);
    }
  else
    {
      int save_errno = errno;

      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (save_errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (save_errno));
    }

  return_vals = picman_procedure_get_return_values (procedure, success,
                                                  error ? *error : NULL);

  if (success)
    picman_value_set_image (picman_value_array_index (return_vals, 1), image);

  picman_unset_busy (picman);

  return return_vals;
}

static PicmanValueArray *
xcf_save_invoker (PicmanProcedure         *procedure,
                  Picman                  *picman,
                  PicmanContext           *context,
                  PicmanProgress          *progress,
                  const PicmanValueArray  *args,
                  GError               **error)
{
  XcfInfo         info;
  PicmanValueArray *return_vals;
  PicmanImage      *image;
  const gchar    *filename;
  gboolean        success = FALSE;

  picman_set_busy (picman);

  image    = picman_value_get_image (picman_value_array_index (args, 1), picman);
  filename = g_value_get_string (picman_value_array_index (args, 3));

  info.fp = g_fopen (filename, "wb");

  if (info.fp)
    {
      info.picman                  = picman;
      info.progress              = progress;
      info.cp                    = 0;
      info.filename              = filename;
      info.active_layer          = NULL;
      info.active_channel        = NULL;
      info.floating_sel_drawable = NULL;
      info.floating_sel          = NULL;
      info.floating_sel_offset   = 0;
      info.swap_num              = 0;
      info.ref_count             = NULL;
      info.compression           = COMPRESS_RLE;

      if (progress)
        {
          gchar *name = g_filename_display_name (filename);
          gchar *msg  = g_strdup_printf (_("Saving '%s'"), name);

          picman_progress_start (progress, msg, FALSE);

          g_free (msg);
          g_free (name);
        }

      xcf_save_choose_format (&info, image);

      success = xcf_save_image (&info, image, error);

      if (success)
        {
          if (fclose (info.fp) == EOF)
            {
              int save_errno = errno;

              g_set_error (error, G_FILE_ERROR,
                           g_file_error_from_errno (save_errno),
                            _("Error saving XCF file: %s"),
                           g_strerror (save_errno));

              success = FALSE;
            }
        }
      else
        {
          fclose (info.fp);
        }

      if (progress)
        picman_progress_end (progress);
    }
  else
    {
      int save_errno = errno;

      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (save_errno),
                   _("Could not open '%s' for writing: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (save_errno));
    }

  return_vals = picman_procedure_get_return_values (procedure, success,
                                                  error ? *error : NULL);

  picman_unset_busy (picman);

  return return_vals;
}
