/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Alias|Wavefront pix/matte image reading and writing code
 * Copyright (C) 1997 Mike Taylor
 * (email: mtaylor@aw.sgi.com, WWW: http://reality.sgi.com/mtaylor)
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
 *
 */

/* This plug-in was written using the online documentation from
 * Alias|Wavefront Inc's PowerAnimator product.
 *
 * Bug reports or suggestions should be e-mailed to mtaylor@aw.sgi.com
 */

/* Event history:
 * V 1.0, MT, 02-Jul-97: initial version of plug-in
 * V 1.1, MT, 04-Dec-97: added .als file extension
 */

/* Features
 *  - loads and saves
 *    - 24-bit (.pix)
 *    - 8-bit (.matte, .alpha, or .mask) images
 *
 * NOTE: pix and matte files do not support alpha channels or indexed
 *       colour, so neither does this plug-in
 */

#include "config.h"

#include <errno.h>
#include <string.h>

#include <glib/gstdio.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define LOAD_PROC      "file-pix-load"
#define SAVE_PROC      "file-pix-save"
#define PLUG_IN_BINARY "file-pix"
#define PLUG_IN_ROLE   "picman-file-pix"


/* #define PIX_DEBUG */

#ifdef PIX_DEBUG
#    define PIX_DEBUG_PRINT(a,b) g_printerr (a,b)
#else
#    define PIX_DEBUG_PRINT(a,b)
#endif

/**************
 * Prototypes *
 **************/

/* Standard Plug-in Functions */

static void     query     (void);
static void     run       (const gchar      *name,
                           gint              nparams,
                           const PicmanParam  *param,
                           gint             *nreturn_vals,
                           PicmanParam       **return_vals);

/* Local Helper Functions */

static gint32   load_image (const gchar     *filename,
                            GError         **error);
static gboolean save_image (const gchar     *filename,
                            gint32           image_ID,
                            gint32           drawable_ID,
                            GError         **error);

static guint16  get_short  (FILE            *file);
static void     put_short  (guint16          value,
                            FILE            *file);

/******************
 * Implementation *
 ******************/

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

MAIN ()

static void
query (void)
{
  /*
   * Description:
   *     Register the services provided by this plug-in
   */
  static const PicmanParamDef load_args[] =
  {
    { PICMAN_PDB_INT32,  "run-mode",      "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_STRING, "filename",      "The name of the file to load" },
    { PICMAN_PDB_STRING, "raw-filename",   "The name entered"            }
  };
  static const PicmanParamDef load_return_vals[] =
  {
    { PICMAN_PDB_IMAGE, "image", "Output image" }
  };

  static const PicmanParamDef save_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",        "Input image"                  },
    { PICMAN_PDB_DRAWABLE, "drawable",     "Drawable to save"             },
    { PICMAN_PDB_STRING,   "filename",     "The name of the file to save the image in" },
    { PICMAN_PDB_STRING,   "raw-filename", "The name of the file to save the image in" }
  };

  picman_install_procedure (LOAD_PROC,
                          "loads files of the Alias|Wavefront Pix file format",
                          "loads files of the Alias|Wavefront Pix file format",
                          "Michael Taylor",
                          "Michael Taylor",
                          "1997",
                          N_("Alias Pix image"),
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (load_args),
                          G_N_ELEMENTS (load_return_vals),
                          load_args, load_return_vals);

  picman_register_load_handler (LOAD_PROC, "pix,matte,mask,alpha,als", "");

  picman_install_procedure (SAVE_PROC,
                          "save file in the Alias|Wavefront pix/matte file format",
                          "save file in the Alias|Wavefront pix/matte file format",
                          "Michael Taylor",
                          "Michael Taylor",
                          "1997",
                          N_("Alias Pix image"),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (save_args), 0,
                          save_args, NULL);

  picman_register_save_handler (SAVE_PROC, "pix,matte,mask,alpha,als", "");
}

/*
 *  Description:
 *      perform registered plug-in function
 *
 *  Arguments:
 *      name         - name of the function to perform
 *      nparams      - number of parameters passed to the function
 *      param        - parameters passed to the function
 *      nreturn_vals - number of parameters returned by the function
 *      return_vals  - parameters returned by the function
 */

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam  values[2];
  PicmanRunMode       run_mode;
  PicmanPDBStatusType status = PICMAN_PDB_SUCCESS;
  gint32            image_ID;
  gint32            drawable_ID;
  PicmanExportReturn  export = PICMAN_EXPORT_CANCEL;
  GError           *error  = NULL;

  INIT_I18N ();
  gegl_init (NULL, NULL);

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

  if (strcmp (name, LOAD_PROC) == 0)
    {
      /* Perform the image load */
      image_ID = load_image (param[1].data.d_string, &error);

      if (image_ID != -1)
        {
          /* The image load was successful */
          *nreturn_vals = 2;
          values[1].type         = PICMAN_PDB_IMAGE;
          values[1].data.d_image = image_ID;
        }
      else
        {
          /* The image load falied */
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
    }
  else if (strcmp (name, SAVE_PROC) == 0)
    {
      image_ID    = param[1].data.d_int32;
      drawable_ID = param[2].data.d_int32;

      /*  eventually export the image */
      switch (run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
        case PICMAN_RUN_WITH_LAST_VALS:
          picman_ui_init (PLUG_IN_BINARY, FALSE);
          export = picman_export_image (&image_ID, &drawable_ID, NULL,
                                      PICMAN_EXPORT_CAN_HANDLE_RGB |
                                      PICMAN_EXPORT_CAN_HANDLE_GRAY);
          if (export == PICMAN_EXPORT_CANCEL)
            {
              values[0].data.d_status = PICMAN_PDB_CANCEL;
              return;
            }
          break;
        default:
          break;
        }

      if (status == PICMAN_PDB_SUCCESS)
        {
          if (! save_image (param[3].data.d_string, image_ID, drawable_ID,
                            &error))
            {
              status = PICMAN_PDB_EXECUTION_ERROR;
            }
        }

      if (export == PICMAN_EXPORT_EXPORT)
        picman_image_delete (image_ID);
    }
  else
    {
      status = PICMAN_PDB_CALLING_ERROR;
    }

  if (status != PICMAN_PDB_SUCCESS && error)
    {
      *nreturn_vals = 2;
      values[1].type          = PICMAN_PDB_STRING;
      values[1].data.d_string = error->message;
    }

  values[0].data.d_status = status;
}


/*
 * Description:
 *     Reads a 16-bit integer from a file in such a way that the machine's
 *     byte order should not matter.
 */

static guint16
get_short (FILE *file)
{
  guchar buf[2];

  fread (buf, 2, 1, file);

  return (buf[0] << 8) + buf[1];
}

/*
 * Description:
 *     Writes a 16-bit integer to a file in such a way that the machine's
 *     byte order should not matter.
 */

static void
put_short (guint16  value,
           FILE    *file)
{
  guchar buf[2];
  buf[0] = (value >> 8) & 0xFF;
  buf[1] = value & 0xFF;

  fwrite (buf, 2, 1, file);
}

/*
 *  Description:
 *      load the given image into picman
 *
 *  Arguments:
 *      filename      - name on the file to read
 *
 *  Return Value:
 *      Image id for the loaded image
 *
 */

static gint32
load_image (const gchar  *filename,
            GError      **error)
{
  GeglBuffer        *buffer;
  FILE              *file;
  PicmanImageBaseType  imgtype;
  PicmanImageType      gdtype;
  guchar            *dest;
  guchar            *dest_base;
  gint32             image_ID;
  gint32             layer_ID;
  gushort            width, height, depth;
  gint               i, j, tile_height, row;

  PIX_DEBUG_PRINT ("Opening file: %s\n", filename);

  file = g_fopen (filename, "rb");

  if (! file)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return -1;
    }

  picman_progress_init_printf (_("Opening '%s'"),
                             picman_filename_to_utf8 (filename));

  /* Read header information */
  width  = get_short (file);
  height = get_short (file);
  get_short (file); /* Discard obsolete fields */
  get_short (file); /* Discard obsolete fields */
  depth  = get_short (file);

  PIX_DEBUG_PRINT ("Width %hu\n", width);
  PIX_DEBUG_PRINT ("Height %hu\n", height);

  if (depth == 8)
    {
      /* Loading a matte file */
      imgtype = PICMAN_GRAY;
      gdtype  = PICMAN_GRAY_IMAGE;
    }
  else if (depth == 24)
    {
      /* Loading an RGB file */
      imgtype = PICMAN_RGB;
      gdtype  = PICMAN_RGB_IMAGE;
    }
  else
    {
      /* Header is invalid */
      fclose (file);
      return -1;
    }

  image_ID = picman_image_new (width, height, imgtype);
  picman_image_set_filename (image_ID, filename);
  layer_ID = picman_layer_new (image_ID, _("Background"),
                             width,
                             height,
                             gdtype, 100, PICMAN_NORMAL_MODE);
  picman_image_insert_layer (image_ID, layer_ID, -1, 0);

  buffer = picman_drawable_get_buffer (layer_ID);

  tile_height = picman_tile_height ();

  if (depth == 24)
    {
      /* Read a 24-bit Pix image */
      guchar record[4];
      gint   readlen;

      dest_base = dest = g_new (guchar, 3 * width * tile_height);

      for (i = 0; i < height;)
        {
          for (dest = dest_base, row = 0;
               row < tile_height && i < height;
               i++, row++)
            {
              guchar count;

              /* Read a row of the image */
              j = 0;
              while (j < width)
                {
                  readlen = fread (record, 1, 4, file);
                  if (readlen < 4)
                    break;

                  for (count = 0; count < record[0]; ++count)
                    {
                      dest[0]   = record[3];
                      dest[1]   = record[2];
                      dest[2]   = record[1];
                      dest += 3;
                      j++;
                      if (j >= width)
                        break;
                    }
                }
            }

          gegl_buffer_set (buffer, GEGL_RECTANGLE (0, i - row, width, row), 0,
                           NULL, dest_base, GEGL_AUTO_ROWSTRIDE);

          picman_progress_update ((double) i / (double) height);
        }

      g_free (dest_base);
    }
  else
    {
      /* Read an 8-bit Matte image */
      guchar record[2];
      gint   readlen;

      dest_base = dest = g_new (guchar, width * tile_height);

      for (i = 0; i < height;)
        {
          for (dest = dest_base, row = 0;
               row < tile_height && i < height;
               i++, row++)
            {
              guchar count;

              /* Read a row of the image */
              j = 0;
              while (j < width)
                {
                  readlen = fread(record, 1, 2, file);
                  if (readlen < 2)
                    break;

                  for (count = 0; count < record[0]; ++count)
                    {
                      dest[j]   = record[1];
                      j++;
                      if (j >= width)
                        break;
                    }
                }

              dest += width;
            }

          gegl_buffer_set (buffer, GEGL_RECTANGLE (0, i - row, width, row), 0,
                           NULL, dest_base, GEGL_AUTO_ROWSTRIDE);

          picman_progress_update ((double) i / (double) height);
        }

      g_free (dest_base);
    }

  g_object_unref (buffer);
  fclose (file);

  picman_progress_update (1.0);

  return image_ID;
}

/*
 *  Description:
 *      save the given file out as an alias pix or matte file
 *
 *  Arguments:
 *      filename    - name of file to save to
 *      image_ID    - ID of image to save
 *      drawable_ID - current drawable
 */

static gboolean
save_image (const gchar  *filename,
            gint32        image_ID,
            gint32        drawable_ID,
            GError      **error)
{
  GeglBuffer *buffer;
  const Babl *format;
  FILE       *file;
  gint        width;
  gint        height;
  gint        depth, i, j, row, tile_height, writelen, rectHeight;
  gboolean    savingColor = TRUE;
  guchar     *src;
  guchar     *src_base;

  /* Get info about image */
  buffer = picman_drawable_get_buffer (drawable_ID);

  width  = gegl_buffer_get_width  (buffer);
  height = gegl_buffer_get_height (buffer);

  savingColor = picman_drawable_is_rgb (drawable_ID);

  if (savingColor)
    format = babl_format ("R'G'B' u8");
  else
    format = babl_format ("Y' u8");

  depth = picman_drawable_bpp (drawable_ID);

  /* Open the output file. */
  file = g_fopen (filename, "wb");
  if (! file)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for writing: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return FALSE;
    }

  picman_progress_init_printf (_("Saving '%s'"),
                             picman_filename_to_utf8 (filename));

  /* Write the image header */
  PIX_DEBUG_PRINT ("Width %hu\n",  width);
  PIX_DEBUG_PRINT ("Height %hu\n", height);
  put_short (width, file);
  put_short (height, file);
  put_short (0, file);
  put_short (0, file);

  tile_height = picman_tile_height ();
  src_base    = g_new (guchar, tile_height * width * depth);

  if (savingColor)
    {
      /* Writing a 24-bit Pix image */
      guchar record[4];

      put_short (24, file);

      for (i = 0; i < height;)
        {
          rectHeight = (tile_height < (height - i - 1)) ?
                        tile_height : (height - i - 1);

          gegl_buffer_get (buffer, GEGL_RECTANGLE (0, i, width, rectHeight), 1.0,
                           format, src_base,
                           GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

          for (src = src_base, row = 0;
               row < tile_height && i < height;
               i += 1, row += 1)
            {
              /* Write a row of the image */
              record[0] = 1;
              record[3] = src[0];
              record[2] = src[1];
              record[1] = src[2];
              src += depth;
              for (j = 1; j < width; ++j)
                {
                  if ((record[3] != src[0]) ||
                       (record[2] != src[1]) ||
                       (record[1] != src[2]) ||
                       (record[0] == 255))
                    {
                      /* Write current RLE record and start a new one */

                      writelen = fwrite (record, 1, 4, file);
                      record[0] = 1;
                      record[3] = src[0];
                      record[2] = src[1];
                      record[1] = src[2];
                    }
                  else
                    {
                      /* increment run length in current record */
                      record[0]++;
                    }
                  src += depth;
                }

              /* Write last record in row */
              writelen = fwrite (record, 1, 4, file);
            }

          picman_progress_update ((double) i / (double) height);
        }
    }
  else
    {
      /* Writing a 8-bit Matte (Mask) image */
      guchar record[2];

      put_short (8, file);

      for (i = 0; i < height;)
        {
          rectHeight = (tile_height < (height - i - 1)) ?
                        tile_height : (height - i - 1);

          gegl_buffer_get (buffer, GEGL_RECTANGLE (0, i, width, rectHeight), 1.0,
                           format, src_base,
                           GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

          for (src = src_base, row = 0;
               row < tile_height && i < height;
               i += 1, row += 1)
            {
              /* Write a row of the image */
              record[0] = 1;
              record[1] = src[0];
              src += depth;
              for (j = 1; j < width; ++j)
                {
                  if ((record[1] != src[0]) || (record[0] == 255))
                    {
                      /* Write current RLE record and start a new one */
                      writelen = fwrite (record, 1, 2, file);
                      record[0] = 1;
                      record[1] = src[0];
                    }
                  else
                    {
                      /* increment run length in current record */
                      record[0] ++;
                    }
                  src += depth;
                }

              /* Write last record in row */
              writelen = fwrite (record, 1, 2, file);
            }

          picman_progress_update ((double) i / (double) height);
        }
    }

  g_free (src_base);
  fclose (file);
  g_object_unref (buffer);

  picman_progress_update (1.0);

  return TRUE;
}
