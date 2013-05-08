/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantagcache.h
 * Copyright (C) 2008 Aurimas Juška <aurisj@svn.gnome.org>
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

#ifndef __PICMAN_TAG_CACHE_H__
#define __PICMAN_TAG_CACHE_H__


#include "picmanobject.h"


#define PICMAN_TYPE_TAG_CACHE            (picman_tag_cache_get_type ())
#define PICMAN_TAG_CACHE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TAG_CACHE, PicmanTagCache))
#define PICMAN_TAG_CACHE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TAG_CACHE, PicmanTagCacheClass))
#define PICMAN_IS_TAG_CACHE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TAG_CACHE))
#define PICMAN_IS_TAG_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TAG_CACHE))
#define PICMAN_TAG_CACHE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TAG_CACHE, PicmanTagCacheClass))


typedef struct _PicmanTagCacheClass  PicmanTagCacheClass;
typedef struct _PicmanTagCachePriv   PicmanTagCachePriv;

struct _PicmanTagCache
{
  PicmanObject        parent_instance;

  PicmanTagCachePriv *priv;
};

struct _PicmanTagCacheClass
{
  PicmanObjectClass  parent_class;
};


GType           picman_tag_cache_get_type      (void) G_GNUC_CONST;

PicmanTagCache *  picman_tag_cache_new           (void);

void            picman_tag_cache_save          (PicmanTagCache  *cache);
void            picman_tag_cache_load          (PicmanTagCache  *cache);

void            picman_tag_cache_add_container (PicmanTagCache  *cache,
                                              PicmanContainer *container);


#endif  /*  __PICMAN_TAG_CACHE_H__  */
