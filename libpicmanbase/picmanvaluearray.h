/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanvaluearray.h ported from GValueArray
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

#if !defined (__PICMAN_BASE_H_INSIDE__) && !defined (PICMAN_BASE_COMPILATION)
#error "Only <libpicmanbase/picmanbase.h> can be included directly."
#endif

#ifndef __PICMAN_VALUE_ARRAY_H__
#define __PICMAN_VALUE_ARRAY_H__

G_BEGIN_DECLS

/**
 * PICMAN_TYPE_VALUE_ARRAY:
 *
 * The type ID of the "PicmanValueArray" type which is a boxed type,
 * used to pass around pointers to PicmanValueArrays.
 *
 * Since: PICMAN 2.10
 */
#define PICMAN_TYPE_VALUE_ARRAY (picman_value_array_get_type ())


GType            picman_value_array_get_type (void) G_GNUC_CONST;

PicmanValueArray * picman_value_array_new      (gint                  n_prealloced);

PicmanValueArray * picman_value_array_ref      (PicmanValueArray       *value_array);
void             picman_value_array_unref    (PicmanValueArray       *value_array);

gint             picman_value_array_length   (const PicmanValueArray *value_array);

GValue         * picman_value_array_index    (const PicmanValueArray *value_array,
                                            gint                  index);

PicmanValueArray * picman_value_array_prepend  (PicmanValueArray       *value_array,
                                            const GValue         *value);
PicmanValueArray * picman_value_array_append   (PicmanValueArray       *value_array,
                                            const GValue         *value);
PicmanValueArray * picman_value_array_insert   (PicmanValueArray       *value_array,
                                            gint                  index,
                                            const GValue         *value);

PicmanValueArray * picman_value_array_remove   (PicmanValueArray       *value_array,
                                            gint                  index);
void             picman_value_array_truncate (PicmanValueArray       *value_array,
                                            gint                  n_values);


/*
 * PICMAN_TYPE_PARAM_VALUE_ARRAY
 */

#define PICMAN_TYPE_PARAM_VALUE_ARRAY           (picman_param_value_array_get_type ())
#define PICMAN_IS_PARAM_SPEC_VALUE_ARRAY(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_VALUE_ARRAY))
#define PICMAN_PARAM_SPEC_VALUE_ARRAY(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_VALUE_ARRAY, PicmanParamSpecValueArray))

typedef struct _PicmanParamSpecValueArray PicmanParamSpecValueArray;

struct _PicmanParamSpecValueArray
{
  GParamSpec  parent_instance;
  GParamSpec *element_spec;
  gint        fixed_n_elements;
};

GType        picman_param_value_array_get_type (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_value_array     (const gchar    *name,
                                              const gchar    *nick,
                                              const gchar    *blurb,
                                              GParamSpec     *element_spec,
                                              GParamFlags     flags);


G_END_DECLS

#endif /* __PICMAN_VALUE_ARRAY_H__ */
