/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanvaluearray.c ported from GValueArray
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

#include "config.h"

#include <string.h>

#include <glib-object.h>

#include "picmanbasetypes.h"

#include "picmanvaluearray.h"


/**
 * SECTION:value_arrays
 * @short_description: A container structure to maintain an array of
 *     generic values
 * @see_also: #GValue, #GParamSpecValueArray, picman_param_spec_value_array()
 * @title: Value arrays
 *
 * The prime purpose of a #PicmanValueArray is for it to be used as an
 * object property that holds an array of values. A #PicmanValueArray wraps
 * an array of #GValue elements in order for it to be used as a boxed
 * type through %PICMAN_TYPE_VALUE_ARRAY.
 */


#define GROUP_N_VALUES (1) /* power of 2 !! */


/**
 * PicmanValueArray:
 * @n_values: number of values contained in the array
 * @values: array of values
 *
 * A #PicmanValueArray contains an array of #GValue elements.
 *
 * Since: PICMAN 2.10
 */
struct _PicmanValueArray
{
  gint    n_values;
  GValue *values;

  gint    n_prealloced;
  gint    ref_count;
};


G_DEFINE_BOXED_TYPE (PicmanValueArray, picman_value_array,
                     picman_value_array_ref, picman_value_array_unref)


/**
 * picman_value_array_index:
 * @value_array: #PicmanValueArray to get a value from
 * @index_: index of the value of interest
 *
 * Return a pointer to the value at @index_ containd in @value_array.
 *
 * Returns: (transfer none): pointer to a value at @index_ in @value_array
 *
 * Since: PICMAN 2.10
 */
GValue *
picman_value_array_index (const PicmanValueArray *value_array,
                        gint                  index)
{
  g_return_val_if_fail (value_array != NULL, NULL);
  g_return_val_if_fail (index < value_array->n_values, NULL);

  return value_array->values + index;
}

static inline void
value_array_grow (PicmanValueArray *value_array,
		  gint            n_values,
		  gboolean        zero_init)
{
  g_return_if_fail ((guint) n_values >= (guint) value_array->n_values);

  value_array->n_values = n_values;
  if (value_array->n_values > value_array->n_prealloced)
    {
      gint i = value_array->n_prealloced;

      value_array->n_prealloced = (value_array->n_values + GROUP_N_VALUES - 1) & ~(GROUP_N_VALUES - 1);
      value_array->values = g_renew (GValue, value_array->values, value_array->n_prealloced);

      if (!zero_init)
	i = value_array->n_values;

      memset (value_array->values + i, 0,
	      (value_array->n_prealloced - i) * sizeof (value_array->values[0]));
    }
}

static inline void
value_array_shrink (PicmanValueArray *value_array)
{
  if (value_array->n_prealloced >= value_array->n_values + GROUP_N_VALUES)
    {
      value_array->n_prealloced = (value_array->n_values + GROUP_N_VALUES - 1) & ~(GROUP_N_VALUES - 1);
      value_array->values = g_renew (GValue, value_array->values, value_array->n_prealloced);
    }
}

/**
 * picman_value_array_new:
 * @n_prealloced: number of values to preallocate space for
 *
 * Allocate and initialize a new #PicmanValueArray, optionally preserve space
 * for @n_prealloced elements. New arrays always contain 0 elements,
 * regardless of the value of @n_prealloced.
 *
 * Returns: a newly allocated #PicmanValueArray with 0 values
 *
 * Since: PICMAN 2.10
 */
PicmanValueArray *
picman_value_array_new (gint n_prealloced)
{
  PicmanValueArray *value_array = g_slice_new (PicmanValueArray);

  value_array->n_values = 0;
  value_array->n_prealloced = 0;
  value_array->values = NULL;
  value_array_grow (value_array, n_prealloced, TRUE);
  value_array->n_values = 0;
  value_array->ref_count = 1;

  return value_array;
}

/**
 * picman_value_array_ref:
 * @value_array: #PicmanValueArray to ref
 *
 * Adds a reference to a #PicmanValueArray.
 *
 * Since: PICMAN 2.10
 */
PicmanValueArray *
picman_value_array_ref (PicmanValueArray *value_array)
{
  g_return_val_if_fail (value_array != NULL, NULL);

  value_array->ref_count++;

  return value_array;
}

/**
 * picman_value_array_unref:
 * @value_array: #PicmanValueArray to unref
 *
 * Unref a #PicmanValueArray. If the reference count drops to zero, the
 * array including its contents are freed.
 *
 * Since: PICMAN 2.10
 */
void
picman_value_array_unref (PicmanValueArray *value_array)
{
  g_return_if_fail (value_array != NULL);

  value_array->ref_count--;

  if (value_array->ref_count < 1)
    {
      gint i;

      for (i = 0; i < value_array->n_values; i++)
        {
          GValue *value = value_array->values + i;

          if (G_VALUE_TYPE (value) != 0) /* we allow unset values in the array */
            g_value_unset (value);
        }
      g_free (value_array->values);
      g_slice_free (PicmanValueArray, value_array);
    }
}

gint
picman_value_array_length (const PicmanValueArray *value_array)
{
  g_return_val_if_fail (value_array != NULL, 0);

  return value_array->n_values;
}

/**
 * picman_value_array_prepend:
 * @value_array: #PicmanValueArray to add an element to
 * @value: (allow-none): #GValue to copy into #PicmanValueArray, or %NULL
 *
 * Insert a copy of @value as first element of @value_array. If @value is
 * %NULL, an uninitialized value is prepended.
 *
 * Returns: (transfer none): the #PicmanValueArray passed in as @value_array
 *
 * Since: PICMAN 2.10
 */
PicmanValueArray *
picman_value_array_prepend (PicmanValueArray *value_array,
                          const GValue   *value)
{
  g_return_val_if_fail (value_array != NULL, NULL);

  return picman_value_array_insert (value_array, 0, value);
}

/**
 * picman_value_array_append:
 * @value_array: #PicmanValueArray to add an element to
 * @value: (allow-none): #GValue to copy into #PicmanValueArray, or %NULL
 *
 * Insert a copy of @value as last element of @value_array. If @value is
 * %NULL, an uninitialized value is appended.
 *
 * Returns: (transfer none): the #PicmanValueArray passed in as @value_array
 *
 * Since: PICMAN 2.10
 */
PicmanValueArray *
picman_value_array_append (PicmanValueArray *value_array,
                         const GValue   *value)
{
  g_return_val_if_fail (value_array != NULL, NULL);

  return picman_value_array_insert (value_array, value_array->n_values, value);
}

/**
 * picman_value_array_insert:
 * @value_array: #PicmanValueArray to add an element to
 * @index_: insertion position, must be &lt;= value_array-&gt;n_values
 * @value: (allow-none): #GValue to copy into #PicmanValueArray, or %NULL
 *
 * Insert a copy of @value at specified position into @value_array. If @value
 * is %NULL, an uninitialized value is inserted.
 *
 * Returns: (transfer none): the #PicmanValueArray passed in as @value_array
 *
 * Since: PICMAN 2.10
 */
PicmanValueArray *
picman_value_array_insert (PicmanValueArray *value_array,
                         gint            index,
                         const GValue   *value)
{
  gint i;

  g_return_val_if_fail (value_array != NULL, NULL);
  g_return_val_if_fail (index <= value_array->n_values, value_array);

  i = value_array->n_values;
  value_array_grow (value_array, value_array->n_values + 1, FALSE);

  if (index + 1 < value_array->n_values)
    g_memmove (value_array->values + index + 1, value_array->values + index,
	       (i - index) * sizeof (value_array->values[0]));

  memset (value_array->values + index, 0, sizeof (value_array->values[0]));

  if (value)
    {
      g_value_init (value_array->values + index, G_VALUE_TYPE (value));
      g_value_copy (value, value_array->values + index);
    }

  return value_array;
}

/**
 * picman_value_array_remove:
 * @value_array: #PicmanValueArray to remove an element from
 * @index_: position of value to remove, which must be less than
 *          <code>value_array-><link
 *          linkend="PicmanValueArray.n-values">n_values</link></code>
 *
 * Remove the value at position @index_ from @value_array.
 *
 * Returns: (transfer none): the #PicmanValueArray passed in as @value_array
 *
 * Since: PICMAN 2.10
 */
PicmanValueArray *
picman_value_array_remove (PicmanValueArray *value_array,
                         gint            index)
{
  g_return_val_if_fail (value_array != NULL, NULL);
  g_return_val_if_fail (index < value_array->n_values, value_array);

  if (G_VALUE_TYPE (value_array->values + index) != 0)
    g_value_unset (value_array->values + index);

  value_array->n_values--;

  if (index < value_array->n_values)
    g_memmove (value_array->values + index, value_array->values + index + 1,
	       (value_array->n_values - index) * sizeof (value_array->values[0]));

  value_array_shrink (value_array);

  if (value_array->n_prealloced > value_array->n_values)
    memset (value_array->values + value_array->n_values, 0, sizeof (value_array->values[0]));

  return value_array;
}

void
picman_value_array_truncate (PicmanValueArray *value_array,
                           gint            n_values)
{
  gint i;

  g_return_if_fail (value_array != NULL);
  g_return_if_fail (n_values > 0 && n_values <= value_array->n_values);

  for (i = value_array->n_values; i > n_values; i--)
    picman_value_array_remove (value_array, i - 1);
}


/*
 * PICMAN_TYPE_PARAM_VALUE_ARRAY
 */

static void       picman_param_value_array_class_init  (GParamSpecClass *klass);
static void       picman_param_value_array_init        (GParamSpec      *pspec);
static void       picman_param_value_array_finalize    (GParamSpec      *pspec);
static void       picman_param_value_array_set_default (GParamSpec      *pspec,
                                                      GValue          *value);
static gboolean   picman_param_value_array_validate    (GParamSpec      *pspec,
                                                      GValue          *value);
static gint       picman_param_value_array_values_cmp  (GParamSpec      *pspec,
                                                      const GValue    *value1,
                                                      const GValue    *value2);

GType
picman_param_value_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_value_array_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecValueArray),
        0,
        (GInstanceInitFunc) picman_param_value_array_init
      };

      type = g_type_register_static (G_TYPE_PARAM_BOXED,
                                     "PicmanParamValueArray", &info, 0);
    }

  return type;
}


static void
picman_param_value_array_class_init (GParamSpecClass *klass)
{
  klass->value_type        = PICMAN_TYPE_VALUE_ARRAY;
  klass->finalize          = picman_param_value_array_finalize;
  klass->value_set_default = picman_param_value_array_set_default;
  klass->value_validate    = picman_param_value_array_validate;
  klass->values_cmp        = picman_param_value_array_values_cmp;
}

static void
picman_param_value_array_init (GParamSpec *pspec)
{
  PicmanParamSpecValueArray *aspec = PICMAN_PARAM_SPEC_VALUE_ARRAY (pspec);

  aspec->element_spec = NULL;
  aspec->fixed_n_elements = 0; /* disable */
}

static inline guint
picman_value_array_ensure_size (PicmanValueArray *value_array,
                              guint           fixed_n_elements)
{
  guint changed = 0;

  if (fixed_n_elements)
    {
      while (picman_value_array_length (value_array) < fixed_n_elements)
        {
          picman_value_array_append (value_array, NULL);
          changed++;
        }

      while (picman_value_array_length (value_array) > fixed_n_elements)
        {
          picman_value_array_remove (value_array,
                                   picman_value_array_length (value_array) - 1);
          changed++;
        }
    }

  return changed;
}

static void
picman_param_value_array_finalize (GParamSpec *pspec)
{
  PicmanParamSpecValueArray *aspec = PICMAN_PARAM_SPEC_VALUE_ARRAY (pspec);
  GParamSpecClass *parent_class = g_type_class_peek (g_type_parent (PICMAN_TYPE_PARAM_VALUE_ARRAY));

  if (aspec->element_spec)
    {
      g_param_spec_unref (aspec->element_spec);
      aspec->element_spec = NULL;
    }

  parent_class->finalize (pspec);
}

static void
picman_param_value_array_set_default (GParamSpec *pspec,
                                    GValue     *value)
{
  PicmanParamSpecValueArray *aspec = PICMAN_PARAM_SPEC_VALUE_ARRAY (pspec);

  if (!value->data[0].v_pointer && aspec->fixed_n_elements)
    value->data[0].v_pointer = picman_value_array_new (aspec->fixed_n_elements);

  if (value->data[0].v_pointer)
    {
      /* g_value_reset (value);  already done */
      picman_value_array_ensure_size (value->data[0].v_pointer,
                                    aspec->fixed_n_elements);
    }
}

static gboolean
picman_param_value_array_validate (GParamSpec *pspec,
                                 GValue     *value)
{
  PicmanParamSpecValueArray *aspec = PICMAN_PARAM_SPEC_VALUE_ARRAY (pspec);
  PicmanValueArray *value_array = value->data[0].v_pointer;
  guint changed = 0;

  if (!value->data[0].v_pointer && aspec->fixed_n_elements)
    value->data[0].v_pointer = picman_value_array_new (aspec->fixed_n_elements);

  if (value->data[0].v_pointer)
    {
      /* ensure array size validity */
      changed += picman_value_array_ensure_size (value_array,
                                               aspec->fixed_n_elements);

      /* ensure array values validity against a present element spec */
      if (aspec->element_spec)
        {
          GParamSpec *element_spec = aspec->element_spec;
          gint        length       = picman_value_array_length (value_array);
          gint        i;

          for (i = 0; i < length; i++)
            {
              GValue *element = picman_value_array_index (value_array, i);

              /* need to fixup value type, or ensure that the array
               * value is initialized at all
               */
              if (! g_value_type_compatible (G_VALUE_TYPE (element),
                                             G_PARAM_SPEC_VALUE_TYPE (element_spec)))
                {
                  if (G_VALUE_TYPE (element) != 0)
                    g_value_unset (element);

                  g_value_init (element, G_PARAM_SPEC_VALUE_TYPE (element_spec));
                  g_param_value_set_default (element_spec, element);
                  changed++;
                }

              /* validate array value against element_spec */
              changed += g_param_value_validate (element_spec, element);
            }
        }
    }

  return changed;
}

static gint
picman_param_value_array_values_cmp (GParamSpec   *pspec,
                                   const GValue *value1,
                                   const GValue *value2)
{
  PicmanParamSpecValueArray *aspec        = PICMAN_PARAM_SPEC_VALUE_ARRAY (pspec);
  PicmanValueArray          *value_array1 = value1->data[0].v_pointer;
  PicmanValueArray          *value_array2 = value2->data[0].v_pointer;
  gint                     length1;
  gint                     length2;

  if (!value_array1 || !value_array2)
    return value_array2 ? -1 : value_array1 != value_array2;

  length1 = picman_value_array_length (value_array1);
  length2 = picman_value_array_length (value_array2);

  if (length1 != length2)
    {
      return length1 < length2 ? -1 : 1;
    }
  else if (! aspec->element_spec)
    {
      /* we need an element specification for comparisons, so there's
       * not much to compare here, try to at least provide stable
       * lesser/greater result
       */
      return length1 < length2 ? -1 : length1 > length2;
    }
  else /* length1 == length2 */
    {
      guint i;

      for (i = 0; i < length1; i++)
        {
          GValue *element1 = picman_value_array_index (value_array1, i);
          GValue *element2 = picman_value_array_index (value_array2, i);
          gint    cmp;

          /* need corresponding element types, provide stable result
           * otherwise
           */
          if (G_VALUE_TYPE (element1) != G_VALUE_TYPE (element2))
            return G_VALUE_TYPE (element1) < G_VALUE_TYPE (element2) ? -1 : 1;

          cmp = g_param_values_cmp (aspec->element_spec, element1, element2);
          if (cmp)
            return cmp;
        }

      return 0;
    }
}

GParamSpec *
picman_param_spec_value_array (const gchar *name,
                             const gchar *nick,
                             const gchar *blurb,
                             GParamSpec  *element_spec,
                             GParamFlags  flags)
{
  PicmanParamSpecValueArray *aspec;

  if (element_spec)
    g_return_val_if_fail (G_IS_PARAM_SPEC (element_spec), NULL);

  aspec = g_param_spec_internal (PICMAN_TYPE_PARAM_VALUE_ARRAY,
                                 name,
                                 nick,
                                 blurb,
                                 flags);
  if (element_spec)
    {
      aspec->element_spec = g_param_spec_ref (element_spec);
      g_param_spec_sink (element_spec);
    }

  return G_PARAM_SPEC (aspec);
}
