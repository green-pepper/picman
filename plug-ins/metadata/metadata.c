/* metadata.c - main() for the metadata editor
 *
 * Copyright (C) 2004-2005, Raphaël Quinet <raphael@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include <libpicman/picman.h>

#include <libexif/exif-data.h>

#include "libpicman/stdplugins-intl.h"

#include "metadata.h"
#include "xmp-schemas.h"
#include "xmp-encode.h"
#include "interface.h"
#include "exif-decode.h"
/* FIXME: uncomment when these are working
#include "exif-encode.h"
#include "iptc-decode.h"
*/


#define METADATA_PARASITE   "picman-metadata"
#define METADATA_MARKER     "PICMAN_XMP_1"
#define METADATA_MARKER_LEN (strlen (METADATA_MARKER))


/* prototypes of local functions */
static void  query (void);
static void  run   (const gchar      *name,
                    gint              nparams,
                    const PicmanParam  *param,
                    gint             *nreturn_vals,
                    PicmanParam       **return_vals);


/* local variables */
const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

/* local functions */
MAIN ()

static void
query (void)
{
  static const PicmanParamDef editor_args[] =
  {
    { PICMAN_PDB_INT32,       "run-mode",  "Run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  },
    { PICMAN_PDB_DRAWABLE,    "drawable",  "Input drawable (unused)"      }
  };

  static const PicmanParamDef decode_xmp_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  },
    { PICMAN_PDB_STRING,      "xmp",       "XMP packet"                   }
  };

  static const PicmanParamDef encode_xmp_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  }
  };
  static const PicmanParamDef encode_xmp_return_vals[] =
  {
    { PICMAN_PDB_STRING,      "xmp",       "XMP packet"                   }
  };

  static const PicmanParamDef decode_exif_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  },
    { PICMAN_PDB_INT32,       "exif-size", "size of the EXIF block"       },
    { PICMAN_PDB_INT8ARRAY,   "exif",      "EXIF block"                   }
  };

/* FIXME: uncomment when these are working
  static const PicmanParamDef encode_exif_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  }
  };
  static const PicmanParamDef encode_exif_return_vals[] =
  {
    { PICMAN_PDB_INT32,       "exif-size", "size of the EXIF block"       },
    { PICMAN_PDB_INT8ARRAY,   "exif",      "EXIF block"                   }
  };
*/

  static const PicmanParamDef get_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  },
    { PICMAN_PDB_STRING,      "schema",    "XMP schema prefix or URI"     },
    { PICMAN_PDB_STRING,      "property",  "XMP property name"            }
  };
  static const PicmanParamDef get_return_vals[] =
  {
    { PICMAN_PDB_INT32,       "type",      "XMP property type"            },
    { PICMAN_PDB_INT32,       "num-vals",  "number of values"             },
    { PICMAN_PDB_STRINGARRAY, "vals",      "XMP property values"          }
  };

  static const PicmanParamDef set_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  },
    { PICMAN_PDB_STRING,      "schema",    "XMP schema prefix or URI"     },
    { PICMAN_PDB_STRING,      "property",  "XMP property name"            },
    { PICMAN_PDB_INT32,       "type",      "XMP property type"            },
    { PICMAN_PDB_INT32,       "num-vals",  "number of values"             },
    { PICMAN_PDB_STRINGARRAY, "vals",      "XMP property values"          }
  };

  static const PicmanParamDef get_simple_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  },
    { PICMAN_PDB_STRING,      "schema",    "XMP schema prefix or URI"     },
    { PICMAN_PDB_STRING,      "property",  "XMP property name"            }
  };
  static const PicmanParamDef get_simple_return_vals[] =
  {
    { PICMAN_PDB_STRING,      "value",     "XMP property value"           }
  };

  static const PicmanParamDef set_simple_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  },
    { PICMAN_PDB_STRING,      "schema",    "XMP schema prefix or URI"     },
    { PICMAN_PDB_STRING,      "property",  "XMP property name"            },
    { PICMAN_PDB_STRING,      "value",     "XMP property value"           }
  };

/* FIXME: uncomment when these are working
  static const PicmanParamDef delete_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  },
    { PICMAN_PDB_STRING,      "schema",    "XMP schema prefix or URI"     },
    { PICMAN_PDB_STRING,      "property",  "XMP property name"            }
  };

  static const PicmanParamDef add_schema_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  },
    { PICMAN_PDB_STRING,      "prefix",    "XMP schema prefix"            },
    { PICMAN_PDB_STRING,      "uri",       "XMP schema URI"               }
  };
*/

  static const PicmanParamDef import_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                        },
    { PICMAN_PDB_STRING,      "filename",  "The name of the XMP file to import" }
  };

  static const PicmanParamDef export_args[] =
  {
    { PICMAN_PDB_IMAGE,       "image",     "Input image"                  },
    { PICMAN_PDB_STRING,      "filename",  "The name of the file to save the XMP packet in" },
    { PICMAN_PDB_INT32,       "overwrite", "Overwrite existing file: { FALSE (0), TRUE (1) }" }
  };

  picman_install_procedure (EDITOR_PROC,
                          N_("View and edit metadata (EXIF, IPTC, XMP)"),
                          "View and edit metadata information attached to the "
                          "current image.  This can include EXIF, IPTC and/or "
                          "XMP information.  Some or all of this metadata "
                          "will be saved in the file, depending on the output "
                          "file format.",
                          "Raphaël Quinet <raphael@picman.org>",
                          "Raphaël Quinet <raphael@picman.org>",
                          "2004-2005",
                          N_("Propert_ies"),
                          "RGB*, INDEXED*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (editor_args), 0,
                          editor_args, NULL);

  picman_plugin_menu_register (EDITOR_PROC, "<Image>/File/Info");
  // XXX picman_plugin_icon_register (EDITOR_PROC, PICMAN_ICON_TYPE_STOCK_ID,

  picman_install_procedure (DECODE_XMP_PROC,
                          "Decode an XMP packet",
                          "Parse an XMP packet and merge the results with "
                          "any metadata already attached to the image.  This "
                          "should be used when an XMP packet is read from an "
                          "image file.",
                          "Raphaël Quinet <raphael@picman.org>",
                          "Raphaël Quinet <raphael@picman.org>",
                          "2005",
                          NULL,
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (decode_xmp_args), 0,
                          decode_xmp_args, NULL);

  picman_install_procedure (ENCODE_XMP_PROC,
                          "Encode metadata into an XMP packet",
                          "Generate an XMP packet from the metadata "
                          "information attached to the image.  The new XMP "
                          "packet can then be saved into a file.",
                          "Róman Joost <romanofski@picman.org>",
                          "Róman Joost <romanofski@picman.org>",
                          "2008",
                          NULL,
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (encode_xmp_args),
                          G_N_ELEMENTS (encode_xmp_return_vals),
                          encode_xmp_args, encode_xmp_return_vals);

  picman_install_procedure (DECODE_EXIF_PROC,
                          "Decode an EXIF block",
                          "Parse an EXIF block and merge the results with "
                          "any metadata already attached to the image.  This "
                          "should be used when an EXIF block is read from an "
                          "image file.",
                          "Raphaël Quinet <raphael@picman.org>",
                          "Raphaël Quinet <raphael@picman.org>",
                          "2005",
                          NULL,
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (decode_exif_args), 0,
                          decode_exif_args, NULL);

/* FIXME: uncomment when these are working
  picman_install_procedure (ENCODE_EXIF_PROC,
                          "Encode metadata into an EXIF block",
                          "Generate an EXIF block from the metadata "
                          "information attached to the image.  The new EXIF "
                          "block can then be saved into a file.",
                          "Raphaël Quinet <raphael@picman.org>",
                          "Raphaël Quinet <raphael@picman.org>",
                          "2005",
                          NULL,
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (encode_exif_args),
                          G_N_ELEMENTS (encode_exif_return_vals),
                          encode_exif_args, encode_exif_return_vals);
*/

  picman_install_procedure (GET_PROC,
                          "Retrieve the values of an XMP property",
                          "Retrieve the list of values associated with "
                          "an XMP property.",
                          "Raphaël Quinet <raphael@picman.org>",
                          "Raphaël Quinet <raphael@picman.org>",
                          "2005",
                          NULL,
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (get_args),
                          G_N_ELEMENTS (get_return_vals),
                          get_args, get_return_vals);

  picman_install_procedure (SET_PROC,
                          "Set the values of an XMP property",
                          "Set the list of values associated with "
                          "an XMP property.  If a property with the same "
                          "name already exists, it will be replaced.",
                          "Raphaël Quinet <raphael@picman.org>",
                          "Raphaël Quinet <raphael@picman.org>",
                          "2005",
                          NULL,
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (set_args), 0,
                          set_args, NULL);

  picman_install_procedure (GET_SIMPLE_PROC,
                          "Retrieve the value of an XMP property",
                          "Retrieve value associated with a scalar XMP "
                          "property.  This can only be done for simple "
                          "property types such as text or integers.  "
                          "Structured types must be retrieved with "
                          "plug_in_metadata_get().",
                          "Raphaël Quinet <raphael@picman.org>",
                          "Raphaël Quinet <raphael@picman.org>",
                          "2005",
                          NULL,
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (get_simple_args),
                          G_N_ELEMENTS (get_simple_return_vals),
                          get_simple_args, get_simple_return_vals);

  picman_install_procedure (SET_SIMPLE_PROC,
                          "Set the value of an XMP property",
                          "Set the value of a scalar XMP property.  This "
                          "can only be done for simple property types such "
                          "as text or integers.  Structured types need to "
                          "be set with plug_in_metadata_set().",
                          "Raphaël Quinet <raphael@picman.org>",
                          "Raphaël Quinet <raphael@picman.org>",
                          "2005",
                          NULL,
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (set_simple_args), 0,
                          set_simple_args, NULL);

  picman_install_procedure (IMPORT_PROC,
                          "Import XMP from a file into the current image",
                          "Load an XMP packet from a file and import it into "
                          "the current image.  This can be used to add a "
                          "license statement or some other predefined "
                          "metadata to an image",
                          "Raphaël Quinet <raphael@picman.org>",
                          "Raphaël Quinet <raphael@picman.org>",
                          "2005",
                          NULL,
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (import_args), 0,
                          import_args, NULL);

  picman_install_procedure (EXPORT_PROC,
                          "Export XMP from the current image to a file",
                          "Export the metadata associated with the current "
                          "image into a file.  The metadata will be saved as "
                          "an XMP packet.  If overwrite is TRUE, then any "
                          "existing file will be overwritten without warning. "
                          "If overwrite is FALSE, then an error will occur if "
                          "the file already exists.",
                          "Raphaël Quinet <raphael@picman.org>",
                          "Raphaël Quinet <raphael@picman.org>",
                          "2005",
                          NULL,
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (export_args), 0,
                          export_args, NULL);
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[4];
  gint32             image_ID;
  XMPModel          *xmp_model;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  PicmanParasite      *parasite = NULL;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

  INIT_I18N();
  g_type_init();

  if (! strcmp (name, EDITOR_PROC))
    image_ID = param[1].data.d_image;
  else
    image_ID = param[0].data.d_image;

  xmp_model = xmp_model_new ();

  /* if there is already a metadata parasite, load it */
  parasite = picman_image_get_parasite (image_ID, METADATA_PARASITE);
  if (parasite)
    {
      GError *error = NULL;

      if (!! strncmp (picman_parasite_data (parasite),
                      METADATA_MARKER, METADATA_MARKER_LEN)
          || ! xmp_model_parse_buffer (xmp_model,
                                       (const gchar *) picman_parasite_data (parasite)
                                       + METADATA_MARKER_LEN,
                                       picman_parasite_data_size (parasite)
                                       - METADATA_MARKER_LEN,
                                       TRUE, &error))
        {
          g_printerr ("\nMetadata parasite seems to be corrupt\n");
          /* continue anyway, we will attach a clean parasite later */
        }
      picman_parasite_free (parasite);
    }

  /* If we have no metadata yet, try to find an XMP packet in the file
   * (but ignore errors if nothing is found).
   *
   * FIXME: This is a workaround until all file plug-ins do the right
   * thing when loading their files.
   */
  if (xmp_model_is_empty (xmp_model)
      && !! strcmp (name, DECODE_XMP_PROC))
    {
      const gchar *filename;
      GError      *error = NULL;

      filename = picman_image_get_filename (image_ID);
      if (filename != NULL)
        if (xmp_model_parse_file (xmp_model, filename, &error))
          /* g_message ("XMP loaded from file '%s'\n", filename) */;
    }

  /* Now check what we are supposed to do */
  if (! strcmp (name, DECODE_XMP_PROC))
    {
      const gchar *buffer;
      GError      *error = NULL;

      buffer = param[1].data.d_string;
      if (! xmp_model_parse_buffer (xmp_model, buffer, strlen (buffer),
                                    FALSE, &error))
        status = PICMAN_PDB_EXECUTION_ERROR;

    }
  else if (! strcmp (name, ENCODE_XMP_PROC))
    {
      /* done below together with the parasite */
    }
  else if (! strcmp (name, DECODE_EXIF_PROC))
    {
        GError      *error         = NULL;

        if (! xmp_merge_from_exifbuffer (xmp_model, image_ID, &error))
          {
            status = PICMAN_PDB_EXECUTION_ERROR;
            g_printerr ("\nExif to XMP merge failed.\n");
          }
    }
  else if (! strcmp (name, GET_PROC))
    {
      g_printerr ("Not implemented yet (GET_PROC)\n"); /* FIXME */
      status = PICMAN_PDB_EXECUTION_ERROR;
    }
  else if (! strcmp (name, SET_PROC))
    {
      g_printerr ("Not implemented yet (SET_PROC)\n"); /* FIXME */
      status = PICMAN_PDB_EXECUTION_ERROR;
    }
  else if (! strcmp (name, GET_SIMPLE_PROC))
    {
      const gchar *schema_name;
      const gchar *property_name;
      const gchar *value;

      schema_name = param[1].data.d_string;
      property_name = param[2].data.d_string;
      value = xmp_model_get_scalar_property (xmp_model, schema_name,
                                             property_name);
      if (value)
        {
          *nreturn_vals = 2;
          values[1].type = PICMAN_PDB_STRING;
          values[1].data.d_string = g_strdup (value);
        }
      else
        status = PICMAN_PDB_EXECUTION_ERROR;
    }
  else if (! strcmp (name, SET_SIMPLE_PROC))
    {
      const gchar *schema_name;
      const gchar *property_name;
      const gchar *property_value;

      schema_name = param[1].data.d_string;
      property_name = param[2].data.d_string;
      property_value = param[3].data.d_string;
      if (! xmp_model_set_scalar_property (xmp_model, schema_name,
                                           property_name, property_value))
        status = PICMAN_PDB_EXECUTION_ERROR;
    }
  else if (! strcmp (name, IMPORT_PROC))
    {
      const gchar *filename;
      gchar       *buffer;
      gsize        buffer_length;
      GError      *error = NULL;

      filename = param[1].data.d_string;
      if (! g_file_get_contents (filename, &buffer, &buffer_length, &error))
        {
          g_error_free (error);
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
      else if (! xmp_model_parse_buffer (xmp_model, buffer, buffer_length,
                                         TRUE, &error))
        {
          g_error_free (error);
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
      g_free (buffer);
    }
  else if (! strcmp (name, EXPORT_PROC))
    {
      /* FIXME: this is easy to implement, but the first thing to do is */
      /* to improve the code of export_dialog_response() in interface.c */
      g_printerr ("Not implemented yet (EXPORT_PROC)\n");
      status = PICMAN_PDB_EXECUTION_ERROR;
    }
  else if (! strcmp (name, EDITOR_PROC))
    {
      PicmanRunMode run_mode;

      run_mode = param[0].data.d_int32;
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        {
          if (! metadata_dialog (image_ID, xmp_model))
            status = PICMAN_PDB_CANCEL;
        }

      g_printerr ("Not implemented yet (EDITOR_PROC)\n");
      status = PICMAN_PDB_EXECUTION_ERROR;
    }
  else
    {
      status = PICMAN_PDB_CALLING_ERROR;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      GString *buffer;

      /* Generate the updated parasite and attach it to the image */
      buffer = g_string_new (METADATA_MARKER);
      xmp_generate_packet (xmp_model, buffer);
      parasite = picman_parasite_new (METADATA_PARASITE,
                                    PICMAN_PARASITE_PERSISTENT,
                                    buffer->len,
                                    (gpointer) buffer->str);
      picman_image_attach_parasite (image_ID, parasite);
      if (! strcmp (name, ENCODE_XMP_PROC))
        {
          *nreturn_vals = 2;
          values[1].type = PICMAN_PDB_STRING;
          values[1].data.d_string = g_strdup (buffer->str
                                              + METADATA_MARKER_LEN);
        }
      g_string_free (buffer, TRUE);
    }

  g_object_unref (xmp_model);

  values[0].data.d_status = status;
}
