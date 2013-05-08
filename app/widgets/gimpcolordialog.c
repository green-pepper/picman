/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * color_dialog module (C) 1998 Austin Donnelly <austin@greenend.org.uk>
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
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanmarshal.h"
#include "core/picmanviewable.h"

#include "picmancolordialog.h"
#include "picmandialogfactory.h"
#include "picmanhelp-ids.h"

#include "picman-intl.h"


#define RESPONSE_RESET   1
#define COLOR_AREA_SIZE 20


enum
{
  UPDATE,
  LAST_SIGNAL
};


static void   picman_color_dialog_dispose        (GObject            *object);

static void   picman_color_dialog_response       (GtkDialog          *dialog,
                                                gint                response_id);

static void   picman_color_dialog_help_func      (const gchar        *help_id,
                                                gpointer            help_data);
static void   picman_color_dialog_color_changed  (PicmanColorSelection *selection,
                                                PicmanColorDialog    *dialog);

static void   picman_color_history_color_clicked (GtkWidget          *widget,
                                                PicmanColorDialog    *dialog);
static void   picman_color_history_color_changed (GtkWidget          *widget,
                                                gpointer            data);
static void   picman_color_history_add_clicked   (GtkWidget          *widget,
                                                PicmanColorDialog    *dialog);


G_DEFINE_TYPE (PicmanColorDialog, picman_color_dialog, PICMAN_TYPE_VIEWABLE_DIALOG)

#define parent_class picman_color_dialog_parent_class

static guint color_dialog_signals[LAST_SIGNAL] = { 0, };

static GList *color_dialogs = NULL;


static void
picman_color_dialog_class_init (PicmanColorDialogClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);

  object_class->dispose  = picman_color_dialog_dispose;

  dialog_class->response = picman_color_dialog_response;

  color_dialog_signals[UPDATE] =
    g_signal_new ("update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanColorDialogClass, update),
                  NULL, NULL,
                  picman_marshal_VOID__BOXED_ENUM,
                  G_TYPE_NONE, 2,
                  PICMAN_TYPE_RGB,
                  PICMAN_TYPE_COLOR_DIALOG_STATE);
}

static void
picman_color_dialog_init (PicmanColorDialog *dialog)
{
  GtkWidget *table;
  GtkWidget *button;
  GtkWidget *arrow;
  gint       i;

  color_dialogs = g_list_prepend (color_dialogs, dialog);

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          PICMAN_STOCK_RESET, RESPONSE_RESET,
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          GTK_STOCK_OK,     GTK_RESPONSE_OK,
                          NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           RESPONSE_RESET,
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  dialog->selection = picman_color_selection_new ();
  gtk_container_set_border_width (GTK_CONTAINER (dialog->selection), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      dialog->selection, TRUE, TRUE, 0);
  gtk_widget_show (dialog->selection);

  g_signal_connect (dialog->selection, "color-changed",
                    G_CALLBACK (picman_color_dialog_color_changed),
                    dialog);

  /* The color history */
  table = gtk_table_new (2, 1 + COLOR_HISTORY_SIZE / 2, TRUE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacing (GTK_TABLE (table), 0, 4);
  gtk_box_pack_end (GTK_BOX (PICMAN_COLOR_SELECTION (dialog->selection)->right_vbox),
                    table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  button = gtk_button_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 0, 1);
  picman_help_set_help_data (button,
                           _("Add the current color to the color history"),
                           NULL);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (picman_color_history_add_clicked),
                    dialog);

  arrow = gtk_arrow_new (GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER (button), arrow);
  gtk_widget_show (arrow);

  for (i = 0; i < COLOR_HISTORY_SIZE; i++)
    {
      PicmanRGB history_color;
      gint    row, column;

      column = i % (COLOR_HISTORY_SIZE / 2);
      row    = i / (COLOR_HISTORY_SIZE / 2);

      button = gtk_button_new ();
      gtk_widget_set_size_request (button, COLOR_AREA_SIZE, COLOR_AREA_SIZE);
      gtk_table_attach_defaults (GTK_TABLE (table), button,
                                 column + 1, column + 2, row, row + 1);
      gtk_widget_show (button);

      color_history_get (i, &history_color);

      dialog->history[i] = picman_color_area_new (&history_color,
                                                PICMAN_COLOR_AREA_SMALL_CHECKS,
                                                GDK_BUTTON2_MASK);
      gtk_container_add (GTK_CONTAINER (button), dialog->history[i]);
      gtk_widget_show (dialog->history[i]);

      g_signal_connect (button, "clicked",
                        G_CALLBACK (picman_color_history_color_clicked),
                        dialog);

      g_signal_connect (dialog->history[i], "color-changed",
                        G_CALLBACK (picman_color_history_color_changed),
                        GINT_TO_POINTER (i));
    }
}

static void
picman_color_dialog_dispose (GObject *object)
{
  PicmanColorDialog *dialog = PICMAN_COLOR_DIALOG (object);

  color_dialogs = g_list_remove (color_dialogs, dialog);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_color_dialog_response (GtkDialog *gtk_dialog,
                            gint       response_id)
{
  PicmanColorDialog *dialog = PICMAN_COLOR_DIALOG (gtk_dialog);
  PicmanRGB          color;

  switch (response_id)
    {
    case RESPONSE_RESET:
      picman_color_selection_reset (PICMAN_COLOR_SELECTION (dialog->selection));
      break;

    case GTK_RESPONSE_OK:
      picman_color_history_add_clicked (NULL, dialog);

      picman_color_selection_get_color (PICMAN_COLOR_SELECTION (dialog->selection),
                                      &color);

      g_signal_emit (dialog, color_dialog_signals[UPDATE], 0,
                     &color, PICMAN_COLOR_DIALOG_OK);
      break;

    default:
      picman_color_selection_get_old_color (PICMAN_COLOR_SELECTION (dialog->selection),
                                          &color);

      g_signal_emit (dialog, color_dialog_signals[UPDATE], 0,
                     &color, PICMAN_COLOR_DIALOG_CANCEL);
      break;
    }
}


/*  public functions  */

GtkWidget *
picman_color_dialog_new (PicmanViewable      *viewable,
                       PicmanContext       *context,
                       const gchar       *title,
                       const gchar       *stock_id,
                       const gchar       *desc,
                       GtkWidget         *parent,
                       PicmanDialogFactory *dialog_factory,
                       const gchar       *dialog_identifier,
                       const PicmanRGB     *color,
                       gboolean           wants_updates,
                       gboolean           show_alpha)
{
  PicmanColorDialog *dialog;
  const gchar     *role;

  g_return_val_if_fail (viewable == NULL || PICMAN_IS_VIEWABLE (viewable), NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (parent), NULL);
  g_return_val_if_fail (dialog_factory == NULL ||
                        PICMAN_IS_DIALOG_FACTORY (dialog_factory), NULL);
  g_return_val_if_fail (dialog_factory == NULL || dialog_identifier != NULL,
                        NULL);
  g_return_val_if_fail (color != NULL, NULL);

  if (! context)
    g_warning ("picman_color_dialog_new() called with a NULL context");

  role = dialog_identifier ? dialog_identifier : "picman-color-selector";

  dialog = g_object_new (PICMAN_TYPE_COLOR_DIALOG,
                         "title",       title,
                         "role",        role,
                         "help-func",   picman_color_dialog_help_func,
                         "help-id",     PICMAN_HELP_COLOR_DIALOG,
                         "stock-id",    stock_id,
                         "description", desc,
                         "parent",      parent,
                         NULL);

  if (viewable)
    {
      picman_viewable_dialog_set_viewable (PICMAN_VIEWABLE_DIALOG (dialog),
                                         viewable, context);
    }
  else
    {
      GtkWidget *parent;

      parent = gtk_widget_get_parent (PICMAN_VIEWABLE_DIALOG (dialog)->icon);
      parent = gtk_widget_get_parent (parent);

      gtk_widget_hide (parent);
    }

  dialog->wants_updates = wants_updates;

  if (dialog_factory)
    picman_dialog_factory_add_foreign (dialog_factory, dialog_identifier,
                                     GTK_WIDGET (dialog));

  picman_color_selection_set_show_alpha (PICMAN_COLOR_SELECTION (dialog->selection),
                                       show_alpha);

  if (context)
    {
      g_object_set_data (G_OBJECT (context->picman->config->color_management),
                         "picman-context", context);

      picman_color_selection_set_config (PICMAN_COLOR_SELECTION (dialog->selection),
                                       context->picman->config->color_management);

      g_object_set_data (G_OBJECT (context->picman->config->color_management),
                         "picman-context", NULL);
    }

  picman_color_selection_set_color (PICMAN_COLOR_SELECTION (dialog->selection),
                                  color);
  picman_color_selection_set_old_color (PICMAN_COLOR_SELECTION (dialog->selection),
                                      color);

  return GTK_WIDGET (dialog);
}

void
picman_color_dialog_set_color (PicmanColorDialog *dialog,
                             const PicmanRGB   *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_DIALOG (dialog));
  g_return_if_fail (color != NULL);

  g_signal_handlers_block_by_func (dialog->selection,
                                   picman_color_dialog_color_changed,
                                   dialog);

  picman_color_selection_set_color (PICMAN_COLOR_SELECTION (dialog->selection),
                                  color);
  picman_color_selection_set_old_color (PICMAN_COLOR_SELECTION (dialog->selection),
                                      color);

  g_signal_handlers_unblock_by_func (dialog->selection,
                                     picman_color_dialog_color_changed,
                                     dialog);
}

void
picman_color_dialog_get_color (PicmanColorDialog *dialog,
                             PicmanRGB         *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_DIALOG (dialog));
  g_return_if_fail (color != NULL);

  picman_color_selection_get_color (PICMAN_COLOR_SELECTION (dialog->selection),
                                  color);
}


/*  private functions  */

static void
picman_color_dialog_help_func (const gchar *help_id,
                             gpointer     help_data)
{
  PicmanColorDialog   *dialog = PICMAN_COLOR_DIALOG (help_data);
  PicmanColorNotebook *notebook;

  notebook =
    PICMAN_COLOR_NOTEBOOK (PICMAN_COLOR_SELECTION (dialog->selection)->notebook);

  help_id = PICMAN_COLOR_SELECTOR_GET_CLASS (notebook->cur_page)->help_id;

  picman_standard_help_func (help_id, NULL);
}

static void
picman_color_dialog_color_changed (PicmanColorSelection *selection,
                                 PicmanColorDialog    *dialog)
{
  if (dialog->wants_updates)
    {
      PicmanRGB color;

      picman_color_selection_get_color (selection, &color);

      g_signal_emit (dialog, color_dialog_signals[UPDATE], 0,
                     &color, PICMAN_COLOR_DIALOG_UPDATE);
    }
}


/*  color history callbacks  */

static void
picman_color_history_color_clicked (GtkWidget       *widget,
                                  PicmanColorDialog *dialog)
{
  PicmanColorArea *color_area;
  PicmanRGB        color;

  color_area = PICMAN_COLOR_AREA (gtk_bin_get_child (GTK_BIN (widget)));

  picman_color_area_get_color (color_area, &color);
  picman_color_selection_set_color (PICMAN_COLOR_SELECTION (dialog->selection),
                                  &color);
}

static void
picman_color_history_color_changed (GtkWidget *widget,
                                  gpointer   data)
{
  PicmanRGB  changed_color;
  gint     color_index;
  GList   *list;

  picman_color_area_get_color (PICMAN_COLOR_AREA (widget), &changed_color);

  color_index = GPOINTER_TO_INT (data);

  color_history_set (color_index, &changed_color);

  for (list = color_dialogs; list; list = g_list_next (list))
    {
      PicmanColorDialog *dialog = list->data;

      if (dialog->history[color_index] != widget)
        {
          g_signal_handlers_block_by_func (dialog->history[color_index],
                                           picman_color_history_color_changed,
                                           data);

          picman_color_area_set_color
            (PICMAN_COLOR_AREA (dialog->history[color_index]), &changed_color);

          g_signal_handlers_unblock_by_func (dialog->history[color_index],
                                             picman_color_history_color_changed,
                                             data);
        }
    }
}

static void
picman_color_history_add_clicked (GtkWidget       *widget,
                                PicmanColorDialog *dialog)
{
  PicmanRGB color;
  gint    shift_begin;
  gint    i;

  picman_color_selection_get_color (PICMAN_COLOR_SELECTION (dialog->selection),
                                  &color);

  shift_begin = color_history_add (&color);

  for (i = shift_begin; i >= 0; i--)
    {
      color_history_get (i, &color);

      picman_color_area_set_color (PICMAN_COLOR_AREA (dialog->history[i]), &color);
    }
}
