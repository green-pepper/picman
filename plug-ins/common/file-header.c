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

#include <string.h>

#include <glib/gstdio.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define SAVE_PROC      "file-header-save"
#define PLUG_IN_BINARY "file-header"
#define PLUG_IN_ROLE   "picman-file-header"


/* Declare some local functions.
 */
static void       query      (void);
static void       run        (const gchar      *name,
                              gint              nparams,
                              const PicmanParam  *param,
                              gint             *nreturn_vals,
                              PicmanParam       **return_vals);
static gboolean   save_image (const gchar      *filename,
                              gint32            image_ID,
                              gint32            drawable_ID);


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
  static const PicmanParamDef save_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",        "Input image" },
    { PICMAN_PDB_DRAWABLE, "drawable",     "Drawable to save" },
    { PICMAN_PDB_STRING,   "filename",     "The name of the file to save the image in" },
    { PICMAN_PDB_STRING,   "raw-filename", "The name of the file to save the image in" }
  };

  picman_install_procedure (SAVE_PROC,
                          "saves files as C unsigned character array",
                          "FIXME: write help",
                          "Spencer Kimball & Peter Mattis",
                          "Spencer Kimball & Peter Mattis",
                          "1997",
                          N_("C source code header"),
                          "INDEXED, RGB",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (save_args), 0,
                          save_args, NULL);

  picman_register_file_handler_mime (SAVE_PROC, "text/x-chdr");
  picman_register_save_handler (SAVE_PROC, "h", "");
}

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

  INIT_I18N ();
  gegl_init (NULL, NULL);

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;
  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

  if (strcmp (name, SAVE_PROC) == 0)
    {
      gint32           image_ID;
      gint32           drawable_ID;
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
                                      PICMAN_EXPORT_CAN_HANDLE_RGB |
                                      PICMAN_EXPORT_CAN_HANDLE_INDEXED);
          if (export == PICMAN_EXPORT_CANCEL)
            {
              values[0].data.d_status = PICMAN_PDB_CANCEL;
              return;
            }
          break;

        default:
          break;
        }

      if (! save_image (param[3].data.d_string, image_ID, drawable_ID))
        {
          status = PICMAN_PDB_EXECUTION_ERROR;
        }

      if (export == PICMAN_EXPORT_EXPORT)
        picman_image_delete (image_ID);
    }
  else
    {
      status = PICMAN_PDB_CALLING_ERROR;
    }

  values[0].data.d_status = status;
}

static gboolean
save_image (const gchar *filename,
            gint32       image_ID,
            gint32       drawable_ID)
{
  GeglBuffer    *buffer;
  const Babl    *format;
  PicmanImageType  drawable_type;
  FILE          *fp;
  gint           x, y, b, c;
  const gchar   *backslash = "\\\\";
  const gchar   *quote     = "\\\"";
  const gchar   *newline   = "\"\n\t\"";
  gchar          buf[4];
  guchar        *d         = NULL;
  guchar        *data      = NULL;
  guchar        *cmap;
  gint           colors;
  gint           width;
  gint           height;

  if ((fp = g_fopen (filename, "w")) == NULL)
    return FALSE;

  buffer = picman_drawable_get_buffer (drawable_ID);

  width  = gegl_buffer_get_width  (buffer);
  height = gegl_buffer_get_height (buffer);

  drawable_type = picman_drawable_type (drawable_ID);

  fprintf (fp, "/*  PICMAN header image file format (%s): %s  */\n\n",
           PICMAN_RGB_IMAGE == drawable_type ? "RGB" : "INDEXED", filename);
  fprintf (fp, "static unsigned int width = %d;\n", width);
  fprintf (fp, "static unsigned int height = %d;\n\n", height);
  fprintf (fp, "/*  Call this macro repeatedly.  After each use, the pixel data can be extracted  */\n\n");

  switch (drawable_type)
    {
    case PICMAN_RGB_IMAGE:
      fprintf (fp,
               "#define HEADER_PIXEL(data,pixel) {\\\n"
               "pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \\\n"
               "pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \\\n"
               "pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \\\n"
               "data += 4; \\\n}\n");
      fprintf (fp, "static char *header_data =\n\t\"");

      format = babl_format ("R'G'B' u8");

      data = g_new (guchar, width * babl_format_get_bytes_per_pixel (format));


      c = 0;
      for (y = 0; y < height; y++)
        {
          gegl_buffer_get (buffer, GEGL_RECTANGLE (0, y, width, 1), 1.0,
                           format, data,
                           GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

          for (x = 0; x < width; x++)
            {
              d = data + x * babl_format_get_bytes_per_pixel (format);

              buf[0] = ((d[0] >> 2) & 0x3F) + 33;
              buf[1] = ((((d[0] & 0x3) << 4) | (d[1] >> 4)) & 0x3F) + 33;
              buf[2] = ((((d[1] & 0xF) << 2) | (d[2] >> 6)) & 0x3F) + 33;
              buf[3] = (d[2] & 0x3F) + 33;

              for (b = 0; b < 4; b++)
                {
                  if (buf[b] == '"')
                    fwrite (quote, 1, 2, fp);
                  else if (buf[b] == '\\')
                    fwrite (backslash, 1, 2, fp);
                  else
                    fwrite (buf + b, 1, 1, fp);
                }

              c++;
              if (c >= 16)
                {
                  fwrite (newline, 1, 4, fp);
                  c = 0;
                }
            }
        }

      fprintf (fp, "\";\n");
      break;

    case PICMAN_INDEXED_IMAGE:
      fprintf (fp,
               "#define HEADER_PIXEL(data,pixel) {\\\n"
               "pixel[0] = header_data_cmap[(unsigned char)data[0]][0]; \\\n"
               "pixel[1] = header_data_cmap[(unsigned char)data[0]][1]; \\\n"
               "pixel[2] = header_data_cmap[(unsigned char)data[0]][2]; \\\n"
               "data ++; }\n\n");
      /* save colormap */
      cmap = picman_image_get_colormap (image_ID, &colors);

      fprintf (fp, "static char header_data_cmap[256][3] = {");
      fprintf (fp, "\n\t{%3d,%3d,%3d}", (int)cmap[0], (int)cmap[1], (int)cmap[2]);

      for (c = 1; c < colors; c++)
        fprintf (fp, ",\n\t{%3d,%3d,%3d}", (int)cmap[3*c], (int)cmap[3*c+1], (int)cmap[3*c+2]);

      /* fill the rest */
      for ( ; c < 256; c++)
        fprintf (fp, ",\n\t{255,255,255}");

      /* close bracket */
      fprintf (fp, "\n\t};\n");
      g_free (cmap);

      /* save image */
      fprintf (fp, "static char header_data[] = {\n\t");

      data = g_new (guchar, width * 1);

      c = 0;
      for (y = 0; y < height; y++)
        {
          gegl_buffer_get (buffer, GEGL_RECTANGLE (0, y, width, 1), 1.0,
                           NULL, data,
                           GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

          for (x = 0; x < width  -1; x++)
            {
              d = data + x * 1;

              fprintf (fp, "%d,", (int)d[0]);

              c++;
              if (c >= 16)
                {
                  fprintf (fp, "\n\t");
                  c = 0;
                }
            }

          if (y != height - 1)
            fprintf (fp, "%d,\n\t", (int)d[1]);
          else
            fprintf (fp, "%d\n\t", (int)d[1]);

          c = 0; /* reset line counter */
        }
      fprintf (fp, "};\n");
      break;

    default:
      g_warning ("unhandled drawable type (%d)", drawable_type);
      fclose (fp);
      g_free (data);
      return FALSE;
    }

  fclose (fp);
  g_free (data);
  g_object_unref (buffer);

  return TRUE;
}
