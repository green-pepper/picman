/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picman-utils.h"
#include "picmanmarshal.h"
#include "picmanobject.h"

#include "picman-debug.h"


enum
{
  DISCONNECT,
  NAME_CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_NAME
};


struct _PicmanObjectPrivate
{
  gchar *name;
  gchar *normalized;
  guint  static_name  : 1;
  guint  disconnected : 1;
};


static void    picman_object_class_init       (PicmanObjectClass *klass);
static void    picman_object_init             (PicmanObject      *object,
                                             PicmanObjectClass *klass);
static void    picman_object_dispose          (GObject         *object);
static void    picman_object_finalize         (GObject         *object);
static void    picman_object_set_property     (GObject         *object,
                                             guint            property_id,
                                             const GValue    *value,
                                             GParamSpec      *pspec);
static void    picman_object_get_property     (GObject         *object,
                                             guint            property_id,
                                             GValue          *value,
                                             GParamSpec      *pspec);
static gint64  picman_object_real_get_memsize (PicmanObject      *object,
                                             gint64          *gui_size);
static void    picman_object_name_normalize   (PicmanObject      *object);


static GObjectClass *parent_class = NULL;

static guint object_signals[LAST_SIGNAL] = { 0 };


GType
picman_object_get_type (void)
{
  static GType object_type = 0;

  if (! object_type)
    {
      const GTypeInfo object_info =
      {
        sizeof (PicmanObjectClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) picman_object_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (PicmanObject),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) picman_object_init,
      };

      object_type = g_type_register_static (G_TYPE_OBJECT,
                                            "PicmanObject",
                                            &object_info, 0);
    }

  return object_type;
}

static void
picman_object_class_init (PicmanObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_signals[DISCONNECT] =
    g_signal_new ("disconnect",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanObjectClass, disconnect),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_signals[NAME_CHANGED] =
    g_signal_new ("name-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanObjectClass, name_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->dispose      = picman_object_dispose;
  object_class->finalize     = picman_object_finalize;
  object_class->set_property = picman_object_set_property;
  object_class->get_property = picman_object_get_property;

  klass->disconnect          = NULL;
  klass->name_changed        = NULL;
  klass->get_memsize         = picman_object_real_get_memsize;

  g_object_class_install_property (object_class, PROP_NAME,
                                   g_param_spec_string ("name",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
  g_type_class_add_private (klass,
                            sizeof (PicmanObjectPrivate));
}

static void
picman_object_init (PicmanObject      *object,
                  PicmanObjectClass *klass)
{
  object->p = G_TYPE_INSTANCE_GET_PRIVATE (object,
                                           PICMAN_TYPE_OBJECT,
                                           PicmanObjectPrivate);
  object->p->name       = NULL;
  object->p->normalized = NULL;

  picman_debug_add_instance (G_OBJECT (object), G_OBJECT_CLASS (klass));
}

static void
picman_object_dispose (GObject *object)
{
  PicmanObject *picman_object = PICMAN_OBJECT (object);

  if (! picman_object->p->disconnected)
    {
      g_signal_emit (object, object_signals[DISCONNECT], 0);

      picman_object->p->disconnected = TRUE;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_object_finalize (GObject *object)
{
  picman_object_name_free (PICMAN_OBJECT (object));

  picman_debug_remove_instance (object);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_object_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  PicmanObject *picman_object = PICMAN_OBJECT (object);

  switch (property_id)
    {
    case PROP_NAME:
      picman_object_set_name (picman_object, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_object_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  PicmanObject *picman_object = PICMAN_OBJECT (object);

  switch (property_id)
    {
    case PROP_NAME:
      if (picman_object->p->static_name)
        g_value_set_static_string (value, picman_object->p->name);
      else
        g_value_set_string (value, picman_object->p->name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

/**
 * picman_object_set_name:
 * @object: a #PicmanObject
 * @name: the @object's new name
 *
 * Sets the @object's name. Takes care of freeing the old name and
 * emitting the ::name_changed signal if the old and new name differ.
 **/
void
picman_object_set_name (PicmanObject  *object,
                      const gchar *name)
{
  g_return_if_fail (PICMAN_IS_OBJECT (object));

  if (! g_strcmp0 (object->p->name, name))
    return;

  picman_object_name_free (object);

  object->p->name = g_strdup (name);
  object->p->static_name = FALSE;

  picman_object_name_changed (object);
  g_object_notify (G_OBJECT (object), "name");
}

/**
 * picman_object_set_name_safe:
 * @object: a #PicmanObject
 * @name: the @object's new name
 *
 * A safe version of picman_object_set_name() that takes care of
 * handling newlines and overly long names. The actual name set
 * may be different to the @name you pass.
 **/
void
picman_object_set_name_safe (PicmanObject  *object,
                           const gchar *name)
{
  g_return_if_fail (PICMAN_IS_OBJECT (object));

  if (! g_strcmp0 (object->p->name, name))
    return;

  picman_object_name_free (object);

  object->p->name = picman_utf8_strtrim (name, 30);
  object->p->static_name = FALSE;

  picman_object_name_changed (object);
  g_object_notify (G_OBJECT (object), "name");
}

void
picman_object_set_static_name (PicmanObject  *object,
                             const gchar *name)
{
  g_return_if_fail (PICMAN_IS_OBJECT (object));

  picman_object_name_free (object);

  object->p->name = (gchar *) name;
  object->p->static_name = TRUE;

  picman_object_name_changed (object);
  g_object_notify (G_OBJECT (object), "name");
}

void
picman_object_take_name (PicmanObject *object,
                       gchar      *name)
{
  g_return_if_fail (PICMAN_IS_OBJECT (object));

  picman_object_name_free (object);

  object->p->name = name;
  object->p->static_name = FALSE;

  picman_object_name_changed (object);
  g_object_notify (G_OBJECT (object), "name");
}

/**
 * picman_object_get_name:
 * @object: a #PicmanObject
 *
 * This function gives access to the name of a PicmanObject. The
 * returned name belongs to the object and must not be freed.
 *
 * Return value: a pointer to the @object's name
 **/
const gchar *
picman_object_get_name (gconstpointer object)
{
  const PicmanObject *object_typed = object;
  g_return_val_if_fail (PICMAN_IS_OBJECT (object_typed), NULL);

  return object_typed->p->name;
}

/**
 * picman_object_name_changed:
 * @object: a #PicmanObject
 *
 * Causes the ::name-changed signal to be emitted.
 **/
void
picman_object_name_changed (PicmanObject *object)
{
  g_return_if_fail (PICMAN_IS_OBJECT (object));

  g_signal_emit (object, object_signals[NAME_CHANGED], 0);
}

/**
 * picman_object_name_free:
 * @object: a #PicmanObject
 *
 * Frees the name of @object and sets the name pointer to %NULL. Also
 * takes care of the normalized name that the object might be caching.
 *
 * In general you should be using picman_object_set_name() instead. But
 * if you ever need to free the object name but don't want the
 * ::name-changed signal to be emitted, then use this function. Never
 * ever free the object name directly!
 **/
void
picman_object_name_free (PicmanObject *object)
{
  if (object->p->normalized)
    {
      if (object->p->normalized != object->p->name)
        g_free (object->p->normalized);

      object->p->normalized = NULL;
    }

  if (object->p->name)
    {
      if (! object->p->static_name)
        g_free (object->p->name);

      object->p->name = NULL;
      object->p->static_name = FALSE;
    }
}

/**
 * picman_object_name_collate:
 * @object1: a #PicmanObject
 * @object2: another #PicmanObject
 *
 * Compares two object names for ordering using the linguistically
 * correct rules for the current locale. It caches the normalized
 * version of the object name to speed up subsequent calls.
 *
 * Return value: -1 if object1 compares before object2,
 *                0 if they compare equal,
 *                1 if object1 compares after object2.
 **/
gint
picman_object_name_collate (PicmanObject *object1,
                          PicmanObject *object2)
{
  if (! object1->p->normalized)
    picman_object_name_normalize (object1);

  if (! object2->p->normalized)
    picman_object_name_normalize (object2);

  return strcmp (object1->p->normalized, object2->p->normalized);
}

static void
picman_object_name_normalize (PicmanObject *object)
{
  g_return_if_fail (object->p->normalized == NULL);

  if (object->p->name)
    {
      gchar *key = g_utf8_collate_key (object->p->name, -1);

      if (strcmp (key, object->p->name))
        {
          object->p->normalized = key;
        }
      else
        {
          g_free (key);
          object->p->normalized = object->p->name;
        }
    }
}


#define DEBUG_MEMSIZE 1

#ifdef DEBUG_MEMSIZE
gboolean picman_debug_memsize = FALSE;
#endif

gint64
picman_object_get_memsize (PicmanObject *object,
                         gint64     *gui_size)
{
  gint64 my_size     = 0;
  gint64 my_gui_size = 0;

  g_return_val_if_fail (object == NULL || PICMAN_IS_OBJECT (object), 0);

  if (! object)
    {
      if (gui_size)
        *gui_size = 0;

      return 0;
    }

#ifdef DEBUG_MEMSIZE
  if (picman_debug_memsize)
    {
      static gint   indent_level     = 0;
      static GList *aggregation_tree = NULL;
      static gchar  indent_buf[256];

      gint64  memsize;
      gint64  gui_memsize = 0;
      gint    i;
      gint    my_indent_level;
      gchar  *object_size;

      indent_level++;

      my_indent_level = indent_level;

      memsize = PICMAN_OBJECT_GET_CLASS (object)->get_memsize (object,
                                                             &gui_memsize);

      indent_level--;

      for (i = 0; i < MIN (my_indent_level * 2, sizeof (indent_buf) - 1); i++)
        indent_buf[i] = ' ';

      indent_buf[i] = '\0';

      object_size = g_strdup_printf ("%s%s \"%s\": "
                                     "%" G_GINT64_FORMAT
                                     "(%" G_GINT64_FORMAT ")\n",
                                     indent_buf,
                                     g_type_name (G_TYPE_FROM_INSTANCE (object)),
                                     object->p->name ? object->p->name : "anonymous",
                                     memsize,
                                     gui_memsize);

      aggregation_tree = g_list_prepend (aggregation_tree, object_size);

      if (indent_level == 0)
        {
          GList *list;

          for (list = aggregation_tree; list; list = g_list_next (list))
            {
              g_print ("%s", (gchar *) list->data);
              g_free (list->data);
            }

          g_list_free (aggregation_tree);
          aggregation_tree = NULL;
        }

      return memsize;
    }
#endif /* DEBUG_MEMSIZE */

  my_size = PICMAN_OBJECT_GET_CLASS (object)->get_memsize (object,
                                                         &my_gui_size);

  if (gui_size)
    *gui_size = my_gui_size;

  return my_size;
}

static gint64
picman_object_real_get_memsize (PicmanObject *object,
                              gint64     *gui_size)
{
  gint64 memsize = 0;

  if (! object->p->static_name)
    memsize += picman_string_get_memsize (object->p->name);

  return memsize + picman_g_object_get_memsize ((GObject *) object);
}
