/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_BASE_TYPES_H__
#define __PICMAN_BASE_TYPES_H__


#include <libpicmancolor/picmancolortypes.h>
#include <libpicmanmath/picmanmathtypes.h>

#include <libpicmanbase/picmanbaseenums.h>
#include <libpicmanbase/picmanparam.h>


G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/* XXX FIXME move these to a separate file */

#ifdef PICMAN_DISABLE_DEPRECATION_WARNINGS
#define PICMAN_DEPRECATED
#define PICMAN_DEPRECATED_FOR(f)
#define PICMAN_UNAVAILABLE(maj,min)
#else
#define PICMAN_DEPRECATED G_DEPRECATED
#define PICMAN_DEPRECATED_FOR(f) G_DEPRECATED_FOR(f)
#define PICMAN_UNAVAILABLE(maj,min) G_UNAVAILABLE(maj,min)
#endif


typedef struct _PicmanParasite     PicmanParasite;
typedef struct _PicmanDatafileData PicmanDatafileData;
typedef struct _PicmanEnumDesc     PicmanEnumDesc;
typedef struct _PicmanFlagsDesc    PicmanFlagsDesc;
typedef struct _PicmanValueArray   PicmanValueArray;


typedef void (* PicmanDatafileLoaderFunc) (const PicmanDatafileData *file_data,
                                         gpointer                user_data);


/**
 * PicmanEnumDesc:
 * @value:      An enum value.
 * @value_desc: The value's description.
 * @value_help: The value's help text.
 *
 * This structure is used to register translatable descriptions and
 * help texts for enum values. See picman_enum_set_value_descriptions().
 **/
struct _PicmanEnumDesc
{
  gint         value;
  const gchar *value_desc;
  const gchar *value_help;
};

/**
 * PicmanFlagsDesc:
 * @value:      A flag value.
 * @value_desc: The value's description.
 * @value_help: The value's help text.
 *
 * This structure is used to register translatable descriptions and
 * help texts for flag values. See picman_flags_set_value_descriptions().
 **/
struct _PicmanFlagsDesc
{
  guint        value;
  const gchar *value_desc;
  const gchar *value_help;
};


void                  picman_type_set_translation_domain  (GType                type,
                                                         const gchar         *domain);
const gchar         * picman_type_get_translation_domain  (GType                type);

void                  picman_type_set_translation_context (GType                type,
                                                         const gchar         *context);
const gchar         * picman_type_get_translation_context (GType                type);

void                  picman_enum_set_value_descriptions  (GType                enum_type,
                                                         const PicmanEnumDesc  *descriptions);
const PicmanEnumDesc  * picman_enum_get_value_descriptions  (GType                enum_type);

void                  picman_flags_set_value_descriptions (GType                flags_type,
                                                         const PicmanFlagsDesc *descriptions);
const PicmanFlagsDesc * picman_flags_get_value_descriptions (GType                flags_type);


G_END_DECLS

#endif  /* __PICMAN_BASE_TYPES_H__ */
