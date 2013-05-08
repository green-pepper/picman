/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmantemplate.h
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_TEMPLATE_H__
#define __PICMAN_TEMPLATE_H__


#include "picmanviewable.h"


#define PICMAN_TEMPLATE_PARAM_COPY_FIRST (1 << (8 + G_PARAM_USER_SHIFT))

/*  The default image aspect ratio is the golden mean. We use
 *  two adjacent fibonacci numbers for the unstable series and
 *  some less odd values for the stable version.
 */

#ifdef PICMAN_UNSTABLE
#define PICMAN_DEFAULT_IMAGE_WIDTH   610
#define PICMAN_DEFAULT_IMAGE_HEIGHT  377
#else
#define PICMAN_DEFAULT_IMAGE_WIDTH   640
#define PICMAN_DEFAULT_IMAGE_HEIGHT  400
#endif


#define PICMAN_TYPE_TEMPLATE            (picman_template_get_type ())
#define PICMAN_TEMPLATE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEMPLATE, PicmanTemplate))
#define PICMAN_TEMPLATE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEMPLATE, PicmanTemplateClass))
#define PICMAN_IS_TEMPLATE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEMPLATE))
#define PICMAN_IS_TEMPLATE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEMPLATE))
#define PICMAN_TEMPLATE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TEMPLATE, PicmanTemplateClass))


typedef struct _PicmanTemplateClass PicmanTemplateClass;

struct _PicmanTemplate
{
  PicmanViewable  parent_instance;
};

struct _PicmanTemplateClass
{
  PicmanViewableClass  parent_instance;
};


GType               picman_template_get_type            (void) G_GNUC_CONST;

PicmanTemplate      * picman_template_new                 (const gchar  *name);

void                picman_template_set_from_image      (PicmanTemplate *template,
                                                       PicmanImage    *image);

gint                picman_template_get_width           (PicmanTemplate *template);
gint                picman_template_get_height          (PicmanTemplate *template);
PicmanUnit            picman_template_get_unit            (PicmanTemplate *template);

gdouble             picman_template_get_resolution_x    (PicmanTemplate *template);
gdouble             picman_template_get_resolution_y    (PicmanTemplate *template);
PicmanUnit            picman_template_get_resolution_unit (PicmanTemplate *template);

PicmanImageBaseType   picman_template_get_base_type       (PicmanTemplate *template);
PicmanPrecision       picman_template_get_precision       (PicmanTemplate *template);

PicmanFillType        picman_template_get_fill_type       (PicmanTemplate *template);

const gchar       * picman_template_get_comment         (PicmanTemplate *template);

guint64             picman_template_get_initial_size    (PicmanTemplate *template);


#endif /* __PICMAN_TEMPLATE__ */
