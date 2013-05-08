/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandata.h
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DATA_H__
#define __PICMAN_DATA_H__

#include <time.h>      /* time_t */

#include "picmanviewable.h"


typedef enum
{
  PICMAN_DATA_ERROR_OPEN,   /*  opening data file failed   */
  PICMAN_DATA_ERROR_READ,   /*  reading data file failed   */
  PICMAN_DATA_ERROR_WRITE,  /*  writing data file failed   */
  PICMAN_DATA_ERROR_DELETE  /*  deleting data file failed  */
} PicmanDataError;


#define PICMAN_TYPE_DATA            (picman_data_get_type ())
#define PICMAN_DATA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DATA, PicmanData))
#define PICMAN_DATA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DATA, PicmanDataClass))
#define PICMAN_IS_DATA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DATA))
#define PICMAN_IS_DATA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DATA))
#define PICMAN_DATA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DATA, PicmanDataClass))


typedef struct _PicmanDataClass PicmanDataClass;

struct _PicmanData
{
  PicmanViewable  parent_instance;
};

struct _PicmanDataClass
{
  PicmanViewableClass  parent_class;

  /*  signals  */
  void          (* dirty)         (PicmanData  *data);

  /*  virtual functions  */
  gboolean      (* save)          (PicmanData  *data,
                                   GError   **error);
  const gchar * (* get_extension) (PicmanData  *data);
  PicmanData    * (* duplicate)     (PicmanData  *data);
};


GType         picman_data_get_type         (void) G_GNUC_CONST;

gboolean      picman_data_save             (PicmanData     *data,
                                          GError      **error);

void          picman_data_dirty            (PicmanData     *data);
void          picman_data_clean            (PicmanData     *data);
gboolean      picman_data_is_dirty         (PicmanData     *data);

void          picman_data_freeze           (PicmanData     *data);
void          picman_data_thaw             (PicmanData     *data);
gboolean      picman_data_is_frozen        (PicmanData     *data);

gboolean      picman_data_delete_from_disk (PicmanData     *data,
                                          GError      **error);

const gchar * picman_data_get_extension    (PicmanData     *data);

void          picman_data_set_filename     (PicmanData     *data,
                                          const gchar  *filename,
                                          gboolean      writable,
                                          gboolean      deletable);
void          picman_data_create_filename  (PicmanData     *data,
                                          const gchar  *dest_dir);
const gchar * picman_data_get_filename     (PicmanData     *data);

void          picman_data_set_folder_tags  (PicmanData     *data,
                                          const gchar  *top_directory);

const gchar * picman_data_get_mime_type    (PicmanData     *data);

gboolean      picman_data_is_writable      (PicmanData     *data);
gboolean      picman_data_is_deletable     (PicmanData     *data);

void          picman_data_set_mtime        (PicmanData     *data,
                                          time_t        mtime);
time_t        picman_data_get_mtime        (PicmanData     *data);

PicmanData    * picman_data_duplicate        (PicmanData     *data);

void          picman_data_make_internal    (PicmanData     *data,
                                          const gchar  *identifier);
gboolean      picman_data_is_internal      (PicmanData     *data);

gint          picman_data_compare          (PicmanData     *data1,
                                          PicmanData     *data2);

#define PICMAN_DATA_ERROR (picman_data_error_quark ())

GQuark        picman_data_error_quark      (void) G_GNUC_CONST;


#endif /* __PICMAN_DATA_H__ */
