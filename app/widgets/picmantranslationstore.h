/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantranslationstore.h
 * Copyright (C) 2009  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_TRANSLATION_STORE_H__
#define __PICMAN_TRANSLATION_STORE_H__


#include "picmanlanguagestore.h"


#define PICMAN_TYPE_TRANSLATION_STORE            (picman_translation_store_get_type ())
#define PICMAN_TRANSLATION_STORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TRANSLATION_STORE, PicmanTranslationStore))
#define PICMAN_TRANSLATION_STORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TRANSLATION_STORE, PicmanTranslationStoreClass))
#define PICMAN_IS_TRANSLATION_STORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TRANSLATION_STORE))
#define PICMAN_IS_TRANSLATION_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TRANSLATION_STORE))
#define PICMAN_TRANSLATION_STORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TRANSLATION_STORE, PicmanTranslationStoreClass))


typedef struct _PicmanTranslationStoreClass  PicmanTranslationStoreClass;


GType          picman_translation_store_get_type (void) G_GNUC_CONST;

GtkListStore * picman_translation_store_new      (void);


#endif  /* __PICMAN_TRANSLATION_STORE_H__ */
