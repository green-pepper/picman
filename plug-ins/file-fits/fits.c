/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * FITS file plugin
 * reading and writing code Copyright (C) 1997 Peter Kirchgessner
 * e-mail: peter@kirchgessner.net, WWW: http://www.kirchgessner.net
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

/* Event history:
 * V 1.00, PK, 05-May-97: Creation
 * V 1.01, PK, 19-May-97: Problem with compilation on Irix fixed
 * V 1.02, PK, 08-Jun-97: Bug with saving gray images fixed
 * V 1.03, PK, 05-Oct-97: Parse rc-file
 * V 1.04, PK, 12-Oct-97: No progress bars for non-interactive mode
 * V 1.05, nn, 20-Dec-97: Initialize image_ID in run()
 * V 1.06, PK, 21-Nov-99: Internationalization
 *                        Fix bug with picman_export_image()
 *                        (moved it from load to save)
 * V 1.07, PK, 16-Aug-06: Fix problems with internationalization
 *                        (writing 255,0 instead of 255.0)
 *                        Fix problem with not filling up properly last record
 */

#include "config.h"

#include <string.h>
#include <errno.h>

#include <glib/gstdio.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "fits-io.h"

#include "libpicman/stdplugins-intl.h"


#define LOAD_PROC      "file-fits-load"
#define SAVE_PROC      "file-fits-save"
#define PLUG_IN_BINARY "file-fits"
#define PLUG_IN_ROLE   "picman-file-fits"


/* Load info */
typedef struct
{
  gint replace;     /* replacement for blank/NaN-values    */
  gint use_datamin; /* Use DATAMIN/MAX-scaling if possible */
  gint compose;     /* compose images with naxis==3        */
} FITSLoadVals;


/* Declare some local functions.
 */
static void          query              (void);
static void          run                (const gchar        *name,
                                         gint                nparams,
                                         const PicmanParam    *param,
                                         gint               *nreturn_vals,
                                         PicmanParam         **return_vals);

static gint32        load_image         (const gchar        *filename,
                                         GError            **error);
static gint          save_image         (const gchar        *filename,
                                         gint32              image_ID,
                                         gint32              drawable_ID,
                                         GError            **error);

static FitsHduList * create_fits_header (FitsFile           *ofp,
                                         guint               width,
                                         guint               height,
                                         guint               channels,
                                         guint               bitpix);

static gint          save_fits        (FitsFile           *ofp,
                                       gint32              image_ID,
                                       gint32              drawable_ID);

static gint32        create_new_image   (const gchar        *filename,
                                         guint               pagenum,
                                         guint               width,
                                         guint               height,
                                         PicmanImageBaseType   itype,
                                         PicmanImageType       dtype,
                                         PicmanPrecision       iprecision,
                                         gint32             *layer_ID,
                                         GeglBuffer        **buffer);

static void          check_load_vals    (void);

static gint32        load_fits          (const gchar        *filename,
                                         FitsFile           *ifp,
                                         guint               picnum,
                                         guint               ncompose);

static gboolean      load_dialog        (void);
static void          show_fits_errors   (void);


static FITSLoadVals plvals =
{
  0,        /* Replace with black */
  0,        /* Do autoscale on pixel-values */
  0         /* Dont compose images */
};

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

/* The run mode */
static PicmanRunMode l_run_mode;


MAIN ()

static void
query (void)

{
  static const PicmanParamDef load_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_STRING,   "filename",     "The name of the file to load" },
    { PICMAN_PDB_STRING,   "raw-filename", "The name of the file to load" },
  };
  static const PicmanParamDef load_return_vals[] =
  {
    { PICMAN_PDB_IMAGE,    "image",        "Output image" },
  };

  static const PicmanParamDef save_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",        "Input image" },
    { PICMAN_PDB_DRAWABLE, "drawable",     "Drawable to save" },
    { PICMAN_PDB_STRING,   "filename",     "The name of the file to save the image in" },
    { PICMAN_PDB_STRING,   "raw-filename", "The name of the file to save the image in" },
  };

  picman_install_procedure (LOAD_PROC,
                          "load file of the FITS file format",
                          "load file of the FITS file format "
                          "(Flexible Image Transport System)",
                          "Peter Kirchgessner",
                          "Peter Kirchgessner (peter@kirchgessner.net)",
                          "1997",
                          N_("Flexible Image Transport System"),
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (load_args),
                          G_N_ELEMENTS (load_return_vals),
                          load_args, load_return_vals);

  picman_register_file_handler_mime (LOAD_PROC, "image/x-fits");
  picman_register_magic_load_handler (LOAD_PROC,
                                    "fit,fits",
                                    "",
                                    "0,string,SIMPLE");

  picman_install_procedure (SAVE_PROC,
                          "save file in the FITS file format",
                          "FITS saving handles all image types except "
                          "those with alpha channels.",
                          "Peter Kirchgessner",
                          "Peter Kirchgessner (peter@kirchgessner.net)",
                          "1997",
                          N_("Flexible Image Transport System"),
                          "RGB, GRAY, INDEXED",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (save_args), 0,
                          save_args, NULL);

  picman_register_file_handler_mime (SAVE_PROC, "image/x-fits");
  picman_register_save_handler (SAVE_PROC, "fit,fits", "");
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

  l_run_mode = run_mode = (PicmanRunMode)param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

  if (strcmp (name, LOAD_PROC) == 0)
    {
      switch (run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
          /*  Possibly retrieve data  */
          picman_get_data (LOAD_PROC, &plvals);

          if (!load_dialog ())
            status = PICMAN_PDB_CANCEL;
          break;

        case PICMAN_RUN_NONINTERACTIVE:
          if (nparams != 3)
            status = PICMAN_PDB_CALLING_ERROR;
          break;

        case PICMAN_RUN_WITH_LAST_VALS:
          /* Possibly retrieve data */
          picman_get_data (LOAD_PROC, &plvals);
          break;

        default:
          break;
        }

      if (status == PICMAN_PDB_SUCCESS)
        {
          check_load_vals ();
          image_ID = load_image (param[1].data.d_string, &error);

          /* Write out error messages of FITS-Library */
          show_fits_errors ();

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

          /*  Store plvals data  */
          if (status == PICMAN_PDB_SUCCESS)
            picman_set_data (LOAD_PROC, &plvals, sizeof (FITSLoadVals));
        }
    }
  else if (strcmp (name, SAVE_PROC) == 0)
    {
      image_ID = param[1].data.d_int32;
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
                                       PICMAN_EXPORT_CAN_HANDLE_INDEXED));
        if (export == PICMAN_EXPORT_CANCEL)
          {
            values[0].data.d_status = PICMAN_PDB_CANCEL;
            return;
          }
        break;
      default:
        break;
      }

      switch (run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
          break;

        case PICMAN_RUN_NONINTERACTIVE:
          /*  Make sure all the arguments are there!  */
          if (nparams != 5)
            status = PICMAN_PDB_CALLING_ERROR;
          break;

        case PICMAN_RUN_WITH_LAST_VALS:
          break;

        default:
          break;
        }

      if (status == PICMAN_PDB_SUCCESS)
        {
          if (! save_image (param[3].data.d_string, image_ID, drawable_ID,
                            &error))
            status = PICMAN_PDB_EXECUTION_ERROR;
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


static gint32
load_image (const gchar  *filename,
            GError      **error)
{
  gint32       image_ID, *image_list, *nl;
  guint        picnum;
  gint         k, n_images, max_images, hdu_picnum;
  gint         compose;
  FILE        *fp;
  FitsFile    *ifp;
  FitsHduList *hdu;

  fp = g_fopen (filename, "rb");
  if (!fp)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return -1;
    }
  fclose (fp);

  ifp = fits_open (filename, "r");
  if (ifp == NULL)
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   "%s", _("Error during open of FITS file"));
      return -1;
    }
  if (ifp->n_pic <= 0)
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   "%s", _("FITS file keeps no displayable images"));
      fits_close (ifp);
      return -1;
    }

  image_list = g_new (gint32, 10);
  n_images = 0;
  max_images = 10;

  for (picnum = 1; picnum <= ifp->n_pic; )
    {
      /* Get image info to see if we can compose them */
      hdu = fits_image_info (ifp, picnum, &hdu_picnum);
      if (hdu == NULL) break;

      /* Get number of FITS-images to compose */
      compose = (   plvals.compose && (hdu_picnum == 1) && (hdu->naxis == 3)
                    && (hdu->naxisn[2] > 1) && (hdu->naxisn[2] <= 4));
      if (compose)
        compose = hdu->naxisn[2];
      else
        compose = 1;  /* Load as GRAY */

      image_ID = load_fits (filename, ifp, picnum, compose);

      /* Write out error messages of FITS-Library */
      show_fits_errors ();

      if (image_ID == -1) break;
      if (n_images == max_images)
        {
          nl = (gint32 *)g_realloc (image_list, (max_images+10)*sizeof (gint32));
          if (nl == NULL) break;
          image_list = nl;
          max_images += 10;
        }
      image_list[n_images++] = image_ID;

      picnum += compose;
    }

  /* Write out error messages of FITS-Library */
  show_fits_errors ();

  fits_close (ifp);

  /* Display images in reverse order. The last will be displayed by PICMAN itself*/
  if (l_run_mode != PICMAN_RUN_NONINTERACTIVE)
    {
      for (k = n_images-1; k >= 1; k--)
        {
          picman_image_undo_enable (image_list[k]);
          picman_image_clean_all (image_list[k]);
          picman_display_new (image_list[k]);
        }
    }

  image_ID = (n_images > 0) ? image_list[0] : -1;
  g_free (image_list);

  return (image_ID);
}


static gint
save_image (const gchar  *filename,
            gint32        image_ID,
            gint32        drawable_ID,
            GError      **error)
{
  FitsFile      *ofp;
  PicmanImageType  drawable_type;
  gint           retval;

  drawable_type = picman_drawable_type (drawable_ID);

  /*  Make sure we're not saving an image with an alpha channel  */
  if (picman_drawable_has_alpha (drawable_ID))
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   "%s",
                   _("FITS save cannot handle images with alpha channels"));
      return FALSE;
    }

  switch (drawable_type)
    {
    case PICMAN_INDEXED_IMAGE: case PICMAN_INDEXEDA_IMAGE:
    case PICMAN_GRAY_IMAGE:    case PICMAN_GRAYA_IMAGE:
    case PICMAN_RGB_IMAGE:     case PICMAN_RGBA_IMAGE:
      break;
    default:
      g_message (_("Cannot operate on unknown image types."));
      return (FALSE);
      break;
    }

  /* Open the output file. */
  ofp = fits_open (filename, "w");
  if (!ofp)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for writing: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return (FALSE);
    }

  picman_progress_init_printf (_("Saving '%s'"),
                             picman_filename_to_utf8 (filename));

  retval = save_fits (ofp,image_ID, drawable_ID);

  fits_close (ofp);

  return (retval);
}


/* Check (and correct) the load values plvals */
static void
check_load_vals (void)
{
  if (plvals.replace > 255) plvals.replace = 255;
}


/* Create an image. Sets layer_ID, drawable and rgn. Returns image_ID */
static gint32
create_new_image (const gchar        *filename,
                  guint               pagenum,
                  guint               width,
                  guint               height,
                  PicmanImageBaseType   itype,
                  PicmanImageType       dtype,
                  PicmanPrecision       iprecision,
                  gint32             *layer_ID,
                  GeglBuffer        **buffer)
{
  gint32  image_ID;
  char   *tmp;

  image_ID = picman_image_new_with_precision (width, height, itype, iprecision);

  if ((tmp = g_malloc (strlen (filename) + 64)) != NULL)
    {
      sprintf (tmp, "%s-img%ld", filename, (long)pagenum);
      picman_image_set_filename (image_ID, tmp);
      g_free (tmp);
    }
  else
    picman_image_set_filename (image_ID, filename);

  picman_image_undo_disable (image_ID);
  *layer_ID = picman_layer_new (image_ID, _("Background"), width, height,
                              dtype, 100, PICMAN_NORMAL_MODE);
  picman_image_insert_layer (image_ID, *layer_ID, -1, 0);

  *buffer = picman_drawable_get_buffer (*layer_ID);

  return image_ID;
}


/* Load FITS image. ncompose gives the number of FITS-images which have
 * to be composed together. This will result in different PICMAN image types:
 * 1: GRAY, 2: GRAYA, 3: RGB, 4: RGBA
 */
static gint32
load_fits (const gchar *filename,
           FitsFile    *ifp,
           guint        picnum,
           guint        ncompose)
{
  register guchar   *dest, *src;
  guchar            *data, *data_end, *linebuf;
  int                width, height, tile_height, scan_lines;
  int                i, j, max_scan;
  double             a, b;
  gint32             layer_ID, image_ID;
  GeglBuffer        *buffer;
  PicmanImageBaseType  itype;
  PicmanImageType      dtype;
  PicmanPrecision      iprecision;
  gint               err = 0;
  FitsHduList       *hdulist;
  FitsPixTransform   trans;
  double             datamax, replacetransform;
  const Babl        *type, *format;

  hdulist = fits_seek_image (ifp, (int)picnum);
  if (hdulist == NULL)
    return -1;

  width  = hdulist->naxisn[0];  /* Set the size of the FITS image */
  height = hdulist->naxisn[1];

  switch (hdulist->bitpix)
    {
    case 8:
      iprecision = PICMAN_PRECISION_U8;
      type = babl_type ("u8");
      datamax = 255.0;
      replacetransform = 1.0;
      break;
    case 16:
      iprecision = PICMAN_PRECISION_U16;
      type = babl_type ("u16");
      datamax = 65535.0;
      replacetransform = 257;
      break;
    case 32:
      iprecision = PICMAN_PRECISION_U32;
      type = babl_type ("u32");
      datamax = 4294967295.0;
      replacetransform = 16843009;
      break;
    case -32:
      iprecision = PICMAN_PRECISION_FLOAT;
      type = babl_type ("float");
      datamax = 1.0;
      replacetransform = 1.0 / 255.0;
      break;
    case -64:
      iprecision = PICMAN_PRECISION_FLOAT;
      type = babl_type ("double");
      datamax = 1.0;
      replacetransform = 1.0 / 255.0;
      break;
    default:
      return -1;
    }

  if (ncompose == 2)
    {
      itype = PICMAN_GRAY;
      dtype = PICMAN_GRAYA_IMAGE;
      format = babl_format_new (babl_model ("Y'A"),
                                type,
                                babl_component ("Y'"),
                                babl_component ("A"),
                                NULL);
    }
  else if (ncompose == 3)
    {
      itype = PICMAN_RGB;
      dtype = PICMAN_RGB_IMAGE;
      format = babl_format_new (babl_model ("R'G'B'"),
                                type,
                                babl_component ("R'"),
                                babl_component ("G'"),
                                babl_component ("B'"),
                                NULL);
    }
  else if (ncompose == 4)
    {
      itype = PICMAN_RGB;
      dtype = PICMAN_RGBA_IMAGE;
      format = babl_format_new (babl_model ("R'G'B'A"),
                                type,
                                babl_component ("R'"),
                                babl_component ("G'"),
                                babl_component ("B'"),
                                babl_component ("A"),
                                NULL);
    }
  else
    {
      ncompose = 1;
      itype = PICMAN_GRAY;
      dtype = PICMAN_GRAY_IMAGE;
      format = babl_format_new (babl_model ("Y'"),
                                type,
                                babl_component ("Y'"),
                                NULL);
    }

  image_ID = create_new_image (filename, picnum, width, height, itype, dtype, iprecision,
                               &layer_ID, &buffer);

  tile_height = picman_tile_height ();

  data = g_malloc (tile_height * width * ncompose * hdulist->bpp);
  if (data == NULL)
    return -1;

  data_end = data + tile_height * width * ncompose * hdulist->bpp;

  /* If the transformation from pixel value to data value has been
   * specified, use it
   */
  if (plvals.use_datamin    &&
      hdulist->used.datamin && hdulist->used.datamax &&
      hdulist->used.bzero   && hdulist->used.bscale)
    {
      a = (hdulist->datamin - hdulist->bzero) / hdulist->bscale;
      b = (hdulist->datamax - hdulist->bzero) / hdulist->bscale;

      if (a < b)
        trans.pixmin = a, trans.pixmax = b;
      else
        trans.pixmin = b, trans.pixmax = a;
    }
  else
    {
      trans.pixmin = hdulist->pixmin;
      trans.pixmax = hdulist->pixmax;
    }

  trans.datamin     = 0.0;
  trans.datamax     = datamax;
  trans.replacement = plvals.replace * replacetransform;
  trans.dsttyp      = 'k';

  /* FITS stores images with bottom row first. Therefore we have to
   * fill the image from bottom to top.
   */

  if (ncompose == 1)
    {
      dest = data + tile_height * width * hdulist->bpp;
      scan_lines = 0;

      for (i = 0; i < height; i++)
        {
          /* Read FITS line */
          dest -= width * hdulist->bpp;
          if (fits_read_pixel (ifp, hdulist, width, &trans, dest) != width)
            {
              err = 1;
              break;
            }

          scan_lines++;

          if ((i % 20) == 0)
            picman_progress_update ((gdouble) (i + 1) / (gdouble) height);

          if ((scan_lines == tile_height) || ((i + 1) == height))
            {
              gegl_buffer_set (buffer,
                               GEGL_RECTANGLE (0, height - i - 1,
                                               width, scan_lines), 0,
                               format, dest, GEGL_AUTO_ROWSTRIDE);

              scan_lines = 0;
              dest = data + tile_height * width * hdulist->bpp;
            }

          if (err)
            break;
        }
    }
  else   /* multiple images to compose */
    {
      gint channel;

      linebuf = g_malloc (width * hdulist->bpp);
      if (linebuf == NULL)
        return -1;

      for (channel = 0; channel < ncompose; channel++)
        {
          dest = data + tile_height * width * hdulist->bpp * ncompose + channel * hdulist->bpp;
          scan_lines = 0;

          for (i = 0; i < height; i++)
            {
              if ((channel > 0) && ((i % tile_height) == 0))
                {
                  /* Reload a region for follow up channels */
                  max_scan = tile_height;

                  if (i + tile_height > height)
                    max_scan = height - i;

                  gegl_buffer_get (buffer,
                                   GEGL_RECTANGLE (0, height - i - max_scan,
                                                   width, max_scan), 1.0,
                                   format, data_end - max_scan * width * hdulist->bpp * ncompose,
                                   GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
                }

              /* Read FITS scanline */
              dest -= width * ncompose * hdulist->bpp;
              if (fits_read_pixel (ifp, hdulist, width, &trans, linebuf) != width)
                {
                  err = 1;
                  break;
                }
              j = width;
              src = linebuf;
              while (j--)
                {
                  memcpy (dest, src, hdulist->bpp);
                  src += hdulist->bpp;
                  dest += ncompose * hdulist->bpp;
                }
              dest -= width * ncompose * hdulist->bpp;
              scan_lines++;

              if ((i % 20) == 0)
                picman_progress_update ((gdouble) (channel * height + i + 1) /
                                      (gdouble) (height * ncompose));

              if ((scan_lines == tile_height) || ((i + 1) == height))
                {
                  gegl_buffer_set (buffer,
                                   GEGL_RECTANGLE (0, height - i - 1,
                                                   width, scan_lines), 0,
                                   format, dest - channel * hdulist->bpp, GEGL_AUTO_ROWSTRIDE);

                  scan_lines = 0;
                  dest = data + tile_height * width * ncompose * hdulist->bpp + channel * hdulist->bpp;
                }

              if (err)
                break;
            }
        }

      g_free (linebuf);
    }

  g_free (data);

  if (err)
    g_message (_("EOF encountered on reading"));

  g_object_unref (buffer);

  picman_progress_update (1.0);

  return err ? -1 : image_ID;
}


static FitsHduList *
create_fits_header (FitsFile *ofp,
                    guint     width,
                    guint     height,
                    guint     channels,
                    guint     bitpix)
{
  FitsHduList *hdulist;
  gint         print_ctype3 = 0; /* The CTYPE3-card may not be FITS-conforming */

  static const char *ctype3_card[] =
  {
    NULL, NULL, NULL,  /* bpp = 0: no additional card */
    "COMMENT Image type within PICMAN: PICMAN_GRAY_IMAGE",
    NULL,
    NULL,
    "COMMENT Image type within PICMAN: PICMAN_GRAYA_IMAGE (gray with alpha channel)",
    "COMMENT Sequence for NAXIS3   : GRAY, ALPHA",
    "CTYPE3  = 'GRAYA   '           / GRAY IMAGE WITH ALPHA CHANNEL",
    "COMMENT Image type within PICMAN: PICMAN_RGB_IMAGE",
    "COMMENT Sequence for NAXIS3   : RED, GREEN, BLUE",
    "CTYPE3  = 'RGB     '           / RGB IMAGE",
    "COMMENT Image type within PICMAN: PICMAN_RGBA_IMAGE (rgb with alpha channel)",
    "COMMENT Sequence for NAXIS3   : RED, GREEN, BLUE, ALPHA",
    "CTYPE3  = 'RGBA    '           / RGB IMAGE WITH ALPHA CHANNEL"
  };

  hdulist = fits_add_hdu (ofp);
  if (hdulist == NULL)
    return NULL;

  hdulist->used.simple  = 1;
  hdulist->bitpix       = bitpix;
  hdulist->naxis        = (channels == 1) ? 2 : 3;
  hdulist->naxisn[0]    = width;
  hdulist->naxisn[1]    = height;
  hdulist->naxisn[2]    = channels;
  hdulist->used.datamin = 1;
  hdulist->datamin      = 0.0;
  hdulist->used.datamax = 1;
  hdulist->used.bzero   = 1;
  hdulist->bzero        = 0.0;
  hdulist->used.bscale  = 1;
  hdulist->bscale       = 1.0;

  switch (bitpix)
    {
    case 8:
      hdulist->datamax = 255;
      break;
    case 16:
      hdulist->datamax = 65535;
      break;
    case 32:
      hdulist->datamax = 4294967295;
      break;
    case -32:
      hdulist->datamax = 1.0;
      break;
    case -64:
      hdulist->datamax = 1.0;
    default:
      return NULL;
    }

  fits_add_card (hdulist, "");
  fits_add_card (hdulist,
                 "HISTORY THIS FITS FILE WAS GENERATED BY PICMAN USING FITSRW");
  fits_add_card (hdulist, "");
  fits_add_card (hdulist,
                 "COMMENT FitsRW is (C) Peter Kirchgessner (peter@kirchgessner.net), but available");
  fits_add_card (hdulist,
                 "COMMENT under the GNU general public licence.");
  fits_add_card (hdulist,
                 "COMMENT For sources see http://www.kirchgessner.net");
  fits_add_card (hdulist, "");
  fits_add_card (hdulist, ctype3_card[channels * 3]);

  if (ctype3_card[channels * 3 + 1] != NULL)
    fits_add_card (hdulist, ctype3_card[channels * 3 + 1]);

  if (print_ctype3 && (ctype3_card[channels * 3 + 2] != NULL))
    fits_add_card (hdulist, ctype3_card[channels * 3 + 2]);

  fits_add_card (hdulist, "");

  return hdulist;
}


/* Save direct colours (GRAY, GRAYA, RGB, RGBA) */
static gint
save_fits (FitsFile *ofp,
           gint32    image_ID,
           gint32    drawable_ID)
{
  gint           height, width, i, j, channel, channelnum;
  gint           tile_height, bpp, bpsl, bitpix, bpc;
  long           nbytes;
  guchar        *data, *src;
  GeglBuffer    *buffer;
  const Babl    *format, *type;
  FitsHduList   *hdu;

  buffer = picman_drawable_get_buffer (drawable_ID);

  width  = gegl_buffer_get_width  (buffer);
  height = gegl_buffer_get_height (buffer);

  format = gegl_buffer_get_format (buffer);
  type   = babl_format_get_type (format, 0);

  if (type == babl_type ("u8"))
    {
      bitpix = 8;
    }
  else if (type == babl_type ("u16"))
    {
      bitpix = 16;
    }
  else if (type == babl_type ("u32"))
    {
      bitpix = 32;
    }
  else if (type == babl_type ("float"))
    {
      bitpix = -32;
    }
  else if (type == babl_type ("half"))
    {
      bitpix = -32;
      type = babl_type ("float");
    }
  else
    {
      return FALSE;
    }

  switch (picman_drawable_type (drawable_ID))
    {
    case PICMAN_GRAY_IMAGE:
      format = babl_format_new (babl_model ("Y'"),
                                type,
                                babl_component ("Y'"),
                                NULL);
      break;

    case PICMAN_GRAYA_IMAGE:
      format = babl_format_new (babl_model ("Y'A"),
                                type,
                                babl_component ("Y'"),
                                babl_component ("A"),
                                NULL);
      break;

    case PICMAN_RGB_IMAGE:
    case PICMAN_INDEXED_IMAGE:
      format = babl_format_new (babl_model ("R'G'B'"),
                                type,
                                babl_component ("R'"),
                                babl_component ("G'"),
                                babl_component ("B'"),
                                NULL);
      break;

    case PICMAN_RGBA_IMAGE:
    case PICMAN_INDEXEDA_IMAGE:
      format = babl_format_new (babl_model ("R'G'B'A"),
                                type,
                                babl_component ("R'"),
                                babl_component ("G'"),
                                babl_component ("B'"),
                                babl_component ("A"),
                                NULL);
      break;
    }

  channelnum = babl_format_get_n_components (format);
  bpp        = babl_format_get_bytes_per_pixel (format);

  bpc  = bpp / channelnum; /* Bytes per channel */
  bpsl = width * bpp;      /* Bytes per scanline */

  tile_height = picman_tile_height ();

  /* allocate a buffer for retrieving information from the pixel region  */
  src = data = (guchar *) g_malloc (width * height * bpp);

  hdu = create_fits_header (ofp, width, height, channelnum, bitpix);
  if (hdu == NULL)
    return FALSE;

  if (fits_write_header (ofp, hdu) < 0)
    return FALSE;

  nbytes = 0;
  for (channel = 0; channel < channelnum; channel++)
    {
      for (i = 0; i < height; i++)
        {
          if ((i % tile_height) == 0)
            {
              gint scan_lines;

              scan_lines = (i + tile_height-1 < height) ?
                            tile_height : (height - i);

              gegl_buffer_get (buffer,
                               GEGL_RECTANGLE (0, height - i - scan_lines,
                                               width, scan_lines), 1.0,
                               format, data,
                               GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

              src = data + bpsl * (scan_lines - 1) + channel * bpc;
            }

          if (channelnum == 1 && bitpix == 8)  /* One channel and 8 bit? Write the scanline */
            {
              fwrite (src, bpc, width, ofp->fp);
              src += bpsl;
            }
          else           /* Multiple channels or high bit depth */
            {
            /* Write out bytes for current channel */
            /* FIXME: Don't assume a little endian arch */
            switch (bitpix)
              {
              case 8:
                for (j = 0; j < width; j++)
                  {
                    putc (*src, ofp->fp);
                    src += bpp;
                  }
                break;
              case 16:
                for (j = 0; j < width; j++)
                  {
                    *((guint16*)src) += 32768;
                    putc (*(src + 1), ofp->fp);
                    putc (*(src + 0), ofp->fp);
                    src += bpp;
                  }
                break;
              case 32:
                for (j = 0; j < width; j++)
                  {
                    *((guint32*)src) += 2147483648;
                    putc (*(src + 3), ofp->fp);
                    putc (*(src + 2), ofp->fp);
                    putc (*(src + 1), ofp->fp);
                    putc (*(src + 0), ofp->fp);
                    src += bpp;
                  }
                break;
              case -32:
                for (j = 0; j < width; j++)
                  {
                    putc (*(src + 3), ofp->fp);
                    putc (*(src + 2), ofp->fp);
                    putc (*(src + 1), ofp->fp);
                    putc (*(src + 0), ofp->fp);
                    src += bpp;
                  }
                break;
              default:
                return FALSE;
              }
            }

          nbytes += width * bpc;
          src -= 2 * bpsl;

          if ((i % 20) == 0)
            picman_progress_update ((gdouble) (i + channel * height) /
                                  (gdouble) (height * channelnum));
        }
    }

  nbytes = nbytes % FITS_RECORD_SIZE;
  if (nbytes)
    {
      while (nbytes++ < FITS_RECORD_SIZE)
        putc (0, ofp->fp);
    }

  g_free (data);

  g_object_unref (buffer);

  picman_progress_update (1.0);

  if (ferror (ofp->fp))
    {
      g_message (_("Write error occurred"));
      return FALSE;
    }

  return TRUE;
}


/*  Load interface functions  */

static gboolean
load_dialog (void)
{
  GtkWidget *dialog;
  GtkWidget *vbox;
  GtkWidget *frame;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Load FITS File"), PLUG_IN_ROLE,
                            NULL, 0,
                            picman_standard_help_func, LOAD_PROC,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OPEN,   GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  picman_window_set_transient (GTK_WINDOW (dialog));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  frame = picman_int_radio_group_new (TRUE, _("Replacement for undefined pixels"),
                                    G_CALLBACK (picman_radio_button_update),
                                    &plvals.replace, plvals.replace,

                                    _("Black"), 0,   NULL,
                                    _("White"), 255, NULL,

                                    NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  frame =
    picman_int_radio_group_new (TRUE, _("Pixel value scaling"),
                              G_CALLBACK (picman_radio_button_update),
                              &plvals.use_datamin, plvals.use_datamin,

                              _("Automatic"),          FALSE, NULL,
                              _("By DATAMIN/DATAMAX"), TRUE,  NULL,

                              NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  frame =
    picman_int_radio_group_new (TRUE, _("Image Composing"),
                              G_CALLBACK (picman_radio_button_update),
                              &plvals.compose, plvals.compose,

                              C_("composing", "None"),   FALSE, NULL,
                              "NAXIS=3, NAXIS3=2,...,4", TRUE,  NULL,

                              NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
show_fits_errors (void)
{
  const gchar *msg;

  /* Write out error messages of FITS-Library */
  while ((msg = fits_get_error ()) != NULL)
    g_message ("%s", msg);
}
