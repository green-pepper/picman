/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanlanguageentry.c
 * Copyright (C) 2008  Sven Neumann <sven@picman.org>
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

/* PicmanLanguageEntry is an entry widget that provides completion on
 * translated language names. It is suited for specifying the language
 * a text is written in.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include "widgets-types.h"

#include "picmanlanguageentry.h"
#include "picmanlanguagestore.h"


enum
{
  PROP_0,
  PROP_MODEL
};

struct _PicmanLanguageEntry
{
  GtkEntry       parent_instance;

  GtkListStore  *store;
  gchar         *code;  /*  ISO 639-1 language code  */
};


static void      picman_language_entry_constructed  (GObject      *object);
static void      picman_language_entry_finalize     (GObject      *object);
static void      picman_language_entry_set_property (GObject      *object,
                                                   guint         property_id,
                                                   const GValue *value,
                                                   GParamSpec   *pspec);
static void      picman_language_entry_get_property (GObject      *object,
                                                   guint         property_id,
                                                   GValue       *value,
                                                   GParamSpec   *pspec);

static gboolean  picman_language_entry_language_selected (GtkEntryCompletion *completion,
                                                        GtkTreeModel       *model,
                                                        GtkTreeIter        *iter,
                                                        PicmanLanguageEntry  *entry);


G_DEFINE_TYPE (PicmanLanguageEntry, picman_language_entry, GTK_TYPE_ENTRY)

#define parent_class picman_language_entry_parent_class


static void
picman_language_entry_class_init (PicmanLanguageEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_language_entry_constructed;
  object_class->finalize     = picman_language_entry_finalize;
  object_class->set_property = picman_language_entry_set_property;
  object_class->get_property = picman_language_entry_get_property;

  g_object_class_install_property (object_class, PROP_MODEL,
                                   g_param_spec_object ("model", NULL, NULL,
                                                        PICMAN_TYPE_LANGUAGE_STORE,
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        PICMAN_PARAM_READWRITE));
}

static void
picman_language_entry_init (PicmanLanguageEntry *entry)
{
}

static void
picman_language_entry_constructed (GObject *object)
{
  PicmanLanguageEntry *entry = PICMAN_LANGUAGE_ENTRY (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  if (entry->store)
    {
      GtkEntryCompletion *completion;

      completion = g_object_new (GTK_TYPE_ENTRY_COMPLETION,
                                 "model",             entry->store,
                                 "inline-selection",  TRUE,
                                 NULL);

      /* Note that we must use this function to set the text column,
       * otherwise we won't get a cell renderer for free.
       */
      gtk_entry_completion_set_text_column (completion,
                                            PICMAN_LANGUAGE_STORE_LABEL);

      gtk_entry_set_completion (GTK_ENTRY (entry), completion);
      g_object_unref (completion);

      g_signal_connect (completion, "match-selected",
                        G_CALLBACK (picman_language_entry_language_selected),
                        entry);
    }
}

static void
picman_language_entry_finalize (GObject *object)
{
  PicmanLanguageEntry *entry = PICMAN_LANGUAGE_ENTRY (object);

  if (entry->store)
    {
      g_object_unref (entry->store);
      entry->store = NULL;
    }

  if (entry->code)
    {
      g_free (entry->code);
      entry->code = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_language_entry_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PicmanLanguageEntry *entry = PICMAN_LANGUAGE_ENTRY (object);

  switch (property_id)
    {
    case PROP_MODEL:
      g_return_if_fail (entry->store == NULL);
      entry->store = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_language_entry_get_property (GObject      *object,
                                  guint         property_id,
                                  GValue       *value,
                                  GParamSpec   *pspec)
{
  PicmanLanguageEntry *entry = PICMAN_LANGUAGE_ENTRY (object);

  switch (property_id)
    {
    case PROP_MODEL:
      g_value_set_object (value, entry->store);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_language_entry_language_selected (GtkEntryCompletion *completion,
                                       GtkTreeModel       *model,
                                       GtkTreeIter        *iter,
                                       PicmanLanguageEntry  *entry)
{
  g_free (entry->code);

  gtk_tree_model_get (model, iter,
                      PICMAN_LANGUAGE_STORE_CODE, &entry->code,
                      -1);

  return FALSE;
}

GtkWidget *
picman_language_entry_new (void)
{
  GtkWidget    *entry;
  GtkListStore *store;

  store = picman_language_store_new ();

  entry = g_object_new (PICMAN_TYPE_LANGUAGE_ENTRY,
                        "model", store,
                        NULL);

  g_object_unref (store);

  return entry;
}

const gchar *
picman_language_entry_get_code (PicmanLanguageEntry *entry)
{
  g_return_val_if_fail (PICMAN_IS_LANGUAGE_ENTRY (entry), NULL);

  return entry->code;
}

gboolean
picman_language_entry_set_code (PicmanLanguageEntry *entry,
                              const gchar       *code)
{
  GtkTreeIter  iter;

  g_return_val_if_fail (PICMAN_IS_LANGUAGE_ENTRY (entry), FALSE);

  if (entry->code)
    {
      g_free (entry->code);
      entry->code = NULL;
    }

  if (! code || ! strlen (code))
    {
      gtk_entry_set_text (GTK_ENTRY (entry), "");

      return TRUE;
    }

  if (picman_language_store_lookup (PICMAN_LANGUAGE_STORE (entry->store),
                                  code, &iter))
    {
      gchar *label;

      gtk_tree_model_get (GTK_TREE_MODEL (entry->store), &iter,
                          PICMAN_LANGUAGE_STORE_LABEL, &label,
                          PICMAN_LANGUAGE_STORE_CODE,  &entry->code,
                          -1);

      gtk_entry_set_text (GTK_ENTRY (entry), label);
      g_free (label);

      return TRUE;
    }

  return FALSE;
}
