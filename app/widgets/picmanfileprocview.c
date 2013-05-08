/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfileprocview.c
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
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

#include <gtk/gtk.h>

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanmarshal.h"

#include "plug-in/picmanpluginprocedure.h"

#include "picmanfileprocview.h"

#include "picman-intl.h"


enum
{
  COLUMN_PROC,
  COLUMN_LABEL,
  COLUMN_EXTENSIONS,
  COLUMN_HELP_ID,
  N_COLUMNS
};

enum
{
  CHANGED,
  LAST_SIGNAL
};


static void  picman_file_proc_view_finalize          (GObject          *object);

static void  picman_file_proc_view_selection_changed (GtkTreeSelection *selection,
                                                    PicmanFileProcView *view);


G_DEFINE_TYPE (PicmanFileProcView, picman_file_proc_view, GTK_TYPE_TREE_VIEW)

#define parent_class picman_file_proc_view_parent_class

static guint view_signals[LAST_SIGNAL] = { 0 };


static void
picman_file_proc_view_class_init (PicmanFileProcViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = picman_file_proc_view_finalize;

  klass->changed         = NULL;

  view_signals[CHANGED] = g_signal_new ("changed",
                                        G_TYPE_FROM_CLASS (klass),
                                        G_SIGNAL_RUN_LAST,
                                        G_STRUCT_OFFSET (PicmanFileProcViewClass,
                                                         changed),
                                        NULL, NULL,
                                        picman_marshal_VOID__VOID,
                                        G_TYPE_NONE, 0);
}

static void
picman_file_proc_view_init (PicmanFileProcView *view)
{
}

static void
picman_file_proc_view_finalize (GObject *object)
{
  PicmanFileProcView *view = PICMAN_FILE_PROC_VIEW (object);

  if (view->meta_extensions)
    {
      g_list_free_full (view->meta_extensions, (GDestroyNotify) g_free);
      view->meta_extensions = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
picman_file_proc_view_new (Picman        *picman,
                         GSList      *procedures,
                         const gchar *automatic,
                         const gchar *automatic_help_id)
{
  GtkTreeView       *view;
  GtkTreeViewColumn *column;
  GtkCellRenderer   *cell;
  GtkListStore      *store;
  GSList            *list;
  GtkTreeIter        iter;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  store = gtk_list_store_new (N_COLUMNS,
                              PICMAN_TYPE_PLUG_IN_PROCEDURE, /*  COLUMN_PROC   */
                              G_TYPE_STRING,          /*  COLUMN_LABEL       */
                              G_TYPE_STRING,          /*  COLUMN_EXTENSIONS  */
                              G_TYPE_STRING);         /*  COLUMN_HELP_ID     */

  view = g_object_new (PICMAN_TYPE_FILE_PROC_VIEW,
                       "model",      store,
                       "rules-hint", TRUE,
                       NULL);

  g_object_unref (store);

  for (list = procedures; list; list = g_slist_next (list))
    {
      PicmanPlugInProcedure *proc = list->data;

      if (! proc->prefixes_list) /*  skip URL loaders  */
        {
          const gchar *label   = picman_plug_in_procedure_get_label (proc);
          gchar       *help_id = picman_plug_in_procedure_get_help_id (proc);
          GSList      *list2;

          if (label)
            {
              gtk_list_store_append (store, &iter);
              gtk_list_store_set (store, &iter,
                                  COLUMN_PROC,       proc,
                                  COLUMN_LABEL,      label,
                                  COLUMN_EXTENSIONS, proc->extensions,
                                  COLUMN_HELP_ID,    help_id,
                                  -1);
            }

          g_free (help_id);

          for (list2 = proc->extensions_list;
               list2;
               list2 = g_slist_next (list2))
            {
              PicmanFileProcView *proc_view = PICMAN_FILE_PROC_VIEW (view);
              const gchar      *ext       = list2->data;
              const gchar      *dot       = strchr (ext, '.');

              if (dot && dot != ext)
                proc_view->meta_extensions =
                  g_list_append (proc_view->meta_extensions,
                                 g_strdup (dot + 1));
            }
        }
    }

  if (automatic)
    {
      gtk_list_store_prepend (store, &iter);

      gtk_list_store_set (store, &iter,
                          COLUMN_PROC,    NULL,
                          COLUMN_LABEL,   automatic,
                          COLUMN_HELP_ID, automatic_help_id,
                          -1);
    }

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, _("File Type"));
  gtk_tree_view_column_set_expand (column, TRUE);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, TRUE);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "text", COLUMN_LABEL,
                                       NULL);

  gtk_tree_view_append_column (view, column);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, _("Extensions"));

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, TRUE);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "text", COLUMN_EXTENSIONS,
                                       NULL);

  gtk_tree_view_append_column (view, column);

  g_signal_connect (gtk_tree_view_get_selection (view), "changed",
                    G_CALLBACK (picman_file_proc_view_selection_changed),
                    view);

  return GTK_WIDGET (view);
}

PicmanPlugInProcedure *
picman_file_proc_view_get_proc (PicmanFileProcView  *view,
                              gchar            **label)
{
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter       iter;

  g_return_val_if_fail (PICMAN_IS_FILE_PROC_VIEW (view), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      PicmanPlugInProcedure *proc;

      if (label)
        gtk_tree_model_get (model, &iter,
                            COLUMN_PROC,  &proc,
                            COLUMN_LABEL, label,
                            -1);
      else
        gtk_tree_model_get (model, &iter,
                            COLUMN_PROC,  &proc,
                            -1);

      if (proc)
        g_object_unref (proc);

      return proc;
    }

  if (label)
    *label = NULL;

  return NULL;
}

gboolean
picman_file_proc_view_set_proc (PicmanFileProcView    *view,
                              PicmanPlugInProcedure *proc)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gboolean      iter_valid;

  g_return_val_if_fail (PICMAN_IS_FILE_PROC_VIEW (view), FALSE);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

  for (iter_valid = gtk_tree_model_get_iter_first (model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, &iter))
    {
      PicmanPlugInProcedure *this;

      gtk_tree_model_get (model, &iter,
                          COLUMN_PROC, &this,
                          -1);

      if (this)
        g_object_unref (this);

      if (this == proc)
        break;
    }

  if (iter_valid)
    {
      GtkTreeSelection *selection;

      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

      gtk_tree_selection_select_iter (selection, &iter);
    }

  return iter_valid;
}

gchar *
picman_file_proc_view_get_help_id (PicmanFileProcView *view)
{
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter       iter;

  g_return_val_if_fail (PICMAN_IS_FILE_PROC_VIEW (view), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gchar *help_id;

      gtk_tree_model_get (model, &iter,
                          COLUMN_HELP_ID, &help_id,
                          -1);

      return help_id;
    }

  return NULL;
}

static void
picman_file_proc_view_selection_changed (GtkTreeSelection *selection,
                                       PicmanFileProcView *view)
{
  g_signal_emit (view, view_signals[CHANGED], 0);
}
