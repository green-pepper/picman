/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * Thumbnail handling according to the Thumbnail Managing Standard.
 * http://triq.net/~pearl/thumbnail-spec/
 *
 * Copyright (C) 2001-2004  Sven Neumann <sven@picman.org>
 *                          Michael Natterer <mitch@picman.org>
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
#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib/gstdio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanparam.h"

#ifdef G_OS_WIN32
#include "libpicmanbase/picmanwin32-io.h"
#include <process.h>
#define _getpid getpid
#endif

#include "picmanthumb-types.h"
#include "picmanthumb-error.h"
#include "picmanthumb-utils.h"
#include "picmanthumbnail.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmanthumbnail
 * @title: PicmanThumbnail
 * @short_description: The PicmanThumbnail object
 *
 * The PicmanThumbnail object
 **/


/*  #define PICMAN_THUMB_DEBUG  */


#if defined (PICMAN_THUMB_DEBUG) && defined (__GNUC__)
#define PICMAN_THUMB_DEBUG_CALL(t) \
        g_printerr ("%s: %s\n", \
                     __FUNCTION__, t->image_uri ? t->image_uri : "(null)")
#else
#define PICMAN_THUMB_DEBUG_CALL(t) ((void)(0))
#endif


#define TAG_DESCRIPTION           "tEXt::Description"
#define TAG_SOFTWARE              "tEXt::Software"
#define TAG_THUMB_URI             "tEXt::Thumb::URI"
#define TAG_THUMB_MTIME           "tEXt::Thumb::MTime"
#define TAG_THUMB_FILESIZE        "tEXt::Thumb::Size"
#define TAG_THUMB_MIMETYPE        "tEXt::Thumb::Mimetype"
#define TAG_THUMB_IMAGE_WIDTH     "tEXt::Thumb::Image::Width"
#define TAG_THUMB_IMAGE_HEIGHT    "tEXt::Thumb::Image::Height"
#define TAG_THUMB_PICMAN_TYPE       "tEXt::Thumb::X-PICMAN::Type"
#define TAG_THUMB_PICMAN_LAYERS     "tEXt::Thumb::X-PICMAN::Layers"


enum
{
  PROP_0,
  PROP_IMAGE_STATE,
  PROP_IMAGE_URI,
  PROP_IMAGE_MTIME,
  PROP_IMAGE_FILESIZE,
  PROP_IMAGE_MIMETYPE,
  PROP_IMAGE_WIDTH,
  PROP_IMAGE_HEIGHT,
  PROP_IMAGE_TYPE,
  PROP_IMAGE_NUM_LAYERS,
  PROP_THUMB_STATE
};


static void      picman_thumbnail_finalize     (GObject        *object);
static void      picman_thumbnail_set_property (GObject        *object,
                                              guint           property_id,
                                              const GValue   *value,
                                              GParamSpec     *pspec);
static void      picman_thumbnail_get_property (GObject        *object,
                                              guint           property_id,
                                              GValue         *value,
                                              GParamSpec     *pspec);
static void      picman_thumbnail_reset_info   (PicmanThumbnail  *thumbnail);

static void      picman_thumbnail_update_image (PicmanThumbnail  *thumbnail);
static void      picman_thumbnail_update_thumb (PicmanThumbnail  *thumbnail,
                                              PicmanThumbSize   size);

static gboolean  picman_thumbnail_save         (PicmanThumbnail  *thumbnail,
                                              PicmanThumbSize   size,
                                              const gchar    *filename,
                                              GdkPixbuf      *pixbuf,
                                              const gchar    *software,
                                              GError        **error);
#ifdef PICMAN_THUMB_DEBUG
static void      picman_thumbnail_debug_notify (GObject        *object,
                                              GParamSpec     *pspec);
#endif


G_DEFINE_TYPE (PicmanThumbnail, picman_thumbnail, G_TYPE_OBJECT)

#define parent_class picman_thumbnail_parent_class


static void
picman_thumbnail_class_init (PicmanThumbnailClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize     = picman_thumbnail_finalize;
  object_class->set_property = picman_thumbnail_set_property;
  object_class->get_property = picman_thumbnail_get_property;

  g_object_class_install_property (object_class,
                                   PROP_IMAGE_STATE,
                                   g_param_spec_enum ("image-state", NULL,
                                                      "State of the image associated to the thumbnail object",
                                                      PICMAN_TYPE_THUMB_STATE,
                                                      PICMAN_THUMB_STATE_UNKNOWN,
                                                      PICMAN_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_IMAGE_URI,
                                   g_param_spec_string ("image-uri", NULL,
                                                       "URI of the image file",
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_IMAGE_MTIME,
                                   g_param_spec_int64 ("image-mtime", NULL,
                                                       "Modification time of the image file in seconds since the Epoch",
                                                       G_MININT64, G_MAXINT64, 0,
                                                       PICMAN_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_IMAGE_FILESIZE,
                                   g_param_spec_int64 ("image-filesize", NULL,
                                                       "Size of the image file in bytes",
                                                       0, G_MAXINT64, 0,
                                                       PICMAN_PARAM_READWRITE));
  /**
   * PicmanThumbnail::image-mimetype:
   *
   * Image mimetype
   *
   * Since: PICMAN 2.2
   **/
  g_object_class_install_property (object_class,
                                   PROP_IMAGE_MIMETYPE,
                                   g_param_spec_string ("image-mimetype", NULL,
                                                        "Image mimetype",
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_IMAGE_WIDTH,
                                   g_param_spec_int ("image-width", NULL,
                                                     "Width of the image in pixels",
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_IMAGE_HEIGHT,
                                   g_param_spec_int ("image-height", NULL,
                                                     "Height of the image in pixels",
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_IMAGE_TYPE,
                                   g_param_spec_string ("image-type", NULL,
                                                        "String describing the type of the image format",
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_IMAGE_NUM_LAYERS,
                                   g_param_spec_int ("image-num-layers", NULL,
                                                     "The number of layers in the image",
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_THUMB_STATE,
                                   g_param_spec_enum ("thumb-state", NULL,
                                                      "State of the thumbnail file",
                                                      PICMAN_TYPE_THUMB_STATE,
                                                      PICMAN_THUMB_STATE_UNKNOWN,
                                                      PICMAN_PARAM_READWRITE));
}

static void
picman_thumbnail_init (PicmanThumbnail *thumbnail)
{
  thumbnail->image_state      = PICMAN_THUMB_STATE_UNKNOWN;
  thumbnail->image_uri        = NULL;
  thumbnail->image_filename   = NULL;
  thumbnail->image_mtime      = 0;
  thumbnail->image_filesize   = 0;
  thumbnail->image_mimetype   = NULL;
  thumbnail->image_width      = 0;
  thumbnail->image_height     = 0;
  thumbnail->image_type       = NULL;
  thumbnail->image_num_layers = 0;

  thumbnail->thumb_state      = PICMAN_THUMB_STATE_UNKNOWN;
  thumbnail->thumb_size       = -1;
  thumbnail->thumb_filename   = NULL;
  thumbnail->thumb_mtime      = 0;
  thumbnail->thumb_filesize   = 0;

#ifdef PICMAN_THUMB_DEBUG
  g_signal_connect (thumbnail, "notify",
                    G_CALLBACK (picman_thumbnail_debug_notify),
                    NULL);
#endif
}

static void
picman_thumbnail_finalize (GObject *object)
{
  PicmanThumbnail *thumbnail = PICMAN_THUMBNAIL (object);

  if (thumbnail->image_uri)
    {
      g_free (thumbnail->image_uri);
      thumbnail->image_uri = NULL;
    }
  if (thumbnail->image_filename)
    {
      g_free (thumbnail->image_filename);
      thumbnail->image_filename = NULL;
    }
  if (thumbnail->image_mimetype)
    {
      g_free (thumbnail->image_mimetype);
      thumbnail->image_mimetype = NULL;
    }
  if (thumbnail->image_type)
    {
      g_free (thumbnail->image_type);
      thumbnail->image_type = NULL;
    }
  if (thumbnail->thumb_filename)
    {
      g_free (thumbnail->thumb_filename);
      thumbnail->thumb_filename = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_thumbnail_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PicmanThumbnail *thumbnail = PICMAN_THUMBNAIL (object);

  switch (property_id)
    {
    case PROP_IMAGE_STATE:
      thumbnail->image_state = g_value_get_enum (value);
      break;
    case PROP_IMAGE_URI:
      picman_thumbnail_set_uri (PICMAN_THUMBNAIL (object),
                              g_value_get_string (value));
      break;
    case PROP_IMAGE_MTIME:
      thumbnail->image_mtime = g_value_get_int64 (value);
      break;
    case PROP_IMAGE_FILESIZE:
      thumbnail->image_filesize = g_value_get_int64 (value);
      break;
    case PROP_IMAGE_MIMETYPE:
      g_free (thumbnail->image_mimetype);
      thumbnail->image_mimetype = g_value_dup_string (value);
      break;
    case PROP_IMAGE_WIDTH:
      thumbnail->image_width = g_value_get_int (value);
      break;
    case PROP_IMAGE_HEIGHT:
      thumbnail->image_height = g_value_get_int (value);
      break;
    case PROP_IMAGE_TYPE:
      g_free (thumbnail->image_type);
      thumbnail->image_type = g_value_dup_string (value);
      break;
    case PROP_IMAGE_NUM_LAYERS:
      thumbnail->image_num_layers = g_value_get_int (value);
      break;
    case PROP_THUMB_STATE:
      thumbnail->thumb_state = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_thumbnail_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PicmanThumbnail *thumbnail = PICMAN_THUMBNAIL (object);

  switch (property_id)
    {
    case PROP_IMAGE_STATE:
      g_value_set_enum (value, thumbnail->image_state);
      break;
    case PROP_IMAGE_URI:
      g_value_set_string (value, thumbnail->image_uri);
      break;
    case PROP_IMAGE_MTIME:
      g_value_set_int64 (value, thumbnail->image_mtime);
      break;
    case PROP_IMAGE_FILESIZE:
      g_value_set_int64 (value, thumbnail->image_filesize);
      break;
    case PROP_IMAGE_MIMETYPE:
      g_value_set_string (value, thumbnail->image_mimetype);
      break;
    case PROP_IMAGE_WIDTH:
      g_value_set_int (value, thumbnail->image_width);
      break;
    case PROP_IMAGE_HEIGHT:
      g_value_set_int (value, thumbnail->image_height);
      break;
    case PROP_IMAGE_TYPE:
      g_value_set_string (value, thumbnail->image_type);
      break;
    case PROP_IMAGE_NUM_LAYERS:
      g_value_set_int (value, thumbnail->image_num_layers);
      break;
    case PROP_THUMB_STATE:
      g_value_set_enum (value, thumbnail->thumb_state);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

/**
 * picman_thumbnail_new:
 *
 * Creates a new #PicmanThumbnail object.
 *
 * Return value: a newly allocated PicmanThumbnail object
 **/
PicmanThumbnail *
picman_thumbnail_new (void)
{
  return g_object_new (PICMAN_TYPE_THUMBNAIL, NULL);
}

/**
 * picman_thumbnail_set_uri:
 * @thumbnail: a #PicmanThumbnail object
 * @uri: an escaped URI
 *
 * Sets the location of the image file associated with the #thumbnail.
 *
 * All informations stored in the #PicmanThumbnail are reset.
 **/
void
picman_thumbnail_set_uri (PicmanThumbnail *thumbnail,
                        const gchar   *uri)
{
  g_return_if_fail (PICMAN_IS_THUMBNAIL (thumbnail));

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  if (thumbnail->image_uri)
    g_free (thumbnail->image_uri);

  thumbnail->image_uri = g_strdup (uri);

  if (thumbnail->image_filename)
    {
      g_free (thumbnail->image_filename);
      thumbnail->image_filename = NULL;
    }

  if (thumbnail->thumb_filename)
    {
      g_free (thumbnail->thumb_filename);
      thumbnail->thumb_filename = NULL;
    }

  thumbnail->thumb_size     = -1;
  thumbnail->thumb_filesize = 0;
  thumbnail->thumb_mtime    = 0;

  g_object_set (thumbnail,
                "image-state",      PICMAN_THUMB_STATE_UNKNOWN,
                "image-filesize",   (gint64) 0,
                "image-mtime",      (gint64) 0,
                "image-mimetype",   NULL,
                "image-width",      0,
                "image-height",     0,
                "image-type",       NULL,
                "image-num-layers", 0,
                "thumb-state",      PICMAN_THUMB_STATE_UNKNOWN,
                NULL);
}

/**
 * picman_thumbnail_set_filename:
 * @thumbnail: a #PicmanThumbnail object
 * @filename: a local filename in the encoding of the filesystem
 * @error: return location for possible errors
 *
 * Sets the location of the image file associated with the #thumbnail.
 *
 * Return value: %TRUE if the filename was successfully set,
 *               %FALSE otherwise
 **/
gboolean
picman_thumbnail_set_filename (PicmanThumbnail  *thumbnail,
                             const gchar    *filename,
                             GError        **error)
{
  gchar *uri = NULL;

  g_return_val_if_fail (PICMAN_IS_THUMBNAIL (thumbnail), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  if (filename)
    uri = g_filename_to_uri (filename, NULL, error);

  picman_thumbnail_set_uri (thumbnail, uri);

  g_free (uri);

  return (!filename || uri);
}

/**
 * picman_thumbnail_set_from_thumb:
 * @thumbnail: a #PicmanThumbnail object
 * @filename: filename of a local thumbnail file
 * @error: return location for possible errors
 *
 * This function tries to load the thumbnail file pointed to by
 * @filename and retrieves the URI of the original image file from
 * it. This allows you to find the image file associated with a
 * thumbnail file.
 *
 * This will only work with thumbnails from the global thumbnail
 * directory that contain a valid Thumb::URI tag.
 *
 * Return value: %TRUE if the pixbuf could be loaded, %FALSE otherwise
 **/
gboolean
picman_thumbnail_set_from_thumb (PicmanThumbnail  *thumbnail,
                               const gchar    *filename,
                               GError        **error)
{
  GdkPixbuf   *pixbuf;
  const gchar *uri;

  g_return_val_if_fail (PICMAN_IS_THUMBNAIL (thumbnail), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  pixbuf = gdk_pixbuf_new_from_file (filename, error);
  if (! pixbuf)
    return FALSE;

  uri = gdk_pixbuf_get_option (pixbuf, TAG_THUMB_URI);
  if (! uri)
    {
      g_set_error (error, PICMAN_THUMB_ERROR, 0,
                   _("Thumbnail contains no Thumb::URI tag"));
      g_object_unref (pixbuf);
      return FALSE;
    }

  picman_thumbnail_set_uri (thumbnail, uri);
  g_object_unref (pixbuf);

  return TRUE;
}

/**
 * picman_thumbnail_peek_image:
 * @thumbnail: a #PicmanThumbnail object
 *
 * Checks the image file associated with the @thumbnail and updates
 * information such as state, filesize and modification time.
 *
 * Return value: the image's #PicmanThumbState after the update
 **/
PicmanThumbState
picman_thumbnail_peek_image (PicmanThumbnail *thumbnail)
{
  g_return_val_if_fail (PICMAN_IS_THUMBNAIL (thumbnail),
                        PICMAN_THUMB_STATE_UNKNOWN);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  g_object_freeze_notify (G_OBJECT (thumbnail));

  picman_thumbnail_update_image (thumbnail);

  g_object_thaw_notify (G_OBJECT (thumbnail));

  return thumbnail->image_state;
}

/**
 * picman_thumbnail_peek_thumb:
 * @thumbnail: a #PicmanThumbnail object
 * @size: the preferred size of the thumbnail image
 *
 * Checks if a thumbnail file for the @thumbnail exists. It doesn't
 * load the thumbnail image and thus cannot check if the thumbnail is
 * valid and uptodate for the image file asosciated with the
 * @thumbnail.
 *
 * If you want to check the thumbnail, either attempt to load it using
 * picman_thumbnail_load_thumb(), or, if you don't need the resulting
 * thumbnail pixbuf, use picman_thumbnail_check_thumb().
 *
 * Return value: the thumbnail's #PicmanThumbState after the update
 **/
PicmanThumbState
picman_thumbnail_peek_thumb (PicmanThumbnail *thumbnail,
                           PicmanThumbSize  size)
{
  g_return_val_if_fail (PICMAN_IS_THUMBNAIL (thumbnail),
                        PICMAN_THUMB_STATE_UNKNOWN);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  g_object_freeze_notify (G_OBJECT (thumbnail));

  picman_thumbnail_update_image (thumbnail);
  picman_thumbnail_update_thumb (thumbnail, size);

  g_object_thaw_notify (G_OBJECT (thumbnail));

  return thumbnail->thumb_state;
}

/**
 * picman_thumbnail_check_thumb:
 * @thumbnail: a #PicmanThumbnail object
 * @size: the preferred size of the thumbnail image
 *
 * Checks if a thumbnail file for the @thumbnail exists, loads it and
 * verifies it is valid and uptodate for the image file asosciated
 * with the @thumbnail.
 *
 * Return value: the thumbnail's #PicmanThumbState after the update
 *
 * Since: PICMAN 2.2
 **/
PicmanThumbState
picman_thumbnail_check_thumb (PicmanThumbnail *thumbnail,
                            PicmanThumbSize  size)
{
  GdkPixbuf *pixbuf;

  g_return_val_if_fail (PICMAN_IS_THUMBNAIL (thumbnail), FALSE);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  if (picman_thumbnail_peek_thumb (thumbnail, size) == PICMAN_THUMB_STATE_OK)
    return PICMAN_THUMB_STATE_OK;

  pixbuf = picman_thumbnail_load_thumb (thumbnail, size, NULL);

  if (pixbuf)
    g_object_unref (pixbuf);

  return thumbnail->thumb_state;
}

static void
picman_thumbnail_update_image (PicmanThumbnail *thumbnail)
{
  PicmanThumbState  state;
  gint64          mtime    = 0;
  gint64          filesize = 0;

  if (! thumbnail->image_uri)
    return;

  state = thumbnail->image_state;

  switch (state)
    {
    case PICMAN_THUMB_STATE_UNKNOWN:
      g_return_if_fail (thumbnail->image_filename == NULL);

      thumbnail->image_filename =
        _picman_thumb_filename_from_uri (thumbnail->image_uri);

      if (! thumbnail->image_filename)
        state = PICMAN_THUMB_STATE_REMOTE;

      break;

    case PICMAN_THUMB_STATE_REMOTE:
      break;

    default:
      g_return_if_fail (thumbnail->image_filename != NULL);
      break;
    }

  switch (state)
    {
    case PICMAN_THUMB_STATE_REMOTE:
      break;

    default:
      switch (picman_thumb_file_test (thumbnail->image_filename,
                                    &mtime, &filesize,
                                    &thumbnail->image_not_found_errno))
        {
        case PICMAN_THUMB_FILE_TYPE_REGULAR:
          state = PICMAN_THUMB_STATE_EXISTS;
          break;

        case PICMAN_THUMB_FILE_TYPE_FOLDER:
          state = PICMAN_THUMB_STATE_FOLDER;
          break;

        case PICMAN_THUMB_FILE_TYPE_SPECIAL:
          state = PICMAN_THUMB_STATE_SPECIAL;
          break;

        default:
          state = PICMAN_THUMB_STATE_NOT_FOUND;
          break;
        }
      break;
    }

  if (state != thumbnail->image_state)
    {
      g_object_set (thumbnail,
                    "image-state", state,
                    NULL);
    }

  if (mtime != thumbnail->image_mtime || filesize != thumbnail->image_filesize)
    {
      g_object_set (thumbnail,
                    "image-mtime",    mtime,
                    "image-filesize", filesize,
                    NULL);

      if (thumbnail->thumb_state == PICMAN_THUMB_STATE_OK)
        g_object_set (thumbnail,
                      "thumb-state", PICMAN_THUMB_STATE_OLD,
                      NULL);
    }
}

static void
picman_thumbnail_update_thumb (PicmanThumbnail *thumbnail,
                             PicmanThumbSize  size)
{
  gchar          *filename;
  PicmanThumbState  state;
  gint64          filesize = 0;
  gint64          mtime    = 0;

  if (! thumbnail->image_uri)
    return;

  state = thumbnail->thumb_state;

  filename = picman_thumb_find_thumb (thumbnail->image_uri, &size);

  if (! filename)
    state = PICMAN_THUMB_STATE_NOT_FOUND;

  switch (state)
    {
    case PICMAN_THUMB_STATE_EXISTS:
    case PICMAN_THUMB_STATE_OLD:
    case PICMAN_THUMB_STATE_FAILED:
    case PICMAN_THUMB_STATE_OK:
      g_return_if_fail (thumbnail->thumb_filename != NULL);

      if (thumbnail->thumb_size     == size     &&
          thumbnail->thumb_filesize == filesize &&
          thumbnail->thumb_mtime    == mtime)
        {
          g_free (filename);
          return;
        }
      break;
    default:
      break;
    }

  if (thumbnail->thumb_filename)
    g_free (thumbnail->thumb_filename);

  thumbnail->thumb_filename = filename;

  if (filename)
    state = (size > PICMAN_THUMB_SIZE_FAIL ?
             PICMAN_THUMB_STATE_EXISTS : PICMAN_THUMB_STATE_FAILED);

  thumbnail->thumb_size     = size;
  thumbnail->thumb_filesize = filesize;
  thumbnail->thumb_mtime    = mtime;

  if (state != thumbnail->thumb_state)
    {
      g_object_freeze_notify (G_OBJECT (thumbnail));

      g_object_set (thumbnail, "thumb-state", state, NULL);
      picman_thumbnail_reset_info (thumbnail);

      g_object_thaw_notify (G_OBJECT (thumbnail));
    }
}

static void
picman_thumbnail_reset_info (PicmanThumbnail *thumbnail)
{
  g_object_set (thumbnail,
                "image-width",      0,
                "image-height",     0,
                "image-type",       NULL,
                "image-num-layers", 0,
                NULL);
}

static void
picman_thumbnail_set_info_from_pixbuf (PicmanThumbnail *thumbnail,
                                     GdkPixbuf     *pixbuf)
{
  const gchar  *option;
  gint          num;

  g_object_freeze_notify (G_OBJECT (thumbnail));

  picman_thumbnail_reset_info (thumbnail);

  g_free (thumbnail->image_mimetype);
  thumbnail->image_mimetype =
    g_strdup (gdk_pixbuf_get_option (pixbuf, TAG_THUMB_MIMETYPE));

  option = gdk_pixbuf_get_option (pixbuf, TAG_THUMB_IMAGE_WIDTH);
  if (option && sscanf (option, "%d", &num) == 1)
    thumbnail->image_width = num;

  option = gdk_pixbuf_get_option (pixbuf, TAG_THUMB_IMAGE_HEIGHT);
  if (option && sscanf (option, "%d", &num) == 1)
    thumbnail->image_height = num;

  thumbnail->image_type =
    g_strdup (gdk_pixbuf_get_option (pixbuf, TAG_THUMB_PICMAN_TYPE));

  option = gdk_pixbuf_get_option (pixbuf, TAG_THUMB_PICMAN_LAYERS);
  if (option && sscanf (option, "%d", &num) == 1)
    thumbnail->image_num_layers = num;

  g_object_thaw_notify (G_OBJECT (thumbnail));
}

static gboolean
picman_thumbnail_save (PicmanThumbnail  *thumbnail,
                     PicmanThumbSize   size,
                     const gchar    *filename,
                     GdkPixbuf      *pixbuf,
                     const gchar    *software,
                     GError        **error)
{
  const gchar  *keys[12];
  gchar        *values[12];
  gchar        *basename;
  gchar        *dirname;
  gchar        *tmpname;
  gboolean      success;
  gint          i = 0;

  keys[i]   = TAG_DESCRIPTION;
  values[i] = g_strdup_printf ("Thumbnail of %s",  thumbnail->image_uri);
  i++;

  keys[i]   = TAG_SOFTWARE;
  values[i] = g_strdup (software);
  i++;

  keys[i]   = TAG_THUMB_URI;
  values[i] = g_strdup (thumbnail->image_uri);
  i++;

  keys[i]   = TAG_THUMB_MTIME;
  values[i] = g_strdup_printf ("%" G_GINT64_FORMAT, thumbnail->image_mtime);
  i++;

  keys[i]   = TAG_THUMB_FILESIZE;
  values[i] = g_strdup_printf ("%" G_GINT64_FORMAT, thumbnail->image_filesize);
  i++;

  if (thumbnail->image_mimetype)
    {
      keys[i]   = TAG_THUMB_MIMETYPE;
      values[i] = g_strdup (thumbnail->image_mimetype);
      i++;
    }

  if (thumbnail->image_width > 0)
    {
      keys[i]   = TAG_THUMB_IMAGE_WIDTH;
      values[i] = g_strdup_printf ("%d", thumbnail->image_width);
      i++;
    }

  if (thumbnail->image_height > 0)
    {
      keys[i]   = TAG_THUMB_IMAGE_HEIGHT;
      values[i] = g_strdup_printf ("%d", thumbnail->image_height);
      i++;
    }

  if (thumbnail->image_type)
    {
      keys[i]   = TAG_THUMB_PICMAN_TYPE;
      values[i] = g_strdup (thumbnail->image_type);
      i++;
    }

  if (thumbnail->image_num_layers > 0)
    {
      keys[i]   = TAG_THUMB_PICMAN_LAYERS;
      values[i] = g_strdup_printf ("%d", thumbnail->image_num_layers);
      i++;
    }

  keys[i]   = NULL;
  values[i] = NULL;

  basename = g_path_get_basename (filename);
  dirname  = g_path_get_dirname (filename);

  tmpname = g_strdup_printf ("%s%cpicman-thumb-%d-%.8s",
                             dirname, G_DIR_SEPARATOR, getpid (), basename);

  g_free (dirname);
  g_free (basename);

  success = gdk_pixbuf_savev (pixbuf, tmpname, "png",
                              (gchar **) keys, values,
                              error);

  for (i = 0; keys[i]; i++)
    g_free (values[i]);

  if (success)
    {
#ifdef PICMAN_THUMB_DEBUG
      g_printerr ("thumbnail saved to temporary file %s\n", tmpname);
#endif

      success = (g_rename (tmpname, filename) == 0);

      if (! success)
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                     _("Could not create thumbnail for %s: %s"),
                     thumbnail->image_uri, g_strerror (errno));
    }

  if (success)
    {
#ifdef PICMAN_THUMB_DEBUG
      g_printerr ("temporary thumbnail file renamed to %s\n", filename);
#endif

      success = (g_chmod (filename, 0600) == 0);

      if (! success)
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                     "Could not set permissions of thumbnail for %s: %s",
                     thumbnail->image_uri, g_strerror (errno));

      g_object_freeze_notify (G_OBJECT (thumbnail));

      picman_thumbnail_update_thumb (thumbnail, size);

      if (success &&
          thumbnail->thumb_state == PICMAN_THUMB_STATE_EXISTS &&
          strcmp (filename, thumbnail->thumb_filename) == 0)
        {
          thumbnail->thumb_state = PICMAN_THUMB_STATE_OK;
        }

      g_object_thaw_notify (G_OBJECT (thumbnail));
    }

  g_unlink (tmpname);
  g_free (tmpname);

  return success;
}

#ifdef PICMAN_THUMB_DEBUG
static void
picman_thumbnail_debug_notify (GObject    *object,
                             GParamSpec *pspec)
{
  GValue       value = { 0, };
  gchar       *str   = NULL;
  const gchar *name;

  g_value_init (&value, pspec->value_type);
  g_object_get_property (object, pspec->name, &value);

  if (G_VALUE_HOLDS_STRING (&value))
    {
      str = g_value_dup_string (&value);
    }
  else if (g_value_type_transformable (pspec->value_type, G_TYPE_STRING))
    {
      GValue  tmp = { 0, };

      g_value_init (&tmp, G_TYPE_STRING);
      g_value_transform (&value, &tmp);

      str = g_value_dup_string (&tmp);

      g_value_unset (&tmp);
    }

  g_value_unset (&value);

  name = PICMAN_THUMBNAIL (object)->image_uri;

  g_printerr (" PicmanThumb (%s) %s: %s\n",
              name ? name : "(null)", pspec->name, str);

  g_free (str);
}
#endif


/**
 * picman_thumbnail_load_thumb:
 * @thumbnail: a #PicmanThumbnail object
 * @size: the preferred #PicmanThumbSize for the preview
 * @error: return location for possible errors
 *
 * Attempts to load a thumbnail preview for the image associated with
 * @thumbnail. Before you use this function you need need to set an
 * image location using picman_thumbnail_set_uri() or
 * picman_thumbnail_set_filename(). You can also peek at the thumb
 * before loading it using picman_thumbnail_peek_thumb.
 *
 * This function will return the best matching pixbuf for the
 * specified @size. It returns the pixbuf as loaded from disk. It is
 * left to the caller to scale it to the desired size. The returned
 * pixbuf may also represent an outdated preview of the image file.
 * In order to verify if the preview is uptodate, you should check the
 * "thumb_state" property after calling this function.
 *
 * Return value: a preview pixbuf or %NULL if no thumbnail was found
 **/
GdkPixbuf *
picman_thumbnail_load_thumb (PicmanThumbnail  *thumbnail,
                           PicmanThumbSize   size,
                           GError        **error)
{
  PicmanThumbState  state;
  GdkPixbuf      *pixbuf;
  const gchar    *option;
  gint64          image_mtime;
  gint64          image_size;

  g_return_val_if_fail (PICMAN_IS_THUMBNAIL (thumbnail), NULL);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  if (! thumbnail->image_uri)
    return NULL;

  state = picman_thumbnail_peek_thumb (thumbnail, size);

  if (state < PICMAN_THUMB_STATE_EXISTS || state == PICMAN_THUMB_STATE_FAILED)
    return NULL;

  pixbuf = gdk_pixbuf_new_from_file (thumbnail->thumb_filename, NULL);
  if (! pixbuf)
    return NULL;

#ifdef PICMAN_THUMB_DEBUG
  g_printerr ("thumbnail loaded from %s\n", thumbnail->thumb_filename);
#endif

  g_object_freeze_notify (G_OBJECT (thumbnail));

  /* URI and mtime from the thumbnail need to match our file */
  option = gdk_pixbuf_get_option (pixbuf, TAG_THUMB_URI);
  if (!option)
    goto finish;

  if (strcmp (option, thumbnail->image_uri))
    {
      /*  might be a local thumbnail, try if the local part matches  */
      const gchar *baseuri = strrchr (thumbnail->image_uri, '/');

      if (!baseuri || strcmp (option, baseuri))
        goto finish;
    }

  state = PICMAN_THUMB_STATE_OLD;

  option = gdk_pixbuf_get_option (pixbuf, TAG_THUMB_MTIME);
  if (!option || sscanf (option, "%" G_GINT64_FORMAT, &image_mtime) != 1)
    goto finish;

  option = gdk_pixbuf_get_option (pixbuf, TAG_THUMB_FILESIZE);
  if (option && sscanf (option, "%" G_GINT64_FORMAT, &image_size) != 1)
    goto finish;

  /* TAG_THUMB_FILESIZE is optional but must match if present */
  if (image_mtime == thumbnail->image_mtime &&
      (option == NULL || image_size == thumbnail->image_filesize))
    {
      if (thumbnail->thumb_size == PICMAN_THUMB_SIZE_FAIL)
        state = PICMAN_THUMB_STATE_FAILED;
      else
        state = PICMAN_THUMB_STATE_OK;
    }

  if (state == PICMAN_THUMB_STATE_FAILED)
    picman_thumbnail_reset_info (thumbnail);
  else
    picman_thumbnail_set_info_from_pixbuf (thumbnail, pixbuf);

 finish:
  if (thumbnail->thumb_size == PICMAN_THUMB_SIZE_FAIL ||
      (state != PICMAN_THUMB_STATE_OLD && state != PICMAN_THUMB_STATE_OK))
    {
      g_object_unref (pixbuf);
      pixbuf = NULL;
    }

  g_object_set (thumbnail,
                "thumb-state", state,
                NULL);

  g_object_thaw_notify (G_OBJECT (thumbnail));

  return pixbuf;
}

/**
 * picman_thumbnail_save_thumb:
 * @thumbnail: a #PicmanThumbnail object
 * @pixbuf: a #GdkPixbuf representing the preview thumbnail
 * @software: a string describing the software saving the thumbnail
 * @error: return location for possible errors
 *
 * Saves a preview thumbnail for the image associated with @thumbnail.
 * to the global thumbnail repository.
 *
 * The caller is responsible for setting the image file location, it's
 * filesize, modification time. One way to set this info is to is to
 * call picman_thumbnail_set_uri() followed by picman_thumbnail_peek_image().
 * Since this won't work for remote images, it is left to the user of
 * picman_thumbnail_save_thumb() to do this or to set the information
 * using the @thumbnail object properties.
 *
 * The image format type and the number of layers can optionally be
 * set in order to be stored with the preview image.
 *
 * Return value: %TRUE if a thumbnail was successfully written,
 *               %FALSE otherwise
 **/
gboolean
picman_thumbnail_save_thumb (PicmanThumbnail  *thumbnail,
                           GdkPixbuf      *pixbuf,
                           const gchar    *software,
                           GError        **error)
{
  PicmanThumbSize  size;
  gchar         *name;
  gboolean       success;

  g_return_val_if_fail (PICMAN_IS_THUMBNAIL (thumbnail), FALSE);
  g_return_val_if_fail (thumbnail->image_uri != NULL, FALSE);
  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), FALSE);
  g_return_val_if_fail (software != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  size = MAX (gdk_pixbuf_get_width (pixbuf), gdk_pixbuf_get_height (pixbuf));
  if (size < 1)
    return TRUE;

  name = picman_thumb_name_from_uri (thumbnail->image_uri, size);
  if (! name)
    return TRUE;

  if (! picman_thumb_ensure_thumb_dir (size, error))
    {
      g_free (name);
      return FALSE;
    }

  success = picman_thumbnail_save (thumbnail,
                                 size, name, pixbuf, software,
                                 error);
  g_free (name);

  return success;
}

/**
 * picman_thumbnail_save_thumb_local:
 * @thumbnail: a #PicmanThumbnail object
 * @pixbuf: a #GdkPixbuf representing the preview thumbnail
 * @software: a string describing the software saving the thumbnail
 * @error: return location for possible errors
 *
 * Saves a preview thumbnail for the image associated with @thumbnail
 * to the local thumbnail repository. Local thumbnails have been added
 * with version 0.7 of the spec.
 *
 * Please see also picman_thumbnail_save_thumb(). The notes made there
 * apply here as well.
 *
 * Return value: %TRUE if a thumbnail was successfully written,
 *               %FALSE otherwise
 *
 * Since: PICMAN 2.2
 **/
gboolean
picman_thumbnail_save_thumb_local (PicmanThumbnail  *thumbnail,
                                 GdkPixbuf      *pixbuf,
                                 const gchar    *software,
                                 GError        **error)
{
  PicmanThumbSize  size;
  gchar         *name;
  gchar         *filename;
  gchar         *dirname;
  gboolean       success;

  g_return_val_if_fail (PICMAN_IS_THUMBNAIL (thumbnail), FALSE);
  g_return_val_if_fail (thumbnail->image_uri != NULL, FALSE);
  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), FALSE);
  g_return_val_if_fail (software != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  size = MAX (gdk_pixbuf_get_width (pixbuf), gdk_pixbuf_get_height (pixbuf));
  if (size < 1)
    return TRUE;

  filename = _picman_thumb_filename_from_uri (thumbnail->image_uri);
  if (! filename)
    return TRUE;

  dirname = g_path_get_dirname (filename);
  g_free (filename);

  name = picman_thumb_name_from_uri_local (thumbnail->image_uri, size);
  if (! name)
    {
      g_free (dirname);
      return TRUE;
    }

  if (! picman_thumb_ensure_thumb_dir_local (dirname, size, error))
    {
      g_free (name);
      g_free (dirname);
      return FALSE;
    }

  g_free (dirname);

  success = picman_thumbnail_save (thumbnail,
                                 size, name, pixbuf, software,
                                 error);
  g_free (name);

  return success;
}

/**
 * picman_thumbnail_save_failure:
 * @thumbnail: a #PicmanThumbnail object
 * @software: a string describing the software saving the thumbnail
 * @error: return location for possible errors
 *
 * Saves a failure thumbnail for the image associated with
 * @thumbnail. This is an empty pixbuf that indicates that an attempt
 * to create a preview for the image file failed. It should be used to
 * prevent the software from further attempts to create this thumbnail.
 *
 * Return value: %TRUE if a failure thumbnail was successfully written,
 *               %FALSE otherwise
 **/
gboolean
picman_thumbnail_save_failure (PicmanThumbnail  *thumbnail,
                             const gchar    *software,
                             GError        **error)
{
  GdkPixbuf *pixbuf;
  gchar     *name;
  gchar     *desc;
  gchar     *time_str;
  gchar     *size_str;
  gboolean   success;

  g_return_val_if_fail (PICMAN_IS_THUMBNAIL (thumbnail), FALSE);
  g_return_val_if_fail (thumbnail->image_uri != NULL, FALSE);
  g_return_val_if_fail (software != NULL, FALSE);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  name = picman_thumb_name_from_uri (thumbnail->image_uri, PICMAN_THUMB_SIZE_FAIL);
  if (! name)
    return TRUE;

  if (! picman_thumb_ensure_thumb_dir (PICMAN_THUMB_SIZE_FAIL, error))
    {
      g_free (name);
      return FALSE;
    }

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 1, 1);

  desc = g_strdup_printf ("Thumbnail failure for %s", thumbnail->image_uri);
  time_str = g_strdup_printf ("%" G_GINT64_FORMAT, thumbnail->image_mtime);
  size_str = g_strdup_printf ("%" G_GINT64_FORMAT, thumbnail->image_filesize);

  success = gdk_pixbuf_save (pixbuf, name, "png", error,
                             TAG_DESCRIPTION,    desc,
                             TAG_SOFTWARE,       software,
                             TAG_THUMB_URI,      thumbnail->image_uri,
                             TAG_THUMB_MTIME,    time_str,
                             TAG_THUMB_FILESIZE, size_str,
                             NULL);
  if (success)
    {
      success = (g_chmod (name, 0600) == 0);

      if (success)
        picman_thumbnail_update_thumb (thumbnail, PICMAN_THUMB_SIZE_NORMAL);
      else
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                     "Could not set permissions of thumbnail '%s': %s",
                     name, g_strerror (errno));
    }

  g_object_unref (pixbuf);

  g_free (size_str);
  g_free (time_str);
  g_free (desc);
  g_free (name);

  return success;
}

/**
 * picman_thumbnail_delete_failure:
 * @thumbnail: a #PicmanThumbnail object
 *
 * Removes a failure thumbnail if one exists. This function should be
 * used after a thumbnail has been successfully created.
 *
 * Since: PICMAN 2.2
 **/
void
picman_thumbnail_delete_failure (PicmanThumbnail *thumbnail)
{
  gchar *filename;

  g_return_if_fail (PICMAN_IS_THUMBNAIL (thumbnail));
  g_return_if_fail (thumbnail->image_uri != NULL);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  filename = picman_thumb_name_from_uri (thumbnail->image_uri,
                                       PICMAN_THUMB_SIZE_FAIL);
  if (filename)
    {
      g_unlink (filename);
      g_free (filename);
    }
}

/**
 * picman_thumbnail_delete_others:
 * @thumbnail: a #PicmanThumbnail object
 * @size: the thumbnail size which should not be deleted
 *
 * Removes all other thumbnails from the global thumbnail
 * repository. Only the thumbnail for @size is not deleted.  This
 * function should be used after a thumbnail has been successfully
 * updated. See the spec for a more detailed description on when to
 * delete thumbnails.
 *
 * Since: PICMAN 2.2
 **/
void
picman_thumbnail_delete_others (PicmanThumbnail *thumbnail,
                              PicmanThumbSize  size)
{
  g_return_if_fail (PICMAN_IS_THUMBNAIL (thumbnail));
  g_return_if_fail (thumbnail->image_uri != NULL);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  _picman_thumbs_delete_others (thumbnail->image_uri, size);
}

/**
 * picman_thumbnail_has_failed:
 * @thumbnail: a #PicmanThumbnail object
 *
 * Checks if a valid failure thumbnail for the given thumbnail exists
 * in the global thumbnail repository. This may be the case even if
 * picman_thumbnail_peek_thumb() doesn't return %PICMAN_THUMB_STATE_FAILED
 * since there might be a real thumbnail and a failure thumbnail for
 * the same image file.
 *
 * The application should not attempt to create the thumbnail if a
 * valid failure thumbnail exists.
 *
 * Return value: %TRUE if a failure thumbnail exists or
 *
 * Since: PICMAN 2.2
 **/
gboolean
picman_thumbnail_has_failed (PicmanThumbnail *thumbnail)
{
  GdkPixbuf   *pixbuf;
  const gchar *option;
  gchar       *filename;
  gint64       image_mtime;
  gint64       image_size;
  gboolean     failed = FALSE;

  g_return_val_if_fail (PICMAN_IS_THUMBNAIL (thumbnail), FALSE);
  g_return_val_if_fail (thumbnail->image_uri != NULL, FALSE);

  PICMAN_THUMB_DEBUG_CALL (thumbnail);

  filename = picman_thumb_name_from_uri (thumbnail->image_uri,
                                       PICMAN_THUMB_SIZE_FAIL);
  if (! filename)
    return FALSE;

  pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
  g_free (filename);

  if (! pixbuf)
    return FALSE;

  if (picman_thumbnail_peek_image (thumbnail) < PICMAN_THUMB_STATE_EXISTS)
    goto finish;

  /* URI and mtime from the thumbnail need to match our file */
  option = gdk_pixbuf_get_option (pixbuf, TAG_THUMB_URI);
  if (! option || strcmp (option, thumbnail->image_uri))
    goto finish;

  option = gdk_pixbuf_get_option (pixbuf, TAG_THUMB_MTIME);
  if (!option || sscanf (option, "%" G_GINT64_FORMAT, &image_mtime) != 1)
    goto finish;

  option = gdk_pixbuf_get_option (pixbuf, TAG_THUMB_FILESIZE);
  if (option && sscanf (option, "%" G_GINT64_FORMAT, &image_size) != 1)
    goto finish;

  /* TAG_THUMB_FILESIZE is optional but must match if present */
  if (image_mtime == thumbnail->image_mtime &&
      (option == NULL || image_size == thumbnail->image_filesize))
    {
      failed = TRUE;
    }

 finish:
  g_object_unref (pixbuf);

  return failed;
}
