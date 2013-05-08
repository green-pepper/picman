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
 * EXIF-handling code for the metadata library.
 */

#include "config.h"

#include <stdlib.h>

#include <libpicman/picman.h>

#include <libexif/exif-data.h>
#include <libexif/exif-content.h>
#include <libexif/exif-utils.h>

#include "picmanexif.h"


#define EXIF_HEADER_SIZE 8

/*
 * picman_metadata_store_exif:
 * @image_ID:    the PICMAN image to work on.
 * @exif_data:   the Exif data that is to be stored.
 *
 * This function is supposed to load the "picman-metadata" parasite
 * (which is in XMP format), parse it, add the exif information,
 * reformat it into XMP, and store the result as the new parasite.
 * The infrastructure to do this is not yet available, so for the
 * moment it does something much simpler -- it calls upon libexif
 * to serialize the exif data, and stores the result in a parasite
 * called "exif-data".
 */
void picman_metadata_store_exif    (gint32    image_ID,
                                  ExifData *exif_data)
{
  PicmanParam    *return_vals;
  gint          nreturn_vals;
  PicmanParasite *parasite      = NULL;
  guchar       *exif_buf      = NULL;
  guint         exif_buf_len  = 0;

  exif_data_save_data (exif_data, &exif_buf, &exif_buf_len);

  if (exif_buf_len > EXIF_HEADER_SIZE)
    {
      parasite = picman_parasite_new ("exif-data",
                                    PICMAN_PARASITE_PERSISTENT,
                                    exif_buf_len, exif_buf);
      picman_image_attach_parasite (image_ID, parasite);
      picman_parasite_free (parasite);
    }
  return_vals = picman_run_procedure ("plug-in-metadata-decode-exif",
                                    &nreturn_vals,
                                    PICMAN_PDB_IMAGE,      image_ID,
                                    PICMAN_PDB_INT32,      7,
                                    PICMAN_PDB_INT8ARRAY,  "unused",
                                    PICMAN_PDB_END);
  if (return_vals[0].data.d_status != PICMAN_PDB_SUCCESS)
    g_warning ("JPEG Exif -> XMP Merge failed");

  free (exif_buf);
}

/*
 * picman_metadata_generate_exif:
 * @image_ID:   the ID of the PICMAN image to work on.
 *
 * This function is supposed to load the "picman-metadata" parasite
 * (which is a block of XMP info), parse it, extract the exif
 * values, and build an ExifData structure from them.
 * The infrastructure to do this is not yet available, so for the
 * moment it does something much simpler -- it loads the "exif-data"
 * parasite (which is a serialized representation of the exif data),
 * and calls upon libexif to parse it into an ExifData struct.
 *
 * returns: The reconstructed exif data, or NULL if none exists.
 */
ExifData *
picman_metadata_generate_exif (gint32 image_ID)
{
  ExifData     *exif_data;
  PicmanParasite *parasite = picman_image_get_parasite (image_ID, "exif-data");

  if (parasite)
    {
      exif_data = exif_data_new_from_data (picman_parasite_data (parasite),
                                           picman_parasite_data_size (parasite));

      picman_parasite_free (parasite);
      return exif_data;
    }

  return NULL;
}

/*
 * picman_exif_content_get_value:
 * @content:   ExifContent block from which to get value
 * @tag:       Tag whose value to get
 * @value:     Place to put the result
 * @maxlen:    Maximum size of returned string
 *
 * This function is a wrapper around the libexif function
 * exif_content_get_value(), necessary to deal with an incompatible
 * API change.  It looks up the value of the specified tag,
 * returning the result as a human-readable string.  Note that
 * @value must be pre-allocated.
 */
const gchar *
picman_exif_content_get_value (ExifContent *content,
                             ExifTag      tag,
                             gchar       *value,
                             gint         maxlen)
{
  ExifEntry *entry = exif_content_get_entry (content, tag);

  if (entry)
    return exif_entry_get_value (entry, value, maxlen);
  else
    return NULL;
}

/*
 * picman_exif_data_remove_entry:
 * @data:      ExifData pointer
 * @ifd:       Index of the ifd holding the entry to be removed
 * @tag:       Tag of the entry to be removed
 *
 * This is a convenience function for removing a specified
 * entry from an exif data structure.  If no such entry
 * exists, the function returns without doing anything.
 */
void
picman_exif_data_remove_entry (ExifData *exif_data,
                             ExifIfd   ifd,
                             ExifTag   tag)
{
  ExifEntry  *entry = exif_content_get_entry (exif_data->ifd[ifd],
                                              tag);

  if (entry)
    exif_content_remove_entry (exif_data->ifd[ifd], entry);
}
