/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Decompose plug-in (C) 1997 Peter Kirchgessner
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
 */

/*
 * This filter decomposes RGB-images into several types of channels
 */

/*  Lab colorspace support originally written by Alexey Dyachenko,
 *  merged into the officical plug-in by Sven Neumann.
 */


#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"

/* cbrt() is a GNU extension, which C99 accepted */
#if !defined (__GLIBC__) && !(defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define cbrt(x) (pow(x, 1.0/3.0))
#endif


#define PLUG_IN_PROC      "plug-in-decompose"
#define PLUG_IN_PROC_REG  "plug-in-decompose-registered"
#define PLUG_IN_BINARY    "decompose"
#define PLUG_IN_ROLE      "picman-decompose"


/* Declare local functions
 */
static void    query            (void);
static void    run              (const gchar        *name,
                                 gint                nparams,
                                 const PicmanParam    *param,
                                 gint               *nreturn_vals,
                                 PicmanParam         **return_vals);

static gint32  decompose        (gint32              image_id,
                                 gint32              drawable_ID,
                                 const gchar        *extract_type,
                                 gint32             *image_ID_dst,
                                 gint32             *num_layers,
                                 gint32             *layer_ID_dst);
static gint32  create_new_image (const gchar        *filename,
                                 const gchar        *layername,
                                 guint               width,
                                 guint               height,
                                 PicmanImageBaseType   type,
                                 gdouble             xres,
                                 gdouble             yres,
                                 gint32             *layer_ID,
                                 PicmanDrawable      **drawable,
                                 PicmanPixelRgn       *pixel_rgn);
static gint32  create_new_layer (gint32              image_ID,
                                 gint                position,
                                 const gchar        *layername,
                                 guint               width,
                                 guint               height,
                                 PicmanImageBaseType   type,
                                 PicmanDrawable      **drawable,
                                 PicmanPixelRgn       *pixel_rgn);

static void transfer_registration_color (const guchar *src,
                                         gint bpp, gint numpix, guchar **dst,
                                         gint num_channels);

static void extract_rgb      (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_red      (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_green    (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_blue     (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_rgba     (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_alpha    (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_hsv      (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_hsl      (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_hue      (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_sat      (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_val      (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_huel     (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_satl     (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_lightness     (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_cmy      (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_cyan     (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_magenta  (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_yellow   (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_cmyk     (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_cyank    (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_magentak (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_yellowk  (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_lab      (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_ycbcr470 (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_ycbcr709 (const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_ycbcr470f(const guchar *src,
                              gint bpp, gint numpix, guchar **dst);
static void extract_ycbcr709f(const guchar *src,
                              gint bpp, gint numpix, guchar **dst);

static gboolean  decompose_dialog (void);


/* Maximum number of images/layers generated by an extraction */
#define MAX_EXTRACT_IMAGES 4

/* Description of an extraction */
typedef struct
{
  const gchar *type;        /* What to extract */
  gboolean     dialog;      /* Dialog-Flag. Set it to TRUE if you want to appear
                             * this extract function within the dialog */
  gint         num_images;  /* Number of images to create */

  /* Names of channels to extract */
  const gchar *channel_name[MAX_EXTRACT_IMAGES];

  /* Function that performs the extraction */
  void   (* extract_fun) (const guchar  *src,
                          int            bpp,
                          gint           numpix,
                          guchar       **dst);
} EXTRACT;

static EXTRACT extract[] =
{
  { N_("RGB"),        TRUE,  3, { N_("red"),
                                  N_("green"),
                                  N_("blue") }, extract_rgb },

  { N_("Red"),        FALSE, 1, { N_("red")   }, extract_red },
  { N_("Green"),      FALSE, 1, { N_("green") }, extract_green },
  { N_("Blue"),       FALSE, 1, { N_("blue")  }, extract_blue },

  { N_("RGBA"),       TRUE,  4, { N_("red"),
                                  N_("green"),
                                  N_("blue"),
                                  N_("alpha") }, extract_rgba },


  { N_("HSV"),        TRUE,  3, { N_("hue"),
                                  N_("saturation"),
                                  N_("value")    }, extract_hsv },

  { N_("Hue"),        FALSE, 1, { N_("hue")      }, extract_hue },
  { N_("Saturation"), FALSE, 1, { N_("saturation") }, extract_sat },
  { N_("Value"),      FALSE, 1, { N_("value")    }, extract_val },


  { N_("HSL"),        TRUE,  3, { N_("hue_l"),
                                  N_("saturation_l"),
                                  N_("lightness")    }, extract_hsl },

  { N_("Hue (HSL)"),  FALSE, 1, { N_("hue_l")   }, extract_huel },
  { N_("Saturation (HSL)"), FALSE, 1, { N_("saturation_l") }, extract_satl },
  { N_("Lightness"),  FALSE, 1, { N_("lightness")}, extract_lightness },


  { N_("CMY"),        TRUE,  3, { N_("cyan"),
                                  N_("magenta"),
                                  N_("yellow")  }, extract_cmy },

  { N_("Cyan"),       FALSE, 1, { N_("cyan")    }, extract_cyan },
  { N_("Magenta"),    FALSE, 1, { N_("magenta") }, extract_magenta },
  { N_("Yellow"),     FALSE, 1, { N_("yellow")  }, extract_yellow },


  { N_("CMYK"),       TRUE,  4, { N_("cyan-k"),
                                  N_("magenta-k"),
                                  N_("yellow-k"),
                                  N_("black")     }, extract_cmyk },

  { N_("Cyan_K"),     FALSE, 1, { N_("cyan-k")    }, extract_cyank },
  { N_("Magenta_K"),  FALSE, 1, { N_("magenta-k") }, extract_magentak },
  { N_("Yellow_K"),   FALSE, 1, { N_("yellow-k")  }, extract_yellowk },


  { N_("Alpha"),      TRUE,  1, { N_("alpha") }, extract_alpha },


  { N_("LAB"),        TRUE,  3, { "L",
                                  "A",
                                  "B" }, extract_lab },


  { "YCbCr_ITU_R470",     TRUE, 3, { N_("luma-y470"),
                                     N_("blueness-cb470"),
                                     N_("redness-cr470") }, extract_ycbcr470 },

  { "YCbCr_ITU_R709",     TRUE, 3, { N_("luma-y709"),
                                     N_("blueness-cb709"),
                                     N_("redness-cr709") }, extract_ycbcr709 },

  { "YCbCr ITU R470 256", TRUE, 3, { N_("luma-y470f"),
                                     N_("blueness-cb470f"),
                                     N_("redness-cr470f") }, extract_ycbcr470f },

  { "YCbCr ITU R709 256", TRUE, 3, { N_("luma-y709f"),
                                     N_("blueness-cb709f"),
                                     N_("redness-cr709f") }, extract_ycbcr709f }
};


typedef struct
{
  gchar     extract_type[32];
  gboolean  as_layers;
  gboolean  use_registration;
} DecoVals;

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static DecoVals decovals =
{
  "rgb",    /* Decompose type      */
  TRUE,     /* Decompose to Layers */
  FALSE     /* use registration color */
};

static PicmanRunMode run_mode;


MAIN ()

static void
query (void)
{
  static PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",       "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",          "Input image (unused)"         },
    { PICMAN_PDB_DRAWABLE, "drawable",       "Input drawable"               },
    { PICMAN_PDB_STRING,   "decompose-type", NULL                           },
    { PICMAN_PDB_INT32,    "layers-mode",    "Create channels as layers in a single image" }
  };
  static const PicmanParamDef return_vals[] =
  {
    { PICMAN_PDB_IMAGE, "new-image", "Output gray image" },
    { PICMAN_PDB_IMAGE, "new-image", "Output gray image (N/A for single channel extract)" },
    { PICMAN_PDB_IMAGE, "new-image", "Output gray image (N/A for single channel extract)" },
    { PICMAN_PDB_IMAGE, "new-image", "Output gray image (N/A for single channel extract)" }
  };

  GString *type_desc;
  int i;

  type_desc = g_string_new ("What to decompose: ");
  g_string_append_c (type_desc, '"');
  g_string_append (type_desc, extract[0].type);
  g_string_append_c (type_desc, '"');

  for (i = 1; i < G_N_ELEMENTS (extract); i++)
    {
      g_string_append (type_desc, ", ");
      g_string_append_c (type_desc, '"');
      g_string_append (type_desc, extract[i].type);
      g_string_append_c (type_desc, '"');
    }

  args[3].description = type_desc->str;

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Decompose an image into separate colorspace components"),
                          "This function creates new gray images with "
                          "different channel information in each of them",
                          "Peter Kirchgessner",
                          "Peter Kirchgessner",
                          "1997",
                          N_("_Decompose..."),
                          "RGB*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args),
                          G_N_ELEMENTS (return_vals),
                          args, return_vals);

  picman_install_procedure (PLUG_IN_PROC_REG,
                          N_("Decompose an image into separate colorspace components"),
                          "This function creates new gray images with "
                          "different channel information in each of them. "
                          "Pixels in the foreground color will appear black "
                          "in all output images.  This can be used for "
                          "things like crop marks that have to show up on "
                          "all channels.",
                          "Peter Kirchgessner",
                          "Peter Kirchgessner, Clarence Risher",
                          "1997",
                          N_("_Decompose..."),
                          "RGB*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args),
                          G_N_ELEMENTS (return_vals),
                          args, return_vals);

  picman_plugin_menu_register (PLUG_IN_PROC_REG, "<Image>/Colors/Components");

  g_string_free (type_desc, TRUE);
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam  values[MAX_EXTRACT_IMAGES + 1];
  PicmanPDBStatusType status = PICMAN_PDB_SUCCESS;
  gint32            num_images;
  gint32            image_ID_extract[MAX_EXTRACT_IMAGES];
  gint32            layer_ID_extract[MAX_EXTRACT_IMAGES];
  gint              j;
  gint32            layer;
  gint32            num_layers;
  gint32            image_ID;

  INIT_I18N ();

  run_mode = param[0].data.d_int32;
  image_ID = param[1].data.d_image;
  layer    = param[2].data.d_drawable;

  *nreturn_vals = MAX_EXTRACT_IMAGES+1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  for (j = 0; j < MAX_EXTRACT_IMAGES; j++)
    {
      values[j+1].type         = PICMAN_PDB_IMAGE;
      values[j+1].data.d_int32 = -1;
    }

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &decovals);

      /*  First acquire information with a dialog  */
      if (! decompose_dialog ())
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 4 && nparams != 5 && nparams != 6)
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      else
        {
          strncpy (decovals.extract_type, param[3].data.d_string,
                   sizeof (decovals.extract_type));
          decovals.extract_type[sizeof (decovals.extract_type)-1] = '\0';

          decovals.as_layers = nparams > 4 ? param[4].data.d_int32 : FALSE;
          decovals.use_registration = (strcmp (name, PLUG_IN_PROC_REG) == 0);
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &decovals);
      break;

    default:
      break;
    }

  /*  Make sure that the drawable is RGB color  */
  if (picman_drawable_type_with_alpha (layer) != PICMAN_RGBA_IMAGE)
    {
      g_message ("Can only work on RGB images.");
      status = PICMAN_PDB_CALLING_ERROR;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      picman_progress_init (_("Decomposing"));

      num_images = decompose (image_ID, layer,
                              decovals.extract_type,
                              image_ID_extract,
                              &num_layers,
                              layer_ID_extract);

      if (num_images <= 0)
        {
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
      else
        {
          /* create decompose-data parasite */
          GString *data = g_string_new ("");

          g_string_printf (data, "source=%d type=%s ",
                           layer, decovals.extract_type);

          for (j = 0; j < num_layers; j++)
            g_string_append_printf (data, "%d ", layer_ID_extract[j]);

          for (j = 0; j < num_images; j++)
            {
              PicmanParasite *parasite;

              values[j+1].data.d_int32 = image_ID_extract[j];

              picman_image_undo_enable (image_ID_extract[j]);
              picman_image_clean_all (image_ID_extract[j]);

              parasite = picman_parasite_new ("decompose-data",
                                            0, data->len + 1, data->str);
              picman_image_attach_parasite (image_ID_extract[j], parasite);
              picman_parasite_free (parasite);

              if (run_mode != PICMAN_RUN_NONINTERACTIVE)
                picman_display_new (image_ID_extract[j]);
            }

          /*  Store data  */
          if (run_mode == PICMAN_RUN_INTERACTIVE)
            picman_set_data (PLUG_IN_PROC, &decovals, sizeof (DecoVals));
        }

      picman_progress_end ();
    }

  values[0].data.d_status = status;
}


/* Decompose an image. It returns the number of new (gray) images.
   The image IDs for the new images are returned in image_ID_dst.
   On failure, -1 is returned.
*/
static gint32
decompose (gint32       image_ID,
           gint32       drawable_ID,
           const gchar *extract_type,
           gint32      *image_ID_dst,
           gint32      *nlayers,
           gint32      *layer_ID_dst)
{
  const gchar  *layername;
  gint          i, j, extract_idx, scan_lines;
  gint          height, width, tile_height, num_layers;
  gchar        *filename;
  guchar       *src;
  guchar       *dst[MAX_EXTRACT_IMAGES];
  PicmanDrawable *drawable_src;
  PicmanDrawable *drawable_dst[MAX_EXTRACT_IMAGES];
  PicmanPixelRgn  pixel_rgn_src;
  PicmanPixelRgn  pixel_rgn_dst[MAX_EXTRACT_IMAGES];

  extract_idx = -1;   /* Search extract type */
  for (j = 0; j < G_N_ELEMENTS (extract); j++)
    {
      if (g_ascii_strcasecmp (extract_type, extract[j].type) == 0)
        {
          extract_idx = j;
          break;
        }
    }
  if (extract_idx < 0)
    return -1;

  /* Check structure of source image */
  drawable_src = picman_drawable_get (drawable_ID);
  if (drawable_src->bpp < 3)
    {
      g_message ("Not an RGB image.");
      return -1;
    }
  if ((extract[extract_idx].extract_fun == extract_alpha ||
       extract[extract_idx].extract_fun == extract_rgba) &&
      (!picman_drawable_has_alpha (drawable_ID)))
    {
      g_message ("No alpha channel available.");
      return -1;
    }

  width  = drawable_src->width;
  height = drawable_src->height;

  tile_height = picman_tile_height ();
  picman_pixel_rgn_init (&pixel_rgn_src, drawable_src, 0, 0, width, height,
                       FALSE, FALSE);

  /* allocate a buffer for retrieving information from the src pixel region  */
  src = g_new (guchar, tile_height * width * drawable_src->bpp);

  /* Create all new gray images */
  num_layers = extract[extract_idx].num_images;
  if (num_layers > MAX_EXTRACT_IMAGES)
    num_layers = MAX_EXTRACT_IMAGES;

  for (j = 0; j < num_layers; j++)
    {
      /* Build a filename like <imagename>-<channel>.<extension> */
      gchar   *fname;
      gchar   *extension;
      gdouble  xres, yres;

      fname = picman_image_get_filename (image_ID);

      if (fname)
        {
          extension = fname + strlen (fname) - 1;

          while (extension >= fname)
            {
              if (*extension == '.') break;
              extension--;
            }
          if (extension >= fname)
            {
              *(extension++) = '\0';

              if (decovals.as_layers)
                filename = g_strdup_printf ("%s-%s.%s", fname,
                                            gettext (extract[extract_idx].type),
                                            extension);
              else
                filename = g_strdup_printf ("%s-%s.%s", fname,
                                            gettext (extract[extract_idx].channel_name[j]),
                                            extension);
            }
          else
            {
              if (decovals.as_layers)
                filename = g_strdup_printf ("%s-%s", fname,
                                            gettext (extract[extract_idx].type));
              else
                filename = g_strdup_printf ("%s-%s", fname,
                                            gettext (extract[extract_idx].channel_name[j]));
            }
        }
      else
        {
          filename = g_strdup (gettext (extract[extract_idx].channel_name[j]));
        }

      picman_image_get_resolution (image_ID, &xres, &yres);

      if (decovals.as_layers)
        {
          layername = gettext (extract[extract_idx].channel_name[j]);

          if (j == 0)
            image_ID_dst[j] = create_new_image (filename, layername,
                                                width, height, PICMAN_GRAY,
                                                xres, yres,
                                                layer_ID_dst + j,
                                                drawable_dst + j,
                                                pixel_rgn_dst + j);
          else
            layer_ID_dst[j] = create_new_layer (image_ID_dst[0], j, layername,
                                                width, height, PICMAN_GRAY,
                                                drawable_dst + j,
                                                pixel_rgn_dst + j);
        }
      else
        {
          image_ID_dst[j] = create_new_image (filename, NULL,
                                              width, height, PICMAN_GRAY,
                                              xres, yres,
                                              layer_ID_dst + j,
                                              drawable_dst + j,
                                              pixel_rgn_dst + j);
        }

      g_free (filename);
      g_free (fname);
      dst[j] = g_new (guchar, tile_height * width);
    }

  i = 0;
  while (i < height)
    {
      /* Get source pixel region */
      scan_lines = (i+tile_height-1 < height) ? tile_height : (height-i);
      picman_pixel_rgn_get_rect (&pixel_rgn_src, src, 0, i, width, scan_lines);

      /* Extract the channel information */
      extract[extract_idx].extract_fun (src, drawable_src->bpp, scan_lines*width,
                                        dst);

      /* Transfer the registration color */
      if (decovals.use_registration)
        transfer_registration_color (src, drawable_src->bpp, scan_lines*width,
                                     dst, extract[extract_idx].num_images);

      /* Set destination pixel regions */
      for (j = 0; j < num_layers; j++)
        picman_pixel_rgn_set_rect (&(pixel_rgn_dst[j]), dst[j], 0, i, width,
                                 scan_lines);
      i += scan_lines;

      picman_progress_update ((gdouble) i / (gdouble) height);
    }
  picman_progress_update (1.0);

  g_free (src);

  for (j = 0; j < num_layers; j++)
    {
      picman_drawable_detach (drawable_dst[j]);
      picman_drawable_update (layer_ID_dst[j], 0, 0,
                            picman_drawable_width (layer_ID_dst[j]),
                            picman_drawable_height (layer_ID_dst[j]));
      picman_layer_add_alpha (layer_ID_dst[j]);
      g_free (dst[j]);
    }

  picman_drawable_detach (drawable_src);

  *nlayers = num_layers;

  return (decovals.as_layers ? 1 : num_layers);
}


/* Create an image. Sets layer_ID, drawable and rgn. Returns image_ID */
static gint32
create_new_image (const gchar        *filename,
                  const gchar        *layername,
                  guint               width,
                  guint               height,
                  PicmanImageBaseType   type,
                  gdouble             xres,
                  gdouble             yres,
                  gint32             *layer_ID,
                  PicmanDrawable      **drawable,
                  PicmanPixelRgn       *pixel_rgn)
{
  gint32 image_ID;

  image_ID = picman_image_new (width, height, type);

  picman_image_undo_disable (image_ID);
  picman_image_set_filename (image_ID, filename);
  picman_image_set_resolution (image_ID, xres, yres);

  *layer_ID = create_new_layer (image_ID, 0,
                                layername, width, height, type,
                                drawable, pixel_rgn);

  return image_ID;
}


static gint32
create_new_layer (gint32              image_ID,
                  gint                position,
                  const gchar        *layername,
                  guint               width,
                  guint               height,
                  PicmanImageBaseType   type,
                  PicmanDrawable      **drawable,
                  PicmanPixelRgn       *pixel_rgn)
{
  gint32        layer_ID;
  PicmanImageType gdtype = PICMAN_RGB_IMAGE;

  switch (type)
    {
    case PICMAN_RGB:
      gdtype = PICMAN_RGB_IMAGE;
      break;
    case PICMAN_GRAY:
      gdtype = PICMAN_GRAY_IMAGE;
      break;
    case PICMAN_INDEXED:
      gdtype = PICMAN_INDEXED_IMAGE;
      break;
    }

  if (!layername)
    layername = _("Background");

  layer_ID = picman_layer_new (image_ID, layername, width, height,
                             gdtype, 100, PICMAN_NORMAL_MODE);
  picman_image_insert_layer (image_ID, layer_ID, -1, position);

  *drawable = picman_drawable_get (layer_ID);
  picman_pixel_rgn_init (pixel_rgn, *drawable, 0, 0, (*drawable)->width,
                       (*drawable)->height, TRUE, FALSE);

  return layer_ID;
}

/* Registration Color function */

static void
transfer_registration_color (const guchar  *src,
                             gint           bpp,
                             gint           numpix,
                             guchar       **dst,
                             gint           num_channels)
{
  register const guchar *rgb_src = src;
  guchar *dsts[4];
  register gint count = numpix;
  gint channel;
  PicmanRGB color;
  guchar red, green, blue;

  picman_context_get_foreground (&color);
  picman_rgb_get_uchar (&color, &red, &green, &blue);

  for (channel = 0; channel < num_channels; channel++)
    dsts[channel] = dst[channel];

  while (count-- > 0)
    {
      if (rgb_src[0] == red && rgb_src[1] == green && rgb_src[2] == blue)
        for (channel = 0; channel < num_channels; channel++)
          *(dsts[channel]++) = 255;
      else
        for (channel = 0; channel < num_channels; channel++)
          dsts[channel]++;

      rgb_src += bpp;
    }
}


/* Extract functions */

static void
extract_rgb (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *red_dst = dst[0];
  register guchar *green_dst = dst[1];
  register guchar *blue_dst = dst[2];
  register gint count = numpix, offset = bpp-3;

  while (count-- > 0)
    {
      *(red_dst++) = *(rgb_src++);
      *(green_dst++) = *(rgb_src++);
      *(blue_dst++) = *(rgb_src++);
      rgb_src += offset;
    }
}

static void
extract_rgba (const guchar  *src,
              gint           bpp,
              gint           numpix,
              guchar       **dst)
{
  register const guchar *rgba_src = src;
  register guchar *red_dst = dst[0];
  register guchar *green_dst = dst[1];
  register guchar *blue_dst = dst[2];
  register guchar *alpha_dst = dst[3];
  register gint count = numpix, offset = bpp-4;

  while (count-- > 0)
    {
      *(red_dst++) = *(rgba_src++);
      *(green_dst++) = *(rgba_src++);
      *(blue_dst++) = *(rgba_src++);
      *(alpha_dst++) = *(rgba_src++);
      rgba_src += offset;
    }
}

static void
extract_red (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *red_dst = dst[0];
  register gint count = numpix, offset = bpp;

  while (count-- > 0)
    {
      *(red_dst++) = *rgb_src;
      rgb_src += offset;
    }
}


static void
extract_green (const guchar  *src,
               gint           bpp,
               gint           numpix,
               guchar       **dst)
{
  register const guchar *rgb_src = src+1;
  register guchar *green_dst = dst[0];
  register gint count = numpix, offset = bpp;

  while (count-- > 0)
    {
      *(green_dst++) = *rgb_src;
      rgb_src += offset;
    }
}


static void
extract_blue (const guchar  *src,
              gint           bpp,
              gint           numpix,
              guchar       **dst)
{
  register const guchar *rgb_src = src+2;
  register guchar *blue_dst = dst[0];
  register gint count = numpix, offset = bpp;

  while (count-- > 0)
    {
      *(blue_dst++) = *rgb_src;
      rgb_src += offset;
    }
}


static void
extract_alpha (const guchar  *src,
               gint           bpp,
               gint           numpix,
               guchar       **dst)
{
  register const guchar *rgb_src = src+3;
  register guchar *alpha_dst = dst[0];
  register gint count = numpix, offset = bpp;

  while (count-- > 0)
    {
      *(alpha_dst++) = *rgb_src;
      rgb_src += offset;
    }
}


static void
extract_cmy (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *cyan_dst = dst[0];
  register guchar *magenta_dst = dst[1];
  register guchar *yellow_dst = dst[2];
  register gint count = numpix, offset = bpp-3;

  while (count-- > 0)
    {
      *(cyan_dst++) = 255 - *(rgb_src++);
      *(magenta_dst++) = 255 - *(rgb_src++);
      *(yellow_dst++) = 255 - *(rgb_src++);
      rgb_src += offset;
    }
}


static void
extract_hsv (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *hue_dst = dst[0];
  register guchar *sat_dst = dst[1];
  register guchar *val_dst = dst[2];
  register gint count = numpix, offset = bpp;
  gdouble hue, sat, val;

  while (count-- > 0)
    {
      picman_rgb_to_hsv4 (rgb_src, &hue, &sat, &val);
      *hue_dst++ = (guchar) (hue * 255.999);
      *sat_dst++ = (guchar) (sat * 255.999);
      *val_dst++ = (guchar) (val * 255.999);
      rgb_src += offset;
    }
}


static void
extract_hue (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *hue_dst = dst[0];
  register gint count = numpix, offset = bpp;
  gdouble hue, dummy;

  while (count-- > 0)
    {
      picman_rgb_to_hsv4 (rgb_src, &hue, &dummy, &dummy);
      *hue_dst++ = (guchar) (hue * 255.999);
      rgb_src += offset;
    }
}


static void
extract_sat (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *sat_dst = dst[0];
  register gint count = numpix, offset = bpp;
  gdouble sat, dummy;

  while (count-- > 0)
    {
      picman_rgb_to_hsv4 (rgb_src, &dummy, &sat, &dummy);
      *sat_dst++ = (guchar) (sat * 255.999);
      rgb_src += offset;
    }
}


static void
extract_val (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *val_dst = dst[0];
  register gint count = numpix, offset = bpp;
  gdouble val, dummy;

  while (count-- > 0)
    {
      picman_rgb_to_hsv4 (rgb_src, &dummy, &dummy, &val);
      *val_dst++ = (guchar) (val * 255.999);
      rgb_src += offset;
    }
}


static void
extract_hsl (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *hue_dst = dst[0];
  register guchar *sat_dst = dst[1];
  register guchar *lum_dst = dst[2];
  register gint count = numpix, offset = bpp;

  while (count-- > 0)
    {
      PicmanRGB rgb;
      PicmanHSL hsl;

      picman_rgb_set_uchar (&rgb, rgb_src[0], rgb_src[1], rgb_src[2]);
      picman_rgb_to_hsl (&rgb, &hsl);

      *hue_dst++ = (guchar) (hsl.h * 255.999);
      *sat_dst++ = (guchar) (hsl.s * 255.999);
      *lum_dst++ = (guchar) (hsl.l * 255.999);

      rgb_src += offset;
    }
}


static void
extract_huel (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *hue_dst = dst[0];
  register gint count = numpix, offset = bpp;

  while (count-- > 0)
    {
      PicmanRGB rgb;
      PicmanHSL hsl;

      picman_rgb_set_uchar (&rgb, rgb_src[0], rgb_src[1], rgb_src[2]);
      picman_rgb_to_hsl (&rgb, &hsl);

      *hue_dst++ = (guchar) (hsl.h * 255.999);
      rgb_src += offset;
    }
}


static void
extract_satl (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *sat_dst = dst[0];
  register gint count = numpix, offset = bpp;

  while (count-- > 0)
    {
      PicmanRGB rgb;
      PicmanHSL hsl;

      picman_rgb_set_uchar (&rgb, rgb_src[0], rgb_src[1], rgb_src[2]);
      picman_rgb_to_hsl (&rgb, &hsl);

      *sat_dst++ = (guchar) (hsl.s * 255.999);
      rgb_src += offset;
    }
}


static void
extract_lightness (const guchar  *src,
                   gint           bpp,
                   gint           numpix,
                   guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *lum_dst = dst[0];
  register gint count = numpix, offset = bpp;

  while (count-- > 0)
    {
      PicmanRGB rgb;
      PicmanHSL hsl;

      picman_rgb_set_uchar (&rgb, rgb_src[0], rgb_src[1], rgb_src[2]);
      picman_rgb_to_hsl (&rgb, &hsl);
      *lum_dst++ = (guchar) (hsl.l * 255.999);
      rgb_src += offset;
    }
}


static void
extract_cyan (const guchar  *src,
              gint           bpp,
              gint           numpix,
              guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *cyan_dst = dst[0];
  register gint count = numpix, offset = bpp-1;

  while (count-- > 0)
    {
      *(cyan_dst++) = 255 - *(rgb_src++);
      rgb_src += offset;
    }
}


static void
extract_magenta (const guchar  *src,
                 gint           bpp,
                 gint           numpix,
                 guchar       **dst)
{
  register const guchar *rgb_src = src+1;
  register guchar *magenta_dst = dst[0];
  register gint count = numpix, offset = bpp-1;

  while (count-- > 0)
    {
      *(magenta_dst++) = 255 - *(rgb_src++);
      rgb_src += offset;
    }
}


static void
extract_yellow (const guchar  *src,
                gint           bpp,
                gint           numpix,
                guchar       **dst)
{
  register const guchar *rgb_src = src+2;
  register guchar *yellow_dst = dst[0];
  register gint count = numpix, offset = bpp-1;

  while (count-- > 0)
    {
      *(yellow_dst++) = 255 - *(rgb_src++);
      rgb_src += offset;
    }
}


static void
extract_cmyk (const guchar  *src,
              gint           bpp,
              gint           numpix,
              guchar       **dst)

{
  register const guchar *rgb_src = src;
  register guchar *cyan_dst = dst[0];
  register guchar *magenta_dst = dst[1];
  register guchar *yellow_dst = dst[2];
  register guchar *black_dst = dst[3];
  register gint count = numpix, offset = bpp-3;
  PicmanCMYK gcmyk;

  picman_cmyk_set (&gcmyk, 0, 0, 0, 0);

  while (count-- > 0)
    {
      PicmanRGB grgb;
      guchar  r, g, b;

      r = *rgb_src++;
      g = *rgb_src++;
      b = *rgb_src++;

      picman_rgb_set_uchar (&grgb, r,g,b);
      picman_rgb_to_cmyk (&grgb, 1.0, &gcmyk);
      picman_cmyk_get_uchar (&gcmyk,
                           cyan_dst++, magenta_dst++, yellow_dst++,
                           black_dst++);

      rgb_src += offset;
    }
}


static void
extract_cyank (const guchar  *src,
               gint           bpp,
               gint           numpix,
               guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *cyan_dst = dst[0];
  register gint count = numpix, offset = bpp-3;
  PicmanCMYK gcmyk;

  picman_cmyk_set(&gcmyk, 0, 0, 0, 0);

  while (count-- > 0)
    {
      PicmanRGB grgb;
      guchar  r, g, b;

      r = *rgb_src++;
      g = *rgb_src++;
      b = *rgb_src++;

      picman_rgb_set_uchar (&grgb, r,g,b);
      picman_rgb_to_cmyk (&grgb, 1.0, &gcmyk);
      picman_cmyk_get_uchar (&gcmyk, cyan_dst++, NULL, NULL, NULL);

      rgb_src += offset;
    }
}


static void
extract_magentak (const guchar  *src,
                  gint           bpp,
                  gint           numpix,
                  guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *magenta_dst = dst[0];
  register gint count = numpix, offset = bpp-3;
  PicmanCMYK gcmyk;

  picman_cmyk_set(&gcmyk, 0, 0, 0, 0);

  while (count-- > 0)
    {
      PicmanRGB grgb;
      guchar  r, g, b;

      r = *rgb_src++;
      g = *rgb_src++;
      b = *rgb_src++;

      picman_rgb_set_uchar (&grgb, r,g,b);
      picman_rgb_to_cmyk (&grgb, 1.0, &gcmyk);
      picman_cmyk_get_uchar (&gcmyk, NULL, magenta_dst++, NULL, NULL);

      rgb_src += offset;
    }
}


static void
extract_yellowk (const guchar  *src,
                 gint           bpp,
                 gint           numpix,
                 guchar       **dst)

{
  register const guchar *rgb_src = src;
  register guchar *yellow_dst = dst[0];
  register gint count = numpix, offset = bpp-3;
  PicmanCMYK gcmyk;

  picman_cmyk_set(&gcmyk, 0, 0, 0, 0);

  while (count-- > 0)
    {
      PicmanRGB grgb;
      guchar  r, g, b;

      r = *rgb_src++;
      g = *rgb_src++;
      b = *rgb_src++;

      picman_rgb_set_uchar (&grgb, r,g,b);
      picman_rgb_to_cmyk (&grgb, 1.0, &gcmyk);
      picman_cmyk_get_uchar (&gcmyk, NULL, NULL, yellow_dst++, NULL);

      rgb_src += offset;
    }
}

static void
extract_lab (const guchar  *src,
             gint           bpp,
             gint           numpix,
             guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *l_dst = dst[0];
  register guchar *a_dst = dst[1];
  register guchar *b_dst = dst[2];
  register gint count = numpix, offset = bpp;

  gdouble red, green, blue;
  gdouble x, y, z;
  gdouble l; /*, a, b; */
  gdouble tx, ty, tz;
  gdouble ftx, fty, ftz;

  gdouble sixteenth = 16.0 / 116.0;

  /* LAB colorspace constants */
#define LAB_Xn 0.951
#define LAB_Yn 1.0
#define LAB_Zn 1.089

  while (count-- > 0)
    {
      red   = rgb_src[0] / 255.0;
      green = rgb_src[1] / 255.0;
      blue  = rgb_src[2] / 255.0;

      x = 0.431 * red + 0.342 * green + 0.178 * blue;
      y = 0.222 * red + 0.707 * green + 0.071 * blue;
      z = 0.020 * red + 0.130 * green + 0.939 * blue;

      if ((ty = y / LAB_Yn) > 0.008856)
        {
          l   = 116.0 * cbrt (ty) - 16.0;
          fty = cbrt (ty);
        }
      else
        {
          l   = 903.3  * ty;
          fty =   7.78 * ty + sixteenth;
        }

      ftx = ((tx = x / LAB_Xn) > 0.008856) ? cbrt (tx) : 7.78 * tx + sixteenth;
      ftz = ((tz = z / LAB_Zn) > 0.008856) ? cbrt (tz) : 7.78 * tz + sixteenth;

      *l_dst++ = (guchar) CLAMP (l * 2.5599, 0., 256.);
      *a_dst++ = (guchar) CLAMP (128.0 + (ftx - fty) * 635, 0., 256.);
      *b_dst++ = (guchar) CLAMP (128.0 + (fty - ftz) * 254, 0., 256.);

      rgb_src += offset;
    }
}


/* these are here so the code is more readable and we can use
   the standard values instead of some scaled and rounded fixpoint values */
#define FIX(a) ((int)((a)*256.0*256.0 + 0.5))
#define FIXY(a) ((int)((a)*256.0*256.0*219.0/255.0 + 0.5))
#define FIXC(a) ((int)((a)*256.0*256.0*224.0/255.0 + 0.5))

static void
extract_ycbcr470 (const guchar  *src,
                  gint           bpp,
                  gint           numpix,
                  guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *y_dst  = dst[0];
  register guchar *cb_dst = dst[1];
  register guchar *cr_dst = dst[2];
  register gint count = numpix, offset = bpp-3;

  while (count-- > 0)
    {
      register int r, g, b;
      r= *(rgb_src++);
      g= *(rgb_src++);
      b= *(rgb_src++);
      *(y_dst++)  = ( FIXY(0.2989)*r + FIXY(0.5866)*g + FIXY(0.1145)*b + FIX( 16.5))>>16;
      *(cb_dst++) = (-FIXC(0.1688)*r - FIXC(0.3312)*g + FIXC(0.5000)*b + FIX(128.5))>>16;
      *(cr_dst++) = ( FIXC(0.5000)*r - FIXC(0.4184)*g - FIXC(0.0816)*b + FIX(128.5))>>16;

      rgb_src += offset;
    }
}


static void
extract_ycbcr709 (const guchar  *src,
                  gint           bpp,
                  gint           numpix,
                  guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *y_dst  = dst[0];
  register guchar *cb_dst = dst[1];
  register guchar *cr_dst = dst[2];
  register gint count = numpix, offset = bpp-3;

  while (count-- > 0)
    {
      register int r, g, b;
      r= *(rgb_src++);
      g= *(rgb_src++);
      b= *(rgb_src++);
      *(y_dst++)  = ( FIXY(0.2126)*r + FIXY(0.7152)*g + FIXY(0.0722)*b + FIX( 16.5))>>16;
      *(cb_dst++) = (-FIXC(0.1150)*r - FIXC(0.3860)*g + FIXC(0.5000)*b + FIX(128.5))>>16;
      *(cr_dst++) = ( FIXC(0.5000)*r - FIXC(0.4540)*g - FIXC(0.0460)*b + FIX(128.5))>>16;

      rgb_src += offset;
    }
}


static void
extract_ycbcr470f (const guchar  *src,
                   gint           bpp,
                   gint           numpix,
                   guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *y_dst  = dst[0];
  register guchar *cb_dst = dst[1];
  register guchar *cr_dst = dst[2];
  register gint count = numpix, offset = bpp-3;

  while (count-- > 0)
    {
      register int r, g, b, cb, cr;
      r= *(rgb_src++);
      g= *(rgb_src++);
      b= *(rgb_src++);
      *(y_dst++ ) = ( FIX(0.2989)*r + FIX(0.5866)*g + FIX(0.1145)*b + FIX(  0.5))>>16;
      cb          = (-FIX(0.1688)*r - FIX(0.3312)*g + FIX(0.5000)*b + FIX(128.5))>>16;
      cr          = ( FIX(0.5000)*r - FIX(0.4184)*g - FIX(0.0816)*b + FIX(128.5))>>16;
      if(cb>255) cb=255;
      if(cr>255) cr=255;
      *(cb_dst++) = cb;
      *(cr_dst++) = cr;
      rgb_src += offset;
    }
}


static void
extract_ycbcr709f (const guchar  *src,
                   gint           bpp,
                   gint           numpix,
                   guchar       **dst)
{
  register const guchar *rgb_src = src;
  register guchar *y_dst  = dst[0];
  register guchar *cb_dst = dst[1];
  register guchar *cr_dst = dst[2];
  register gint count = numpix, offset = bpp-3;

  while (count-- > 0)
    {
      register int r, g, b, cb, cr;
      r= *(rgb_src++);
      g= *(rgb_src++);
      b= *(rgb_src++);
      *(y_dst++)  = ( FIX(0.2126)*r + FIX(0.7152)*g + FIX(0.0722)*b + FIX(  0.5))>>16;
      cb          = (-FIX(0.1150)*r - FIX(0.3860)*g + FIX(0.5000)*b + FIX(128.5))>>16;
      cr          = ( FIX(0.5000)*r - FIX(0.4540)*g - FIX(0.0460)*b + FIX(128.5))>>16;
      if(cb>255) cb=255;
      if(cr>255) cr=255;
      *(cb_dst++) = cb;
      *(cr_dst++) = cr;

      rgb_src += offset;
    }
}


static gboolean
decompose_dialog (void)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *combo;
  GtkWidget *toggle;
  gint       j;
  gint       extract_idx;
  gboolean   run;

  extract_idx = 0;
  for (j = 0; j < G_N_ELEMENTS (extract); j++)
    {
      if (extract[j].dialog &&
          g_ascii_strcasecmp (decovals.extract_type, extract[j].type) == 0)
        {
          extract_idx = j;
          break;
        }
    }

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Decompose"), PLUG_IN_ROLE,
                            NULL, 0,
                            picman_standard_help_func, PLUG_IN_PROC,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  picman_window_set_transient (GTK_WINDOW (dialog));

  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  frame = picman_frame_new (_("Extract Channels"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("Color _model:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = g_object_new (PICMAN_TYPE_INT_COMBO_BOX, NULL);
  for (j = 0; j < G_N_ELEMENTS (extract); j++)
    {
      if (extract[j].dialog)
        {
          gchar *label = g_strdup (gettext (extract[j].type));
          gchar *l;

          for (l = label; *l; l++)
            if (*l == '-' || *l == '_')
              *l = ' ';

          picman_int_combo_box_append (PICMAN_INT_COMBO_BOX (combo),
                                     PICMAN_INT_STORE_LABEL, label,
                                     PICMAN_INT_STORE_VALUE, j,
                                     -1);
          g_free (label);
        }
    }

  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
  gtk_widget_show (combo);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);

  picman_int_combo_box_connect (PICMAN_INT_COMBO_BOX (combo),
                              extract_idx,
                              G_CALLBACK (picman_int_combo_box_get_active),
                              &extract_idx);

  toggle = gtk_check_button_new_with_mnemonic (_("_Decompose to layers"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                decovals.as_layers);
  gtk_box_pack_start (GTK_BOX (main_vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &decovals.as_layers);

  toggle =
    gtk_check_button_new_with_mnemonic (_("_Foreground as registration color"));
  picman_help_set_help_data (toggle, _("Pixels in the foreground color will "
                                     "appear black in all output images.  "
                                     "This can be used for things like crop "
                                     "marks that have to show up on all "
                                     "channels."), PLUG_IN_PROC);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                decovals.use_registration);
  gtk_box_pack_start (GTK_BOX (main_vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &decovals.use_registration);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  if (run)
    strncpy (decovals.extract_type, extract[extract_idx].type, sizeof decovals.extract_type - 1);

  return run;
}
