/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * Thumbnail handling according to the Thumbnail Managing Standard.
 * http://triq.net/~pearl/thumbnail-spec/
 *
 * Copyright (C) 2001-2003  Sven Neumann <sven@picman.org>
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

#include <errno.h>
#include <string.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef PLATFORM_OSX
#include <AppKit/AppKit.h>
#endif

#include <glib-object.h>
#include <glib/gstdio.h>

#ifdef G_OS_WIN32
#include "libpicmanbase/picmanwin32-io.h"
#endif

#include "picmanthumb-error.h"
#include "picmanthumb-types.h"
#include "picmanthumb-utils.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmanthumb-utils
 * @title: PicmanThumb-utils
 * @short_description: Utility functions provided and used by libpicmanthumb
 *
 * Utility functions provided and used by libpicmanthumb
 **/


static gint           picman_thumb_size       (PicmanThumbSize  size);
static gchar        * picman_thumb_png_lookup (const gchar   *name,
                                             const gchar   *basedir,
                                             PicmanThumbSize *size) G_GNUC_MALLOC;
static const gchar  * picman_thumb_png_name   (const gchar   *uri);
static void           picman_thumb_exit       (void);



static gboolean      picman_thumb_initialized = FALSE;
static gint          thumb_num_sizes        = 0;
static gint         *thumb_sizes            = NULL;
static const gchar **thumb_sizenames        = NULL;
static gchar        *thumb_dir              = NULL;
static gchar       **thumb_subdirs          = NULL;
static gchar        *thumb_fail_subdir      = NULL;


/**
 * picman_thumb_init:
 * @creator: an ASCII string that identifies the thumbnail creator
 * @thumb_basedir: an absolute path or %NULL to use the default
 *
 * This function initializes the thumbnail system. It must be called
 * before any other functions from libpicmanthumb are used. You may call
 * it more than once if you want to change the @thumb_basedir but if
 * you do that, you should make sure that no thread is still using the
 * library. Apart from this function, libpicmanthumb is multi-thread
 * safe.
 *
 * The @creator string must be 7bit ASCII and should contain the name
 * of the software that creates the thumbnails. It is used to handle
 * thumbnail creation failures. See the spec for more details.
 *
 * Usually you will pass %NULL for @thumb_basedir. Thumbnails will
 * then be stored in the user's personal thumbnail directory as
 * defined in the spec. If you wish to use libpicmanthumb to store
 * application-specific thumbnails, you can specify a different base
 * directory here.
 *
 * Return value: %TRUE if the library was successfully initialized.
 **/
gboolean
picman_thumb_init (const gchar *creator,
                 const gchar *thumb_basedir)
{
  GEnumClass *enum_class;
  GEnumValue *enum_value;
  gint        i;

  g_return_val_if_fail (creator != NULL, FALSE);
  g_return_val_if_fail (thumb_basedir == NULL ||
                        g_path_is_absolute (thumb_basedir), FALSE);

  if (picman_thumb_initialized)
    picman_thumb_exit ();

  if (thumb_basedir)
    {
      thumb_dir = g_strdup (thumb_basedir);
    }
  else
    {
#ifdef PLATFORM_OSX

      NSAutoreleasePool *pool;
      NSArray           *path;
      NSString          *cache_dir;

      pool = [[NSAutoreleasePool alloc] init];

      path = NSSearchPathForDirectoriesInDomains (NSCachesDirectory,
                                                  NSUserDomainMask, YES);
      cache_dir = [path objectAtIndex:0];

      thumb_dir = g_build_filename ([cache_dir UTF8String], "org.freedesktop.thumbnails",
                                    NULL);

      [pool drain];

#else

      const gchar *home_dir = g_get_home_dir ();

      if (home_dir && g_file_test (home_dir, G_FILE_TEST_IS_DIR))
        {
          thumb_dir = g_build_filename (home_dir, ".thumbnails", NULL);
        }

#endif

      if (! thumb_dir)
        {
          gchar *name = g_filename_display_name (g_get_tmp_dir ());

          g_message (_("Cannot determine a valid home directory.\n"
                       "Thumbnails will be stored in the folder for "
                       "temporary files (%s) instead."), name);
          g_free (name);

          thumb_dir = g_build_filename (g_get_tmp_dir (), ".thumbnails", NULL);
        }
    }

  enum_class = g_type_class_ref (PICMAN_TYPE_THUMB_SIZE);

  thumb_num_sizes = enum_class->n_values;
  thumb_sizes     = g_new (gint, thumb_num_sizes);
  thumb_sizenames = g_new (const gchar *, thumb_num_sizes);
  thumb_subdirs   = g_new (gchar *, thumb_num_sizes);

  for (i = 0, enum_value = enum_class->values;
       i < enum_class->n_values;
       i++, enum_value++)
    {
      thumb_sizes[i]     = enum_value->value;
      thumb_sizenames[i] = enum_value->value_nick;
      thumb_subdirs[i]   = g_build_filename (thumb_dir,
                                             enum_value->value_nick, NULL);
    }

  thumb_fail_subdir = thumb_subdirs[0];
  thumb_subdirs[0]  = g_build_filename (thumb_fail_subdir, creator, NULL);

  g_type_class_unref (enum_class);

  picman_thumb_initialized = TRUE;

  return picman_thumb_initialized;
}

/**
 * picman_thumb_get_thumb_dir:
 * @size: a PicmanThumbSize
 *
 * Retrieve the name of the thumbnail folder for a specific size. The
 * returned pointer will become invalid if picman_thumb_init() is used
 * again. It must not be changed or freed.
 *
 * Return value: the thumbnail directory in the encoding of the filesystem
 **/
const gchar *
picman_thumb_get_thumb_dir (PicmanThumbSize  size)
{
  g_return_val_if_fail (picman_thumb_initialized, NULL);

  size = picman_thumb_size (size);

  return thumb_subdirs[size];
}

/**
 * picman_thumb_get_thumb_dir_local:
 * @dirname: the basename of the dir, without the actual dirname itself
 * @size:    a PicmanThumbSize
 *
 * Retrieve the name of the local thumbnail folder for a specific
 * size.  Unlike picman_thumb_get_thumb_dir() the returned string is not
 * constant and should be free'd when it is not any longer needed.
 *
 * Return value: the thumbnail directory in the encoding of the filesystem
 *
 * Since: PICMAN 2.2
 **/
gchar *
picman_thumb_get_thumb_dir_local (const gchar   *dirname,
                                PicmanThumbSize  size)
{
  g_return_val_if_fail (picman_thumb_initialized, NULL);
  g_return_val_if_fail (dirname != NULL, NULL);
  g_return_val_if_fail (size > PICMAN_THUMB_SIZE_FAIL, NULL);

  size = picman_thumb_size (size);

  return g_build_filename (dirname, thumb_sizenames[size], NULL);
}

/**
 * picman_thumb_ensure_thumb_dir:
 * @size: a PicmanThumbSize
 * @error: return location for possible errors
 *
 * This function checks if the directory that is required to store
 * thumbnails for a particular @size exist and attempts to create it
 * if necessary.
 *
 * You shouldn't have to call this function directly since
 * picman_thumbnail_save_thumb() and picman_thumbnail_save_failure() will
 * do this for you.
 *
 * Return value: %TRUE is the directory exists, %FALSE if it could not
 *               be created
 **/
gboolean
picman_thumb_ensure_thumb_dir (PicmanThumbSize   size,
                             GError        **error)
{
  g_return_val_if_fail (picman_thumb_initialized, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  size = picman_thumb_size (size);

  if (g_file_test (thumb_subdirs[size], G_FILE_TEST_IS_DIR))
    return TRUE;

  if (g_file_test (thumb_dir, G_FILE_TEST_IS_DIR) ||
      (g_mkdir_with_parents (thumb_dir, S_IRUSR | S_IWUSR | S_IXUSR) == 0))
    {
      if (size == 0)
        g_mkdir_with_parents (thumb_fail_subdir, S_IRUSR | S_IWUSR | S_IXUSR);

      g_mkdir_with_parents (thumb_subdirs[size], S_IRUSR | S_IWUSR | S_IXUSR);
    }

  if (g_file_test (thumb_subdirs[size], G_FILE_TEST_IS_DIR))
    return TRUE;

  g_set_error (error,
               PICMAN_THUMB_ERROR, PICMAN_THUMB_ERROR_MKDIR,
               _("Failed to create thumbnail folder '%s'."),
               thumb_subdirs[size]);

  return FALSE;
}

/**
 * picman_thumb_ensure_thumb_dir_local:
 * @dirname: the basename of the dir, without the actual dirname itself
 * @size:    a PicmanThumbSize
 * @error:   return location for possible errors
 *
 * This function checks if the directory that is required to store
 * local thumbnails for a particular @size exist and attempts to
 * create it if necessary.
 *
 * You shouldn't have to call this function directly since
 * picman_thumbnail_save_thumb_local() will do this for you.
 *
 * Return value: %TRUE is the directory exists, %FALSE if it could not
 *               be created
 *
 * Since: PICMAN 2.2
 **/
gboolean
picman_thumb_ensure_thumb_dir_local (const gchar    *dirname,
                                   PicmanThumbSize   size,
                                   GError        **error)
{
  gchar *basedir;
  gchar *subdir;

  g_return_val_if_fail (picman_thumb_initialized, FALSE);
  g_return_val_if_fail (dirname != NULL, FALSE);
  g_return_val_if_fail (g_path_is_absolute (dirname), FALSE);
  g_return_val_if_fail (size > PICMAN_THUMB_SIZE_FAIL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  size = picman_thumb_size (size);

  subdir = g_build_filename (dirname,
                             ".thumblocal", thumb_sizenames[size],
                             NULL);

  if (g_file_test (subdir, G_FILE_TEST_IS_DIR))
    {
      g_free (subdir);
      return TRUE;
    }

  basedir = g_build_filename (dirname, ".thumblocal", NULL);

  if (g_file_test (basedir, G_FILE_TEST_IS_DIR) ||
      (g_mkdir (thumb_dir, S_IRUSR | S_IWUSR | S_IXUSR) == 0))
    {
      g_mkdir (subdir, S_IRUSR | S_IWUSR | S_IXUSR);
    }

  g_free (basedir);

  if (g_file_test (subdir, G_FILE_TEST_IS_DIR))
    {
      g_free (subdir);
      return TRUE;
    }

  g_set_error (error,
               PICMAN_THUMB_ERROR, PICMAN_THUMB_ERROR_MKDIR,
               _("Failed to create thumbnail folder '%s'."),
               subdir);
  g_free (subdir);

  return FALSE;
}

/**
 * picman_thumb_name_from_uri:
 * @uri: an escaped URI
 * @size: a #PicmanThumbSize
 *
 * Creates the name of the thumbnail file of the specified @size that
 * belongs to an image file located at the given @uri.
 *
 * Return value: a newly allocated filename in the encoding of the
 *               filesystem or %NULL if @uri points to the user's
 *               thumbnail repository.
 **/
gchar *
picman_thumb_name_from_uri (const gchar   *uri,
                          PicmanThumbSize  size)
{
  g_return_val_if_fail (picman_thumb_initialized, NULL);
  g_return_val_if_fail (uri != NULL, NULL);

  if (strstr (uri, thumb_dir))
    return NULL;

  size = picman_thumb_size (size);

  return g_build_filename (thumb_subdirs[size],
                           picman_thumb_png_name (uri),
                           NULL);
}

/**
 * picman_thumb_name_from_uri_local:
 * @uri: an escaped URI
 * @size: a #PicmanThumbSize
 *
 * Creates the name of a local thumbnail file of the specified @size
 * that belongs to an image file located at the given @uri. Local
 * thumbnails have been introduced with version 0.7 of the spec.
 *
 * Return value: a newly allocated filename in the encoding of the
 *               filesystem or %NULL if @uri is a remote file or
 *               points to the user's thumbnail repository.
 *
 * Since: PICMAN 2.2
 **/
gchar *
picman_thumb_name_from_uri_local (const gchar   *uri,
                                PicmanThumbSize  size)
{
  gchar *filename;
  gchar *result = NULL;

  g_return_val_if_fail (picman_thumb_initialized, NULL);
  g_return_val_if_fail (uri != NULL, NULL);
  g_return_val_if_fail (size > PICMAN_THUMB_SIZE_FAIL, NULL);

  if (strstr (uri, thumb_dir))
    return NULL;

  filename = _picman_thumb_filename_from_uri (uri);

  if (filename)
    {
      const gchar *baseuri = strrchr (uri, '/');

      if (baseuri && baseuri[0] && baseuri[1])
        {
          gchar *dirname = g_path_get_dirname (filename);
          gint   i       = picman_thumb_size (size);

          result = g_build_filename (dirname,
                                     ".thumblocal", thumb_sizenames[i],
                                     picman_thumb_png_name (uri),
                                     NULL);

          g_free (dirname);
        }

      g_free (filename);
    }

  return result;
}

/**
 * picman_thumb_find_thumb:
 * @uri: an escaped URI
 * @size: pointer to a #PicmanThumbSize
 *
 * This function attempts to locate a thumbnail for the given
 * @uri. First it tries the size that is stored at @size. If no
 * thumbnail of that size is found, it will look for a larger
 * thumbnail, then falling back to a smaller size.
 *
 * If the user's thumbnail repository doesn't provide a thumbnail but
 * a local thumbnail repository exists for the folder the image is
 * located in, the same search is done among the local thumbnails (if
 * there are any).
 *
 * If a thumbnail is found, it's size is written to the variable
 * pointer to by @size and the file location is returned.
 *
 * Return value: a newly allocated string in the encoding of the
 *               filesystem or %NULL if no thumbnail for @uri was found
 **/
gchar *
picman_thumb_find_thumb (const gchar   *uri,
                       PicmanThumbSize *size)
{
  gchar *result;

  g_return_val_if_fail (picman_thumb_initialized, NULL);
  g_return_val_if_fail (uri != NULL, NULL);
  g_return_val_if_fail (size != NULL, NULL);
  g_return_val_if_fail (*size > PICMAN_THUMB_SIZE_FAIL, NULL);

  result = picman_thumb_png_lookup (picman_thumb_png_name (uri), NULL, size);

  if (! result)
    {
      gchar *filename = _picman_thumb_filename_from_uri (uri);

      if (filename)
        {
          const gchar *baseuri = strrchr (uri, '/');

          if (baseuri && baseuri[0] && baseuri[1])
            {
              gchar *dirname = g_path_get_dirname (filename);

              result = picman_thumb_png_lookup (picman_thumb_png_name (baseuri + 1),
                                              dirname, size);

              g_free (dirname);
            }

          g_free (filename);
        }
    }

  return result;
}

/**
 * picman_thumb_file_test:
 * @filename: a filename in the encoding of the filesystem
 * @mtime: return location for modification time
 * @size: return location for file size
 * @err_no: return location for system "errno"
 *
 * This is a convenience and portability wrapper around stat(). It
 * checks if the given @filename exists and returns modification time
 * and file size in 64bit integer values.
 *
 * Return value: The type of the file, or #PICMAN_THUMB_FILE_TYPE_NONE if
 *               the file doesn't exist.
 **/
PicmanThumbFileType
picman_thumb_file_test (const gchar *filename,
                      gint64      *mtime,
                      gint64      *size,
                      gint        *err_no)
{
  struct stat s;

  g_return_val_if_fail (filename != NULL, FALSE);

  if (g_stat (filename, &s) == 0)
    {
      if (mtime)  *mtime  = s.st_mtime;
      if (size)   *size   = s.st_size;
      if (err_no) *err_no = 0;

      if (S_ISREG (s.st_mode))
        {
          return PICMAN_THUMB_FILE_TYPE_REGULAR;
        }
      else if (S_ISDIR (s.st_mode))
        {
          return PICMAN_THUMB_FILE_TYPE_FOLDER;
        }

      return PICMAN_THUMB_FILE_TYPE_SPECIAL;
    }

  if (mtime)  *mtime  = 0;
  if (size)   *size   = 0;
  if (err_no) *err_no = errno;

  return PICMAN_THUMB_FILE_TYPE_NONE;
}

/**
 * picman_thumbs_delete_for_uri:
 * @uri: an escaped URI
 *
 * Deletes all thumbnails for the image file specified by @uri from the
 * user's thumbnail repository.
 *
 * Since: PICMAN 2.2
 **/
void
picman_thumbs_delete_for_uri (const gchar *uri)
{
  gint i;

  g_return_if_fail (picman_thumb_initialized);
  g_return_if_fail (uri != NULL);

  for (i = 0; i < thumb_num_sizes; i++)
    {
      gchar *filename = picman_thumb_name_from_uri (uri, thumb_sizes[i]);

      if (filename)
        {
          g_unlink (filename);
          g_free (filename);
        }
    }
}

/**
 * picman_thumbs_delete_for_uri_local:
 * @uri: an escaped URI
 *
 * Deletes all thumbnails for the image file specified by @uri from
 * the local thumbnail repository.
 *
 * Since: PICMAN 2.2
 **/
void
picman_thumbs_delete_for_uri_local (const gchar *uri)
{
  gint i;

  g_return_if_fail (picman_thumb_initialized);
  g_return_if_fail (uri != NULL);

  for (i = 0; i < thumb_num_sizes; i++)
    {
      gchar *filename = picman_thumb_name_from_uri_local (uri, thumb_sizes[i]);

      if (filename)
        {
          g_unlink (filename);
          g_free (filename);
        }
    }
}

void
_picman_thumbs_delete_others (const gchar   *uri,
                            PicmanThumbSize  size)
{
  gint i;

  g_return_if_fail (picman_thumb_initialized);
  g_return_if_fail (uri != NULL);

  size = picman_thumb_size (size);

  for (i = 0; i < thumb_num_sizes; i++)
    {
      gchar *filename;

      if (i == size)
        continue;

      filename = picman_thumb_name_from_uri (uri, thumb_sizes[i]);
      if (filename)
        {
          g_unlink (filename);
          g_free (filename);
        }
    }
}

gchar *
_picman_thumb_filename_from_uri (const gchar *uri)
{
  gchar *filename;
  gchar *hostname;

  g_return_val_if_fail (uri != NULL, NULL);

  filename = g_filename_from_uri (uri, &hostname, NULL);

  if (!filename)
    return NULL;

  if (hostname)
    {
      /*  we have a file: URI with a hostname                           */

#ifdef G_OS_WIN32
      /*  on Win32, create a valid UNC path and use it as the filename  */
      gchar *tmp = g_build_filename ("//", hostname, filename, NULL);

      g_free (filename);
      filename = tmp;
#else
      /*  otherwise return NULL, caller should use URI then             */
      g_free (filename);
      filename = NULL;
#endif

      g_free (hostname);
    }

  return filename;
}

static void
picman_thumb_exit (void)
{
  gint i;

  g_free (thumb_dir);
  g_free (thumb_sizes);
  g_free (thumb_sizenames);
  for (i = 0; i < thumb_num_sizes; i++)
    g_free (thumb_subdirs[i]);
  g_free (thumb_subdirs);
  g_free (thumb_fail_subdir);

  thumb_num_sizes        = 0;
  thumb_sizes            = NULL;
  thumb_sizenames        = NULL;
  thumb_dir              = NULL;
  thumb_subdirs          = NULL;
  thumb_fail_subdir      = NULL;
  picman_thumb_initialized = FALSE;
}

static gint
picman_thumb_size (PicmanThumbSize size)
{
  gint i = 0;

  if (size > PICMAN_THUMB_SIZE_FAIL)
    {
      for (i = 1;
           i < thumb_num_sizes && thumb_sizes[i] < size;
           i++)
        /* nothing */;

      if (i == thumb_num_sizes)
        i--;
    }

  return i;
}

static gchar *
picman_thumb_png_lookup (const gchar   *name,
                       const gchar   *basedir,
                       PicmanThumbSize *size)
{
  gchar  *thumb_name = NULL;
  gchar **subdirs    = NULL;
  gint    i, n;

  if (basedir)
    {
      gchar *dir = g_build_filename (basedir, ".thumblocal", NULL);

      if (g_file_test (basedir, G_FILE_TEST_IS_DIR))
        {
          gint i;

          subdirs = g_new (gchar *, thumb_num_sizes);

          subdirs[0] = NULL;  /*  PICMAN_THUMB_SIZE_FAIL  */

          for (i = 1; i < thumb_num_sizes; i++)
            subdirs[i] = g_build_filename (dir, thumb_sizenames[i], NULL);
        }

      g_free (dir);
    }
  else
    {
      subdirs = thumb_subdirs;
    }

  if (! subdirs)
    return NULL;

  i = n = picman_thumb_size (*size);

  for (; i < thumb_num_sizes; i++)
    {
      if (! subdirs[i])
        continue;

      thumb_name = g_build_filename (subdirs[i], name, NULL);

      if (picman_thumb_file_test (thumb_name,
                                NULL, NULL,
                                NULL) == PICMAN_THUMB_FILE_TYPE_REGULAR)
        {
          *size = thumb_sizes[i];
          goto finish;
        }

      g_free (thumb_name);
    }

  for (i = n - 1; i >= 0; i--)
    {
      if (! subdirs[i])
        continue;

      thumb_name = g_build_filename (subdirs[i], name, NULL);

      if (picman_thumb_file_test (thumb_name,
                                NULL, NULL,
                                NULL) == PICMAN_THUMB_FILE_TYPE_REGULAR)
        {
          *size = thumb_sizes[i];
          goto finish;
        }

      g_free (thumb_name);
    }

  thumb_name = NULL;

 finish:
  if (basedir)
    {
      for (i = 0; i < thumb_num_sizes; i++)
        g_free (subdirs[i]);
      g_free (subdirs);
    }

  return thumb_name;
}

static const gchar *
picman_thumb_png_name (const gchar *uri)
{
  static gchar name[40];

  GChecksum *checksum;
  guchar     digest[16];
  gsize      len = sizeof (digest);
  gsize      i;

  checksum = g_checksum_new (G_CHECKSUM_MD5);
  g_checksum_update (checksum, (const guchar *) uri, -1);
  g_checksum_get_digest (checksum, digest, &len);
  g_checksum_free (checksum);

  for (i = 0; i < len; i++)
    {
      guchar n;

      n = (digest[i] >> 4) & 0xF;
      name[i * 2]     = (n > 9) ? 'a' + n - 10 : '0' + n;

      n = digest[i] & 0xF;
      name[i * 2 + 1] = (n > 9) ? 'a' + n - 10 : '0' + n;
    }

  strncpy (name + 32, ".png", 5);

  return (const gchar *) name;
}
