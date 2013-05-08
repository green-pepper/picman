/* tiff saving for PICMAN
 *  -Peter Mattis
 *
 * The TIFF loading code has been completely revamped by Nick Lamb
 * njl195@zepler.org.uk -- 18 May 1998
 * And it now gains support for tiles (and doubtless a zillion bugs)
 * njl195@zepler.org.uk -- 12 June 1999
 * LZW patent fuss continues :(
 * njl195@zepler.org.uk -- 20 April 2000
 * The code for this filter is based on "tifftopnm" and "pnmtotiff",
 *  2 programs that are a part of the netpbm package.
 * khk@khk.net -- 13 May 2000
 * Added support for ICCPROFILE tiff tag. If this tag is present in a
 * TIFF file, then a parasite is created and vice versa.
 * peter@kirchgessner.net -- 29 Oct 2002
 * Progress bar only when run interactive
 * Added support for layer offsets - pablo.dangelo@web.de -- 7 Jan 2004
 * Honor EXTRASAMPLES tag while loading images with alphachannel
 * pablo.dangelo@web.de -- 16 Jan 2004
 */

/*
 * tifftopnm.c - converts a Tagged Image File to a portable anymap
 *
 * Derived by Jef Poskanzer from tif2ras.c, which is:
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Author: Patrick J. Naughton
 * naughton@wind.sun.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 */

#include "config.h"

#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <fcntl.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib/gstdio.h>
#ifdef G_OS_WIN32
#include <libpicmanbase/picmanwin32-io.h>
#endif

#ifndef _O_BINARY
#define _O_BINARY 0
#endif

#include <tiffio.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define SAVE_PROC      "file-tiff-save"
#define SAVE2_PROC     "file-tiff-save2"
#define PLUG_IN_BINARY "file-tiff-save"
#define PLUG_IN_ROLE   "picman-file-tiff-save"


typedef struct
{
  gint      compression;
  gint      fillorder;
  gboolean  save_transp_pixels;
} TiffSaveVals;

typedef struct
{
  gint32        ID;
  PicmanDrawable *drawable;
  PicmanPixelRgn  pixel_rgn;
  guchar       *pixels;
  guchar       *pixel;
} channel_data;

/* Declare some local functions.
 */
static void   query     (void);
static void   run       (const gchar      *name,
                         gint              nparams,
                         const PicmanParam  *param,
                         gint             *nreturn_vals,
                         PicmanParam       **return_vals);

static gboolean  image_is_monochrome (gint32 image);

static gboolean  save_paths             (TIFF         *tif,
                                         gint32        image);
static gboolean  save_image             (const gchar  *filename,
                                         gint32        image,
                                         gint32        drawable,
                                         gint32        orig_image,
                                         GError      **error);

static gboolean  save_dialog            (gboolean      has_alpha,
                                         gboolean      is_monochrome);

static void      comment_entry_callback (GtkWidget    *widget,
                                         gpointer      data);

static void      byte2bit               (const guchar *byteline,
                                         gint          width,
                                         guchar       *bitline,
                                         gboolean      invert);

static void      tiff_warning           (const gchar *module,
                                         const gchar *fmt,
                                         va_list      ap);
static void      tiff_error             (const gchar *module,
                                         const gchar *fmt,
                                         va_list      ap);
static TIFF     *tiff_open              (const gchar *filename,
                                         const gchar *mode,
                                         GError     **error);

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static TiffSaveVals tsvals =
{
  COMPRESSION_NONE,    /*  compression    */
  TRUE,                /*  alpha handling */
};

static gchar       *image_comment = NULL;
static PicmanRunMode  run_mode      = PICMAN_RUN_INTERACTIVE;


MAIN ()

static void
query (void)
{
#define COMMON_SAVE_ARGS \
    { PICMAN_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },\
    { PICMAN_PDB_IMAGE,    "image",        "Input image" },\
    { PICMAN_PDB_DRAWABLE, "drawable",     "Drawable to save" },\
    { PICMAN_PDB_STRING,   "filename",     "The name of the file to save the image in" },\
    { PICMAN_PDB_STRING,   "raw-filename", "The name of the file to save the image in" },\
    { PICMAN_PDB_INT32,    "compression",  "Compression type: { NONE (0), LZW (1), PACKBITS (2), DEFLATE (3), JPEG (4), CCITT G3 Fax (5), CCITT G4 Fax (6) }" }

  static const PicmanParamDef save_args_old[] =
  {
    COMMON_SAVE_ARGS
  };

  static const PicmanParamDef save_args[] =
  {
    COMMON_SAVE_ARGS,
    { PICMAN_PDB_INT32, "save-transp-pixels", "Keep the color data masked by an alpha channel intact" }
  };

  picman_install_procedure (SAVE_PROC,
                          "saves files in the tiff file format",
                          "Saves files in the Tagged Image File Format.  "
                          "The value for the saved comment is taken "
                          "from the 'picman-comment' parasite.",
                          "Spencer Kimball & Peter Mattis",
                          "Spencer Kimball & Peter Mattis",
                          "1995-1996,2000-2003",
                          N_("TIFF image"),
                          "RGB*, GRAY*, INDEXED",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (save_args_old), 0,
                          save_args_old, NULL);

  picman_register_file_handler_mime (SAVE_PROC, "image/tiff");
  picman_register_save_handler (SAVE_PROC, "tif,tiff", "");

  picman_install_procedure (SAVE2_PROC,
                          "saves files in the tiff file format",
                          "Saves files in the Tagged Image File Format.  "
                          "The value for the saved comment is taken "
                          "from the 'picman-comment' parasite.",
                          "Spencer Kimball & Peter Mattis",
                          "Spencer Kimball & Peter Mattis",
                          "1995-1996,2000-2003",
                          N_("TIFF image"),
                          "RGB*, GRAY*, INDEXED",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (save_args), 0,
                          save_args, NULL);
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
  PicmanParasite      *parasite;
  gint32             image;
  gint32             drawable;
  gint32             orig_image;
  PicmanExportReturn   export = PICMAN_EXPORT_CANCEL;
  GError            *error  = NULL;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

  TIFFSetWarningHandler (tiff_warning);
  TIFFSetErrorHandler (tiff_error);

  if ((strcmp (name, SAVE_PROC) == 0) ||
      (strcmp (name, SAVE2_PROC) == 0))
    {
      /* Plug-in is either file_tiff_save or file_tiff_save2 */
      image = orig_image = param[1].data.d_int32;
      drawable = param[2].data.d_int32;

      /* Do this right this time, if POSSIBLE query for parasites, otherwise
         or if there isn't one, choose the default comment from the picmanrc. */

      /*  eventually export the image */
      switch (run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
        case PICMAN_RUN_WITH_LAST_VALS:
          picman_ui_init (PLUG_IN_BINARY, FALSE);
          export = picman_export_image (&image, &drawable, NULL,
                                      (PICMAN_EXPORT_CAN_HANDLE_RGB |
                                       PICMAN_EXPORT_CAN_HANDLE_GRAY |
                                       PICMAN_EXPORT_CAN_HANDLE_INDEXED |
                                       PICMAN_EXPORT_CAN_HANDLE_ALPHA ));
          if (export == PICMAN_EXPORT_CANCEL)
            {
              values[0].data.d_status = PICMAN_PDB_CANCEL;
              return;
            }
          break;
        default:
          break;
        }

      parasite = picman_image_get_parasite (orig_image, "picman-comment");
      if (parasite)
        {
          image_comment = g_strndup (picman_parasite_data (parasite),
                                     picman_parasite_data_size (parasite));
          picman_parasite_free (parasite);
        }

      switch (run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
          /*  Possibly retrieve data  */
          picman_get_data (SAVE_PROC, &tsvals);

          parasite = picman_image_get_parasite (orig_image, "tiff-save-options");
          if (parasite)
            {
              const TiffSaveVals *pvals = picman_parasite_data (parasite);

              tsvals.compression        = pvals->compression;
              tsvals.save_transp_pixels = pvals->save_transp_pixels;
            }
          picman_parasite_free (parasite);

          /*  First acquire information with a dialog  */
          if (! save_dialog (picman_drawable_has_alpha (drawable),
                             image_is_monochrome (image)))
            status = PICMAN_PDB_CANCEL;
          break;

        case PICMAN_RUN_NONINTERACTIVE:
          /*  Make sure all the arguments are there!  */
          if (nparams == 6 || nparams == 7)
            {
              switch (param[5].data.d_int32)
                {
                case 0: tsvals.compression = COMPRESSION_NONE;      break;
                case 1: tsvals.compression = COMPRESSION_LZW;       break;
                case 2: tsvals.compression = COMPRESSION_PACKBITS;  break;
                case 3: tsvals.compression = COMPRESSION_DEFLATE;   break;
                case 4: tsvals.compression = COMPRESSION_JPEG;      break;
                case 5: tsvals.compression = COMPRESSION_CCITTFAX3; break;
                case 6: tsvals.compression = COMPRESSION_CCITTFAX4; break;
                default: status = PICMAN_PDB_CALLING_ERROR; break;
                }

              if (nparams == 7)
                tsvals.save_transp_pixels = param[6].data.d_int32;
              else
                tsvals.save_transp_pixels = TRUE;
            }
          else
            {
              status = PICMAN_PDB_CALLING_ERROR;
            }
          break;

        case PICMAN_RUN_WITH_LAST_VALS:
          /*  Possibly retrieve data  */
          picman_get_data (SAVE_PROC, &tsvals);

          parasite = picman_image_get_parasite (orig_image, "tiff-save-options");
          if (parasite)
            {
              const TiffSaveVals *pvals = picman_parasite_data (parasite);

              tsvals.compression        = pvals->compression;
              tsvals.save_transp_pixels = pvals->save_transp_pixels;
            }
          picman_parasite_free (parasite);
          break;

        default:
          break;
        }

      if (status == PICMAN_PDB_SUCCESS)
        {
          if (save_image (param[3].data.d_string, image, drawable, orig_image,
                          &error))
            {
              /*  Store mvals data  */
              picman_set_data (SAVE_PROC, &tsvals, sizeof (TiffSaveVals));
            }
          else
            {
              status = PICMAN_PDB_EXECUTION_ERROR;
            }
        }

      if (export == PICMAN_EXPORT_EXPORT)
        picman_image_delete (image);
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

static void
tiff_warning (const gchar *module,
              const gchar *fmt,
              va_list      ap)
{
  va_list ap_test;

  /* Workaround for: http://bugzilla.gnome.org/show_bug.cgi?id=131975 */
  /* Ignore the warnings about unregistered private tags (>= 32768) */
  if (! strcmp (fmt, "%s: unknown field with tag %d (0x%x) encountered"))
    {
      G_VA_COPY (ap_test, ap);
      if (va_arg (ap_test, char *));  /* ignore first argument */
      if (va_arg (ap_test, int) >= 32768)
        return;
    }
  /* for older versions of libtiff? */
  else if (! strcmp (fmt, "unknown field with tag %d (0x%x) ignored"))
    {
      G_VA_COPY (ap_test, ap);
      if (va_arg (ap_test, int) >= 32768)
        return;
    }

  g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, fmt, ap);
}

static void
tiff_error (const gchar *module,
            const gchar *fmt,
            va_list      ap)
{
  /* Workaround for: http://bugzilla.gnome.org/show_bug.cgi?id=132297 */
  /* Ignore the errors related to random access and JPEG compression */
  if (! strcmp (fmt, "Compression algorithm does not support random access"))
    return;
  g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, fmt, ap);
}

static TIFF *
tiff_open (const gchar  *filename,
           const gchar  *mode,
           GError      **error)
{
#ifdef G_OS_WIN32
  gunichar2 *utf16_filename = g_utf8_to_utf16 (filename, -1, NULL, NULL, error);

  if (utf16_filename)
    {
      TIFF *tif = TIFFOpenW (utf16_filename, mode);

      g_free (utf16_filename);

      return tif;
    }

  return NULL;
#else
  return TIFFOpen (filename, mode);
#endif
}

static gboolean
image_is_monochrome (gint32 image)
{
  guchar   *colors;
  gint      num_colors;
  gboolean  monochrome = FALSE;

  g_return_val_if_fail (image != -1, FALSE);

  colors = picman_image_get_colormap (image, &num_colors);

  if (colors)
    {
      if (num_colors == 2 || num_colors == 1)
        {
          const guchar  bw_map[] = { 0, 0, 0, 255, 255, 255 };
          const guchar  wb_map[] = { 255, 255, 255, 0, 0, 0 };

          if (memcmp (colors, bw_map, 3 * num_colors) == 0 ||
              memcmp (colors, wb_map, 3 * num_colors) == 0)
            {
              monochrome = TRUE;
            }
        }

      g_free (colors);
    }

  return monochrome;
}


static void
double_to_psd_fixed (gdouble value, gchar *target)
{
  gdouble in, frac;
  gint    i, f;

  frac = modf (value, &in);
  if (frac < 0)
    {
      in -= 1;
      frac += 1;
    }

  i = (gint) CLAMP (in, -16, 15);
  f = CLAMP ((gint) (frac * 0xFFFFFF), 0, 0xFFFFFF);

  target[0] = i & 0xFF;
  target[1] = (f >> 16) & 0xFF;
  target[2] = (f >>  8) & 0xFF;
  target[3] = f & 0xFF;
}


static gboolean
save_paths (TIFF   *tif,
            gint32  image)
{
  gint id = 2000; /* Photoshop paths have IDs >= 2000 */
  gint num_vectors, *vectors, v;
  gint num_strokes, *strokes, s;
  gdouble width, height;
  GString *ps_tag;

  width = picman_image_width (image);
  height = picman_image_height (image);
  vectors = picman_image_get_vectors (image, &num_vectors);

  if (num_vectors <= 0)
    return FALSE;

  ps_tag = g_string_new ("");

  /* Only up to 1000 paths supported */
  for (v = 0; v < MIN (num_vectors, 1000); v++)
    {
      GString *data;
      gchar   *name, *nameend;
      gsize    len;
      gint     lenpos;
      gchar    pointrecord[26] = { 0, };
      gchar   *tmpname;
      GError  *err = NULL;

      data = g_string_new ("8BIM");
      g_string_append_c (data, id / 256);
      g_string_append_c (data, id % 256);

      /*
       * - use iso8859-1 if possible
       * - otherwise use UTF-8, prepended with \xef\xbb\xbf (Byte-Order-Mark)
       */
      name = picman_item_get_name (vectors[v]);
      tmpname = g_convert (name, -1, "iso8859-1", "utf-8", NULL, &len, &err);

      if (tmpname && err == NULL)
        {
          g_string_append_c (data, MIN (len, 255));
          g_string_append_len (data, tmpname, MIN (len, 255));
          g_free (tmpname);
        }
      else
        {
          /* conversion failed, we fall back to UTF-8 */
          len = g_utf8_strlen (name, 255 - 3);  /* need three marker-bytes */

          nameend = g_utf8_offset_to_pointer (name, len);
          len = nameend - name; /* in bytes */
          g_assert (len + 3 <= 255);

          g_string_append_c (data, len + 3);
          g_string_append_len (data, "\xEF\xBB\xBF", 3); /* Unicode 0xfeff */
          g_string_append_len (data, name, len);

          if (tmpname)
            g_free (tmpname);
        }

      if (data->len % 2)  /* padding to even size */
        g_string_append_c (data, 0);
      g_free (name);

      lenpos = data->len;
      g_string_append_len (data, "\0\0\0\0", 4); /* will be filled in later */
      len = data->len; /* to calculate the data size later */

      pointrecord[1] = 6;  /* fill rule record */
      g_string_append_len (data, pointrecord, 26);

      strokes = picman_vectors_get_strokes (vectors[v], &num_strokes);

      for (s = 0; s < num_strokes; s++)
        {
          PicmanVectorsStrokeType type;
          gdouble  *points;
          gint      num_points;
          gboolean  closed;
          gint      p = 0;

          type = picman_vectors_stroke_get_points (vectors[v], strokes[s],
                                                 &num_points, &points, &closed);

          if (type != PICMAN_VECTORS_STROKE_TYPE_BEZIER ||
              num_points > 65535 ||
              num_points % 6)
            {
              g_printerr ("tiff-save: unsupported stroke type: "
                          "%d (%d points)\n", type, num_points);
              continue;
            }

          memset (pointrecord, 0, 26);
          pointrecord[1] = closed ? 0 : 3;
          pointrecord[2] = (num_points / 6) / 256;
          pointrecord[3] = (num_points / 6) % 256;
          g_string_append_len (data, pointrecord, 26);

          for (p = 0; p < num_points; p += 6)
            {
              pointrecord[1] = closed ? 2 : 5;

              double_to_psd_fixed (points[p+1] / height, pointrecord + 2);
              double_to_psd_fixed (points[p+0] / width,  pointrecord + 6);
              double_to_psd_fixed (points[p+3] / height, pointrecord + 10);
              double_to_psd_fixed (points[p+2] / width,  pointrecord + 14);
              double_to_psd_fixed (points[p+5] / height, pointrecord + 18);
              double_to_psd_fixed (points[p+4] / width,  pointrecord + 22);

              g_string_append_len (data, pointrecord, 26);
            }
        }

      g_free (strokes);

      /* fix up the length */

      len = data->len - len;
      data->str[lenpos + 0] = (len & 0xFF000000) >> 24;
      data->str[lenpos + 1] = (len & 0x00FF0000) >> 16;
      data->str[lenpos + 2] = (len & 0x0000FF00) >>  8;
      data->str[lenpos + 3] = (len & 0x000000FF) >>  0;

      g_string_append_len (ps_tag, data->str, data->len);
      g_string_free (data, TRUE);
      id ++;
    }

  TIFFSetField (tif, TIFFTAG_PHOTOSHOP, ps_tag->len, ps_tag->str);
  g_string_free (ps_tag, TRUE);

  g_free (vectors);

  return TRUE;
}


/*
** pnmtotiff.c - converts a portable anymap to a Tagged Image File
**
** Derived by Jef Poskanzer from ras2tif.c, which is:
**
** Copyright (c) 1990 by Sun Microsystems, Inc.
**
** Author: Patrick J. Naughton
** naughton@wind.sun.com
**
** This file is provided AS IS with no warranties of any kind.  The author
** shall have no liability with respect to the infringement of copyrights,
** trade secrets or any patents by this file or any part thereof.  In no
** event will the author be liable for any lost revenue or profits or
** other special, indirect and consequential damages.
*/

static gboolean
save_image (const gchar  *filename,
            gint32        image,
            gint32        layer,
            gint32        orig_image,  /* the export function might have */
            GError      **error)       /* created a duplicate            */
{
  TIFF          *tif;
  gushort        red[256];
  gushort        grn[256];
  gushort        blu[256];
  gint           cols, col, rows, row, i;
  glong          rowsperstrip;
  gushort        compression;
  gushort        extra_samples[1];
  gboolean       alpha;
  gshort         predictor;
  gshort         photometric;
  gshort         samplesperpixel;
  gshort         bitspersample;
  gint           bytesperrow;
  guchar        *t, *src, *data;
  guchar        *cmap;
  gint           num_colors;
  gint           success;
  PicmanDrawable  *drawable;
  PicmanImageType  drawable_type;
  PicmanPixelRgn   pixel_rgn;
  gint           tile_height;
  gint           y, yend;
  gboolean       is_bw    = FALSE;
  gboolean       invert   = TRUE;
  const guchar   bw_map[] = { 0, 0, 0, 255, 255, 255 };
  const guchar   wb_map[] = { 255, 255, 255, 0, 0, 0 };

  compression = tsvals.compression;

  /* Disabled because this isn't in older releases of libtiff, and it
     wasn't helping much anyway */
#if 0
  if (TIFFFindCODEC((uint16) compression) == NULL)
    compression = COMPRESSION_NONE; /* CODEC not available */
#endif

  predictor = 0;
  tile_height = picman_tile_height ();
  rowsperstrip = tile_height;

  tif = tiff_open (filename, "w", error);

  if (! tif)
    {
      if (! error)
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                     _("Could not open '%s' for writing: %s"),
                     picman_filename_to_utf8 (filename), g_strerror (errno));
      return FALSE;
    }

  TIFFSetWarningHandler (tiff_warning);
  TIFFSetErrorHandler (tiff_error);

  picman_progress_init_printf (_("Saving '%s'"),
                             picman_filename_to_utf8 (filename));

  drawable = picman_drawable_get (layer);
  drawable_type = picman_drawable_type (layer);
  picman_pixel_rgn_init (&pixel_rgn, drawable,
                       0, 0, drawable->width, drawable->height, FALSE, FALSE);

  cols = drawable->width;
  rows = drawable->height;

  picman_tile_cache_ntiles (1 + drawable->width / picman_tile_width ());

  switch (drawable_type)
    {
    case PICMAN_RGB_IMAGE:
      predictor       = 2;
      samplesperpixel = 3;
      bitspersample   = 8;
      photometric     = PHOTOMETRIC_RGB;
      bytesperrow     = cols * 3;
      alpha           = FALSE;
      break;

    case PICMAN_GRAY_IMAGE:
      samplesperpixel = 1;
      bitspersample   = 8;
      photometric     = PHOTOMETRIC_MINISBLACK;
      bytesperrow     = cols;
      alpha           = FALSE;
      break;

    case PICMAN_RGBA_IMAGE:
      predictor       = 2;
      samplesperpixel = 4;
      bitspersample   = 8;
      photometric     = PHOTOMETRIC_RGB;
      bytesperrow     = cols * 4;
      alpha           = TRUE;
      break;

    case PICMAN_GRAYA_IMAGE:
      samplesperpixel = 2;
      bitspersample   = 8;
      photometric     = PHOTOMETRIC_MINISBLACK;
      bytesperrow     = cols * 2;
      alpha           = TRUE;
      break;

    case PICMAN_INDEXED_IMAGE:
      cmap = picman_image_get_colormap (image, &num_colors);

      if (num_colors == 2 || num_colors == 1)
        {
          is_bw = (memcmp (cmap, bw_map, 3 * num_colors) == 0);
          photometric = PHOTOMETRIC_MINISWHITE;

          if (!is_bw)
            {
              is_bw = (memcmp (cmap, wb_map, 3 * num_colors) == 0);

              if (is_bw)
                invert = FALSE;
            }
       }

      if (is_bw)
        {
          bitspersample = 1;
        }
      else
        {
          bitspersample = 8;
          photometric   = PHOTOMETRIC_PALETTE;

          for (i = 0; i < num_colors; i++)
            {
              red[i] = cmap[i * 3 + 0] * 65535 / 255;
              grn[i] = cmap[i * 3 + 1] * 65535 / 255;
              blu[i] = cmap[i * 3 + 2] * 65535 / 255;
            }
       }

      samplesperpixel = 1;
      bytesperrow     = cols;
      alpha           = FALSE;

      g_free (cmap);
      break;

    case PICMAN_INDEXEDA_IMAGE:
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   "%s",
                   "TIFF save cannot handle indexed images with alpha channel.");
    default:
      return FALSE;
    }

  if (compression == COMPRESSION_CCITTFAX3 ||
      compression == COMPRESSION_CCITTFAX4)
    {
      if (bitspersample != 1 || samplesperpixel != 1)
        {
          g_message ("Only monochrome pictures can be compressed with \"CCITT Group 4\" or \"CCITT Group 3\".");
          return FALSE;
        }
    }

  /* Set TIFF parameters. */
  TIFFSetField (tif, TIFFTAG_SUBFILETYPE, 0);
  TIFFSetField (tif, TIFFTAG_IMAGEWIDTH, cols);
  TIFFSetField (tif, TIFFTAG_IMAGELENGTH, rows);
  TIFFSetField (tif, TIFFTAG_BITSPERSAMPLE, bitspersample);
  TIFFSetField (tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField (tif, TIFFTAG_COMPRESSION, compression);

  if ((compression == COMPRESSION_LZW || compression == COMPRESSION_DEFLATE)
      && (predictor != 0))
    {
      TIFFSetField (tif, TIFFTAG_PREDICTOR, predictor);
    }

  if (alpha)
    {
      if (tsvals.save_transp_pixels)
        extra_samples [0] = EXTRASAMPLE_UNASSALPHA;
      else
        extra_samples [0] = EXTRASAMPLE_ASSOCALPHA;

      TIFFSetField (tif, TIFFTAG_EXTRASAMPLES, 1, extra_samples);
    }

  TIFFSetField (tif, TIFFTAG_PHOTOMETRIC, photometric);
  TIFFSetField (tif, TIFFTAG_DOCUMENTNAME, filename);
  TIFFSetField (tif, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
  TIFFSetField (tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
  /* TIFFSetField( tif, TIFFTAG_STRIPBYTECOUNTS, rows / rowsperstrip ); */
  TIFFSetField (tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

  /* resolution fields */
  {
    gdouble  xresolution;
    gdouble  yresolution;
    gushort  save_unit = RESUNIT_INCH;
    PicmanUnit unit;
    gfloat   factor;

    picman_image_get_resolution (orig_image, &xresolution, &yresolution);
    unit = picman_image_get_unit (orig_image);
    factor = picman_unit_get_factor (unit);

    /*  if we have a metric unit, save the resolution as centimeters
     */
    if ((ABS (factor - 0.0254) < 1e-5) ||  /* m  */
        (ABS (factor - 0.254) < 1e-5) ||   /* dm */
        (ABS (factor - 2.54) < 1e-5) ||    /* cm */
        (ABS (factor - 25.4) < 1e-5))      /* mm */
      {
        save_unit = RESUNIT_CENTIMETER;
        xresolution /= 2.54;
        yresolution /= 2.54;
      }

    if (xresolution > 1e-5 && yresolution > 1e-5)
      {
        TIFFSetField (tif, TIFFTAG_XRESOLUTION, xresolution);
        TIFFSetField (tif, TIFFTAG_YRESOLUTION, yresolution);
        TIFFSetField (tif, TIFFTAG_RESOLUTIONUNIT, save_unit);
      }

/* TODO: enable in 2.6

    gint     offset_x, offset_y;

    picman_drawable_offsets (layer, &offset_x, &offset_y);

    if (offset_x || offset_y)
      {
        TIFFSetField (tif, TIFFTAG_XPOSITION, offset_x / xresolution);
        TIFFSetField (tif, TIFFTAG_YPOSITION, offset_y / yresolution);
      }
*/
  }

  /* The TIFF spec explicitely says ASCII for the image description. */
  if (image_comment)
    {
      const gchar *c = image_comment;
      gint         len;

      for (len = strlen (c); len; c++, len--)
        {
          if ((guchar) *c > 127)
            {
              g_message (_("The TIFF format only supports comments in\n"
                           "7bit ASCII encoding. No comment is saved."));

              g_free (image_comment);
              image_comment = NULL;

              break;
            }
        }
    }

  /* do we have a comment?  If so, create a new parasite to hold it,
   * and attach it to the image. The attach function automatically
   * detaches a previous incarnation of the parasite. */
  if (image_comment && *image_comment)
    {
      PicmanParasite *parasite;

      TIFFSetField (tif, TIFFTAG_IMAGEDESCRIPTION, image_comment);
      parasite = picman_parasite_new ("picman-comment",
                                    PICMAN_PARASITE_PERSISTENT,
                                    strlen (image_comment) + 1, image_comment);
      picman_image_attach_parasite (orig_image, parasite);
      picman_parasite_free (parasite);
    }

  /* do we have an ICC profile? If so, write it to the TIFF file */
#ifdef TIFFTAG_ICCPROFILE
  {
    PicmanParasite *parasite;
    uint32        profile_size;
    const guchar *icc_profile;

    parasite = picman_image_get_parasite (orig_image, "icc-profile");
    if (parasite)
      {
        profile_size = picman_parasite_data_size (parasite);
        icc_profile = picman_parasite_data (parasite);

        TIFFSetField (tif, TIFFTAG_ICCPROFILE, profile_size, icc_profile);
        picman_parasite_free (parasite);
      }
  }
#endif

  /* save path data */
  save_paths (tif, orig_image);

  if (!is_bw && drawable_type == PICMAN_INDEXED_IMAGE)
    TIFFSetField (tif, TIFFTAG_COLORMAP, red, grn, blu);

  /* array to rearrange data */
  src = g_new (guchar, bytesperrow * tile_height);
  data = g_new (guchar, bytesperrow);

  /* Now write the TIFF data. */
  for (y = 0; y < rows; y = yend)
    {
      yend = y + tile_height;
      yend = MIN (yend, rows);

      picman_pixel_rgn_get_rect (&pixel_rgn, src, 0, y, cols, yend - y);

      for (row = y; row < yend; row++)
        {
          t = src + bytesperrow * (row - y);

          switch (drawable_type)
            {
            case PICMAN_INDEXED_IMAGE:
              if (is_bw)
                {
                  byte2bit (t, bytesperrow, data, invert);
                  success = (TIFFWriteScanline (tif, data, row, 0) >= 0);
                }
              else
                {
                  success = (TIFFWriteScanline (tif, t, row, 0) >= 0);
                }
              break;

            case PICMAN_GRAY_IMAGE:
              success = (TIFFWriteScanline (tif, t, row, 0) >= 0);
              break;

            case PICMAN_GRAYA_IMAGE:
              for (col = 0; col < cols*samplesperpixel; col+=samplesperpixel)
                {
                  if (tsvals.save_transp_pixels)
                    {
                      data[col + 0] = t[col + 0];
                    }
                  else
                    {
                      /* pre-multiply gray by alpha */
                      data[col + 0] = (t[col + 0] * t[col + 1]) / 255;
                    }

                  data[col + 1] = t[col + 1];  /* alpha channel */
                }

              success = (TIFFWriteScanline (tif, data, row, 0) >= 0);
              break;

            case PICMAN_RGB_IMAGE:
              success = (TIFFWriteScanline (tif, t, row, 0) >= 0);
              break;

            case PICMAN_RGBA_IMAGE:
              for (col = 0; col < cols*samplesperpixel; col+=samplesperpixel)
                {
                  if (tsvals.save_transp_pixels)
                    {
                      data[col+0] = t[col + 0];
                      data[col+1] = t[col + 1];
                      data[col+2] = t[col + 2];
                    }
                  else
                    {
                      /* pre-multiply rgb by alpha */
                      data[col+0] = t[col + 0] * t[col + 3] / 255;
                      data[col+1] = t[col + 1] * t[col + 3] / 255;
                      data[col+2] = t[col + 2] * t[col + 3] / 255;
                    }

                  data[col+3] = t[col + 3];  /* alpha channel */
                }

              success = (TIFFWriteScanline (tif, data, row, 0) >= 0);
              break;

            default:
              success = FALSE;
              break;
            }

          if (!success)
            {
              g_message ("Failed a scanline write on row %d", row);
              return FALSE;
            }
        }

      if ((row % 32) == 0)
        picman_progress_update ((gdouble) row / (gdouble) rows);
    }

  TIFFFlushData (tif);
  TIFFClose (tif);

  picman_progress_update (1.0);

  picman_drawable_detach (drawable);
  g_free (data);
  g_free (src);

  return TRUE;
}

static gboolean
save_dialog (gboolean has_alpha,
             gboolean is_monochrome)
{
  GtkWidget *dialog;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *toggle;
  GtkWidget *g3;
  GtkWidget *g4;
  gboolean   run;

  dialog = picman_export_dialog_new (_("TIFF"), PLUG_IN_BINARY, SAVE_PROC);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (picman_export_dialog_get_content_area (dialog)),
                      vbox, FALSE, TRUE, 0);

  /*  compression  */
  frame = picman_int_radio_group_new (TRUE, _("Compression"),
                                    G_CALLBACK (picman_radio_button_update),
                                    &tsvals.compression, tsvals.compression,

                                    _("_None"),      COMPRESSION_NONE,     NULL,
                                    _("_LZW"),       COMPRESSION_LZW,      NULL,
                                    _("_Pack Bits"), COMPRESSION_PACKBITS, NULL,
                                    _("_Deflate"),   COMPRESSION_DEFLATE,  NULL,
                                    _("_JPEG"),      COMPRESSION_JPEG,     NULL,
                                    _("CCITT Group _3 fax"), COMPRESSION_CCITTFAX3, &g3,
                                    _("CCITT Group _4 fax"), COMPRESSION_CCITTFAX4, &g4,

                                    NULL);

  gtk_widget_set_sensitive (g3, is_monochrome);
  gtk_widget_set_sensitive (g4, is_monochrome);

  if (! is_monochrome)
    {
      if (tsvals.compression == COMPRESSION_CCITTFAX3 ||
          tsvals.compression ==  COMPRESSION_CCITTFAX4)
        {
          picman_int_radio_group_set_active (GTK_RADIO_BUTTON (g3),
                                           COMPRESSION_NONE);
        }
    }

  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /* Keep colors behind alpha mask */
  toggle = gtk_check_button_new_with_mnemonic
    ( _("Save _color values from transparent pixels"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                has_alpha && tsvals.save_transp_pixels);
  gtk_widget_set_sensitive (toggle, has_alpha);
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &tsvals.save_transp_pixels);

  /* comment entry */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new ( _("Comment:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  gtk_entry_set_text (GTK_ENTRY (entry), image_comment ? image_comment : "");

  g_signal_connect (entry, "changed",
                    G_CALLBACK (comment_entry_callback),
                    NULL);

  gtk_widget_show (frame);

  gtk_widget_show (vbox);
  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
comment_entry_callback (GtkWidget *widget,
                        gpointer   data)
{
  const gchar *text = gtk_entry_get_text (GTK_ENTRY (widget));

  g_free (image_comment);
  image_comment = g_strdup (text);
}

/* Convert n bytes of 0/1 to a line of bits */
static void
byte2bit (const guchar *byteline,
          gint          width,
          guchar       *bitline,
          gboolean      invert)
{
  guchar bitval;
  guchar rest[8];

  while (width >= 8)
    {
      bitval = 0;
      if (*(byteline++)) bitval |= 0x80;
      if (*(byteline++)) bitval |= 0x40;
      if (*(byteline++)) bitval |= 0x20;
      if (*(byteline++)) bitval |= 0x10;
      if (*(byteline++)) bitval |= 0x08;
      if (*(byteline++)) bitval |= 0x04;
      if (*(byteline++)) bitval |= 0x02;
      if (*(byteline++)) bitval |= 0x01;
      *(bitline++) = invert ? ~bitval : bitval;
      width -= 8;
    }
  if (width > 0)
    {
      memset (rest, 0, 8);
      memcpy (rest, byteline, width);
      bitval = 0;
      byteline = rest;
      if (*(byteline++)) bitval |= 0x80;
      if (*(byteline++)) bitval |= 0x40;
      if (*(byteline++)) bitval |= 0x20;
      if (*(byteline++)) bitval |= 0x10;
      if (*(byteline++)) bitval |= 0x08;
      if (*(byteline++)) bitval |= 0x04;
      if (*(byteline++)) bitval |= 0x02;
      *bitline = invert ? ~bitval & (0xff << (8 - width)) : bitval;
    }
}
