/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpdbdialog.c
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"

#include "pdb/picmanpdb.h"

#include "picmanmenufactory.h"
#include "picmanpdbdialog.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_PDB,
  PROP_CONTEXT,
  PROP_SELECT_TYPE,
  PROP_INITIAL_OBJECT,
  PROP_CALLBACK_NAME,
  PROP_MENU_FACTORY
};


static void   picman_pdb_dialog_class_init      (PicmanPdbDialogClass *klass);
static void   picman_pdb_dialog_init            (PicmanPdbDialog      *dialog,
                                               PicmanPdbDialogClass *klass);

static void   picman_pdb_dialog_constructed     (GObject            *object);
static void   picman_pdb_dialog_dispose         (GObject            *object);
static void   picman_pdb_dialog_set_property    (GObject            *object,
                                               guint               property_id,
                                               const GValue       *value,
                                               GParamSpec         *pspec);

static void   picman_pdb_dialog_response        (GtkDialog          *dialog,
                                               gint                response_id);

static void   picman_pdb_dialog_context_changed (PicmanContext        *context,
                                               PicmanObject         *object,
                                               PicmanPdbDialog      *dialog);
static void   picman_pdb_dialog_plug_in_closed  (PicmanPlugInManager  *manager,
                                               PicmanPlugIn         *plug_in,
                                               PicmanPdbDialog      *dialog);


static PicmanDialogClass *parent_class = NULL;


GType
picman_pdb_dialog_get_type (void)
{
  static GType dialog_type = 0;

  if (! dialog_type)
    {
      const GTypeInfo dialog_info =
      {
        sizeof (PicmanPdbDialogClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) picman_pdb_dialog_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (PicmanPdbDialog),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) picman_pdb_dialog_init,
      };

      dialog_type = g_type_register_static (PICMAN_TYPE_DIALOG,
                                            "PicmanPdbDialog",
                                            &dialog_info,
                                            G_TYPE_FLAG_ABSTRACT);
    }

  return dialog_type;
}

static void
picman_pdb_dialog_class_init (PicmanPdbDialogClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->constructed  = picman_pdb_dialog_constructed;
  object_class->dispose      = picman_pdb_dialog_dispose;
  object_class->set_property = picman_pdb_dialog_set_property;
  object_class->set_property = picman_pdb_dialog_set_property;

  dialog_class->response     = picman_pdb_dialog_response;

  klass->run_callback        = NULL;

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context", NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_PDB,
                                   g_param_spec_object ("pdb", NULL, NULL,
                                                        PICMAN_TYPE_PDB,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_SELECT_TYPE,
                                   g_param_spec_gtype ("select-type",
                                                       NULL, NULL,
                                                       PICMAN_TYPE_OBJECT,
                                                       PICMAN_PARAM_WRITABLE |
                                                       G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_INITIAL_OBJECT,
                                   g_param_spec_object ("initial-object",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_OBJECT,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CALLBACK_NAME,
                                   g_param_spec_string ("callback-name",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_MENU_FACTORY,
                                   g_param_spec_object ("menu-factory",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_MENU_FACTORY,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_pdb_dialog_init (PicmanPdbDialog      *dialog,
                      PicmanPdbDialogClass *klass)
{
  klass->dialogs = g_list_prepend (klass->dialogs, dialog);

  gtk_dialog_add_button (GTK_DIALOG (dialog),
                         GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
}

static void
picman_pdb_dialog_constructed (GObject *object)
{
  PicmanPdbDialog *dialog = PICMAN_PDB_DIALOG (object);
  const gchar   *signal_name;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PDB (dialog->pdb));
  g_assert (PICMAN_IS_CONTEXT (dialog->caller_context));
  g_assert (g_type_is_a (dialog->select_type, PICMAN_TYPE_OBJECT));

  dialog->context = picman_context_new (dialog->caller_context->picman,
                                      G_OBJECT_TYPE_NAME (object),
                                      NULL);

  picman_context_set_by_type (dialog->context, dialog->select_type,
                            dialog->initial_object);

  dialog->initial_object = NULL;

  signal_name = picman_context_type_to_signal_name (dialog->select_type);

  g_signal_connect_object (dialog->context, signal_name,
                           G_CALLBACK (picman_pdb_dialog_context_changed),
                           dialog, 0);
  g_signal_connect_object (dialog->context->picman->plug_in_manager,
                           "plug-in-closed",
                           G_CALLBACK (picman_pdb_dialog_plug_in_closed),
                           dialog, 0);
}

static void
picman_pdb_dialog_dispose (GObject *object)
{
  PicmanPdbDialog      *dialog = PICMAN_PDB_DIALOG (object);
  PicmanPdbDialogClass *klass  = PICMAN_PDB_DIALOG_GET_CLASS (object);

  klass->dialogs = g_list_remove (klass->dialogs, object);

  if (dialog->pdb)
    {
      g_object_unref (dialog->pdb);
      dialog->pdb = NULL;
    }

  if (dialog->caller_context)
    {
      g_object_unref (dialog->caller_context);
      dialog->caller_context = NULL;
    }

  if (dialog->context)
    {
      g_object_unref (dialog->context);
      dialog->context = NULL;
    }

  if (dialog->callback_name)
    {
      g_free (dialog->callback_name);
      dialog->callback_name = NULL;
    }

  if (dialog->menu_factory)
    {
      g_object_unref (dialog->menu_factory);
      dialog->menu_factory = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_pdb_dialog_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanPdbDialog *dialog = PICMAN_PDB_DIALOG (object);

  switch (property_id)
    {
    case PROP_PDB:
      dialog->pdb = g_value_dup_object (value);
      break;

    case PROP_CONTEXT:
      dialog->caller_context = g_value_dup_object (value);
      break;

    case PROP_SELECT_TYPE:
      dialog->select_type = g_value_get_gtype (value);
      break;

    case PROP_INITIAL_OBJECT:
      /* don't ref, see constructor */
      dialog->initial_object = g_value_get_object (value);
      break;

    case PROP_CALLBACK_NAME:
      if (dialog->callback_name)
        g_free (dialog->callback_name);
      dialog->callback_name = g_value_dup_string (value);
      break;

    case PROP_MENU_FACTORY:
      dialog->menu_factory = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_pdb_dialog_response (GtkDialog *gtk_dialog,
                          gint       response_id)
{
  PicmanPdbDialog *dialog = PICMAN_PDB_DIALOG (gtk_dialog);

  picman_pdb_dialog_run_callback (dialog, TRUE);
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
picman_pdb_dialog_run_callback (PicmanPdbDialog *dialog,
                              gboolean       closing)
{
  PicmanPdbDialogClass *klass = PICMAN_PDB_DIALOG_GET_CLASS (dialog);
  PicmanObject         *object;

  object = picman_context_get_by_type (dialog->context, dialog->select_type);

  if (object                &&
      klass->run_callback   &&
      dialog->callback_name &&
      ! dialog->callback_busy)
    {
      dialog->callback_busy = TRUE;

      if (picman_pdb_lookup_procedure (dialog->pdb, dialog->callback_name))
        {
          PicmanValueArray *return_vals;
          GError         *error = NULL;

          return_vals = klass->run_callback (dialog, object, closing, &error);

          if (g_value_get_enum (picman_value_array_index (return_vals, 0)) !=
              PICMAN_PDB_SUCCESS)
            {
              picman_message (dialog->context->picman, G_OBJECT (dialog),
                            PICMAN_MESSAGE_ERROR,
                            _("Unable to run %s callback. "
                              "The corresponding plug-in may have "
                              "crashed."),
                            g_type_name (G_TYPE_FROM_INSTANCE (dialog)));
            }
          else if (error)
            {
              picman_message_literal (dialog->context->picman, G_OBJECT (dialog),
				    PICMAN_MESSAGE_ERROR,
				    error->message);
              g_error_free (error);
            }

          picman_value_array_unref (return_vals);
        }

      dialog->callback_busy = FALSE;
    }
}

PicmanPdbDialog *
picman_pdb_dialog_get_by_callback (PicmanPdbDialogClass *klass,
                                 const gchar        *callback_name)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_PDB_DIALOG_CLASS (klass), NULL);
  g_return_val_if_fail (callback_name != NULL, NULL);

  for (list = klass->dialogs; list; list = g_list_next (list))
    {
      PicmanPdbDialog *dialog = list->data;

      if (dialog->callback_name &&
          ! strcmp (callback_name, dialog->callback_name))
        return dialog;
    }

  return NULL;
}


/*  private functions  */

static void
picman_pdb_dialog_context_changed (PicmanContext   *context,
                                 PicmanObject    *object,
                                 PicmanPdbDialog *dialog)
{
  if (object)
    picman_pdb_dialog_run_callback (dialog, FALSE);
}

static void
picman_pdb_dialog_plug_in_closed (PicmanPlugInManager *manager,
                                PicmanPlugIn        *plug_in,
                                PicmanPdbDialog     *dialog)
{
  if (dialog->caller_context && dialog->callback_name)
    {
      if (! picman_pdb_lookup_procedure (dialog->pdb, dialog->callback_name))
        {
          gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);
        }
    }
}
