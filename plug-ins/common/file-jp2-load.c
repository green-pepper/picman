/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * file-jp2.c -- JPEG 2000 file format plug-in
 * Copyright (C) 2009 Aurimas Juška <aurimas.juska@gmail.com>
 * Copyright (C) 2004 Florian Traverse <florian.traverse@cpe.fr>
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>

#include <glib/gstdio.h>

#ifdef G_OS_WIN32
#include <io.h>
#endif

#ifndef _O_BINARY
#define _O_BINARY 0
#endif

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"

#include <jasper/jasper.h>


#define LOAD_PROC "file-jp2-load"


static void     query             (void);
static void     run               (const gchar      *name,
                                   gint              nparams,
                                   const PicmanParam  *param,
                                   gint             *nreturn_vals,
                                   PicmanParam       **return_vals);
static gint32   load_image        (const gchar      *filename,
                                   GError          **error);
static void     load_icc_profile  (jas_image_t      *jas_image,
                                   gint              image_ID);


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

  picman_install_procedure (LOAD_PROC,
                          "Loads JPEG 2000 images.",
                          "The JPEG 2000 image loader.",
                          "Aurimas Juška",
                          "Aurimas Juška, Florian Traverse",
                          "2009",
                          N_("JPEG 2000 image"),
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (load_args),
                          G_N_ELEMENTS (load_return_vals),
                          load_args, load_return_vals);

  picman_register_magic_load_handler (LOAD_PROC,
                                    "jp2,jpc,jpx,j2k,jpf",
                                    "",
                                    "4,string,jP,0,string,\xff\x4f\xff\x51\x00");

  picman_register_file_handler_mime (LOAD_PROC, "image/jp2");
  picman_register_file_handler_mime (LOAD_PROC, "image/jpeg2000");
  picman_register_file_handler_mime (LOAD_PROC, "image/jpx");
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
  gint               image_ID;
  GError            *error = NULL;

  INIT_I18N ();
  gegl_init (NULL, NULL);

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

  if (strcmp (name, LOAD_PROC) == 0)
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
    }
  else
    {
      status = PICMAN_PDB_CALLING_ERROR;
    }

  if (status != PICMAN_PDB_SUCCESS && error)
    {
      *nreturn_vals = 2;
      values[1].type           = PICMAN_PDB_STRING;
      values[1].data.d_string  = error->message;
    }

  values[0].data.d_status = status;
}

static gint32
load_image (const gchar  *filename,
            GError      **error)
{
  gint               fd;
  jas_stream_t      *stream;
  gint32             image_ID = -1;
  jas_image_t       *image;
  gint32             layer_ID;
  PicmanImageType      image_type;
  PicmanImageBaseType  base_type;
  gint               width;
  gint               height;
  gint               num_components;
  gint               colourspace_family;
  GeglBuffer        *buffer;
  gint               i, j, k;
  guchar            *pixels;
  jas_matrix_t      *matrix;
  gint               components[4];

  jas_init ();

  picman_progress_init_printf (_("Opening '%s'"),
                             picman_filename_to_utf8 (filename));

  fd = g_open (filename, O_RDONLY | _O_BINARY, 0);
  if (fd == -1)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return -1;
    }

  stream = jas_stream_fdopen (fd, "rb");
  if (! stream)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return -1;
    }

  image = jas_image_decode (stream, -1, 0);
  if (!image)
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Couldn't decode '%s'."),
                   picman_filename_to_utf8 (filename));
      return -1;
    }

  picman_progress_update (80);

  jas_stream_close (stream);
  close (fd);

  width  = jas_image_width  (image);
  height = jas_image_height (image);

  /* determine image type */
  colourspace_family = jas_clrspc_fam (jas_image_clrspc (image));
  switch (colourspace_family)
    {
    case JAS_CLRSPC_FAM_GRAY:
      base_type = PICMAN_GRAY;
      components[0] = jas_image_getcmptbytype (image, JAS_IMAGE_CT_GRAY_Y);
      if (components[0] == -1)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("The image '%s' is in grayscale but does not contain "
                         "any gray component."),
                       picman_filename_to_utf8 (filename));
          return -1;
        }

      components[1] = jas_image_getcmptbytype (image, JAS_IMAGE_CT_OPACITY);
      if (components[1] != -1)
        {
          num_components = 2;
          image_type = PICMAN_GRAYA_IMAGE;
        }
      else
        {
          num_components = 1;
          image_type = PICMAN_GRAY_IMAGE;
        }
      break;

    case JAS_CLRSPC_FAM_RGB:
      base_type = PICMAN_RGB;
      components[0] = jas_image_getcmptbytype (image, JAS_IMAGE_CT_RGB_R);
      components[1] = jas_image_getcmptbytype (image, JAS_IMAGE_CT_RGB_G);
      components[2] = jas_image_getcmptbytype (image, JAS_IMAGE_CT_RGB_B);
      if (components[0] == -1 || components[1] == -1 || components[2] == -1)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("The image '%s' is in RGB, but is missing some of the "
                         "components."),
                       picman_filename_to_utf8 (filename));
          return -1;
        }
      components[3] = jas_image_getcmptbytype (image, JAS_IMAGE_CT_OPACITY);

      /* ImageMagick seems to write out the 'matte' component type
	 (number 3) */
      if (components[3] == -1)
        components[3] = jas_image_getcmptbytype (image, 3);

      if (components[3] != -1)
        {
          num_components = 4;
          image_type = PICMAN_RGBA_IMAGE;
        }
      else
        {
          num_components = 3;
          image_type = PICMAN_RGB_IMAGE;
        }
      break;

    case JAS_CLRSPC_FAM_XYZ:
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("The image '%s' is in the CIEXYZ color space, but there is "
                     "no code in place to convert it to RGB."),
                   picman_filename_to_utf8 (filename));
      return -1;

    case JAS_CLRSPC_FAM_LAB:
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("The image '%s' is in the CIELAB color space, but there is "
                     "no code in place to convert it to RGB."),
                   picman_filename_to_utf8 (filename));
      return -1;

    case JAS_CLRSPC_FAM_YCBCR:
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("The image '%s' is in the YCbCr color space, but there is "
                     "no code in place to convert it to RGB."),
                   picman_filename_to_utf8 (filename));
      return -1;

    case JAS_CLRSPC_FAM_UNKNOWN:
    default:
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("The image '%s' is in an unknown color space."),
                   picman_filename_to_utf8 (filename));
      return -1;
    }

  /* check all components if their dimensions match image dimensions */
  for (i = 0; i < num_components; i++)
    {
      if (jas_image_cmpttlx (image, components[i]) != jas_image_tlx (image) ||
          jas_image_cmpttly (image, components[i]) != jas_image_tly (image) ||
          jas_image_cmptbrx (image, components[i]) != jas_image_brx (image) ||
          jas_image_cmptbry (image, components[i]) != jas_image_bry (image))
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("Image component %d of image '%s' does not have the "
                         "same size as the image. This is currently not "
                         "supported."),
                       i, picman_filename_to_utf8 (filename));
          return -1;
        }

      if (jas_image_cmpthstep (image, components[i]) != 1 ||
          jas_image_cmptvstep (image, components[i]) != 1)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("Image component %d of image '%s' does not have both "
                         "a hstep and vstep."),
                       i, picman_filename_to_utf8 (filename));
          return -1;
        }

      if (jas_image_cmptsgnd (image, components[i]))
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("Image component %d of image '%s' is signed. "
                         "This is currently not supported."),
                       i, picman_filename_to_utf8 (filename));
          return -1;
        }
    }

  image_ID = picman_image_new (width, height, base_type);
  picman_image_set_filename (image_ID, filename);

  layer_ID = picman_layer_new (image_ID,
                             _("Background"),
                             width, height,
                             image_type, 100, PICMAN_NORMAL_MODE);
  picman_image_insert_layer (image_ID, layer_ID, -1, 0);

  buffer = picman_drawable_get_buffer (layer_ID);

  pixels = malloc (width * num_components);
  matrix = jas_matrix_create (1, width);

  for (i = 0; i < height; i++)
    {
      for (j = 0; j < num_components; j++)
        {
          const int channel_prec = 8;

          jas_image_readcmpt (image, components[j], 0, i, width, 1, matrix);

          if (jas_image_cmptprec (image, components[j]) >= channel_prec)
            {
              int shift = MAX (jas_image_cmptprec (image, components[j]) - channel_prec, 0);

              for (k = 0; k < width; k++)
                {
                  pixels[k * num_components + j] = jas_matrix_get (matrix, 0, k) >> shift;
                }
            }
          else
            {
              int mul = 1 << (channel_prec - jas_image_cmptprec (image, components[j]));

              for (k = 0; k < width; k++)
                {
                  pixels[k * num_components + j] = jas_matrix_get (matrix, 0, k) * mul;
                }

            }
        }

      gegl_buffer_set (buffer, GEGL_RECTANGLE (0, i, width, 1), 0,
                       NULL, pixels, GEGL_AUTO_ROWSTRIDE);
    }

  load_icc_profile (image, image_ID);

  jas_matrix_destroy (matrix);
  free (pixels);
  jas_image_destroy (image);

  g_object_unref (buffer);

  jas_cleanup ();

  picman_progress_update (1.0);

  return image_ID;
}

static void
load_icc_profile (jas_image_t *jas_image,
                  gint         image_ID)
{
  jas_cmprof_t  *cm_prof;
  jas_iccprof_t *jas_icc;
  jas_stream_t  *stream;
  guint32        profile_size;
  guchar        *jas_iccile;
  PicmanParasite  *parasite;

  cm_prof = jas_image_cmprof (jas_image);
  if (!cm_prof)
    return;

  jas_icc = jas_iccprof_createfromcmprof (cm_prof);
  if (!jas_icc)
    return;

  stream = jas_stream_memopen (NULL, -1);
  if (!stream)
    return;

  jas_iccprof_save (jas_icc, stream);

  jas_stream_rewind (stream);
  profile_size = jas_stream_length (stream);

  jas_iccile = g_malloc (profile_size);
  jas_stream_read (stream, jas_iccile, profile_size);

  parasite = picman_parasite_new ("icc-profile",
                                PICMAN_PARASITE_PERSISTENT |
                                PICMAN_PARASITE_UNDOABLE,
                                profile_size, jas_iccile);
  picman_image_attach_parasite (image_ID, parasite);
  picman_parasite_free (parasite);

  g_free (jas_iccile);
  jas_stream_close (stream);
  jas_iccprof_destroy (jas_icc);
}
