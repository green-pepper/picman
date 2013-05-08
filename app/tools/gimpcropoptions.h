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

#ifndef __PICMAN_CROP_OPTIONS_H__
#define __PICMAN_CROP_OPTIONS_H__


#include "core/picmantooloptions.h"


#define PICMAN_TYPE_CROP_OPTIONS            (picman_crop_options_get_type ())
#define PICMAN_CROP_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CROP_OPTIONS, PicmanCropOptions))
#define PICMAN_CROP_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CROP_OPTIONS, PicmanCropOptionsClass))
#define PICMAN_IS_CROP_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CROP_OPTIONS))
#define PICMAN_IS_CROP_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CROP_OPTIONS))
#define PICMAN_CROP_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CROP_OPTIONS, PicmanCropOptionsClass))


typedef struct _PicmanCropOptions      PicmanCropOptions;
typedef struct _PicmanToolOptionsClass PicmanCropOptionsClass;

struct _PicmanCropOptions
{
  PicmanToolOptions  parent_instence;

  /* Work on the current layer rather than the image. */
  gboolean         layer_only;

  /* Allow the crop rectangle to be larger than the image/layer. This
   * will resize the image/layer.
   */
  gboolean         allow_growing;
};


GType       picman_crop_options_get_type (void) G_GNUC_CONST;

GtkWidget * picman_crop_options_gui      (PicmanToolOptions *tool_options);


#endif /* __PICMAN_CROP_OPTIONS_H__ */
