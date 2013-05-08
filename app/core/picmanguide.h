/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanGuide
 * Copyright (C) 2003  Henrik Brix Andersen <brix@picman.org>
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

#ifndef __PICMAN_GUIDE_H__
#define __PICMAN_GUIDE_H__


#include "picmanobject.h"


#define PICMAN_TYPE_GUIDE            (picman_guide_get_type ())
#define PICMAN_GUIDE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_GUIDE, PicmanGuide))
#define PICMAN_GUIDE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_GUIDE, PicmanGuideClass))
#define PICMAN_IS_GUIDE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_GUIDE))
#define PICMAN_IS_GUIDE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_GUIDE))
#define PICMAN_GUIDE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_GUIDE, PicmanGuideClass))


typedef struct _PicmanGuideClass PicmanGuideClass;

struct _PicmanGuide
{
  GObject              parent_instance;

  guint32              guide_ID;
  PicmanOrientationType  orientation;
  gint                 position;
};

struct _PicmanGuideClass
{
  GObjectClass         parent_class;

  /*  signals  */
  void (* removed)    (PicmanGuide  *guide);
};


GType               picman_guide_get_type        (void) G_GNUC_CONST;

PicmanGuide *         picman_guide_new             (PicmanOrientationType  orientation,
                                                guint32              guide_ID);

guint32             picman_guide_get_ID          (PicmanGuide           *guide);

PicmanOrientationType picman_guide_get_orientation (PicmanGuide           *guide);
void                picman_guide_set_orientation (PicmanGuide           *guide,
                                                PicmanOrientationType  orientation);

gint                picman_guide_get_position    (PicmanGuide           *guide);
void                picman_guide_set_position    (PicmanGuide           *guide,
                                                gint                 position);
void                picman_guide_removed         (PicmanGuide           *guide);

#endif /* __PICMAN_GUIDE_H__ */
