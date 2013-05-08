/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmancontainer.c
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#include <gegl.h>

#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman.h"
#include "picman-utils.h"
#include "picmancontainer.h"
#include "picmanmarshal.h"


/* #define DEBUG_CONTAINER */

#ifdef DEBUG_CONTAINER
#define D(stmnt) stmnt
#else
#define D(stmnt)
#endif


enum
{
  ADD,
  REMOVE,
  REORDER,
  FREEZE,
  THAW,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_CHILDREN_TYPE,
  PROP_POLICY
};


typedef struct
{
  gchar     *signame;
  GCallback  callback;
  gpointer   callback_data;

  GQuark     quark;  /*  used to attach the signal id's of child signals  */
} PicmanContainerHandler;

struct _PicmanContainerPriv
{
  GType                children_type;
  PicmanContainerPolicy  policy;
  gint                 n_children;

  GList               *handlers;
  gint                 freeze_count;
};


/*  local function prototypes  */

static void   picman_container_config_iface_init   (PicmanConfigInterface *iface);

static void       picman_container_dispose         (GObject          *object);

static void       picman_container_set_property    (GObject          *object,
                                                  guint             property_id,
                                                  const GValue     *value,
                                                  GParamSpec       *pspec);
static void       picman_container_get_property    (GObject          *object,
                                                  guint             property_id,
                                                  GValue           *value,
                                                  GParamSpec       *pspec);

static gint64     picman_container_get_memsize     (PicmanObject       *object,
                                                  gint64           *gui_size);

static void       picman_container_real_add        (PicmanContainer    *container,
                                                  PicmanObject       *object);
static void       picman_container_real_remove     (PicmanContainer    *container,
                                                  PicmanObject       *object);

static gboolean   picman_container_serialize       (PicmanConfig       *config,
                                                  PicmanConfigWriter *writer,
                                                  gpointer          data);
static gboolean   picman_container_deserialize     (PicmanConfig       *config,
                                                  GScanner         *scanner,
                                                  gint              nest_level,
                                                  gpointer          data);

static void   picman_container_disconnect_callback (PicmanObject       *object,
                                                  gpointer          data);


G_DEFINE_TYPE_WITH_CODE (PicmanContainer, picman_container, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_container_config_iface_init))

#define parent_class picman_container_parent_class

static guint container_signals[LAST_SIGNAL] = { 0, };


static void
picman_container_class_init (PicmanContainerClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  container_signals[ADD] =
    g_signal_new ("add",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContainerClass, add),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_OBJECT);

  container_signals[REMOVE] =
    g_signal_new ("remove",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContainerClass, remove),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_OBJECT);

  container_signals[REORDER] =
    g_signal_new ("reorder",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContainerClass, reorder),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT_INT,
                  G_TYPE_NONE, 2,
                  PICMAN_TYPE_OBJECT,
                  G_TYPE_INT);

  container_signals[FREEZE] =
    g_signal_new ("freeze",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanContainerClass, freeze),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  container_signals[THAW] =
    g_signal_new ("thaw",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanContainerClass, thaw),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->dispose          = picman_container_dispose;
  object_class->set_property     = picman_container_set_property;
  object_class->get_property     = picman_container_get_property;

  picman_object_class->get_memsize = picman_container_get_memsize;

  klass->add                     = picman_container_real_add;
  klass->remove                  = picman_container_real_remove;
  klass->reorder                 = NULL;
  klass->freeze                  = NULL;
  klass->thaw                    = NULL;

  klass->clear                   = NULL;
  klass->have                    = NULL;
  klass->foreach                 = NULL;
  klass->get_child_by_name       = NULL;
  klass->get_child_by_index      = NULL;
  klass->get_child_index         = NULL;

  g_object_class_install_property (object_class, PROP_CHILDREN_TYPE,
                                   g_param_spec_gtype ("children-type",
                                                       NULL, NULL,
                                                       PICMAN_TYPE_OBJECT,
                                                       PICMAN_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_POLICY,
                                   g_param_spec_enum ("policy",
                                                      NULL, NULL,
                                                      PICMAN_TYPE_CONTAINER_POLICY,
                                                      PICMAN_CONTAINER_POLICY_STRONG,
                                                      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (PicmanContainerPriv));
}

static void
picman_container_config_iface_init (PicmanConfigInterface *iface)
{
  iface->serialize   = picman_container_serialize;
  iface->deserialize = picman_container_deserialize;
}

static void
picman_container_init (PicmanContainer *container)
{
  container->priv = G_TYPE_INSTANCE_GET_PRIVATE (container,
                                                 PICMAN_TYPE_CONTAINER,
                                                 PicmanContainerPriv);
  container->priv->handlers      = NULL;
  container->priv->freeze_count  = 0;

  container->priv->children_type = G_TYPE_NONE;
  container->priv->policy        = PICMAN_CONTAINER_POLICY_STRONG;
  container->priv->n_children    = 0;
}

static void
picman_container_dispose (GObject *object)
{
  PicmanContainer *container = PICMAN_CONTAINER (object);

  picman_container_clear (container);

  while (container->priv->handlers)
    picman_container_remove_handler (container,
                                   ((PicmanContainerHandler *)
                                    container->priv->handlers->data)->quark);

  if (container->priv->children_type != G_TYPE_NONE)
    {
      g_type_class_unref (g_type_class_peek (container->priv->children_type));
      container->priv->children_type = G_TYPE_NONE;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_container_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PicmanContainer *container = PICMAN_CONTAINER (object);

  switch (property_id)
    {
    case PROP_CHILDREN_TYPE:
      container->priv->children_type = g_value_get_gtype (value);
      g_type_class_ref (container->priv->children_type);
      break;
    case PROP_POLICY:
      container->priv->policy = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_container_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PicmanContainer *container = PICMAN_CONTAINER (object);

  switch (property_id)
    {
    case PROP_CHILDREN_TYPE:
      g_value_set_gtype (value, container->priv->children_type);
      break;
    case PROP_POLICY:
      g_value_set_enum (value, container->priv->policy);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_container_get_memsize (PicmanObject *object,
                            gint64     *gui_size)
{
  PicmanContainer *container = PICMAN_CONTAINER (object);
  gint64         memsize   = 0;
  GList         *list;

  for (list = container->priv->handlers; list; list = g_list_next (list))
    {
      PicmanContainerHandler *handler = list->data;

      memsize += (sizeof (GList) +
                  sizeof (PicmanContainerHandler) +
                  picman_string_get_memsize (handler->signame));
    }

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_container_real_add (PicmanContainer *container,
                         PicmanObject    *object)
{
  container->priv->n_children++;
}

static void
picman_container_real_remove (PicmanContainer *container,
                            PicmanObject    *object)
{
  container->priv->n_children--;
}


typedef struct
{
  PicmanConfigWriter *writer;
  gpointer          data;
  gboolean          success;
} SerializeData;

static void
picman_container_serialize_foreach (GObject       *object,
                                  SerializeData *serialize_data)
{
  PicmanConfigInterface *config_iface;
  const gchar         *name;

  config_iface = PICMAN_CONFIG_GET_INTERFACE (object);

  if (! config_iface)
    serialize_data->success = FALSE;

  if (! serialize_data->success)
    return;

  picman_config_writer_open (serialize_data->writer,
                           g_type_name (G_TYPE_FROM_INSTANCE (object)));

  name = picman_object_get_name (object);

  if (name)
    picman_config_writer_string (serialize_data->writer, name);
  else
    picman_config_writer_print (serialize_data->writer, "NULL", 4);

  serialize_data->success = config_iface->serialize (PICMAN_CONFIG (object),
                                                     serialize_data->writer,
                                                     serialize_data->data);
  picman_config_writer_close (serialize_data->writer);
}

static gboolean
picman_container_serialize (PicmanConfig       *config,
                          PicmanConfigWriter *writer,
                          gpointer          data)
{
  PicmanContainer *container = PICMAN_CONTAINER (config);
  SerializeData  serialize_data;

  serialize_data.writer  = writer;
  serialize_data.data    = data;
  serialize_data.success = TRUE;

  picman_container_foreach (container,
                          (GFunc) picman_container_serialize_foreach,
                          &serialize_data);

  return serialize_data.success;
}

static gboolean
picman_container_deserialize (PicmanConfig *config,
                            GScanner   *scanner,
                            gint        nest_level,
                            gpointer    data)
{
  PicmanContainer *container = PICMAN_CONTAINER (config);
  GTokenType     token;

  token = G_TOKEN_LEFT_PAREN;

  while (g_scanner_peek_next_token (scanner) == token)
    {
      token = g_scanner_get_next_token (scanner);

      switch (token)
        {
        case G_TOKEN_LEFT_PAREN:
          token = G_TOKEN_IDENTIFIER;
          break;

        case G_TOKEN_IDENTIFIER:
          {
            PicmanObject *child;
            GType       type;
            gchar      *name      = NULL;
            gboolean    add_child = FALSE;

            type = g_type_from_name (scanner->value.v_identifier);

            if (! type)
              {
                g_scanner_error (scanner,
                                 "unable to determine type of '%s'",
                                 scanner->value.v_identifier);
                return FALSE;
              }

            if (! g_type_is_a (type, container->priv->children_type))
              {
                g_scanner_error (scanner,
                                 "'%s' is not a subclass of '%s'",
                                 scanner->value.v_identifier,
                                 g_type_name (container->priv->children_type));
                return FALSE;
              }

            if (! g_type_is_a (type, PICMAN_TYPE_CONFIG))
              {
                g_scanner_error (scanner,
                                 "'%s' does not implement PicmanConfigInterface",
                                 scanner->value.v_identifier);
                return FALSE;
              }

            if (! picman_scanner_parse_string (scanner, &name))
              {
                token = G_TOKEN_STRING;
                break;
              }

            if (! name)
              name = g_strdup ("");

            child = picman_container_get_child_by_name (container, name);

            if (! child)
              {
                if (PICMAN_IS_PICMAN (data))
                  child = g_object_new (type, "picman", data, NULL);
                else
                  child = g_object_new (type, NULL);

                add_child = TRUE;
              }

            /*  always use the deserialized name. while it normally
             *  doesn't make a difference there are obscure case like
             *  template migration.
             */
            picman_object_take_name (child, name);

            if (! PICMAN_CONFIG_GET_INTERFACE (child)->deserialize (PICMAN_CONFIG (child),
                                                                  scanner,
                                                                  nest_level + 1,
                                                                  NULL))
              {
                if (add_child)
                  g_object_unref (child);

                /*  warning should be already set by child  */
                return FALSE;
              }

            if (add_child)
              {
                picman_container_add (container, child);

                if (container->priv->policy == PICMAN_CONTAINER_POLICY_STRONG)
                  g_object_unref (child);
              }
          }
          token = G_TOKEN_RIGHT_PAREN;
          break;

        case G_TOKEN_RIGHT_PAREN:
          token = G_TOKEN_LEFT_PAREN;
          break;

        default: /* do nothing */
          break;
        }
    }

  return picman_config_deserialize_return (scanner, token, nest_level);
}

static void
picman_container_disconnect_callback (PicmanObject *object,
                                    gpointer    data)
{
  PicmanContainer *container = PICMAN_CONTAINER (data);

  picman_container_remove (container, object);
}

GType
picman_container_get_children_type (const PicmanContainer *container)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), G_TYPE_NONE);

  return container->priv->children_type;
}

PicmanContainerPolicy
picman_container_get_policy (const PicmanContainer *container)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), 0);

  return container->priv->policy;
}

gint
picman_container_get_n_children (const PicmanContainer *container)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), 0);

  return container->priv->n_children;
}

gboolean
picman_container_add (PicmanContainer *container,
                    PicmanObject    *object)
{
  GList *list;
  gint   n_children;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), FALSE);
  g_return_val_if_fail (object != NULL, FALSE);
  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (object,
                                                    container->priv->children_type),
                        FALSE);

  if (picman_container_have (container, object))
    {
      g_warning ("%s: container %p already contains object %p",
                 G_STRFUNC, container, object);
      return FALSE;
    }

  for (list = container->priv->handlers; list; list = g_list_next (list))
    {
      PicmanContainerHandler *handler = list->data;
      gulong                handler_id;

      handler_id = g_signal_connect (object,
                                     handler->signame,
                                     handler->callback,
                                     handler->callback_data);

      g_object_set_qdata (G_OBJECT (object), handler->quark,
                          GUINT_TO_POINTER (handler_id));
    }

  switch (container->priv->policy)
    {
    case PICMAN_CONTAINER_POLICY_STRONG:
      g_object_ref (object);
      break;

    case PICMAN_CONTAINER_POLICY_WEAK:
      g_signal_connect (object, "disconnect",
                        G_CALLBACK (picman_container_disconnect_callback),
                        container);
      break;
    }

  n_children = container->priv->n_children;

  g_signal_emit (container, container_signals[ADD], 0, object);

  if (n_children == container->priv->n_children)
    {
      g_warning ("%s: PicmanContainer::add() implementation did not "
                 "chain up. Please report this at http://www.picman.org/bugs/",
                 G_STRFUNC);

      container->priv->n_children++;
    }

  return TRUE;
}

gboolean
picman_container_remove (PicmanContainer *container,
                       PicmanObject    *object)
{
  GList *list;
  gint   n_children;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), FALSE);
  g_return_val_if_fail (object != NULL, FALSE);
  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (object,
                                                    container->priv->children_type),
                        FALSE);

  if (! picman_container_have (container, object))
    {
      g_warning ("%s: container %p does not contain object %p",
                 G_STRFUNC, container, object);
      return FALSE;
    }

  for (list = container->priv->handlers; list; list = g_list_next (list))
    {
      PicmanContainerHandler *handler = list->data;
      gulong                handler_id;

      handler_id = GPOINTER_TO_UINT (g_object_get_qdata (G_OBJECT (object),
                                                         handler->quark));

      if (handler_id)
        {
          g_signal_handler_disconnect (object, handler_id);

          g_object_set_qdata (G_OBJECT (object), handler->quark, NULL);
        }
    }

  n_children = container->priv->n_children;

  g_signal_emit (container, container_signals[REMOVE], 0, object);

  if (n_children == container->priv->n_children)
    {
      g_warning ("%s: PicmanContainer::remove() implementation did not "
                 "chain up. Please report this at http://www.picman.org/bugs/",
                 G_STRFUNC);

      container->priv->n_children--;
    }

  switch (container->priv->policy)
    {
    case PICMAN_CONTAINER_POLICY_STRONG:
      g_object_unref (object);
      break;

    case PICMAN_CONTAINER_POLICY_WEAK:
      g_signal_handlers_disconnect_by_func (object,
                                            picman_container_disconnect_callback,
                                            container);
      break;
    }

  return TRUE;
}

gboolean
picman_container_insert (PicmanContainer *container,
                       PicmanObject    *object,
                       gint           index)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), FALSE);
  g_return_val_if_fail (object != NULL, FALSE);
  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (object,
                                                    container->priv->children_type),
                        FALSE);

  g_return_val_if_fail (index >= -1 &&
                        index <= container->priv->n_children, FALSE);

  if (picman_container_have (container, object))
    {
      g_warning ("%s: container %p already contains object %p",
                 G_STRFUNC, container, object);
      return FALSE;
    }

  if (picman_container_add (container, object))
    {
      return picman_container_reorder (container, object, index);
    }

  return FALSE;
}

gboolean
picman_container_reorder (PicmanContainer *container,
                        PicmanObject    *object,
                        gint           new_index)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), FALSE);
  g_return_val_if_fail (object != NULL, FALSE);
  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (object,
                                                    container->priv->children_type),
                        FALSE);

  g_return_val_if_fail (new_index >= -1 &&
                        new_index < container->priv->n_children, FALSE);

  if (! picman_container_have (container, object))
    {
      g_warning ("%s: container %p does not contain object %p",
                 G_STRFUNC, container, object);
      return FALSE;
    }

  if (container->priv->n_children == 1)
    return TRUE;

  g_signal_emit (container, container_signals[REORDER], 0,
                 object, new_index);

  return TRUE;
}

void
picman_container_freeze (PicmanContainer *container)
{
  g_return_if_fail (PICMAN_IS_CONTAINER (container));

  container->priv->freeze_count++;

  if (container->priv->freeze_count == 1)
    g_signal_emit (container, container_signals[FREEZE], 0);
}

void
picman_container_thaw (PicmanContainer *container)
{
  g_return_if_fail (PICMAN_IS_CONTAINER (container));

  if (container->priv->freeze_count > 0)
    container->priv->freeze_count--;

  if (container->priv->freeze_count == 0)
    g_signal_emit (container, container_signals[THAW], 0);
}

gboolean
picman_container_frozen (PicmanContainer *container)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), FALSE);

  return (container->priv->freeze_count > 0) ? TRUE : FALSE;
}

void
picman_container_clear (PicmanContainer *container)
{
  g_return_if_fail (PICMAN_IS_CONTAINER (container));

  if (container->priv->n_children > 0)
    {
      picman_container_freeze (container);
      PICMAN_CONTAINER_GET_CLASS (container)->clear (container);
      picman_container_thaw (container);
    }
}

gboolean
picman_container_is_empty (const PicmanContainer *container)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), FALSE);

  return (container->priv->n_children == 0);
}

gboolean
picman_container_have (const PicmanContainer *container,
                     PicmanObject          *object)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), FALSE);

  if (container->priv->n_children < 1)
    return FALSE;

  return PICMAN_CONTAINER_GET_CLASS (container)->have (container, object);
}

void
picman_container_foreach (const PicmanContainer *container,
                        GFunc                func,
                        gpointer             user_data)
{
  g_return_if_fail (PICMAN_IS_CONTAINER (container));
  g_return_if_fail (func != NULL);

  if (container->priv->n_children > 0)
    PICMAN_CONTAINER_GET_CLASS (container)->foreach (container, func, user_data);
}

PicmanObject *
picman_container_get_child_by_name (const PicmanContainer *container,
                                  const gchar         *name)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);

  if (!name)
    return NULL;

  return PICMAN_CONTAINER_GET_CLASS (container)->get_child_by_name (container,
                                                                  name);
}

PicmanObject *
picman_container_get_child_by_index (const PicmanContainer *container,
                                   gint                 index)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);

  if (index < 0 || index >= container->priv->n_children)
    return NULL;

  return PICMAN_CONTAINER_GET_CLASS (container)->get_child_by_index (container,
                                                                   index);
}

/**
 * picman_container_get_first_child:
 * @container: a #PicmanContainer
 *
 * Return value: the first child object stored in @container or %NULL if the
 *               container is empty
 */
PicmanObject *
picman_container_get_first_child (const PicmanContainer *container)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);

  if (container->priv->n_children > 0)
    return PICMAN_CONTAINER_GET_CLASS (container)->get_child_by_index (container,
                                                                     0);

  return NULL;
}

/**
 * picman_container_get_last_child:
 * @container: a #PicmanContainer
 *
 * Return value: the last child object stored in @container or %NULL if the
 *               container is empty
 */
PicmanObject *
picman_container_get_last_child (const PicmanContainer *container)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);

  if (container->priv->n_children > 0)
    return PICMAN_CONTAINER_GET_CLASS (container)->get_child_by_index (container,
                                                                     container->priv->n_children - 1);

  return NULL;
}

gint
picman_container_get_child_index (const PicmanContainer *container,
                                const PicmanObject    *object)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), -1);
  g_return_val_if_fail (object != NULL, -1);
  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (object,
                                                    container->priv->children_type),
                        -1);

  return PICMAN_CONTAINER_GET_CLASS (container)->get_child_index (container,
                                                                object);
}

PicmanObject *
picman_container_get_neighbor_of (const PicmanContainer *container,
                                const PicmanObject    *object)
{
  gint index;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (PICMAN_IS_OBJECT (object), NULL);

  index = picman_container_get_child_index (container, object);

  if (index != -1)
    {
      PicmanObject *new;

      new = picman_container_get_child_by_index (container, index + 1);

      if (! new && index > 0)
        new = picman_container_get_child_by_index (container, index - 1);

      return new;
    }

  return NULL;
}

static void
picman_container_get_name_array_foreach_func (PicmanObject   *object,
                                            gchar      ***iter)
{
  gchar **array = *iter;

  *array = g_strdup (picman_object_get_name (object));
  (*iter)++;
}

gchar **
picman_container_get_name_array (const PicmanContainer *container,
                               gint                *length)
{
  gchar **names;
  gchar **iter;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (length != NULL, NULL);

  *length = picman_container_get_n_children (container);
  if (*length == 0)
    return NULL;

  names = iter = g_new (gchar *, *length);

  picman_container_foreach (container,
                          (GFunc) picman_container_get_name_array_foreach_func,
                          &iter);

  return names;
}

static void
picman_container_add_handler_foreach_func (PicmanObject           *object,
                                         PicmanContainerHandler *handler)
{
  gulong handler_id;

  handler_id = g_signal_connect (object,
                                 handler->signame,
                                 handler->callback,
                                 handler->callback_data);

  g_object_set_qdata (G_OBJECT (object), handler->quark,
                      GUINT_TO_POINTER (handler_id));
}

GQuark
picman_container_add_handler (PicmanContainer *container,
                            const gchar   *signame,
                            GCallback      callback,
                            gpointer       callback_data)
{
  PicmanContainerHandler *handler;
  gchar                *key;

  static gint           handler_id = 0;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), 0);
  g_return_val_if_fail (signame != NULL, 0);
  g_return_val_if_fail (callback != NULL, 0);

  if (! g_str_has_prefix (signame, "notify::"))
    g_return_val_if_fail (g_signal_lookup (signame,
                                           container->priv->children_type), 0);

  handler = g_slice_new0 (PicmanContainerHandler);

  /*  create a unique key for this handler  */
  key = g_strdup_printf ("%s-%d", signame, handler_id++);

  handler->signame       = g_strdup (signame);
  handler->callback      = callback;
  handler->callback_data = callback_data;
  handler->quark         = g_quark_from_string (key);

  D (g_print ("%s: key = %s, id = %d\n", G_STRFUNC, key, handler->quark));

  g_free (key);

  container->priv->handlers = g_list_prepend (container->priv->handlers, handler);

  picman_container_foreach (container,
                          (GFunc) picman_container_add_handler_foreach_func,
                          handler);

  return handler->quark;
}

static void
picman_container_remove_handler_foreach_func (PicmanObject           *object,
                                            PicmanContainerHandler *handler)
{
  gulong handler_id;

  handler_id = GPOINTER_TO_UINT (g_object_get_qdata (G_OBJECT (object),
                                                     handler->quark));

  if (handler_id)
    {
      g_signal_handler_disconnect (object, handler_id);

      g_object_set_qdata (G_OBJECT (object), handler->quark, NULL);
    }
}

void
picman_container_remove_handler (PicmanContainer *container,
                               GQuark         id)
{
  PicmanContainerHandler *handler;
  GList                *list;

  g_return_if_fail (PICMAN_IS_CONTAINER (container));
  g_return_if_fail (id != 0);

  for (list = container->priv->handlers; list; list = g_list_next (list))
    {
      handler = (PicmanContainerHandler *) list->data;

      if (handler->quark == id)
        break;
    }

  if (! list)
    {
      g_warning ("%s: tried to remove handler which unknown id %d",
                 G_STRFUNC, id);
      return;
    }

  D (g_print ("%s: id = %d\n", G_STRFUNC, handler->quark));

  picman_container_foreach (container,
                          (GFunc) picman_container_remove_handler_foreach_func,
                          handler);

  container->priv->handlers = g_list_remove (container->priv->handlers, handler);

  g_free (handler->signame);
  g_slice_free (PicmanContainerHandler, handler);
}
