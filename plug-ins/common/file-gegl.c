/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * file-gegl.c -- GEGL based file format plug-in
 * Copyright (C) 2012 Simon Budig <simon@picman.org>
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
#include <errno.h>

#include <glib/gstdio.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_BINARY "file-gegl"


typedef struct _FileFormat FileFormat;

struct _FileFormat
{
  const gchar *file_type;
  const gchar *mime_type;
  const gchar *extensions;
  const gchar *magic;

  const gchar *load_proc;
  const gchar *load_blurb;
  const gchar *load_help;

  const gchar *save_proc;
  const gchar *save_blurb;
  const gchar *save_help;
};


static void     query      (void);
static void     run        (const gchar      *name,
                            gint              nparams,
                            const PicmanParam  *param,
                            gint             *nreturn_vals,
                            PicmanParam       **return_vals);
static gint32   load_image (const gchar      *filename,
                            GError          **error);
static gboolean save_image (const gchar      *filename,
                            gint32            image_ID,
                            gint32            drawable_ID,
                            GError          **error);


static const FileFormat file_formats[] =
{
  {
    N_("EXR image"),
    "image/x-exr",
    "exr,hdr",
    "",

    "file-exr-load",
    "Loads EXR images using GEGL",
    "This procedure loads images in EXR format, using gegl:load",

    "file-exr-save",
    "Save EXR images using GEGL",
    "This procedure saves images in EXR format, using gegl:save"
  },

  {
    N_("NEF image"),
    "image/x-nikon-nef",
    "nef",
    "",

    "file-nef-load",
    "Loads NEF images using GEGL",
    "This procedure loads images in NEF format, using gegl:load",

    NULL, NULL, NULL /* no NEF saving */
  }
};


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc */
  NULL,  /* quit_proc */
  query, /* query proc */
  run,   /* run_proc */
};

MAIN ()

static void
query (void)
{
  static const PicmanParamDef load_args[] =
  {
    { PICMAN_PDB_INT32,  "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_STRING, "filename",     "The name of the file to load." },
    { PICMAN_PDB_STRING, "raw-filename", "The name entered" },
  };

  static const PicmanParamDef load_return_vals[] =
  {
    { PICMAN_PDB_IMAGE,  "image",        "Output image" }
  };

  static const PicmanParamDef save_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",        "Input image" },
    { PICMAN_PDB_DRAWABLE, "drawable",     "Drawable to save" },
    { PICMAN_PDB_STRING,   "filename",     "The name of the file to save the image in" },
    { PICMAN_PDB_STRING,   "raw-filename", "The name of the file to save the image in" }
  };

  gint i;

  for (i = 0; i < G_N_ELEMENTS (file_formats); i++)
    {
      const FileFormat *format = &file_formats[i];

      picman_install_procedure (format->load_proc,
                              format->load_blurb,
                              format->load_help,
                              "Simon Budig",
                              "Simon Budig",
                              "2012",
                              format->file_type,
                              NULL,
                              PICMAN_PLUGIN,
                              G_N_ELEMENTS (load_args),
                              G_N_ELEMENTS (load_return_vals),
                              load_args, load_return_vals);

      picman_register_file_handler_mime (format->load_proc,
                                       format->mime_type);
      picman_register_magic_load_handler (format->load_proc,
                                        format->extensions,
                                        "",
                                        format->magic);

      if (format->save_proc)
        {
          picman_install_procedure (format->save_proc,
                                  format->save_blurb,
                                  format->save_help,
                                  "Simon Budig",
                                  "Simon Budig",
                                  "2012",
                                  format->file_type,
                                  "*",
                                  PICMAN_PLUGIN,
                                  G_N_ELEMENTS (save_args), 0,
                                  save_args, NULL);

          picman_register_file_handler_mime (format->save_proc,
                                           format->mime_type);
          picman_register_save_handler (format->save_proc,
                                      format->extensions, "");
        }
    }
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[2];
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  PicmanRunMode        run_mode;
  gint               image_ID;
  gint               drawable_ID;
  GError            *error = NULL;
  gint               i;

  INIT_I18N ();
  gegl_init (NULL, NULL);

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

  for (i = 0; i < G_N_ELEMENTS (file_formats); i++)
    {
      const FileFormat *format = &file_formats[i];

      if (! strcmp (name, format->load_proc))
        {
          image_ID = load_image (param[1].data.d_string, &error);

          if (image_ID != -1)
            {
              *nreturn_vals = 2;
              values[1].type         = PICMAN_PDB_IMAGE;
              values[1].data.d_image = image_ID;
            }
          else
            {
              status = PICMAN_PDB_EXECUTION_ERROR;
            }

          break;
        }
      else if (! strcmp (name, format->save_proc))
        {
          PicmanExportReturn export = PICMAN_EXPORT_CANCEL;

          image_ID    = param[1].data.d_int32;
          drawable_ID = param[2].data.d_int32;

          /*  eventually export the image */
          switch (run_mode)
            {
            case PICMAN_RUN_INTERACTIVE:
            case PICMAN_RUN_WITH_LAST_VALS:
              picman_ui_init (PLUG_IN_BINARY, FALSE);
              export = picman_export_image (&image_ID, &drawable_ID, NULL,
                                          (PICMAN_EXPORT_CAN_HANDLE_RGB |
                                           PICMAN_EXPORT_CAN_HANDLE_GRAY |
                                           PICMAN_EXPORT_CAN_HANDLE_INDEXED |
                                           PICMAN_EXPORT_CAN_HANDLE_ALPHA));

              if (export == PICMAN_EXPORT_CANCEL)
                {
                  *nreturn_vals = 1;
                  values[0].data.d_status = PICMAN_PDB_CANCEL;
                  return;
                }
              break;

            default:
              break;
            }

          if (! save_image (param[3].data.d_string, image_ID, drawable_ID,
                            &error))
            {
              status = PICMAN_PDB_EXECUTION_ERROR;
            }

          if (export == PICMAN_EXPORT_EXPORT)
            picman_image_delete (image_ID);

          break;
        }
    }

  if (i == G_N_ELEMENTS (file_formats))
    status = PICMAN_PDB_CALLING_ERROR;

  if (status != PICMAN_PDB_SUCCESS && error)
    {
      *nreturn_vals = 2;
      values[1].type           = PICMAN_PDB_STRING;
      values[1].data.d_string  = error->message;
    }

  values[0].data.d_status = status;

  gegl_exit ();
}

static gint32
load_image (const gchar  *filename,
            GError      **error)
{
  gint32             image_ID = -1;
  gint32             layer_ID;
  PicmanImageType      image_type;
  PicmanImageBaseType  base_type;
  PicmanPrecision      precision;
  gint               width;
  gint               height;
  GeglNode          *graph;
  GeglNode          *sink;
  GeglNode          *source;
  GeglBuffer        *src_buf  = NULL;
  GeglBuffer        *dest_buf = NULL;
  const Babl        *format;
  const Babl        *model;
  const Babl        *type;

  picman_progress_init_printf (_("Opening '%s'"),
                             picman_filename_to_utf8 (filename));

  graph = gegl_node_new ();

  source = gegl_node_new_child (graph,
                                "operation", "gegl:load",
                                "path",      filename,
                                NULL);
  sink = gegl_node_new_child (graph,
                              "operation", "gegl:buffer-sink",
                              "buffer",    &src_buf,
                              NULL);

  gegl_node_connect_to (source, "output",
                        sink,   "input");

  gegl_node_process (sink);

  g_object_unref (graph);

  if (! src_buf)
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Could not open '%s'"),
                   picman_filename_to_utf8 (filename));
      return -1;
    }

  picman_progress_update (0.33);

  width  = gegl_buffer_get_width (src_buf);
  height = gegl_buffer_get_height (src_buf);
  format = gegl_buffer_get_format (src_buf);

  model = babl_format_get_model (format);

  if (model == babl_model ("Y")  ||
      model == babl_model ("Y'") ||
      model == babl_model ("YA") ||
      model == babl_model ("Y'A"))
    {
      base_type = PICMAN_GRAY;

      if (babl_format_has_alpha (format))
        image_type = PICMAN_GRAYA_IMAGE;
      else
        image_type = PICMAN_GRAY_IMAGE;
    }
  else if (babl_format_is_palette (format))
    {
      base_type = PICMAN_INDEXED;

      if (babl_format_has_alpha (format))
        image_type = PICMAN_INDEXEDA_IMAGE;
      else
        image_type = PICMAN_INDEXED_IMAGE;
    }
  else
    {
      base_type = PICMAN_RGB;

      if (babl_format_has_alpha (format))
        image_type = PICMAN_RGBA_IMAGE;
      else
        image_type = PICMAN_RGB_IMAGE;
    }

  type = babl_format_get_type (format, 0);

  if (type == babl_type ("u8"))
    precision = PICMAN_PRECISION_U8;
  else if (type == babl_type ("u16"))
    precision = PICMAN_PRECISION_U16;
  else if (type == babl_type ("u32"))
    precision = PICMAN_PRECISION_U32;
  else if (type == babl_type ("half"))
    precision = PICMAN_PRECISION_HALF;
  else
    precision = PICMAN_PRECISION_FLOAT;

  image_ID = picman_image_new_with_precision (width, height,
                                            base_type, precision);
  picman_image_set_filename (image_ID, filename);

  layer_ID = picman_layer_new (image_ID,
                             _("Background"),
                             width, height,
                             image_type, 100, PICMAN_NORMAL_MODE);
  picman_image_insert_layer (image_ID, layer_ID, -1, 0);
  dest_buf = picman_drawable_get_buffer (layer_ID);

  picman_progress_update (0.66);

  gegl_buffer_copy (src_buf, NULL, dest_buf, NULL);

  g_object_unref (src_buf);
  g_object_unref (dest_buf);

  picman_progress_update (1.0);

  return image_ID;
}

static gboolean
save_image (const gchar  *filename,
            gint32        image_ID,
            gint32        drawable_ID,
            GError      **error)
{
  GeglNode   *graph;
  GeglNode   *source;
  GeglNode   *sink;
  GeglBuffer *src_buf;

  src_buf = picman_drawable_get_buffer (drawable_ID);

  graph = gegl_node_new ();

  source = gegl_node_new_child (graph,
                                "operation", "gegl:buffer-source",
                                "buffer",    src_buf,
                                NULL);
  sink = gegl_node_new_child (graph,
                              "operation", "gegl:save",
                              "path",      filename,
                              NULL);

  gegl_node_connect_to (source, "output",
                        sink,   "input");

  gegl_node_process (sink);

  g_object_unref (graph);
  g_object_unref (src_buf);

  return TRUE;
}
