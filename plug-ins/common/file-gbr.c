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

/*
 * gbr plug-in version 1.00
 * Loads/saves version 2 PICMAN .gbr files, by Tim Newsome <drz@frody.bloke.com>
 * Some bits stolen from the .99.7 source tree.
 *
 * Added in GBR version 1 support after learning that there wasn't a
 * tool to read them.
 * July 6, 1998 by Seth Burgess <sjburges@picman.org>
 *
 * Dec 17, 2000
 * Load and save PICMAN brushes in GRAY or RGBA.  jtl + neo
 *
 *
 * TODO: Give some better error reporting on not opening files/bad headers
 *       etc.
 */

#include "config.h"

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "app/core/picmanbrush-header.h"
#include "app/core/picmanpattern-header.h"

#include "libpicman/stdplugins-intl.h"


#define LOAD_PROC      "file-gbr-load"
#define SAVE_PROC      "file-gbr-save"
#define PLUG_IN_BINARY "file-gbr"
#define PLUG_IN_ROLE   "picman-file-gbr"


typedef struct
{
  gchar description[256];
  gint  spacing;
} BrushInfo;


/*  local function prototypes  */

static void       query          (void);
static void       run            (const gchar      *name,
                                  gint              nparams,
                                  const PicmanParam  *param,
                                  gint             *nreturn_vals,
                                  PicmanParam       **return_vals);

static gint32     load_image     (GFile            *file,
                                  GError          **error);
static gboolean   save_image     (GFile            *file,
                                  gint32            image_ID,
                                  gint32            drawable_ID,
                                  GError          **error);

static gboolean   save_dialog    (void);
static void       entry_callback (GtkWidget        *widget,
                                  gpointer          data);


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};


/*  private variables  */

static BrushInfo info =
{
  "PICMAN Brush",
  10
};


MAIN ()

static void
query (void)
{
  static const PicmanParamDef load_args[] =
  {
    { PICMAN_PDB_INT32,  "run-mode", "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_STRING, "uri",      "The URI of the file to load" },
    { PICMAN_PDB_STRING, "raw-uri",  "The URI of the file to load" }
  };
  static const PicmanParamDef load_return_vals[] =
  {
    { PICMAN_PDB_IMAGE,  "image",        "Output image" }
  };

  static const PicmanParamDef save_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",    "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",       "Input image" },
    { PICMAN_PDB_DRAWABLE, "drawable",    "Drawable to save" },
    { PICMAN_PDB_STRING,   "uri",         "The URI of the file to save the image in" },
    { PICMAN_PDB_STRING,   "raw-uri",     "The URI of the file to save the image in" },
    { PICMAN_PDB_INT32,    "spacing",     "Spacing of the brush" },
    { PICMAN_PDB_STRING,   "description", "Short description of the brush" }
  };

  picman_install_procedure (LOAD_PROC,
                          "Loads PICMAN brushes",
                          "Loads PICMAN brushes (1 or 4 bpp and old .gpb format)",
                          "Tim Newsome, Jens Lautenbacher, Sven Neumann",
                          "Tim Newsome, Jens Lautenbacher, Sven Neumann",
                          "1997-2005",
                          N_("PICMAN brush"),
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (load_args),
                          G_N_ELEMENTS (load_return_vals),
                          load_args, load_return_vals);

  picman_plugin_icon_register (LOAD_PROC, PICMAN_ICON_TYPE_STOCK_ID,
                             (const guint8 *) PICMAN_STOCK_BRUSH);
  picman_register_file_handler_mime (LOAD_PROC, "image/x-picman-gbr");
  picman_register_file_handler_uri (LOAD_PROC);
  picman_register_magic_load_handler (LOAD_PROC,
                                    "gbr, gpb",
                                    "",
                                    "20, string, PICMAN");

  picman_install_procedure (SAVE_PROC,
                          "Saves files in the PICMAN brush file format",
                          "Saves files in the PICMAN brush file format",
                          "Tim Newsome, Jens Lautenbacher, Sven Neumann",
                          "Tim Newsome, Jens Lautenbacher, Sven Neumann",
                          "1997-2000",
                          N_("PICMAN brush"),
                          "RGB*, GRAY*, INDEXED*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (save_args), 0,
                          save_args, NULL);

  picman_plugin_icon_register (SAVE_PROC, PICMAN_ICON_TYPE_STOCK_ID,
                             (const guint8 *) PICMAN_STOCK_BRUSH);
  picman_register_file_handler_mime (SAVE_PROC, "image/x-picman-gbr");
  picman_register_file_handler_uri (SAVE_PROC);
  picman_register_save_handler (SAVE_PROC, "gbr", "");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[2];
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  gint32             image_ID;
  gint32             drawable_ID;
  PicmanExportReturn   export = PICMAN_EXPORT_CANCEL;
  GError            *error  = NULL;

  INIT_I18N ();
  gegl_init (NULL, NULL);

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

  if (strcmp (name, LOAD_PROC) == 0)
    {
      image_ID = load_image (g_file_new_for_uri (param[1].data.d_string),
                             &error);

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
  else if (strcmp (name, SAVE_PROC) == 0)
    {
      PicmanParasite *parasite;
      gint32        orig_image_ID;

      image_ID    = param[1].data.d_int32;
      drawable_ID = param[2].data.d_int32;

      orig_image_ID = image_ID;

      switch (run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
        case PICMAN_RUN_WITH_LAST_VALS:
          picman_ui_init (PLUG_IN_BINARY, FALSE);
          export = picman_export_image (&image_ID, &drawable_ID, NULL,
                                      PICMAN_EXPORT_CAN_HANDLE_GRAY    |
                                      PICMAN_EXPORT_CAN_HANDLE_RGB     |
                                      PICMAN_EXPORT_CAN_HANDLE_INDEXED |
                                      PICMAN_EXPORT_CAN_HANDLE_ALPHA);
          if (export == PICMAN_EXPORT_CANCEL)
            {
              values[0].data.d_status = PICMAN_PDB_CANCEL;
              return;
            }

          /*  Possibly retrieve data  */
          picman_get_data (SAVE_PROC, &info);
          break;

        default:
          break;
        }

      parasite = picman_image_get_parasite (orig_image_ID, "picman-brush-name");
      if (parasite)
        {
          gchar *name = g_strndup (picman_parasite_data (parasite),
                                   picman_parasite_data_size (parasite));
          picman_parasite_free (parasite);

          strncpy (info.description, name, sizeof (info.description));
          info.description[sizeof (info.description) - 1] = '\0';

          g_free (name);
        }

      switch (run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
          if (! save_dialog ())
            status = PICMAN_PDB_CANCEL;
          break;

        case PICMAN_RUN_NONINTERACTIVE:
          if (nparams != 7)
            {
              status = PICMAN_PDB_CALLING_ERROR;
            }
          else
            {
              info.spacing = (param[5].data.d_int32);
              strncpy (info.description, param[6].data.d_string,
                       sizeof (info.description));
              info.description[sizeof (info.description) - 1] = '\0';
            }
          break;

        default:
          break;
        }

      if (status == PICMAN_PDB_SUCCESS)
        {
          if (save_image (g_file_new_for_uri (param[3].data.d_string),
                          image_ID, drawable_ID, &error))
            {
              picman_set_data (SAVE_PROC, &info, sizeof (info));
            }
          else
            {
              status = PICMAN_PDB_EXECUTION_ERROR;
            }
        }

      if (export == PICMAN_EXPORT_EXPORT)
        picman_image_delete (image_ID);

      if (info.description && strlen (info.description))
        {
          PicmanParasite *parasite;

          parasite = picman_parasite_new ("picman-brush-name",
                                        PICMAN_PARASITE_PERSISTENT,
                                        strlen (info.description) + 1,
                                        info.description);
          picman_image_attach_parasite (orig_image_ID, parasite);
          picman_parasite_free (parasite);
        }
      else
        {
          picman_image_detach_parasite (orig_image_ID, "picman-brush-name");
        }
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

static gint32
load_image (GFile   *file,
            GError **error)
{
  GInputStream      *input;
  gchar             *name;
  BrushHeader        bh;
  guchar            *brush_buf = NULL;
  gint32             image_ID;
  gint32             layer_ID;
  PicmanParasite      *parasite;
  GeglBuffer        *buffer;
  const Babl        *format;
  PicmanImageBaseType  base_type;
  PicmanImageType      image_type;
  gsize              bytes_read;
  gsize              size;
  gint               i;

  input = G_INPUT_STREAM (g_file_read (file, NULL, error));
  if (! input)
    return -1;

  picman_progress_init_printf (_("Opening '%s'"),
                             g_file_get_parse_name (file));

  size = G_STRUCT_OFFSET (BrushHeader, magic_number);

  if (! g_input_stream_read_all (input, &bh, size,
                                 &bytes_read, NULL, error) ||
      bytes_read != size)
    {
      g_object_unref (input);
      return -1;
    }

  /*  rearrange the bytes in each unsigned int  */
  bh.header_size  = g_ntohl (bh.header_size);
  bh.version      = g_ntohl (bh.version);
  bh.width        = g_ntohl (bh.width);
  bh.height       = g_ntohl (bh.height);
  bh.bytes        = g_ntohl (bh.bytes);

  /* Sanitize values */
  if ((bh.width  == 0) || (bh.width  > PICMAN_MAX_IMAGE_SIZE) ||
      (bh.height == 0) || (bh.height > PICMAN_MAX_IMAGE_SIZE) ||
      ((bh.bytes != 1) && (bh.bytes != 2) && (bh.bytes != 4) &&
       (bh.bytes != 18)) ||
      (G_MAXSIZE / bh.width / bh.height / bh.bytes < 1))
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Invalid header data in '%s': width=%lu, height=%lu, "
                     "bytes=%lu"), g_file_get_parse_name (file),
                   (unsigned long int)bh.width, (unsigned long int)bh.height,
                   (unsigned long int)bh.bytes);
      return -1;
    }

  switch (bh.version)
    {
    case 1:
      /* Version 1 didn't have a magic number and had no spacing  */
      bh.spacing = 25;
      bh.header_size += 8;
      break;

    case 2:
    case 3: /*  cinepaint brush  */
      size = sizeof (bh.magic_number) + sizeof (bh.spacing);

      if (! g_input_stream_read_all (input,
                                     (guchar *) &bh +
                                     G_STRUCT_OFFSET (BrushHeader,
                                                      magic_number), size,
                                     &bytes_read, NULL, error) ||
          bytes_read != size)
        {
          g_object_unref (input);
          return -1;
        }

      bh.magic_number = g_ntohl (bh.magic_number);
      bh.spacing      = g_ntohl (bh.spacing);

      if (bh.version == 3)
        {
          if (bh.bytes == 18 /* FLOAT16_GRAY_GIMAGE */)
            {
              bh.bytes = 2;
            }
          else
            {
              g_message (_("Unsupported brush format"));
              g_object_unref (input);
              return -1;
            }
        }

      if (bh.magic_number == GBRUSH_MAGIC &&
          bh.header_size  >  sizeof (BrushHeader))
        break;

    default:
      g_message (_("Unsupported brush format"));
      g_object_unref (input);
      return -1;
    }

  if ((size = (bh.header_size - sizeof (BrushHeader))) > 0)
    {
      gchar *temp = g_new (gchar, size);

      if (! g_input_stream_read_all (input, temp, size,
                                     &bytes_read, NULL, error) ||
          bytes_read != size)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("Error in PICMAN brush file '%s'"),
                       g_file_get_parse_name (file));
          g_object_unref (input);
          g_free (temp);
          return -1;
        }

      name = picman_any_to_utf8 (temp, -1,
                               _("Invalid UTF-8 string in brush file '%s'."),
                               g_file_get_parse_name (file));
      g_free (temp);
    }
  else
    {
      name = g_strdup (_("Unnamed"));
    }

  /* Now there's just raw data left. */

  size = bh.width * bh.height * bh.bytes;
  brush_buf = g_malloc (size);

  if (! g_input_stream_read_all (input, brush_buf, size,
                                 &bytes_read, NULL, error) ||
      bytes_read != size)
    {
      g_object_unref (input);
      g_free (brush_buf);
      g_free (name);
      return -1;
    }

  switch (bh.bytes)
    {
    case 1:
      {
        PatternHeader ph;

        /*  For backwards-compatibility, check if a pattern follows.
            The obsolete .gpb format did it this way.  */

        if (g_input_stream_read_all (input, &ph, sizeof (PatternHeader),
                                     &bytes_read, NULL, NULL) &&
            bytes_read == sizeof(PatternHeader))
          {
            /*  rearrange the bytes in each unsigned int  */
            ph.header_size  = g_ntohl (ph.header_size);
            ph.version      = g_ntohl (ph.version);
            ph.width        = g_ntohl (ph.width);
            ph.height       = g_ntohl (ph.height);
            ph.bytes        = g_ntohl (ph.bytes);
            ph.magic_number = g_ntohl (ph.magic_number);

            if (ph.magic_number == GPATTERN_MAGIC        &&
                ph.version      == 1                     &&
                ph.header_size  > sizeof (PatternHeader) &&
                ph.bytes        == 3                     &&
                ph.width        == bh.width              &&
                ph.height       == bh.height             &&
                g_input_stream_skip (input,
                                     ph.header_size - sizeof (PatternHeader),
                                     NULL, NULL) ==
                ph.header_size - sizeof (PatternHeader))
              {
                guchar *plain_brush = brush_buf;
                gint    i;

                bh.bytes = 4;
                brush_buf = g_malloc (4 * bh.width * bh.height);

                for (i = 0; i < ph.width * ph.height; i++)
                  {
                    if (! g_input_stream_read_all (input,
                                                   brush_buf + i * 4, 3,
                                                   &bytes_read, NULL, error) ||
                        bytes_read != 3)
                      {
                        g_object_unref (input);
                        g_free (name);
                        g_free (plain_brush);
                        g_free (brush_buf);
                        return -1;
                      }

                    brush_buf[i * 4 + 3] = plain_brush[i];
                  }

                g_free (plain_brush);
              }
          }
      }
      break;

    case 2:
      {
        guint16 *buf = (guint16 *) brush_buf;

        for (i = 0; i < bh.width * bh.height; i++, buf++)
          {
            union
            {
              guint16 u[2];
              gfloat  f;
            } short_float;

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
            short_float.u[0] = 0;
            short_float.u[1] = GUINT16_FROM_BE (*buf);
#else
            short_float.u[0] = GUINT16_FROM_BE (*buf);
            short_float.u[1] = 0;
#endif

            brush_buf[i] = (guchar) (short_float.f * 255.0 + 0.5);
          }

        bh.bytes = 1;
      }
      break;

    default:
      break;
    }

  /*
   * Create a new image of the proper size and
   * associate the filename with it.
   */

  switch (bh.bytes)
    {
    case 1:
      base_type = PICMAN_GRAY;
      image_type = PICMAN_GRAY_IMAGE;
      format = babl_format ("Y' u8");
      break;

    case 4:
      base_type = PICMAN_RGB;
      image_type = PICMAN_RGBA_IMAGE;
      format = babl_format ("R'G'B'A u8");
      break;

    default:
      g_message ("Unsupported brush depth: %d\n"
                 "PICMAN Brushes must be GRAY or RGBA\n",
                 bh.bytes);
      g_free (name);
      return -1;
    }

  image_ID = picman_image_new (bh.width, bh.height, base_type);
  picman_image_set_filename (image_ID, g_file_get_uri (file));

  parasite = picman_parasite_new ("picman-brush-name",
                                PICMAN_PARASITE_PERSISTENT,
                                strlen (name) + 1, name);
  picman_image_attach_parasite (image_ID, parasite);
  picman_parasite_free (parasite);

  layer_ID = picman_layer_new (image_ID, name, bh.width, bh.height,
                             image_type, 100, PICMAN_NORMAL_MODE);
  picman_image_insert_layer (image_ID, layer_ID, -1, 0);

  g_free (name);

  buffer = picman_drawable_get_buffer (layer_ID);

  /*  invert  */
  if (image_type == PICMAN_GRAY_IMAGE)
    for (i = 0; i < bh.width * bh.height; i++)
      brush_buf[i] = 255 - brush_buf[i];

  gegl_buffer_set (buffer, GEGL_RECTANGLE (0, 0, bh.width, bh.height), 0,
                   format, brush_buf, GEGL_AUTO_ROWSTRIDE);

  g_free (brush_buf);
  g_object_unref (buffer);
  g_object_unref (input);

  picman_progress_update (1.0);

  return image_ID;
}

static gboolean
save_image (GFile   *file,
            gint32   image_ID,
            gint32   drawable_ID,
            GError **error)
{
  GOutputStream *output;
  BrushHeader    bh;
  guchar        *brush_buf;
  GeglBuffer    *buffer;
  const Babl    *format;
  gint           line;
  gint           x;
  gint           bpp;
  gint           file_bpp;
  gint           width;
  gint           height;
  PicmanRGB        gray, white;
  gsize          bytes_written;

  picman_rgba_set_uchar (&white, 255, 255, 255, 255);

  switch (picman_drawable_type (drawable_ID))
    {
    case PICMAN_GRAY_IMAGE:
      file_bpp = 1;
      format = babl_format ("Y' u8");
      break;

    case PICMAN_GRAYA_IMAGE:
      file_bpp = 1;
      format = babl_format ("Y'A u8");
      break;

    default:
      file_bpp = 4;
      format = babl_format ("R'G'B'A u8");
      break;
    }

  bpp = babl_format_get_bytes_per_pixel (format);

  output = G_OUTPUT_STREAM (g_file_replace (file, NULL, FALSE, 0, NULL, error));
  if (! output)
    return FALSE;

  picman_progress_init_printf (_("Saving '%s'"),
                             g_file_get_parse_name (file));

  buffer = picman_drawable_get_buffer (drawable_ID);

  width  = picman_drawable_width  (drawable_ID);
  height = picman_drawable_height (drawable_ID);

  bh.header_size  = g_htonl (sizeof (BrushHeader) +
                             strlen (info.description) + 1);
  bh.version      = g_htonl (2);
  bh.width        = g_htonl (width);
  bh.height       = g_htonl (height);
  bh.bytes        = g_htonl (file_bpp);
  bh.magic_number = g_htonl (GBRUSH_MAGIC);
  bh.spacing      = g_htonl (info.spacing);

  if (! g_output_stream_write_all (output, &bh, sizeof (BrushHeader),
                                   &bytes_written, NULL, error) ||
      bytes_written != sizeof (BrushHeader))
    {
      g_object_unref (output);
      return FALSE;
    }

  if (! g_output_stream_write_all (output,
                                   info.description,
                                   strlen (info.description) + 1,
                                   &bytes_written, NULL, error) ||
      bytes_written != strlen (info.description) + 1)
    {
      g_object_unref (output);
      return FALSE;
    }

  brush_buf = g_new (guchar, width * bpp);

  for (line = 0; line < height; line++)
    {
      gegl_buffer_get (buffer, GEGL_RECTANGLE (0, line, width, 1), 1.0,
                       format, brush_buf,
                       GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

      switch (bpp)
        {
        case 1:
          /*  invert  */
          for (x = 0; x < width; x++)
            brush_buf[x] = 255 - brush_buf[x];
          break;

        case 2:
          for (x = 0; x < width; x++)
            {
              /*  apply alpha channel  */
              picman_rgba_set_uchar (&gray,
                                   brush_buf[2 * x],
                                   brush_buf[2 * x],
                                   brush_buf[2 * x],
                                   brush_buf[2 * x + 1]);
              picman_rgb_composite (&gray, &white, PICMAN_RGB_COMPOSITE_BEHIND);
              picman_rgba_get_uchar (&gray, &brush_buf[x], NULL, NULL, NULL);
              /* invert */
              brush_buf[x] = 255 - brush_buf[x];
            }
          break;
        }

      if (! g_output_stream_write_all (output, brush_buf, width * file_bpp,
                                       &bytes_written, NULL, error) ||
          bytes_written != width * file_bpp)
        {
          g_free (brush_buf);
          g_object_unref (output);
          return FALSE;
        }

      picman_progress_update ((gdouble) line / (gdouble) height);
    }

  g_free (brush_buf);
  g_object_unref (buffer);
  g_object_unref (output);

  picman_progress_update (1.0);

  return TRUE;
}

static gboolean
save_dialog (void)
{
  GtkWidget *dialog;
  GtkWidget *table;
  GtkWidget *entry;
  GtkWidget *spinbutton;
  GtkObject *adj;
  gboolean   run;

  dialog = picman_export_dialog_new (_("Brush"), PLUG_IN_BINARY, SAVE_PROC);

  /* The main table */
  table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 12);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (picman_export_dialog_get_content_area (dialog)),
                      table, TRUE, TRUE, 0);
  gtk_widget_show (table);

  spinbutton = picman_spin_button_new (&adj,
                                     info.spacing, 1, 1000, 1, 10, 0, 1, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("Spacing:"), 1.0, 0.5,
                             spinbutton, 1, TRUE);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &info.spacing);

  entry = gtk_entry_new ();
  gtk_widget_set_size_request (entry, 200, -1);
  gtk_entry_set_text (GTK_ENTRY (entry), info.description);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             _("Description:"), 1.0, 0.5,
                             entry, 1, FALSE);

  g_signal_connect (entry, "changed",
                    G_CALLBACK (entry_callback),
                    info.description);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
entry_callback (GtkWidget *widget,
                gpointer   data)
{
  strncpy (info.description, gtk_entry_get_text (GTK_ENTRY (widget)),
           sizeof (info.description));
  info.description[sizeof (info.description) - 1] = '\0';
}
