/*
 * pat plug-in version 1.01
 * Loads/saves version 1 PICMAN .pat files, by Tim Newsome <drz@frody.bloke.com>
 *
 * PICMAN - The GNU Image Manipulation Program
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

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "app/core/picmanpattern-header.h"

#include "libpicman/stdplugins-intl.h"


#define LOAD_PROC      "file-pat-load"
#define SAVE_PROC      "file-pat-save"
#define PLUG_IN_BINARY "file-pat"
#define PLUG_IN_ROLE   "picman-file-pat"


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


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};


/*  private variables  */

static gchar description[256] = "PICMAN Pattern";


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
    { PICMAN_PDB_IMAGE, "image", "Output image" }
  };

  static const PicmanParamDef save_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",    "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }"     },
    { PICMAN_PDB_IMAGE,    "image",       "Input image"                      },
    { PICMAN_PDB_DRAWABLE, "drawable",    "Drawable to save"                 },
    { PICMAN_PDB_STRING,   "uri",         "The URI of the file to save the image in" },
    { PICMAN_PDB_STRING,   "raw-uri",     "The URI of the file to save the image in" },
    { PICMAN_PDB_STRING,   "description", "Short description of the pattern" }
  };

  picman_install_procedure (LOAD_PROC,
                          "Loads Picman's .PAT pattern files",
                          "The images in the pattern dialog can be loaded "
                          "directly with this plug-in",
                          "Tim Newsome",
                          "Tim Newsome",
                          "1997",
                          N_("PICMAN pattern"),
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (load_args),
                          G_N_ELEMENTS (load_return_vals),
                          load_args, load_return_vals);

  picman_plugin_icon_register (LOAD_PROC, PICMAN_ICON_TYPE_STOCK_ID,
                             (const guint8 *) PICMAN_STOCK_PATTERN);
  picman_register_file_handler_mime (LOAD_PROC, "image/x-picman-pat");
  picman_register_file_handler_uri (LOAD_PROC);
  picman_register_magic_load_handler (LOAD_PROC,
                                    "pat",
                                    "",
                                    "20,string,GPAT");

  picman_install_procedure (SAVE_PROC,
                          "Saves Picman pattern file (.PAT)",
                          "New Picman patterns can be created by saving them "
                          "in the appropriate place with this plug-in.",
                          "Tim Newsome",
                          "Tim Newsome",
                          "1997",
                          N_("PICMAN pattern"),
                          "RGB*, GRAY*, INDEXED*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (save_args), 0,
                          save_args, NULL);

  picman_plugin_icon_register (SAVE_PROC, PICMAN_ICON_TYPE_STOCK_ID,
                             (const guint8 *) PICMAN_STOCK_PATTERN);
  picman_register_file_handler_mime (SAVE_PROC, "image/x-picman-pat");
  picman_register_file_handler_uri (SAVE_PROC);
  picman_register_save_handler (SAVE_PROC, "pat", "");
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
          picman_get_data (SAVE_PROC, description);
          break;

        default:
          break;
        }

      parasite = picman_image_get_parasite (orig_image_ID, "picman-pattern-name");
      if (parasite)
        {
          gchar *name = g_strndup (picman_parasite_data (parasite),
                                   picman_parasite_data_size (parasite));
          picman_parasite_free (parasite);

          strncpy (description, name, sizeof (description));
          description[sizeof (description) - 1] = '\0';

          g_free (name);
        }

      switch (run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
          if (!save_dialog ())
            status = PICMAN_PDB_CANCEL;
          break;

        case PICMAN_RUN_NONINTERACTIVE:
          if (nparams != 6)
            {
              status = PICMAN_PDB_CALLING_ERROR;
            }
          else
            {
              strncpy (description, param[5].data.d_string,
                       sizeof (description));
              description[sizeof (description) - 1] = '\0';
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
              picman_set_data (SAVE_PROC, description, sizeof (description));
            }
          else
            {
              status = PICMAN_PDB_EXECUTION_ERROR;
            }
        }

      if (export == PICMAN_EXPORT_EXPORT)
        picman_image_delete (image_ID);

      if (strlen (description))
        {
          PicmanParasite *parasite;

          parasite = picman_parasite_new ("picman-pattern-name",
                                        PICMAN_PARASITE_PERSISTENT,
                                        strlen (description) + 1,
                                        description);
          picman_image_attach_parasite (orig_image_ID, parasite);
          picman_parasite_free (parasite);
        }
      else
        {
          picman_image_detach_parasite (orig_image_ID, "picman-pattern-name");
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
  GInputStream     *input;
  PatternHeader     ph;
  gchar            *name;
  gchar            *temp;
  guchar           *buf;
  gint32            image_ID;
  gint32            layer_ID;
  PicmanParasite     *parasite;
  GeglBuffer       *buffer;
  const Babl       *file_format;
  gint              line;
  PicmanImageBaseType base_type;
  PicmanImageType     image_type;
  gsize             bytes_read;

  input = G_INPUT_STREAM (g_file_read (file, NULL, error));
  if (! input)
    return -1;

  picman_progress_init_printf (_("Opening '%s'"),
                             g_file_get_parse_name (file));

  if (! g_input_stream_read_all (input, &ph, sizeof (PatternHeader),
                                 &bytes_read, NULL, error) ||
      bytes_read != sizeof (PatternHeader))
    {
      g_object_unref (input);
      return -1;
    }

  /*  rearrange the bytes in each unsigned int  */
  ph.header_size  = g_ntohl (ph.header_size);
  ph.version      = g_ntohl (ph.version);
  ph.width        = g_ntohl (ph.width);
  ph.height       = g_ntohl (ph.height);
  ph.bytes        = g_ntohl (ph.bytes);
  ph.magic_number = g_ntohl (ph.magic_number);

  if (ph.magic_number != GPATTERN_MAGIC ||
      ph.version      != 1 ||
      ph.header_size  <= sizeof (PatternHeader))
    {
      g_object_unref (input);
      return -1;
    }

  temp = g_new (gchar, ph.header_size - sizeof (PatternHeader));

  if (! g_input_stream_read_all (input,
                                 temp, ph.header_size - sizeof (PatternHeader),
                                 &bytes_read, NULL, error) ||
      bytes_read != ph.header_size - sizeof (PatternHeader))
    {
      g_free (temp);
      g_object_unref (input);
      return -1;
    }

  name = picman_any_to_utf8 (temp, -1,
                           _("Invalid UTF-8 string in pattern file '%s'."),
                           g_file_get_parse_name (file));
  g_free (temp);

  /* Now there's just raw data left. */

  /*
   * Create a new image of the proper size and associate the filename with it.
   */

  switch (ph.bytes)
    {
    case 1:
      base_type = PICMAN_GRAY;
      image_type = PICMAN_GRAY_IMAGE;
      file_format = babl_format ("Y' u8");
      break;
    case 2:
      base_type = PICMAN_GRAY;
      image_type = PICMAN_GRAYA_IMAGE;
      file_format = babl_format ("Y'A u8");
      break;
    case 3:
      base_type = PICMAN_RGB;
      image_type = PICMAN_RGB_IMAGE;
      file_format = babl_format ("R'G'B' u8");
      break;
    case 4:
      base_type = PICMAN_RGB;
      image_type = PICMAN_RGBA_IMAGE;
      file_format = babl_format ("R'G'B'A u8");
      break;
    default:
      g_message ("Unsupported pattern depth: %d\n"
                 "PICMAN Patterns must be GRAY or RGB", ph.bytes);
      g_object_unref (input);
      return -1;
    }

  /* Sanitize input dimensions and guard against overflows. */
  if ((ph.width == 0) || (ph.width > PICMAN_MAX_IMAGE_SIZE) ||
      (ph.height == 0) || (ph.height > PICMAN_MAX_IMAGE_SIZE) ||
      (G_MAXSIZE / ph.width / ph.bytes < 1))
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Invalid header data in '%s': width=%lu, height=%lu, "
                     "bytes=%lu"), g_file_get_parse_name (file),
                   (unsigned long int)ph.width, (unsigned long int)ph.height,
                   (unsigned long int)ph.bytes);
      g_object_unref (input);
      return -1;
    }

  image_ID = picman_image_new (ph.width, ph.height, base_type);
  picman_image_set_filename (image_ID, g_file_get_uri (file));

  parasite = picman_parasite_new ("picman-pattern-name",
                                PICMAN_PARASITE_PERSISTENT,
                                strlen (name) + 1, name);
  picman_image_attach_parasite (image_ID, parasite);
  picman_parasite_free (parasite);

  layer_ID = picman_layer_new (image_ID, name, ph.width, ph.height,
                             image_type, 100, PICMAN_NORMAL_MODE);
  picman_image_insert_layer (image_ID, layer_ID, -1, 0);

  g_free (name);

  buffer = picman_drawable_get_buffer (layer_ID);

  /* this can't overflow because ph.width is <= PICMAN_MAX_IMAGE_SIZE */
  buf = g_malloc (ph.width * ph.bytes);

  for (line = 0; line < ph.height; line++)
    {
      if (! g_input_stream_read_all (input, buf, ph.width * ph.bytes,
                                     &bytes_read, NULL, error) ||
          bytes_read != ph.width * ph.bytes)
        {
          if (line == 0)
            {
              g_free (buf);
              g_object_unref (buffer);
              g_object_unref (input);
              return -1;
            }
          else
            {
              g_message ("PICMAN Pattern file is truncated "
                         "(%d of %d lines recovered).",
                         line - 1, ph.height);
              break;
            }
        }

      gegl_buffer_set (buffer, GEGL_RECTANGLE (0, line, ph.width, 1), 0,
                       file_format, buf, GEGL_AUTO_ROWSTRIDE);

      picman_progress_update ((gdouble) line / (gdouble) ph.height);
    }

  g_free (buf);
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
  PatternHeader  ph;
  GeglBuffer    *buffer;
  const Babl    *file_format;
  guchar        *buf;
  gint           width;
  gint           height;
  gint           line_size;
  gint           line;
  gsize          bytes_written;

  switch (picman_drawable_type (drawable_ID))
    {
    case PICMAN_GRAY_IMAGE:
      file_format = babl_format ("Y' u8");
      break;

    case PICMAN_GRAYA_IMAGE:
      file_format = babl_format ("Y'A u8");
      break;

    case PICMAN_RGB_IMAGE:
    case PICMAN_INDEXED_IMAGE:
      file_format = babl_format ("R'G'B' u8");
      break;

    case PICMAN_RGBA_IMAGE:
    case PICMAN_INDEXEDA_IMAGE:
      file_format = babl_format ("R'G'B'A u8");
      break;

    default:
      g_message ("Unsupported image type: %d\n"
                 "PICMAN Patterns must be GRAY or RGB",
                 picman_drawable_type (drawable_ID));
      return FALSE;
    }

  output = G_OUTPUT_STREAM (g_file_replace (file, NULL, FALSE, 0, NULL, error));
  if (! output)
    return FALSE;

  picman_progress_init_printf (_("Saving '%s'"),
                             g_file_get_parse_name (file));

  buffer = picman_drawable_get_buffer (drawable_ID);

  width  = gegl_buffer_get_width (buffer);
  height = gegl_buffer_get_height (buffer);

  ph.header_size  = g_htonl (sizeof (PatternHeader) + strlen (description) + 1);
  ph.version      = g_htonl (1);
  ph.width        = g_htonl (width);
  ph.height       = g_htonl (height);
  ph.bytes        = g_htonl (babl_format_get_bytes_per_pixel (file_format));
  ph.magic_number = g_htonl (GPATTERN_MAGIC);

  if (! g_output_stream_write_all (output, &ph, sizeof (PatternHeader),
                                   &bytes_written, NULL, error) ||
      bytes_written != sizeof (PatternHeader))
    {
      g_object_unref (output);
      return FALSE;
    }

  if (! g_output_stream_write_all (output,
                                   description, strlen (description) + 1,
                                   &bytes_written, NULL, error) ||
      bytes_written != strlen (description) + 1)
    {
      g_object_unref (output);
      return FALSE;
    }

  line_size = width * babl_format_get_bytes_per_pixel (file_format);

  /* this can't overflow because drawable->width is <= PICMAN_MAX_IMAGE_SIZE */
  buf = g_alloca (line_size);

  for (line = 0; line < height; line++)
    {
      gegl_buffer_get (buffer, GEGL_RECTANGLE (0, line, width, 1), 1.0,
                       file_format, buf,
                       GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

      if (! g_output_stream_write_all (output, buf, line_size,
                                       &bytes_written, NULL, error) ||
          bytes_written != line_size)
        {
          g_object_unref (buffer);
          g_object_unref (output);
          return FALSE;
        }

      picman_progress_update ((gdouble) line / (gdouble) ph.height);
    }

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
  gboolean   run;

  dialog = picman_export_dialog_new (_("Pattern"), PLUG_IN_BINARY, SAVE_PROC);

  /* The main table */
  table = gtk_table_new (1, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_container_set_border_width (GTK_CONTAINER (table), 12);
  gtk_box_pack_start (GTK_BOX (picman_export_dialog_get_content_area (dialog)),
                      table, TRUE, TRUE, 0);
  gtk_widget_show (table);

  entry = gtk_entry_new ();
  gtk_widget_set_size_request (entry, 200, -1);
  gtk_entry_set_text (GTK_ENTRY (entry), description);
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("Description:"), 1.0, 0.5,
                             entry, 1, FALSE);
  gtk_widget_show (entry);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  if (run)
    {
      strncpy (description, gtk_entry_get_text (GTK_ENTRY (entry)),
               sizeof (description));
      description[sizeof (description) - 1] = '\0';
    }

  gtk_widget_destroy (dialog);

  return run;
}
