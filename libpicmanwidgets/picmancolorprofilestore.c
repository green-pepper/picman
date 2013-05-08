/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanprofilestore.c
 * Copyright (C) 2004-2008  Sven Neumann <sven@picman.org>
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "picmanwidgetstypes.h"

#include "picmancolorprofilestore.h"
#include "picmancolorprofilestore-private.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmancolorprofilestore
 * @title: PicmanColorProfileStore
 * @short_description: A #GtkListStore subclass that keep color profiles.
 *
 * A #GtkListStore subclass that keep color profiles.
 **/


#define HISTORY_SIZE  8

enum
{
  PROP_0,
  PROP_HISTORY
};


static void      picman_color_profile_store_constructed    (GObject               *object);
static void      picman_color_profile_store_dispose        (GObject               *object);
static void      picman_color_profile_store_finalize       (GObject               *object);
static void      picman_color_profile_store_set_property   (GObject               *object,
                                                          guint                  property_id,
                                                          const GValue          *value,
                                                          GParamSpec            *pspec);
static void      picman_color_profile_store_get_property   (GObject               *object,
                                                          guint                  property_id,
                                                          GValue                *value,
                                                          GParamSpec            *pspec);

static gboolean  picman_color_profile_store_history_insert (PicmanColorProfileStore *store,
                                                          GtkTreeIter           *iter,
                                                          const gchar           *filename,
                                                          const gchar           *label,
                                                          gint                   index);
static void      picman_color_profile_store_get_separator  (PicmanColorProfileStore  *store,
                                                          GtkTreeIter            *iter,
                                                          gboolean                top);
static gboolean  picman_color_profile_store_save           (PicmanColorProfileStore  *store,
                                                          const gchar            *filename,
                                                          GError                **error);
static gboolean  picman_color_profile_store_load           (PicmanColorProfileStore  *store,
                                                          const gchar            *filename,
                                                          GError                **error);


G_DEFINE_TYPE (PicmanColorProfileStore,
               picman_color_profile_store, GTK_TYPE_LIST_STORE)

#define parent_class picman_color_profile_store_parent_class


static void
picman_color_profile_store_class_init (PicmanColorProfileStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_color_profile_store_constructed;
  object_class->dispose      = picman_color_profile_store_dispose;
  object_class->finalize     = picman_color_profile_store_finalize;
  object_class->set_property = picman_color_profile_store_set_property;
  object_class->get_property = picman_color_profile_store_get_property;

  /**
   * PicmanColorProfileStore:history:
   *
   * Filename of the color history used to populate the profile store.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class,
                                   PROP_HISTORY,
                                   g_param_spec_string ("history", NULL, NULL,
                                                        NULL,
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        PICMAN_PARAM_READWRITE));
}

static void
picman_color_profile_store_init (PicmanColorProfileStore *store)
{
  GType types[] =
    {
      G_TYPE_INT,     /*  PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE  */
      G_TYPE_STRING,  /*  PICMAN_COLOR_PROFILE_STORE_LABEL      */
      G_TYPE_STRING,  /*  PICMAN_COLOR_PROFILE_STORE_FILENAME   */
      G_TYPE_INT      /*  PICMAN_COLOR_PROFILE_STORE_INDEX      */
    };

  gtk_list_store_set_column_types (GTK_LIST_STORE (store),
                                   G_N_ELEMENTS (types), types);
}

static void
picman_color_profile_store_constructed (GObject *object)
{
  PicmanColorProfileStore *store = PICMAN_COLOR_PROFILE_STORE (object);
  GtkTreeIter            iter;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  gtk_list_store_append (GTK_LIST_STORE (store), &iter);
  gtk_list_store_set (GTK_LIST_STORE (store), &iter,
                      PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE,
                      PICMAN_COLOR_PROFILE_STORE_ITEM_DIALOG,
                      PICMAN_COLOR_PROFILE_STORE_LABEL,
                      _("Select color profile from disk..."),
                      -1);

  if (store->history)
    {
      picman_color_profile_store_load (store, store->history, NULL);
    }
}

static void
picman_color_profile_store_dispose (GObject *object)
{
  PicmanColorProfileStore *store = PICMAN_COLOR_PROFILE_STORE (object);

  if (store->history)
    {
      picman_color_profile_store_save (store, store->history, NULL);
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_color_profile_store_finalize (GObject *object)
{
  PicmanColorProfileStore *store = PICMAN_COLOR_PROFILE_STORE (object);

  if (store->history)
    {
      g_free (store->history);
      store->history = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_color_profile_store_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PicmanColorProfileStore *store = PICMAN_COLOR_PROFILE_STORE (object);

  switch (property_id)
    {
    case PROP_HISTORY:
      g_return_if_fail (store->history == NULL);
      store->history = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_profile_store_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PicmanColorProfileStore *store = PICMAN_COLOR_PROFILE_STORE (object);

  switch (property_id)
    {
    case PROP_HISTORY:
      g_value_set_string (value, store->history);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/**
 * picman_color_profile_store_new:
 * @history: filename of the profilerc (or %NULL for no history)
 *
 * Creates a new #PicmanColorProfileStore object and populates it with
 * last used profiles read from the file @history. The updated history
 * is written back to disk when the store is disposed.
 *
 * The filename passed as @history is typically created using the
 * following code snippet:
 * <informalexample><programlisting>
 *  gchar *history = picman_personal_rc_file ("profilerc");
 * </programlisting></informalexample>
 *
 * Return value: a new #PicmanColorProfileStore
 *
 * Since: PICMAN 2.4
 **/
GtkListStore *
picman_color_profile_store_new (const gchar *history)
{
  return g_object_new (PICMAN_TYPE_COLOR_PROFILE_STORE,
                       "history", history,
                       NULL);
}

/**
 * picman_color_profile_store_add:
 * @store:    a #PicmanColorProfileStore
 * @filename: filename of the profile to add (or %NULL)
 * @label:    label to use for the profile
 *            (may only be %NULL if @filename is %NULL)
 *
 * Adds a color profile item to the #PicmanColorProfileStore. Items
 * added with this function will be kept at the top, separated from
 * the history of last used color profiles.
 *
 * This function is often used to add a selectable item for the %NULL
 * filename. If you pass %NULL for both @filename and @label, the
 * @label will be set to the string "None" for you (and translated for
 * the user).
 *
 * Since: PICMAN 2.4
 **/
void
picman_color_profile_store_add (PicmanColorProfileStore *store,
                              const gchar           *filename,
                              const gchar           *label)
{
  GtkTreeIter  separator;
  GtkTreeIter  iter;

  g_return_if_fail (PICMAN_IS_COLOR_PROFILE_STORE (store));
  g_return_if_fail (label != NULL || filename == NULL);

  if (! filename && ! label)
    label = C_("profile", "None");

  picman_color_profile_store_get_separator (store, &separator, TRUE);

  gtk_list_store_insert_before (GTK_LIST_STORE (store), &iter, &separator);
  gtk_list_store_set (GTK_LIST_STORE (store), &iter,
                      PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE,
                      PICMAN_COLOR_PROFILE_STORE_ITEM_FILE,
                      PICMAN_COLOR_PROFILE_STORE_FILENAME, filename,
                      PICMAN_COLOR_PROFILE_STORE_LABEL, label,
                      PICMAN_COLOR_PROFILE_STORE_INDEX, -1,
                      -1);
}

/**
 * _picman_color_profile_store_history_add:
 * @store:    a #PicmanColorProfileStore
 * @filename: filename of the profile to add (or %NULL)
 * @label:    label to use for the profile (or %NULL)
 * @iter:     a #GtkTreeIter
 *
 * Return value: %TRUE if the iter is valid and pointing to the item
 *
 * Since: PICMAN 2.4
 **/
gboolean
_picman_color_profile_store_history_add (PicmanColorProfileStore *store,
                                       const gchar           *filename,
                                       const gchar           *label,
                                       GtkTreeIter           *iter)
{
  GtkTreeModel *model;
  gboolean      iter_valid;
  gint          max = -1;

  g_return_val_if_fail (PICMAN_IS_COLOR_PROFILE_STORE (store), FALSE);
  g_return_val_if_fail (iter != NULL, FALSE);

  model = GTK_TREE_MODEL (store);

  for (iter_valid = gtk_tree_model_get_iter_first (model, iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, iter))
    {
      gint   type;
      gint   index;
      gchar *this;

      gtk_tree_model_get (model, iter,
                          PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE, &type,
                          PICMAN_COLOR_PROFILE_STORE_INDEX,     &index,
                          -1);

      if (type != PICMAN_COLOR_PROFILE_STORE_ITEM_FILE)
        continue;

      if (index > max)
        max = index;

      /*  check if we found a filename match  */
      gtk_tree_model_get (model, iter,
                          PICMAN_COLOR_PROFILE_STORE_FILENAME, &this,
                          -1);

      if ((this && filename && strcmp (filename, this) == 0) ||
          (! this && ! filename))
        {
          /*  update the label  */
          if (label && *label)
            gtk_list_store_set (GTK_LIST_STORE (store), iter,
                                PICMAN_COLOR_PROFILE_STORE_LABEL, label,
                                -1);

          g_free (this);
          return TRUE;
        }
    }

  if (! filename)
    return FALSE;

  if (label && *label)
    {
      iter_valid = picman_color_profile_store_history_insert (store, iter,
                                                            filename, label,
                                                            ++max);
    }
  else
    {
      gchar *basename = g_filename_display_basename (filename);

      iter_valid = picman_color_profile_store_history_insert (store, iter,
                                                            filename, basename,
                                                            ++max);
      g_free (basename);
    }

  return iter_valid;
}

/**
 * _picman_color_profile_store_history_reorder
 * @store: a #PicmanColorProfileStore
 * @iter:  a #GtkTreeIter
 *
 * Moves the entry pointed to by @iter to the front of the MRU list.
 *
 * Since: PICMAN 2.4
 **/
void
_picman_color_profile_store_history_reorder (PicmanColorProfileStore *store,
                                           GtkTreeIter           *iter)
{
  GtkTreeModel *model;
  gint          index;
  gboolean      iter_valid;

  g_return_if_fail (PICMAN_IS_COLOR_PROFILE_STORE (store));
  g_return_if_fail (iter != NULL);

  model = GTK_TREE_MODEL (store);

  gtk_tree_model_get (model, iter,
                      PICMAN_COLOR_PROFILE_STORE_INDEX, &index,
                      -1);

  if (index == 0)
    return;  /* already at the top */

  for (iter_valid = gtk_tree_model_get_iter_first (model, iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, iter))
    {
      gint type;
      gint this_index;

      gtk_tree_model_get (model, iter,
                          PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE, &type,
                          PICMAN_COLOR_PROFILE_STORE_INDEX,     &this_index,
                          -1);

      if (type == PICMAN_COLOR_PROFILE_STORE_ITEM_FILE && this_index > -1)
        {
          if (this_index < index)
            {
              this_index++;
            }
          else if (this_index == index)
            {
              this_index = 0;
            }

          gtk_list_store_set (GTK_LIST_STORE (store), iter,
                              PICMAN_COLOR_PROFILE_STORE_INDEX, this_index,
                              -1);
        }
    }
}

static gboolean
picman_color_profile_store_history_insert (PicmanColorProfileStore *store,
                                         GtkTreeIter           *iter,
                                         const gchar           *filename,
                                         const gchar           *label,
                                         gint                   index)
{
  GtkTreeModel *model = GTK_TREE_MODEL (store);
  GtkTreeIter   sibling;
  gboolean      iter_valid;

  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (label != NULL, FALSE);
  g_return_val_if_fail (index > -1, FALSE);

  picman_color_profile_store_get_separator (store, iter, FALSE);

  for (iter_valid = gtk_tree_model_get_iter_first (model, &sibling);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, &sibling))
    {
      gint type;
      gint this_index;

      gtk_tree_model_get (model, &sibling,
                          PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE, &type,
                          PICMAN_COLOR_PROFILE_STORE_INDEX,     &this_index,
                          -1);

      if (type == PICMAN_COLOR_PROFILE_STORE_ITEM_SEPARATOR_BOTTOM)
        {
          gtk_list_store_insert_before (GTK_LIST_STORE (store),
                                        iter, &sibling);
          break;
        }

      if (type == PICMAN_COLOR_PROFILE_STORE_ITEM_FILE && this_index > -1)
        {
          gchar *this_label;

          gtk_tree_model_get (model, &sibling,
                              PICMAN_COLOR_PROFILE_STORE_LABEL, &this_label,
                              -1);

          if (this_label && g_utf8_collate (label, this_label) < 0)
            {
              gtk_list_store_insert_before (GTK_LIST_STORE (store),
                                            iter, &sibling);
              g_free (this_label);
              break;
            }

          g_free (this_label);
        }
    }

  if (iter_valid)
    gtk_list_store_set (GTK_LIST_STORE (store), iter,
                        PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE,
                        PICMAN_COLOR_PROFILE_STORE_ITEM_FILE,
                        PICMAN_COLOR_PROFILE_STORE_FILENAME, filename,
                        PICMAN_COLOR_PROFILE_STORE_LABEL,    label,
                        PICMAN_COLOR_PROFILE_STORE_INDEX,    index,
                        -1);

  return iter_valid;
}

static void
picman_color_profile_store_create_separator (PicmanColorProfileStore *store,
                                           GtkTreeIter           *iter,
                                           gboolean               top)
{
  if (top)
    {
      gtk_list_store_prepend (GTK_LIST_STORE (store), iter);
    }
  else
    {
      GtkTreeModel *model = GTK_TREE_MODEL (store);
      GtkTreeIter   sibling;
      gboolean      iter_valid;

      for (iter_valid = gtk_tree_model_get_iter_first (model, &sibling);
           iter_valid;
           iter_valid = gtk_tree_model_iter_next (model, &sibling))
        {
          gint type;

          gtk_tree_model_get (model, &sibling,
                              PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE, &type,
                              -1);

          if (type == PICMAN_COLOR_PROFILE_STORE_ITEM_DIALOG)
            break;
        }

      if (iter_valid)
        gtk_list_store_insert_before (GTK_LIST_STORE (store), iter, &sibling);
    }

  gtk_list_store_set (GTK_LIST_STORE (store), iter,
                      PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE,
                      top ?
                      PICMAN_COLOR_PROFILE_STORE_ITEM_SEPARATOR_TOP :
                      PICMAN_COLOR_PROFILE_STORE_ITEM_SEPARATOR_BOTTOM,
                      PICMAN_COLOR_PROFILE_STORE_INDEX, -1,
                      -1);
}

static void
picman_color_profile_store_get_separator (PicmanColorProfileStore *store,
                                        GtkTreeIter           *iter,
                                        gboolean               top)
{
  GtkTreeModel *model = GTK_TREE_MODEL (store);
  gboolean      iter_valid;
  gint          needle;

  needle = (top ?
            PICMAN_COLOR_PROFILE_STORE_ITEM_SEPARATOR_TOP :
            PICMAN_COLOR_PROFILE_STORE_ITEM_SEPARATOR_BOTTOM);

  for (iter_valid = gtk_tree_model_get_iter_first (model, iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, iter))
    {
      gint type;

      gtk_tree_model_get (model, iter,
                          PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE, &type,
                          -1);

      if (type == needle)
        return;
    }

  picman_color_profile_store_create_separator (store, iter, top);
}

static GTokenType
picman_color_profile_store_load_profile (PicmanColorProfileStore *store,
                                       GScanner              *scanner,
                                       gint                   index)
{
  GtkTreeIter  iter;
  gchar       *label = NULL;
  gchar       *uri   = NULL;

  if (picman_scanner_parse_string (scanner, &label) &&
      picman_scanner_parse_string (scanner, &uri))
    {
      gchar *filename = g_filename_from_uri (uri, NULL, NULL);

      if (filename && g_file_test (filename, G_FILE_TEST_IS_REGULAR))
        {
          picman_color_profile_store_history_insert (store, &iter,
                                                   filename, label, index);
        }

      g_free (filename);
      g_free (label);
      g_free (uri);

      return G_TOKEN_RIGHT_PAREN;
    }

  g_free (label);
  g_free (uri);

  return G_TOKEN_STRING;
}

static gboolean
picman_color_profile_store_load (PicmanColorProfileStore  *store,
                               const gchar            *filename,
                               GError                **error)
{
  GScanner   *scanner;
  GTokenType  token;
  gint        i = 0;

  scanner = picman_scanner_new_file (filename, error);
  if (! scanner)
    return FALSE;

  g_scanner_scope_add_symbol (scanner, 0, "color-profile", NULL);

  token = G_TOKEN_LEFT_PAREN;

  while (g_scanner_peek_next_token (scanner) == token)
    {
      token = g_scanner_get_next_token (scanner);

      switch (token)
        {
        case G_TOKEN_LEFT_PAREN:
          token = G_TOKEN_SYMBOL;
          break;

        case G_TOKEN_SYMBOL:
          token = picman_color_profile_store_load_profile (store, scanner, i++);
          break;

        case G_TOKEN_RIGHT_PAREN:
          token = G_TOKEN_LEFT_PAREN;
          break;

        default: /* do nothing */
          break;
        }
    }

  if (token != G_TOKEN_LEFT_PAREN)
    {
      g_scanner_get_next_token (scanner);
      g_scanner_unexp_token (scanner, token, NULL, NULL, NULL,
                             _("fatal parse error"), TRUE);
    }

  picman_scanner_destroy (scanner);

  return TRUE;
}

static gboolean
picman_color_profile_store_save (PicmanColorProfileStore  *store,
                               const gchar            *filename,
                               GError                **error)
{
  PicmanConfigWriter *writer;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  gchar            *labels[HISTORY_SIZE]    = { NULL, };
  gchar            *filenames[HISTORY_SIZE] = { NULL, };
  gboolean          iter_valid;
  gint              i;

  writer = picman_config_writer_new_file (filename,
                                        TRUE,
                                        "PICMAN color profile history",
                                        error);
  if (! writer)
    return FALSE;

  model = GTK_TREE_MODEL (store);

  for (iter_valid = gtk_tree_model_get_iter_first (model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, &iter))
    {
      gint type;
      gint index;

      gtk_tree_model_get (model, &iter,
                          PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE, &type,
                          PICMAN_COLOR_PROFILE_STORE_INDEX,     &index,
                          -1);

      if (type == PICMAN_COLOR_PROFILE_STORE_ITEM_FILE &&
          index >= 0                                 &&
          index < HISTORY_SIZE)
        {
          if (labels[index] || filenames[index])
            g_warning ("%s: double index %d", G_STRFUNC, index);

          gtk_tree_model_get (model, &iter,
                              PICMAN_COLOR_PROFILE_STORE_LABEL,
                              &labels[index],
                              PICMAN_COLOR_PROFILE_STORE_FILENAME,
                              &filenames[index],
                              -1);
        }
    }


  for (i = 0; i < HISTORY_SIZE; i++)
    {
      if (filenames[i] && labels[i])
        {
          gchar *uri = g_filename_to_uri (filenames[i], NULL, NULL);

          if (uri)
            {
              picman_config_writer_open (writer, "color-profile");
              picman_config_writer_string (writer, labels[i]);
              picman_config_writer_string (writer, uri);
              picman_config_writer_close (writer);

              g_free (uri);
            }
        }

      g_free (filenames[i]);
      g_free (labels[i]);
    }

  return picman_config_writer_finish (writer,
                                    "end of color profile history", error);
}
