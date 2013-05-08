/* xmp-model.c - treeview model for XMP metadata
 *
 * Copyright (C) 2004-2005, Raphaël Quinet <raphael@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include <libpicman/picman.h>

#include "libpicman/stdplugins-intl.h"

#include "xmp-schemas.h"
#include "xmp-parse.h"
#include "xmp-model.h"

/* Used for converting row-changed events into property-changed and
 * schema-changed events.*/
#define XMP_MODEL_SCHEMA    0
#define XMP_MODEL_PROPERTY  1

/* local function declarations */
static void         tree_model_row_changed  (GtkTreeModel    *model,
                                             GtkTreePath     *path,
                                             GtkTreeIter     *iter,
                                             gpointer         user_data);

static XMPSchema *  find_xmp_schema_by_iter (XMPModel       *xmp_model,
                                             GtkTreeIter    *iter);

enum
{
  PROPERTY_CHANGED,
  SCHEMA_CHANGED,
  LAST_SIGNAL
};


static void xmp_model_finalize (GObject *object);


G_DEFINE_TYPE (XMPModel, xmp_model, GTK_TYPE_TREE_STORE);

static guint xmp_model_signals[LAST_SIGNAL] = { 0 };


static void
xmp_model_class_init (XMPModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  xmp_model_signals[PROPERTY_CHANGED] =
    g_signal_new ("property-changed",
                  PICMAN_TYPE_XMP_MODEL,
                  G_SIGNAL_DETAILED,
                  G_STRUCT_OFFSET (XMPModelClass, property_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOXED,
                  G_TYPE_NONE, 1,
                  GTK_TYPE_TREE_ITER);

  object_class->finalize  = xmp_model_finalize;

  klass->property_changed = NULL;
}

static void
xmp_model_init (XMPModel *xmp_model)
{
  GType types[XMP_MODEL_NUM_COLUMNS];

  types[COL_XMP_NAME]           = G_TYPE_STRING;
  types[COL_XMP_VALUE]          = G_TYPE_STRING;
  types[COL_XMP_VALUE_RAW]      = G_TYPE_POINTER;
  types[COL_XMP_TYPE_XREF]      = G_TYPE_POINTER;
  types[COL_XMP_WIDGET_XREF]    = G_TYPE_POINTER;
  types[COL_XMP_EDITABLE]       = G_TYPE_INT;
  types[COL_XMP_EDIT_ICON]      = GDK_TYPE_PIXBUF;
  types[COL_XMP_VISIBLE]        = G_TYPE_BOOLEAN;
  types[COL_XMP_WEIGHT]         = G_TYPE_INT;
  types[COL_XMP_WEIGHT_SET]     = G_TYPE_BOOLEAN;

  gtk_tree_store_set_column_types (GTK_TREE_STORE (xmp_model),
                                   XMP_MODEL_NUM_COLUMNS, types);

  xmp_model->custom_schemas     = NULL;
  xmp_model->custom_properties  = NULL;
  xmp_model->cached_schema      = NULL;

  g_signal_connect (GTK_TREE_MODEL (xmp_model), "row-changed",
                    G_CALLBACK (tree_model_row_changed),
                    NULL);
}

static void
xmp_model_finalize (GObject *object)
{
  XMPModel      *xmp_model = XMP_MODEL (object);
  GtkTreeModel  *model     = xmp_model_get_tree_model (xmp_model);
  GtkTreeIter    iter;
  GtkTreeIter    child;
  gchar        **value_array;
  gint           i;

  /* we used XMP_FLAG_DEFER_VALUE_FREE for the parser, so now we must free
     all value arrays */

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
      do
        {
          if (gtk_tree_model_iter_children (model, &child, &iter))
            {
              gchar **last_value_array = NULL;

              do
                {
                  gtk_tree_model_get (model, &child,
                                      COL_XMP_VALUE_RAW, &value_array,
                                      -1);
                  if (value_array != last_value_array)
                    {
                      /* FIXME: this does not free everything */
                      for (i = 0; value_array[i] != NULL; i++)
                        g_free (value_array[i]);
                      g_free (value_array);
                    }

                  last_value_array = value_array;
                }
              while (gtk_tree_model_iter_next (model, &child));
            }
        }
      while (gtk_tree_model_iter_next (model, &iter));
    }

  G_OBJECT_CLASS (xmp_model_parent_class)->finalize (object);
}


/**
 * xmp_model_new:
 *
 * Return value: a new #XMPModel.
 **/
XMPModel *
xmp_model_new (void)
{
  return g_object_new (PICMAN_TYPE_XMP_MODEL, NULL);
}

/**
 * xmp_model_is_empty:
 * @xmp_model: an #XMPModel
 *
 * Return value: %TRUE if @xmp_model is empty (no shemas, no properties)
 **/
gboolean
xmp_model_is_empty (XMPModel *xmp_model)
{
  GtkTreeIter iter;

  g_return_val_if_fail (xmp_model != NULL, TRUE);
  if ((xmp_model->custom_schemas != NULL)
      || (xmp_model->custom_properties != NULL))
    return FALSE;
  return !gtk_tree_model_get_iter_first (GTK_TREE_MODEL (xmp_model),
                                         &iter);
}

/* translate a row-changed event into a property-changed or
 * schema-changed event with the detail.
 */
static void
tree_model_row_changed (GtkTreeModel    *model,
                        GtkTreePath     *path,
                        GtkTreeIter     *iter,
                        gpointer         user_data)
{
  gint       depth;
  XMPSchema *schema;

  /* 1. check which iter depth the change was: 0 for schema 1 for
   * property? */
  depth = gtk_tree_store_iter_depth (GTK_TREE_STORE (model), iter);
  if (depth == XMP_MODEL_SCHEMA)
  {
    /* 2. If a schema has changed, emit a schema changed signal */
  }

  if (depth == XMP_MODEL_PROPERTY)
  {
    schema = find_xmp_schema_by_iter (XMP_MODEL (model), iter);
    xmp_model_property_changed (XMP_MODEL (model), schema, iter);
  }
}

static XMPSchema *
find_xmp_schema_by_iter (XMPModel       *xmp_model,
                         GtkTreeIter    *child)
{
  GtkTreeIter  parent;
  XMPSchema   *schema;

  if (! gtk_tree_model_iter_parent (GTK_TREE_MODEL (xmp_model), &parent, child))
    return NULL;

  gtk_tree_model_get (GTK_TREE_MODEL (xmp_model), &parent,
                      COL_XMP_TYPE_XREF, &schema,
                      -1);
  return schema;
}

/* check if the given schema_uri matches a known schema; else return NULL */
static XMPSchema *
find_xmp_schema_by_uri (XMPModel    *xmp_model,
                        const gchar *schema_uri)
{
  int          i;
  GSList      *list;
  const gchar *c;

  /* check if we know about this schema (exact match for URI) */
  for (i = 0; xmp_schemas[i].uri != NULL; ++i)
    {
      if (! strcmp (xmp_schemas[i].uri, schema_uri))
        {
#ifdef DEBUG_XMP_MODEL
          if (xmp_schemas[i].name != NULL)
            g_print ("%s \t[%s]\n", xmp_schemas[i].name, xmp_schemas[i].uri);
          else
            g_print ("(no name) \t[%s]\n", xmp_schemas[i].uri);
#endif
          return &(xmp_schemas[i]);
        }
    }

  /* this is not a standard shema; now check the custom schemas */
  for (list = xmp_model->custom_schemas; list != NULL; list = list->next)
    {
      if (! strcmp (((XMPSchema *)(list->data))->uri, schema_uri))
        {
#ifdef DEBUG_XMP_MODEL
          g_print ("CUSTOM %s \t[%s]\n",
                  ((XMPSchema *)(list->data))->name,
                  ((XMPSchema *)(list->data))->uri);
#endif
          return (XMPSchema *)(list->data);
        }
    }

  /* now check for some common errors and results of bad encoding: */
  /* - check for "http:" without "//", or missing "http://" */
  for (i = 0; xmp_schemas[i].uri != NULL; ++i)
    {
      if (g_str_has_prefix (xmp_schemas[i].uri, "http://")
          && ((! strcmp (xmp_schemas[i].uri + 7, schema_uri))
              || (g_str_has_prefix (schema_uri, "http:")
                  && ! strcmp (xmp_schemas[i].uri + 7, schema_uri + 5))
              ))
        {
#ifdef DEBUG_XMP_MODEL
          g_print ("%s \t~~~[%s]\n", xmp_schemas[i].name, xmp_schemas[i].uri);
#endif
          return &(xmp_schemas[i]);
        }
    }
  /* - check for errors such as "name (uri)" or "name (prefix, uri)"  */
  for (c = schema_uri; *c; c++)
    if ((*c == '(') || (*c == ' ') || (*c == ','))
      {
        gint len;

        c++;
        while (*c == ' ')
          c++;

        if (! *c)
          break;

        for (len = 1; c[len]; len++)
          if ((c[len] == ')') || (c[len] == ' '))
            break;

        for (i = 0; xmp_schemas[i].uri != NULL; ++i)
          {
            if (! strncmp (xmp_schemas[i].uri, c, len))
              {
#ifdef DEBUG_XMP_MODEL
                g_print ("%s \t~~~[%s]\n", xmp_schemas[i].name,
                         xmp_schemas[i].uri);
#endif
                return &(xmp_schemas[i]);
              }
          }
      }

#ifdef DEBUG_XMP_MODEL
  g_print ("Unknown schema URI %s\n", schema_uri);
#endif

  return NULL;
}

/* check if the given prefix matches a known schema; else return NULL */
static XMPSchema *
find_xmp_schema_prefix (XMPModel    *xmp_model,
                        const gchar *prefix)
{
  int     i;
  GSList *list;

  for (i = 0; xmp_schemas[i].uri != NULL; ++i)
    if (! strcmp (xmp_schemas[i].prefix, prefix))
      return &(xmp_schemas[i]);

  for (list = xmp_model->custom_schemas; list != NULL; list = list->next)
    if (! strcmp (((XMPSchema *)(list->data))->prefix, prefix))
      return (XMPSchema *)(list->data);

  return NULL;
}

/* make the next lookup a bit faster if the tree is not modified */
static void
cache_iter_for_schema (XMPModel    *xmp_model,
                      XMPSchema   *schema,
                      GtkTreeIter *iter)
{
  xmp_model->cached_schema = schema;
  if (iter != NULL)
    memcpy (&(xmp_model->cached_schema_iter), iter, sizeof (GtkTreeIter));
}

/* find the GtkTreeIter for the given schema and return TRUE if the schema was
   found in the tree; else return FALSE */
static gboolean
find_iter_for_schema (XMPModel    *xmp_model,
                      XMPSchema   *schema,
                      GtkTreeIter *iter)
{
  XMPSchema *schema_xref;

  /* common case: return the cached iter */
  if (schema == xmp_model->cached_schema)
    {
      memcpy (iter, &(xmp_model->cached_schema_iter), sizeof (GtkTreeIter));
      return TRUE;
    }
  /* find where this schema has been stored in the tree */
  if (! gtk_tree_model_get_iter_first (GTK_TREE_MODEL (xmp_model),
                                       iter))
    return FALSE;

  do
    {
      gtk_tree_model_get (GTK_TREE_MODEL (xmp_model), iter,
                          COL_XMP_TYPE_XREF, &schema_xref,
                          -1);
      if (schema_xref == schema)
        {
          cache_iter_for_schema (xmp_model, schema, iter);
          return TRUE;
        }
    }
  while (gtk_tree_model_iter_next (GTK_TREE_MODEL (xmp_model), iter));

  return FALSE;
}

/* remove a property from the list of children of schema_iter */
static void
find_and_remove_property (XMPModel    *xmp_model,
                          XMPProperty *property,
                          GtkTreeIter *schema_iter)
{
  GtkTreeIter  child_iter;
  XMPProperty *property_xref;

  if (! gtk_tree_model_iter_children (GTK_TREE_MODEL (xmp_model),
                                      &child_iter, schema_iter))
    return;
  for (;;)
    {
      gtk_tree_model_get (GTK_TREE_MODEL (xmp_model), &child_iter,
                          COL_XMP_TYPE_XREF, &property_xref,
                          -1);
      if (property_xref == property)
        {
          if (! gtk_tree_store_remove (GTK_TREE_STORE (xmp_model),
                                       &child_iter))
            break;
        }
      else
        {
          if (! gtk_tree_model_iter_next (GTK_TREE_MODEL(xmp_model),
                                          &child_iter))
            break;
        }
    }
}

/* add a schema to the tree */
static void
add_known_schema (XMPModel    *xmp_model,
                  XMPSchema   *schema,
                  GtkTreeIter *iter)
{
  gtk_tree_store_append (GTK_TREE_STORE (xmp_model), iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (xmp_model), iter,
                      COL_XMP_NAME, schema->name,
                      COL_XMP_VALUE, schema->uri,
                      COL_XMP_VALUE_RAW, NULL,
                      COL_XMP_TYPE_XREF, schema,
                      COL_XMP_WIDGET_XREF, NULL,
                      COL_XMP_EDITABLE, FALSE,
                      COL_XMP_EDIT_ICON, NULL,
                      COL_XMP_VISIBLE, FALSE,
                      COL_XMP_WEIGHT, PANGO_WEIGHT_BOLD,
                      COL_XMP_WEIGHT_SET, TRUE,
                      -1);
  cache_iter_for_schema (xmp_model, schema, iter);
}

/* called by the XMP parser - new schema */
static gpointer
parse_start_schema (XMPParseContext     *context,
                    const gchar         *ns_uri,
                    const gchar         *ns_prefix,
                    gpointer             user_data,
                    GError             **error)
{
  XMPModel    *xmp_model = user_data;
  GtkTreeIter  iter;
  XMPSchema   *schema;

  g_return_val_if_fail (xmp_model != NULL, NULL);
  schema = find_xmp_schema_by_uri (xmp_model, ns_uri);
  if (schema == NULL)
    {
      /* add schema to custom_schemas */
      schema = g_new (XMPSchema, 1);
      schema->uri = g_strdup (ns_uri);
      schema->prefix = g_strdup (ns_prefix);
      schema->name = schema->uri;
      schema->properties = NULL;
      xmp_model->custom_schemas = g_slist_prepend (xmp_model->custom_schemas,
                                                   schema);
    }
  else if (find_iter_for_schema (xmp_model, schema, &iter))
    {
      /* already in the tree, so no need to add it again */
      return schema;
    }

  /* schemas with NULL names are special and should not go in the tree */
  if (schema->name == NULL)
    {
      cache_iter_for_schema (xmp_model, NULL, NULL);
      return schema;
    }

  /* if the schema is not in the tree yet, add it now */
  add_known_schema (xmp_model, schema, &iter);

  return schema;
}

/* called by the XMP parser - end of schema */
static void
parse_end_schema (XMPParseContext  *context,
                  gpointer          ns_user_data,
                  gpointer          user_data,
                  GError          **error)
{
  XMPModel  *xmp_model = user_data;
  XMPSchema *schema = ns_user_data;

  g_return_if_fail (xmp_model != NULL);
  g_return_if_fail (schema != NULL);

  xmp_model->cached_schema = NULL;

#ifdef DEBUG_XMP_MODEL
  if (schema->name)
    g_print ("End of %s\n", schema->name);
#endif
}

/* called by the XMP parser - new property */
static void
parse_set_property (XMPParseContext     *context,
                    const gchar         *name,
                    XMPParseType         type,
                    const gchar        **value,
                    gpointer             ns_user_data,
                    gpointer             user_data,
                    GError             **error)
{
  XMPModel    *xmp_model = user_data;
  XMPSchema   *schema = ns_user_data;
  int          i;
  XMPProperty *property;
  GtkTreeIter  iter;
  GtkTreeIter  child_iter;
  gchar       *tmp_name;
  gchar       *tmp_value;

  g_return_if_fail (xmp_model != NULL);
  g_return_if_fail (schema != NULL);

  if (! find_iter_for_schema (xmp_model, schema, &iter))
    {
      g_printerr ("Unable to set XMP property '%s' because its schema is bad",
                  name);
      return;
    }

  property = NULL;

  if (schema->properties != NULL)
    for (i = 0; schema->properties[i].name != NULL; ++i)
      if (! strcmp (schema->properties[i].name, name))
        {
          property = &(schema->properties[i]);
          break;
        }

  /* if the same property was already present, remove it (replace it) */
  if (property != NULL)
    find_and_remove_property (xmp_model, property, &iter);

  switch (type)
    {
    case XMP_PTYPE_TEXT:
#ifdef DEBUG_XMP_MODEL
      g_print ("\t%s:%s = \"%s\"\n", schema->prefix, name, value[0]);
#endif
      if (property != NULL)
        /* FIXME */;
      else
        {
          property = g_new (XMPProperty, 1);
          property->name = g_strdup (name);
          property->type = XMP_TYPE_TEXT;
          property->editable = TRUE;
          xmp_model->custom_properties =
            g_slist_prepend (xmp_model->custom_properties, property);
        }
      gtk_tree_store_append (GTK_TREE_STORE (xmp_model), &child_iter, &iter);
      gtk_tree_store_set (GTK_TREE_STORE (xmp_model), &child_iter,
                          COL_XMP_NAME, name,
                          COL_XMP_VALUE, value[0],
                          COL_XMP_VALUE_RAW, value,
                          COL_XMP_TYPE_XREF, property,
                          COL_XMP_WIDGET_XREF, NULL,
                          COL_XMP_EDITABLE, property->editable,
                          COL_XMP_EDIT_ICON, NULL,
                          COL_XMP_VISIBLE, TRUE,
                          COL_XMP_WEIGHT, PANGO_WEIGHT_NORMAL,
                          COL_XMP_WEIGHT_SET, FALSE,
                          -1);
      break;

    case XMP_PTYPE_RESOURCE:
#ifdef DEBUG_XMP_MODEL
      g_print ("\t%s:%s @ = \"%s\"\n", ns_prefix, name,
              value[0]);
#endif
      if (property != NULL)
        /* FIXME */;
      else
        {
          property = g_new (XMPProperty, 1);
          property->name = g_strdup (name);
          property->type = XMP_TYPE_URI;
          property->editable = TRUE;
          xmp_model->custom_properties =
            g_slist_prepend (xmp_model->custom_properties, property);
        }
      tmp_name = g_strconcat (name, " @", NULL);
      gtk_tree_store_append (GTK_TREE_STORE (xmp_model), &child_iter, &iter);
      gtk_tree_store_set (GTK_TREE_STORE (xmp_model), &child_iter,
                          COL_XMP_NAME, tmp_name,
                          COL_XMP_VALUE, value[0],
                          COL_XMP_VALUE_RAW, value,
                          COL_XMP_TYPE_XREF, property,
                          COL_XMP_WIDGET_XREF, NULL,
                          COL_XMP_EDITABLE, property->editable,
                          COL_XMP_EDIT_ICON, NULL,
                          COL_XMP_VISIBLE, TRUE,
                          COL_XMP_WEIGHT, PANGO_WEIGHT_NORMAL,
                          COL_XMP_WEIGHT_SET, FALSE,
                          -1);
      g_free (tmp_name);
      break;

    case XMP_PTYPE_ORDERED_LIST:
    case XMP_PTYPE_UNORDERED_LIST:
#ifdef DEBUG_XMP_MODEL
      g_print ("\t%s:%s [] =", ns_prefix, name);
      for (i = 0; value[i] != NULL; i++)
        if (i == 0)
          g_print (" \"%s\"", value[i]);
        else
          g_print (", \"%s\"", value[i]);
      g_print ("\n");
#endif
      if (property != NULL)
        /* FIXME */;
      else
        {
          property = g_new (XMPProperty, 1);
          property->name = g_strdup (name);
          property->type = ((type == XMP_PTYPE_ORDERED_LIST)
                            ? XMP_TYPE_TEXT_BAG
                            : XMP_TYPE_TEXT_SEQ);
          property->editable = TRUE;
          xmp_model->custom_properties =
            g_slist_prepend (xmp_model->custom_properties, property);
        }

      tmp_name = g_strconcat (name, " []", NULL);
      tmp_value = g_strjoinv ("; ", (gchar **) value);
      gtk_tree_store_append (GTK_TREE_STORE (xmp_model), &child_iter, &iter);
      gtk_tree_store_set (GTK_TREE_STORE (xmp_model), &child_iter,
                          COL_XMP_NAME, tmp_name,
                          COL_XMP_VALUE, tmp_value,
                          COL_XMP_VALUE_RAW, value,
                          COL_XMP_TYPE_XREF, property,
                          COL_XMP_WIDGET_XREF, NULL,
                          COL_XMP_EDITABLE, property->editable,
                          COL_XMP_EDIT_ICON, NULL,
                          COL_XMP_VISIBLE, TRUE,
                          COL_XMP_WEIGHT, PANGO_WEIGHT_NORMAL,
                          COL_XMP_WEIGHT_SET, FALSE,
                          -1);
      g_free (tmp_value);
      g_free (tmp_name);
      break;

    case XMP_PTYPE_ALT_THUMBS:
#ifdef DEBUG_XMP_MODEL
      for (i = 0; value[i] != NULL; i += 2)
        g_print ("\t%s:%s [size:%d] = \"...\"\n", ns_prefix, name,
                *(int *)(value[i]));
      g_print ("\n");
#endif
      if (property != NULL)
        /* FIXME */;
      else
        {
          property = g_new (XMPProperty, 1);
          property->name = g_strdup (name);
          property->type = XMP_TYPE_THUMBNAIL_ALT;
          property->editable = TRUE;
          xmp_model->custom_properties =
            g_slist_prepend (xmp_model->custom_properties, property);
        }

      tmp_name = g_strconcat (name, " []", NULL);
      gtk_tree_store_append (GTK_TREE_STORE (xmp_model), &child_iter, &iter);
      gtk_tree_store_set (GTK_TREE_STORE (xmp_model), &child_iter,
                          COL_XMP_NAME, tmp_name,
                          COL_XMP_VALUE, "[FIXME: display thumbnails]",
                          COL_XMP_VALUE_RAW, value,
                          COL_XMP_TYPE_XREF, property,
                          COL_XMP_WIDGET_XREF, NULL,
                          COL_XMP_EDITABLE, property->editable,
                          COL_XMP_EDIT_ICON, NULL,
                          COL_XMP_VISIBLE, TRUE,
                          COL_XMP_WEIGHT, PANGO_WEIGHT_NORMAL,
                          COL_XMP_WEIGHT_SET, FALSE,
                          -1);
      g_free (tmp_name);
      break;

    case XMP_PTYPE_ALT_LANG:
#ifdef DEBUG_XMP_MODEL
      for (i = 0; value[i] != NULL; i += 2)
        g_print ("\t%s:%s [lang:%s] = \"%s\"\n", ns_prefix, name,
                value[i], value[i + 1]);
#endif
      if (property != NULL)
        /* FIXME */;
      else
        {
          property = g_new (XMPProperty, 1);
          property->name = g_strdup (name);
          property->type = XMP_TYPE_LANG_ALT;
          property->editable = TRUE;
          xmp_model->custom_properties =
            g_slist_prepend (xmp_model->custom_properties, property);
        }
      for (i = 0; value[i] != NULL; i += 2)
        {
          tmp_name = g_strconcat (name, " [", value[i], "]", NULL);
          gtk_tree_store_append (GTK_TREE_STORE (xmp_model), &child_iter, &iter);
          gtk_tree_store_set (GTK_TREE_STORE (xmp_model), &child_iter,
                              COL_XMP_NAME, tmp_name,
                              COL_XMP_VALUE, value[i + 1],
                              COL_XMP_VALUE_RAW, value,
                              COL_XMP_TYPE_XREF, property,
                              COL_XMP_WIDGET_XREF, NULL,
                              COL_XMP_EDITABLE, property->editable,
                              COL_XMP_EDIT_ICON, NULL,
                              COL_XMP_VISIBLE, TRUE,
                              COL_XMP_WEIGHT, PANGO_WEIGHT_NORMAL,
                              COL_XMP_WEIGHT_SET, FALSE,
                              -1);
          g_free (tmp_name);
        }
      break;

    case XMP_PTYPE_STRUCTURE:
#ifdef DEBUG_XMP_MODEL
      for (i = 2; value[i] != NULL; i += 2)
        g_print ("\t%s:%s [%s] = \"%s\"\n", ns_prefix, name,
                value[i], value[i + 1]);
#endif
      if (property != NULL)
        /* FIXME */;
      else
        {
          property = g_new (XMPProperty, 1);
          property->name = g_strdup (name);
          property->type = XMP_TYPE_GENERIC_STRUCTURE;
          property->editable = TRUE;
          xmp_model->custom_properties =
            g_slist_prepend (xmp_model->custom_properties, property);
        }
      for (i = 2; value[i] != NULL; i += 2)
        {
          tmp_name = g_strconcat (name, " [", value[i], "]", NULL);
          gtk_tree_store_append (GTK_TREE_STORE (xmp_model), &child_iter, &iter);
          gtk_tree_store_set (GTK_TREE_STORE (xmp_model), &child_iter,
                              COL_XMP_NAME, tmp_name,
                              COL_XMP_VALUE, value[i + 1],
                              COL_XMP_VALUE_RAW, value,
                              COL_XMP_TYPE_XREF, property,
                              COL_XMP_WIDGET_XREF, NULL,
                              COL_XMP_EDITABLE, property->editable,
                              COL_XMP_EDIT_ICON, NULL,
                              COL_XMP_VISIBLE, TRUE,
                              COL_XMP_WEIGHT, PANGO_WEIGHT_NORMAL,
                              COL_XMP_WEIGHT_SET, FALSE,
                              -1);
          g_free (tmp_name);
        }
      break;

    default:
#ifdef DEBUG_XMP_MODEL
      g_print ("\t%s:%s = ?\n", ns_prefix, name);
#endif
      break;
    }
}

/* called by the XMP parser - parse error */
static void
parse_error (XMPParseContext *context,
             GError          *error,
             gpointer         user_data)
{
  g_printerr ("While parsing XMP metadata:\n%s\n", error->message);
}

static const XMPParser xmp_parser =
{
  parse_start_schema,
  parse_end_schema,
  parse_set_property,
  parse_error
};

/**
 * xmp_model_parse_buffer:
 * @xmp_model: pointer to the #XMPModel in which the results will be stored
 * @buffer: buffer to be parsed
 * @buffer_length: length of the @buffer
 * @skip_other_data: if %TRUE, allow arbitrary data before XMP packet marker
 * @error: return location for a #GError
 *
 * Parse a buffer containing XMP metadata and merge the parsed contents into
 * the supplied @xmp_model.  If @skip_other_data is %TRUE, then the parser
 * will try to find the <?xpacket...?> marker in the buffer, skipping any
 * unknown data found before it.
 *
 * (Note: this calls the functions from xmp_parse.c, which will call the
 *  functions in this file through the xmp_parser structure defined above.)
 *
 * Return value: %TRUE on success, %FALSE if an error was set
 **/
gboolean
xmp_model_parse_buffer (XMPModel     *xmp_model,
                        const gchar  *buffer,
                        gssize        buffer_length,
                        gboolean      skip_other_data,
                        GError      **error)
{
  XMPParseFlags    flags;
  XMPParseContext *context;

  flags = XMP_FLAG_DEFER_VALUE_FREE; /* we will free the array ourselves */
  if (skip_other_data)
    flags |= XMP_FLAG_FIND_XPACKET;

  context = xmp_parse_context_new (&xmp_parser, flags, xmp_model, NULL);

  if (! xmp_parse_context_parse (context, buffer, buffer_length, error))
    {
      xmp_parse_context_free (context);
      return FALSE;
    }

  if (! xmp_parse_context_end_parse (context, error))
    {
      xmp_parse_context_free (context);
      return FALSE;
    }

  xmp_parse_context_free (context);

  return TRUE;
}

/**
 * xmp_model_parse_file:
 * @xmp_model: pointer to the #XMPModel in which the results will be stored
 * @filename: name of the file containing XMP metadata to parse
 * @error: return location for a #GError
  *
 * Try to find XMP metadata in a file and merge its contents into the supplied
 * @xmp_model.
 *
 * Return value: %TRUE on success, %FALSE if an error was set
 **/
gboolean
xmp_model_parse_file (XMPModel     *xmp_model,
                      const gchar  *filename,
                      GError      **error)
{
  gchar *buffer;
  gsize  buffer_length;

  g_return_val_if_fail (filename != NULL, FALSE);

  if (! g_file_get_contents (filename, &buffer, &buffer_length, error))
    return FALSE;

  if (! xmp_model_parse_buffer (xmp_model, buffer, buffer_length, TRUE, error))
    return FALSE;

  g_free (buffer);

  return TRUE;
}

/**
 * xmp_model_get_tree_model:
 * @xmp_model: pointer to an #XMPModel
 *
 * Return a pointer to the #GtkTreeModel contained in the #XMPModel.
 **/
GtkTreeModel *
xmp_model_get_tree_model (XMPModel *xmp_model)
{
  g_return_val_if_fail (xmp_model != NULL, NULL);
  return GTK_TREE_MODEL (xmp_model);
}

/**
 * xmp_model_get_scalar_property:
 * @xmp_model: pointer to an #XMPModel
 * @schema_name: full URI or usual prefix of the schema
 * @property_name: name of the property to store
 *
 * Store a new value for the specified XMP property.
 *
 * Return value: string representation of the value of that property,
 *               or %NULL if the property does not exist
 **/
const gchar *
xmp_model_get_scalar_property (XMPModel    *xmp_model,
                               const gchar *schema_name,
                               const gchar *property_name)
{
  XMPSchema    *schema;
  GtkTreeIter   iter;
  XMPProperty  *property = NULL;
  GtkTreeIter   child_iter;
  int           i;
  XMPProperty  *property_xref;
  const gchar  *value;

  g_return_val_if_fail (xmp_model != NULL, NULL);
  g_return_val_if_fail (schema_name != NULL, NULL);
  g_return_val_if_fail (property_name != NULL, NULL);
  schema = find_xmp_schema_by_uri (xmp_model, schema_name);
  if (! schema)
    schema = find_xmp_schema_prefix (xmp_model, schema_name);

  if (! schema)
    return NULL;

  if (! find_iter_for_schema (xmp_model, schema, &iter))
    return NULL;

  if (schema->properties != NULL)
    for (i = 0; schema->properties[i].name != NULL; ++i)
      if (! strcmp (schema->properties[i].name, property_name))
        {
          property = &(schema->properties[i]);
          break;
        }

  if (property == NULL)
    return NULL;

  if (! gtk_tree_model_iter_children (GTK_TREE_MODEL (xmp_model),
                                      &child_iter, &iter))
    return NULL;

  do
    {
      gtk_tree_model_get (GTK_TREE_MODEL (xmp_model), &child_iter,
                          COL_XMP_TYPE_XREF, &property_xref,
                          COL_XMP_VALUE, &value,
                          -1);
      if (property_xref == property)
        return value;
    }
  while (gtk_tree_model_iter_next (GTK_TREE_MODEL(xmp_model),
                                   &child_iter));

  return NULL;
}

/**
 * xmp_model_set_scalar_property:
 * @xmp_model: pointer to an #XMPModel
 * @schema_name: full URI or usual prefix of the schema
 * @property_name: name of the property to store
 * @property_value: value to store
 *
 * Store a new value for the specified XMP property.
 *
 * Return value: %TRUE if the property was set, %FALSE if an error
 *               occurred (for example, the @schema_name is invalid)
 **/
gboolean
xmp_model_set_scalar_property (XMPModel    *xmp_model,
                               const gchar *schema_name,
                               const gchar *property_name,
                               const gchar *property_value)
{
  XMPSchema    *schema;
  XMPProperty  *property = NULL;
  GtkTreeIter   iter;
  GtkTreeIter   child_iter;
  int           i;
  gchar       **value;

  g_return_val_if_fail (xmp_model != NULL, FALSE);
  g_return_val_if_fail (schema_name != NULL, FALSE);
  g_return_val_if_fail (property_name != NULL, FALSE);
  g_return_val_if_fail (property_value != NULL, FALSE);
  schema = find_xmp_schema_by_uri (xmp_model, schema_name);
  if (! schema)
    schema = find_xmp_schema_prefix (xmp_model, schema_name);

  if (! schema)
    return FALSE;

  if (! find_iter_for_schema (xmp_model, schema, &iter))
    add_known_schema (xmp_model, schema, &iter);

  if (schema->properties != NULL)
    for (i = 0; schema->properties[i].name != NULL; ++i)
      if (! strcmp (schema->properties[i].name, property_name))
        {
          property = &(schema->properties[i]);
          break;
        }

  if (property != NULL)
    {
      find_and_remove_property (xmp_model, property, &iter);
    }
  else
    {
      property = g_new (XMPProperty, 1);
      property->name     = g_strdup (property_name);
      property->type     = XMP_TYPE_TEXT;
      property->editable = TRUE;

      xmp_model->custom_properties =
        g_slist_prepend (xmp_model->custom_properties, property);
    }

  value = g_new (gchar *, 2);
  value[0] = g_strdup (property_value);
  value[1] = NULL;
  gtk_tree_store_append (GTK_TREE_STORE (xmp_model), &child_iter, &iter);
  gtk_tree_store_set (GTK_TREE_STORE (xmp_model), &child_iter,
                      COL_XMP_NAME, g_strdup (property_name),
                      COL_XMP_VALUE, value[0],
                      COL_XMP_VALUE_RAW, value,
                      COL_XMP_TYPE_XREF, property,
                      COL_XMP_WIDGET_XREF, NULL,
                      COL_XMP_EDITABLE, property->editable,
                      COL_XMP_EDIT_ICON, NULL,
                      COL_XMP_VISIBLE, TRUE,
                      COL_XMP_WEIGHT, PANGO_WEIGHT_NORMAL,
                      COL_XMP_WEIGHT_SET, FALSE,
                      -1);
  return TRUE;
}

/**
 * xmp_model_property_changed:
 * @xmp_model: An #XMPModel
 * @schema: An #XMPSchema the property belongs to
 * @iter: A valid #GtkTreeIter pointing to the changed row
 *
 * Emits the "property-changed" event based on the @tree_model with
 * detail. The detail is a joined string of xmp-schema-prefix and
 * xmp-property-name (e.g.  property-changed::dc:DocumentID).
 **/
void
xmp_model_property_changed (XMPModel     *xmp_model,
                            XMPSchema    *schema,
                            GtkTreeIter  *iter)
{
  GQuark         detail;
  gchar         *joined;
  const gchar   *property_name;

  g_return_if_fail (PICMAN_IS_XMP_MODEL (xmp_model));
  g_return_if_fail (iter != NULL);

  gtk_tree_model_get (GTK_TREE_MODEL (xmp_model), iter,
                      COL_XMP_NAME, &property_name,
                      -1);
  joined = g_strjoin (":", schema->prefix, property_name, NULL);
  detail = g_quark_from_string (joined);

  g_signal_emit (xmp_model, xmp_model_signals[PROPERTY_CHANGED], detail, iter);
}
