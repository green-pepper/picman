/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmancontainer-filter.c
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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

#include "config.h"

#include <string.h>

#include <glib-object.h>

#include "core-types.h"

#include "picmancontainer.h"
#include "picmancontainer-filter.h"
#include "picmanlist.h"


typedef struct
{
  PicmanObjectFilterFunc   filter;
  PicmanContainer         *container;
  gpointer               user_data;
} PicmanContainerFilterContext;


static void
picman_container_filter_foreach_func (PicmanObject                 *object,
                                    PicmanContainerFilterContext *context)
{
  if (context->filter (object, context->user_data))
    picman_container_add (context->container, object);
}

/**
 * picman_container_filter:
 * @container: a #PicmanContainer to filter
 * @filter: a #PicmanObjectFilterFunc
 * @user_data: a pointer passed to @filter
 *
 * Calls the supplied @filter function on each object in @container.
 * A return value of %TRUE is interpreted as a match.
 *
 * Returns: a weak #PicmanContainer filled with matching objects.
 **/
PicmanContainer *
picman_container_filter (const PicmanContainer  *container,
                       PicmanObjectFilterFunc  filter,
                       gpointer              user_data)
{
  PicmanContainer              *result;
  PicmanContainerFilterContext  context;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (filter != NULL, NULL);

  result =
    g_object_new (G_TYPE_FROM_INSTANCE (container),
                  "children-type", picman_container_get_children_type (container),
                  "policy",        PICMAN_CONTAINER_POLICY_WEAK,
                  NULL);

  context.filter    = filter;
  context.container = result;
  context.user_data = user_data;

  picman_container_foreach (container,
                          (GFunc) picman_container_filter_foreach_func,
                          &context);

  /*  This is somewhat ugly, but it keeps lists in the same order.  */
  if (PICMAN_IS_LIST (result))
    picman_list_reverse (PICMAN_LIST (result));


  return result;
}


static gboolean
picman_object_filter_by_name (const PicmanObject *object,
                            const GRegex     *regex)
{
  return g_regex_match (regex, picman_object_get_name (object), 0, NULL);
}

/**
 * picman_container_filter_by_name:
 * @container: a #PicmanContainer to filter
 * @regexp: a regular expression (as a %NULL-terminated string)
 * @error: error location to report errors or %NULL
 *
 * This function performs a case-insensitive regular expression search
 * on the names of the PicmanObjects in @container.
 *
 * Returns: a weak #PicmanContainer filled with matching objects.
 **/
PicmanContainer *
picman_container_filter_by_name (const PicmanContainer  *container,
                               const gchar          *regexp,
                               GError              **error)
{
  PicmanContainer *result;
  GRegex        *regex;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (regexp != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  regex = g_regex_new (regexp, G_REGEX_CASELESS | G_REGEX_OPTIMIZE, 0,
                       error);

  if (! regex)
    return NULL;

  result =
    picman_container_filter (container,
                           (PicmanObjectFilterFunc) picman_object_filter_by_name,
                           regex);

  g_regex_unref (regex);

  return result;
}


gchar **
picman_container_get_filtered_name_array (const PicmanContainer  *container,
                                        const gchar          *regexp,
                                        gint                 *length)
{
  PicmanContainer *weak;
  GError        *error = NULL;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (length != NULL, NULL);

  if (regexp == NULL || strlen (regexp) == 0)
    return (picman_container_get_name_array (container, length));

  weak = picman_container_filter_by_name (container, regexp, &error);

  if (weak)
    {
      gchar **retval = picman_container_get_name_array (weak, length);

      g_object_unref (weak);

      return retval;
    }
  else
    {
      g_warning ("%s", error->message);
      g_error_free (error);

      *length = 0;
      return NULL;
    }
}
