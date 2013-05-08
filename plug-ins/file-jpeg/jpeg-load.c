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
#include <errno.h>
#include <setjmp.h>

#include <glib/gstdio.h>

#include <jpeglib.h>
#include <jerror.h>

#ifdef HAVE_LIBEXIF
#include <libexif/exif-data.h>
#endif /* HAVE_LIBEXIF */

#ifdef HAVE_LCMS
#include <lcms2.h>
#endif

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"

#include "jpeg.h"
#include "jpeg-icc.h"
#include "jpeg-settings.h"
#include "jpeg-load.h"
#ifdef HAVE_LIBEXIF
#include "jpeg-exif.h"
#include "picmanexif.h"
#endif


static void  jpeg_load_resolution           (gint32    image_ID,
                                             struct jpeg_decompress_struct
                                                       *cinfo);

#ifdef HAVE_LIBEXIF
static gboolean  jpeg_load_exif_resolution  (gint32    image_ID,
                                             ExifData *exif_data);
#endif

static void      jpeg_load_sanitize_comment (gchar    *comment);

static gpointer  jpeg_load_cmyk_transform   (guint8   *profile_data,
                                             gsize     profile_len);
static void      jpeg_load_cmyk_to_rgb      (guchar   *buf,
                                             glong     pixels,
                                             gpointer  transform);


PicmanDrawable    *drawable_global;
gint32 volatile  preview_image_ID;
gint32           preview_layer_ID;


gint32
load_image (const gchar  *filename,
            PicmanRunMode   runmode,
            gboolean      preview,
            GError      **error)
{
  PicmanPixelRgn     pixel_rgn;
  PicmanDrawable    *drawable;
  gint32 volatile  image_ID;
  gint32           layer_ID;
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr           jerr;
  jpeg_saved_marker_ptr         marker;
  FILE            *infile;
  guchar          *buf;
  guchar         **rowbuf;
  gint             image_type;
  gint             layer_type;
  gint             tile_height;
  gint             scanlines;
  gint             i, start, end;
#ifdef HAVE_LIBEXIF
  gint             orientation = 0;
#endif
#ifdef HAVE_LCMS
  cmsHTRANSFORM    cmyk_transform = NULL;
#else
  gpointer         cmyk_transform = NULL;
#endif

  /* We set up the normal JPEG error routines. */
  cinfo.err = jpeg_std_error (&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  if (!preview)
    {
      jerr.pub.output_message = my_output_message;
    }

  if ((infile = g_fopen (filename, "rb")) == NULL)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return -1;
    }

  if (!preview)
    picman_progress_init_printf (_("Opening '%s'"),
                               picman_filename_to_utf8 (filename));

  image_ID = -1;

  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp (jerr.setjmp_buffer))
    {
      /* If we get here, the JPEG code has signaled an error.
       * We need to clean up the JPEG object, close the input file, and return.
       */
      jpeg_destroy_decompress (&cinfo);
      if (infile)
        fclose (infile);

      if (image_ID != -1 && !preview)
        picman_image_delete (image_ID);

      if (preview)
        destroy_preview ();

      return -1;
    }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress (&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src (&cinfo, infile);

  if (! preview)
    {
      /* - step 2.1: tell the lib to save the comments */
      jpeg_save_markers (&cinfo, JPEG_COM, 0xffff);

      /* - step 2.2: tell the lib to save APP1 data (EXIF or XMP) */
      jpeg_save_markers (&cinfo, JPEG_APP0 + 1, 0xffff);

      /* - step 2.3: tell the lib to save APP2 data (ICC profiles) */
      jpeg_save_markers (&cinfo, JPEG_APP0 + 2, 0xffff);
    }

  /* Step 3: read file parameters with jpeg_read_header() */

  jpeg_read_header (&cinfo, TRUE);

  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  jpeg_start_decompress (&cinfo);

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   */

  /* temporary buffer */
  tile_height = picman_tile_height ();
  buf = g_new (guchar,
               tile_height * cinfo.output_width * cinfo.output_components);

  rowbuf = g_new (guchar *, tile_height);

  for (i = 0; i < tile_height; i++)
    rowbuf[i] = buf + cinfo.output_width * cinfo.output_components * i;

  switch (cinfo.output_components)
    {
    case 1:
      image_type = PICMAN_GRAY;
      layer_type = PICMAN_GRAY_IMAGE;
      break;

    case 3:
      image_type = PICMAN_RGB;
      layer_type = PICMAN_RGB_IMAGE;
      break;

    case 4:
      if (cinfo.out_color_space == JCS_CMYK)
        {
          image_type = PICMAN_RGB;
          layer_type = PICMAN_RGB_IMAGE;
          break;
        }
      /*fallthrough*/

    default:
      g_message ("Don't know how to load JPEG images "
                 "with %d color channels, using colorspace %d (%d).",
                 cinfo.output_components, cinfo.out_color_space,
                 cinfo.jpeg_color_space);
      return -1;
      break;
    }

  if (preview)
    {
      image_ID = preview_image_ID;
    }
  else
    {
      image_ID = picman_image_new (cinfo.output_width, cinfo.output_height,
                                 image_type);

      picman_image_undo_disable (image_ID);
      picman_image_set_filename (image_ID, filename);
    }

  if (preview)
    {
      preview_layer_ID = picman_layer_new (preview_image_ID, _("JPEG preview"),
                                         cinfo.output_width,
                                         cinfo.output_height,
                                         layer_type, 100, PICMAN_NORMAL_MODE);
      layer_ID = preview_layer_ID;
    }
  else
    {
      layer_ID = picman_layer_new (image_ID, _("Background"),
                                 cinfo.output_width,
                                 cinfo.output_height,
                                 layer_type, 100, PICMAN_NORMAL_MODE);
    }

  drawable_global = drawable = picman_drawable_get (layer_ID);
  picman_pixel_rgn_init (&pixel_rgn, drawable, 0, 0,
                       drawable->width, drawable->height, TRUE, FALSE);

  if (! preview)
    {
      GString  *comment_buffer = NULL;
      guint8   *profile        = NULL;
      guint     profile_size   = 0;
#ifdef HAVE_LIBEXIF
      ExifData *exif_data      = NULL;
#endif

      /* Step 5.0: save the original JPEG settings in a parasite */
      jpeg_detect_original_settings (&cinfo, image_ID);

      /* Step 5.1: check for comments, or EXIF metadata in APP1 markers */
      for (marker = cinfo.marker_list; marker; marker = marker->next)
        {
          const gchar *data = (const gchar *) marker->data;
          gsize        len  = marker->data_length;

          if (marker->marker == JPEG_COM)
            {
#ifdef PICMAN_UNSTABLE
              g_print ("jpeg-load: found image comment (%d bytes)\n",
                       marker->data_length);
#endif

              if (! comment_buffer)
                {
                  comment_buffer = g_string_new_len (data, len);
                }
              else
                {
                  /* concatenate multiple comments, separate them with LF */
                  g_string_append_c (comment_buffer, '\n');
                  g_string_append_len (comment_buffer, data, len);
                }
            }
          else if ((marker->marker == JPEG_APP0 + 1)
                   && (len > sizeof (JPEG_APP_HEADER_EXIF) + 8)
                   && ! strcmp (JPEG_APP_HEADER_EXIF, data))
            {
#ifdef PICMAN_UNSTABLE
              g_print ("jpeg-load: found EXIF block (%d bytes)\n",
                       (gint) (len - sizeof (JPEG_APP_HEADER_EXIF)));
#endif
#ifdef HAVE_LIBEXIF
              if (! exif_data)
                exif_data = exif_data_new ();
              /* if there are multiple blocks, their data will be merged */
              exif_data_load_data (exif_data, (unsigned char *) data, len);
#endif
            }
        }

#ifdef HAVE_LIBEXIF
      if (!jpeg_load_exif_resolution (image_ID, exif_data))
#endif
        jpeg_load_resolution (image_ID, &cinfo);

      /* if we found any comments, then make a parasite for them */
      if (comment_buffer && comment_buffer->len)
        {
          PicmanParasite *parasite;

          jpeg_load_sanitize_comment (comment_buffer->str);
          parasite = picman_parasite_new ("picman-comment",
                                        PICMAN_PARASITE_PERSISTENT,
                                        strlen (comment_buffer->str) + 1,
                                        comment_buffer->str);
          picman_image_attach_parasite (image_ID, parasite);
          picman_parasite_free (parasite);

          g_string_free (comment_buffer, TRUE);
        }

#ifdef HAVE_LIBEXIF
      /* if we found any EXIF block, then attach the metadata to the image */
      if (exif_data)
        {
          picman_metadata_store_exif (image_ID, exif_data);
          orientation = jpeg_exif_get_orientation (exif_data);
          exif_data_unref (exif_data);
          exif_data = NULL;
        }
#endif

      /* Step 5.2: check for XMP metadata in APP1 markers (after EXIF) */
      for (marker = cinfo.marker_list; marker; marker = marker->next)
        {
          const gchar *data = (const gchar *) marker->data;
          gsize        len  = marker->data_length;

          if ((marker->marker == JPEG_APP0 + 1)
              && (len > sizeof (JPEG_APP_HEADER_XMP) + 20)
              && ! strcmp (JPEG_APP_HEADER_XMP, data))
            {
              PicmanParam *return_vals;
              gint       nreturn_vals;
              gchar     *xmp_packet;

#ifdef PICMAN_UNSTABLE
              g_print ("jpeg-load: found XMP packet (%d bytes)\n",
                       (gint) (len - sizeof (JPEG_APP_HEADER_XMP)));
#endif
              xmp_packet = g_strndup (data + sizeof (JPEG_APP_HEADER_XMP),
                                      len - sizeof (JPEG_APP_HEADER_XMP));

              /* FIXME: running this through the PDB is not very efficient */
              return_vals = picman_run_procedure ("plug-in-metadata-decode-xmp",
                                                &nreturn_vals,
                                                PICMAN_PDB_IMAGE, image_ID,
                                                PICMAN_PDB_STRING, xmp_packet,
                                                PICMAN_PDB_END);

              if (return_vals[0].data.d_status != PICMAN_PDB_SUCCESS)
                {
                  g_warning ("JPEG - unable to decode XMP metadata packet");
                }

              picman_destroy_params (return_vals, nreturn_vals);
              g_free (xmp_packet);
            }
        }

      /* Step 5.3: check for an embedded ICC profile in APP2 markers */
      jpeg_icc_read_profile (&cinfo, &profile, &profile_size);

      if (cinfo.out_color_space == JCS_CMYK)
        {
          cmyk_transform = jpeg_load_cmyk_transform (profile, profile_size);
        }
      else if (profile) /* don't attach the profile if we are transforming */
        {
          PicmanParasite *parasite;

          parasite = picman_parasite_new ("icc-profile",
                                        PICMAN_PARASITE_PERSISTENT |
                                        PICMAN_PARASITE_UNDOABLE,
                                        profile_size, profile);
          picman_image_attach_parasite (image_ID, parasite);
          picman_parasite_free (parasite);
        }

      g_free (profile);

      /* Do not attach the "jpeg-save-options" parasite to the image
       * because this conflicts with the global defaults (bug #75398).
       */
    }

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  while (cinfo.output_scanline < cinfo.output_height)
    {
      start = cinfo.output_scanline;
      end   = cinfo.output_scanline + tile_height;
      end   = MIN (end, cinfo.output_height);

      scanlines = end - start;

      for (i = 0; i < scanlines; i++)
        jpeg_read_scanlines (&cinfo, (JSAMPARRAY) &rowbuf[i], 1);

      if (cinfo.out_color_space == JCS_CMYK)
        jpeg_load_cmyk_to_rgb (buf, drawable->width * scanlines,
                               cmyk_transform);

      picman_pixel_rgn_set_rect (&pixel_rgn, buf,
                               0, start, drawable->width, scanlines);

      if (! preview && (cinfo.output_scanline % 32) == 0)
        picman_progress_update ((gdouble) cinfo.output_scanline /
                              (gdouble) cinfo.output_height);
    }

  /* Step 7: Finish decompression */

  jpeg_finish_decompress (&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

#ifdef HAVE_LCMS
  if (cmyk_transform)
    cmsDeleteTransform (cmyk_transform);
#endif

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress (&cinfo);

  /* free up the temporary buffers */
  g_free (rowbuf);
  g_free (buf);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */
  fclose (infile);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.num_warnings is nonzero).
   */

  /* Detach from the drawable and add it to the image.
   */
  if (! preview)
    {
      picman_progress_update (1.0);
      picman_drawable_detach (drawable);
    }

  picman_image_insert_layer (image_ID, layer_ID, -1, 0);

#ifdef HAVE_LIBEXIF
  jpeg_exif_rotate_query (image_ID, orientation);
#endif

  return image_ID;
}

static void
jpeg_load_resolution (gint32                         image_ID,
                      struct jpeg_decompress_struct *cinfo)
{
  if (cinfo->saw_JFIF_marker && cinfo->X_density != 0 && cinfo->Y_density != 0)
    {
      gdouble xresolution = cinfo->X_density;
      gdouble yresolution = cinfo->Y_density;
      gdouble asymmetry   = 1.0;

      switch (cinfo->density_unit)
        {
        case 0: /* unknown -> set the aspect ratio but use the default
                 *  image resolution
                 */
          asymmetry = xresolution / yresolution;

          picman_image_get_resolution (image_ID, &xresolution, &yresolution);

          xresolution *= asymmetry;
          break;

        case 1: /* dots per inch */
          break;

        case 2: /* dots per cm */
          xresolution *= 2.54;
          yresolution *= 2.54;
          picman_image_set_unit (image_ID, PICMAN_UNIT_MM);
          break;

        default:
          g_message ("Unknown density unit %d, assuming dots per inch.",
                     cinfo->density_unit);
          break;
        }

      picman_image_set_resolution (image_ID, xresolution, yresolution);
    }
}

#ifdef HAVE_LIBEXIF

static gboolean
jpeg_load_exif_resolution (gint32        image_ID,
                           ExifData     *exif_data)
{
  gboolean success;
  gdouble xresolution;
  gdouble yresolution;
  gint    unit;

  if (!exif_data)
    return FALSE;

  if (!jpeg_exif_get_resolution (exif_data,
                                 &xresolution,
                                 &yresolution,
                                 &unit))
    return FALSE;

  switch (unit)
    {
    case 2:
      success = TRUE;
      break;
    case 3: /* dots per cm */
      xresolution *= 2.54;
      yresolution *= 2.54;
      picman_image_set_unit (image_ID, PICMAN_UNIT_MM);
      success = TRUE;
      break;
    default:
      g_warning ("Unknown EXIF resolution unit %d; skipping EXIF resolution.",
                 unit);
      success = FALSE;
    }

  if (success)
    {
      picman_image_set_resolution (image_ID, xresolution, yresolution);
    }

  return success;
}

#endif /* HAVE_LIBEXIF */

/*
 * A number of JPEG files have comments written in a local character set
 * instead of UTF-8.  Some of these files may have been saved by older
 * versions of PICMAN.  It is not possible to reliably detect the character
 * set used, but it is better to keep all characters in the ASCII range
 * and replace the non-ASCII characters instead of discarding the whole
 * comment.  This is especially useful if the comment contains only a few
 * non-ASCII characters such as a copyright sign, a soft hyphen, etc.
 */
static void
jpeg_load_sanitize_comment (gchar *comment)
{
  if (! g_utf8_validate (comment, -1, NULL))
    {
      gchar *c;

      for (c = comment; *c; c++)
        {
          if (*c > 126 || (*c < 32 && *c != '\t' && *c != '\n' && *c != '\r'))
            *c = '?';
        }
    }
}


#ifdef HAVE_LIBEXIF

typedef struct
{
  struct jpeg_source_mgr pub;   /* public fields */

  guchar  *buffer;
  gint     size;
  JOCTET   terminal[2];
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

static void
init_source (j_decompress_ptr cinfo)
{
}


static boolean
fill_input_buffer (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* Since we have given all we have got already
   * we simply fake an end of file
   */

  src->pub.next_input_byte = src->terminal;
  src->pub.bytes_in_buffer = 2;
  src->terminal[0]         = (JOCTET) 0xFF;
  src->terminal[1]         = (JOCTET) JPEG_EOI;

  return TRUE;
}

static void
skip_input_data (j_decompress_ptr cinfo,
                 long             num_bytes)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  src->pub.next_input_byte = src->pub.next_input_byte + num_bytes;
}

static void
term_source (j_decompress_ptr cinfo)
{
}

gint32
load_thumbnail_image (const gchar   *filename,
                      gint          *width,
                      gint          *height,
                      PicmanImageType *type,
                      GError       **error)
{
  gint32 volatile  image_ID;
  ExifData        *exif_data;
  PicmanPixelRgn     pixel_rgn;
  PicmanDrawable    *drawable;
  gint32           layer_ID;
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr           jerr;
  guchar          *buf;
  guchar         **rowbuf;
  gint             image_type;
  gint             layer_type;
  gint             tile_height;
  gint             scanlines;
  gint             i, start, end;
  gint             orientation;
  my_src_ptr       src;
  FILE            *infile;

  image_ID = -1;
  exif_data = jpeg_exif_data_new_from_file (filename, NULL);

  if (! ((exif_data) && (exif_data->data) && (exif_data->size > 0)))
    return -1;

  orientation = jpeg_exif_get_orientation (exif_data);

  cinfo.err = jpeg_std_error (&jerr.pub);
  jerr.pub.error_exit     = my_error_exit;
  jerr.pub.output_message = my_output_message;

  picman_progress_init_printf (_("Opening thumbnail for '%s'"),
                             picman_filename_to_utf8 (filename));

  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp (jerr.setjmp_buffer))
    {
      /* If we get here, the JPEG code has signaled an error.  We
       * need to clean up the JPEG object, close the input file,
       * and return.
       */
      jpeg_destroy_decompress (&cinfo);

      if (image_ID != -1)
        picman_image_delete (image_ID);

      if (exif_data)
        {
          exif_data_unref (exif_data);
          exif_data = NULL;
        }

      return -1;
    }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress (&cinfo);

  /* Step 2: specify data source (eg, a file) */

  if (cinfo.src == NULL)
    cinfo.src = (struct jpeg_source_mgr *)(*cinfo.mem->alloc_small)
      ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
       sizeof (my_source_mgr));

  src = (my_src_ptr) cinfo.src;

  src->pub.init_source       = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data   = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart;
  src->pub.term_source       = term_source;

  src->pub.bytes_in_buffer   = exif_data->size;
  src->pub.next_input_byte   = exif_data->data;

  src->buffer = exif_data->data;
  src->size = exif_data->size;

  /* Step 3: read file parameters with jpeg_read_header() */

  jpeg_read_header (&cinfo, TRUE);

  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  jpeg_start_decompress (&cinfo);

  /* We may need to do some setup of our own at this point before
   * reading the data.  After jpeg_start_decompress() we have the
   * correct scaled output image dimensions available, as well as
   * the output colormap if we asked for color quantization.  In
   * this example, we need to make an output work buffer of the
   * right size.
   */

  /* temporary buffer */
  tile_height = picman_tile_height ();
  buf = g_new (guchar,
               tile_height * cinfo.output_width * cinfo.output_components);

  rowbuf = g_new (guchar *, tile_height);

  for (i = 0; i < tile_height; i++)
    rowbuf[i] = buf + cinfo.output_width * cinfo.output_components * i;

  /* Create a new image of the proper size and associate the
   * filename with it.
   */
  switch (cinfo.output_components)
    {
    case 1:
      image_type = PICMAN_GRAY;
      layer_type = PICMAN_GRAY_IMAGE;
      break;

    case 3:
      image_type = PICMAN_RGB;
      layer_type = PICMAN_RGB_IMAGE;
      break;

    case 4:
      if (cinfo.out_color_space == JCS_CMYK)
        {
          image_type = PICMAN_RGB;
          layer_type = PICMAN_RGB_IMAGE;
          break;
        }
      /*fallthrough*/

    default:
      g_message ("Don't know how to load JPEG images "
                 "with %d color channels, using colorspace %d (%d).",
                 cinfo.output_components, cinfo.out_color_space,
                 cinfo.jpeg_color_space);

      if (exif_data)
        {
          exif_data_unref (exif_data);
          exif_data = NULL;
        }

      return -1;
      break;
    }

  image_ID = picman_image_new (cinfo.output_width, cinfo.output_height,
                             image_type);

  picman_image_undo_disable (image_ID);
  picman_image_set_filename (image_ID, filename);

  jpeg_load_resolution (image_ID, &cinfo);

  layer_ID = picman_layer_new (image_ID, _("Background"),
                             cinfo.output_width,
                             cinfo.output_height,
                             layer_type, 100, PICMAN_NORMAL_MODE);

  drawable_global = drawable = picman_drawable_get (layer_ID);
  picman_pixel_rgn_init (&pixel_rgn, drawable, 0, 0,
                       drawable->width, drawable->height, TRUE, FALSE);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  while (cinfo.output_scanline < cinfo.output_height)
    {
      start = cinfo.output_scanline;
      end   = cinfo.output_scanline + tile_height;
      end   = MIN (end, cinfo.output_height);
      scanlines = end - start;

      for (i = 0; i < scanlines; i++)
        jpeg_read_scanlines (&cinfo, (JSAMPARRAY) &rowbuf[i], 1);

      if (cinfo.out_color_space == JCS_CMYK)
        jpeg_load_cmyk_to_rgb (buf, drawable->width * scanlines, NULL);

      picman_pixel_rgn_set_rect (&pixel_rgn, buf,
                               0, start, drawable->width, scanlines);

      picman_progress_update ((gdouble) cinfo.output_scanline /
                            (gdouble) cinfo.output_height);
    }

  /* Step 7: Finish decompression */

  jpeg_finish_decompress (&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal
   * of memory.
   */
  jpeg_destroy_decompress (&cinfo);

  /* free up the temporary buffers */
  g_free (rowbuf);
  g_free (buf);

  /* At this point you may want to check to see whether any
   * corrupt-data warnings occurred (test whether
   * jerr.num_warnings is nonzero).
   */
  picman_image_insert_layer (image_ID, layer_ID, -1, 0);


  /* NOW to get the dimensions of the actual image to return the
   * calling app
   */
  cinfo.err = jpeg_std_error (&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  jerr.pub.output_message = my_output_message;

  if ((infile = g_fopen (filename, "rb")) == NULL)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));

      if (exif_data)
        {
          exif_data_unref (exif_data);
          exif_data = NULL;
        }

      return -1;
    }

  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp (jerr.setjmp_buffer))
    {
      /* If we get here, the JPEG code has signaled an error.  We
       * need to clean up the JPEG object, close the input file,
       * and return.
       */
      jpeg_destroy_decompress (&cinfo);

      if (image_ID != -1)
        picman_image_delete (image_ID);

      if (exif_data)
        {
          exif_data_unref (exif_data);
          exif_data = NULL;
        }

      return -1;
    }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress (&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src (&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  jpeg_read_header (&cinfo, TRUE);

  jpeg_start_decompress (&cinfo);

  *width  = cinfo.output_width;
  *height = cinfo.output_height;

  /* Step 4: Release JPEG decompression object */

  /* This is an important step since it will release a good deal
   * of memory.
   */
  jpeg_destroy_decompress (&cinfo);

  fclose (infile);

  if (exif_data)
    {
      exif_data_unref (exif_data);
      exif_data = NULL;
    }

  jpeg_exif_rotate (image_ID, orientation);

  *type = layer_type;

  return image_ID;
}

#endif /* HAVE_LIBEXIF */


static gpointer
jpeg_load_cmyk_transform (guint8 *profile_data,
                          gsize   profile_len)
{
#ifdef HAVE_LCMS
  PicmanColorConfig *config       = picman_get_color_configuration ();
  cmsHPROFILE      cmyk_profile = NULL;
  cmsHPROFILE      rgb_profile  = NULL;
  cmsUInt32Number  flags        = 0;
  cmsHTRANSFORM    transform;

  /*  try to load the embedded CMYK profile  */
  if (profile_data)
    {
      cmyk_profile = cmsOpenProfileFromMem (profile_data, profile_len);

      if (cmyk_profile)
        {
          if (! cmsGetColorSpace (cmyk_profile) == cmsSigCmykData)
            {
              cmsCloseProfile (cmyk_profile);
              cmyk_profile = NULL;
            }
        }
    }

  /*  if that fails, try to load the CMYK profile configured in the prefs  */
  if (! cmyk_profile && config->cmyk_profile)
    {
      cmyk_profile = cmsOpenProfileFromFile (config->cmyk_profile, "r");

      if (cmyk_profile && ! cmsGetColorSpace (cmyk_profile) == cmsSigCmykData)
        {
          cmsCloseProfile (cmyk_profile);
          cmyk_profile = NULL;
        }
    }

  /*  bail out if we can't load any CMYK profile  */
  if (! cmyk_profile)
    {
      g_object_unref (config);
      return NULL;
    }

  /*  try to load the RGB profile configured in the prefs  */
  if (config->rgb_profile)
    {
      rgb_profile = cmsOpenProfileFromFile (config->rgb_profile, "r");

      if (rgb_profile && ! cmsGetColorSpace (rgb_profile) == cmsSigRgbData)
        {
          cmsCloseProfile (rgb_profile);
          rgb_profile = NULL;
        }
    }

  /*  use the built-in sRGB profile as fallback  */
  if (! rgb_profile)
    {
      rgb_profile = cmsCreate_sRGBProfile ();
    }

  if (config->display_intent ==
      PICMAN_COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC)
    {
      flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
    }

  transform = cmsCreateTransform (cmyk_profile, TYPE_CMYK_8_REV,
                                  rgb_profile,  TYPE_RGB_8,
                                  config->display_intent,
                                  flags);

  cmsCloseProfile (cmyk_profile);
  cmsCloseProfile (rgb_profile);

  g_object_unref (config);

  return transform;
#else  /* HAVE_LCMS */
  return NULL;
#endif
}


static void
jpeg_load_cmyk_to_rgb (guchar   *buf,
                       glong     pixels,
                       gpointer  transform)
{
  const guchar *src  = buf;
  guchar       *dest = buf;

#ifdef HAVE_LCMS
  if (transform)
    {
      cmsDoTransform (transform, buf, buf, pixels);
      return;
    }
#endif

  while (pixels--)
    {
      guint c = src[0];
      guint m = src[1];
      guint y = src[2];
      guint k = src[3];

      dest[0] = (c * k) / 255;
      dest[1] = (m * k) / 255;
      dest[2] = (y * k) / 255;

      src  += 4;
      dest += 3;
    }
}
