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

#ifndef  __PICMAN_INK_H__
#define  __PICMAN_INK_H__


#include "picmanpaintcore.h"
#include "picmanink-blob.h"


#define PICMAN_TYPE_INK            (picman_ink_get_type ())
#define PICMAN_INK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_INK, PicmanInk))
#define PICMAN_INK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_INK, PicmanInkClass))
#define PICMAN_IS_INK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_INK))
#define PICMAN_IS_INK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_INK))
#define PICMAN_INK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_INK, PicmanInkClass))


typedef struct _PicmanInkClass PicmanInkClass;

struct _PicmanInk
{
  PicmanPaintCore  parent_instance;

  PicmanBlob      *start_blob;   /*  starting blob (for undo)       */

  PicmanBlob      *cur_blob;     /*  current blob                   */
  PicmanBlob      *last_blob;    /*  blob for last cursor position  */
};

struct _PicmanInkClass
{
  PicmanPaintCoreClass  parent_class;
};


void    picman_ink_register (Picman                      *picman,
                           PicmanPaintRegisterCallback  callback);

GType   picman_ink_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_INK_H__  */
