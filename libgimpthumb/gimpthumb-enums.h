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

#ifndef __PICMAN_THUMB_ENUMS_H__
#define __PICMAN_THUMB_ENUMS_H__

G_BEGIN_DECLS


/**
 * SECTION: picmanthumb-enums
 * @title: PicmanThumb-enums
 * @short_description: Enumerations used by libpicmanthumb
 *
 * Enumerations used by libpicmanthumb
 **/


/**
 * PicmanThumbFileType:
 * @PICMAN_THUMB_FILE_TYPE_NONE:    file does not exist
 * @PICMAN_THUMB_FILE_TYPE_REGULAR: a regular file
 * @PICMAN_THUMB_FILE_TYPE_FOLDER:  a directory
 * @PICMAN_THUMB_FILE_TYPE_SPECIAL: a special file (device node, fifo, socket, ...)
 *
 * File types as returned by picman_thumb_file_test().
 **/
#define PICMAN_TYPE_THUMB_FILE_TYPE (picman_thumb_file_type_get_type ())

GType picman_thumb_file_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_THUMB_FILE_TYPE_NONE,
  PICMAN_THUMB_FILE_TYPE_REGULAR,
  PICMAN_THUMB_FILE_TYPE_FOLDER,
  PICMAN_THUMB_FILE_TYPE_SPECIAL
} PicmanThumbFileType;


/**
 * PicmanThumbSize:
 * @PICMAN_THUMB_SIZE_FAIL:   special size used to indicate a thumbnail
 *                          creation failure
 * @PICMAN_THUMB_SIZE_NORMAL: normal thumbnail size (128 pixels)
 * @PICMAN_THUMB_SIZE_LARGE:  large thumbnail size (256 pixels)
 *
 * Possible thumbnail sizes as defined by the Thumbnail Managaging
 * Standard.
 **/
#define PICMAN_TYPE_THUMB_SIZE (picman_thumb_size_get_type ())

GType picman_thumb_size_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_THUMB_SIZE_FAIL   = 0,
  PICMAN_THUMB_SIZE_NORMAL = 128,
  PICMAN_THUMB_SIZE_LARGE  = 256
} PicmanThumbSize;


/**
 * PicmanThumbState:
 * @PICMAN_THUMB_STATE_UNKNOWN:   nothing is known about the file/thumbnail
 * @PICMAN_THUMB_STATE_REMOTE:    the file is on a remote file system
 * @PICMAN_THUMB_STATE_FOLDER:    the file is a directory
 * @PICMAN_THUMB_STATE_SPECIAL:   the file is a special file
 * @PICMAN_THUMB_STATE_NOT_FOUND: the file/thumbnail doesn't exist
 * @PICMAN_THUMB_STATE_EXISTS:    the file/thumbnail exists
 * @PICMAN_THUMB_STATE_OLD:       the thumbnail may be outdated
 * @PICMAN_THUMB_STATE_FAILED:    the thumbnail couldn't be created
 * @PICMAN_THUMB_STATE_OK:        the thumbnail exists and matches the image
 *
 * Possible image and thumbnail file states used by libpicmanthumb.
 **/
#define PICMAN_TYPE_THUMB_STATE (picman_thumb_state_get_type ())

GType picman_thumb_state_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_THUMB_STATE_UNKNOWN,
  PICMAN_THUMB_STATE_REMOTE,
  PICMAN_THUMB_STATE_FOLDER,
  PICMAN_THUMB_STATE_SPECIAL,
  PICMAN_THUMB_STATE_NOT_FOUND,
  PICMAN_THUMB_STATE_EXISTS,
  PICMAN_THUMB_STATE_OLD,
  PICMAN_THUMB_STATE_FAILED,
  PICMAN_THUMB_STATE_OK
} PicmanThumbState;


G_END_DECLS

#endif  /* __PICMAN_THUMB_ENUMS_H__ */
