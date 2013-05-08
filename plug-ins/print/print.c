/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "print.h"
#include "print-settings.h"
#include "print-page-layout.h"
#include "print-page-setup.h"
#include "print-draw-page.h"

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_BINARY       "print"
#define PLUG_IN_ROLE         "picman-print"
#define PRINT_PROC_NAME      "file-print-gtk"

#ifndef EMBED_PAGE_SETUP
#define PAGE_SETUP_PROC_NAME "file-print-gtk-page-setup"
#define PRINT_TEMP_PROC_NAME "file-print-gtk-page-setup-notify-temp"
#endif


static void        query (void);
static void        run   (const gchar       *name,
                          gint               nparams,
                          const PicmanParam   *param,
                          gint              *nreturn_vals,
                          PicmanParam        **return_vals);

static PicmanPDBStatusType  print_image       (gint32             image_ID,
                                             gboolean           interactive,
                                             GError           **error);
#ifndef EMBED_PAGE_SETUP
static PicmanPDBStatusType  page_setup        (gint32             image_ID);
#endif

static void        print_show_error         (const gchar       *message);
static void        print_operation_set_name (GtkPrintOperation *operation,
                                             gint               image_ID);

static void        begin_print              (GtkPrintOperation *operation,
                                             GtkPrintContext   *context,
                                             PrintData         *data);
static void        end_print                (GtkPrintOperation *operation,
                                             GtkPrintContext   *context,
                                             gint32            *layer_ID);
static void        draw_page                (GtkPrintOperation *print,
                                             GtkPrintContext   *context,
                                             gint               page_nr,
                                             PrintData         *data);

static GtkWidget * create_custom_widget     (GtkPrintOperation *operation,
                                             PrintData         *data);

#ifndef EMBED_PAGE_SETUP
static gchar     * print_temp_proc_name     (gint32             image_ID);
static gchar     * print_temp_proc_install  (gint32             image_ID);

/*  Keep a reference to the current GtkPrintOperation
 *  for access by the temporary procedure.
 */
static GtkPrintOperation *print_operation = NULL;
#endif


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

MAIN ()

static void
query (void)
{
  static const PicmanParamDef print_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode", "The run mode { RUN-INTERACTIVE (0) }" },
    { PICMAN_PDB_IMAGE,    "image",    "Image to print"                       }
  };

  picman_install_procedure (PRINT_PROC_NAME,
                          N_("Print the image"),
                          "Print the image using the GTK+ Print API.",
                          "Bill Skaggs, Sven Neumann, Stefan Röllin",
                          "Bill Skaggs <weskaggs@primate.ucdavis.edu>",
                          "2006 - 2008",
                          N_("_Print..."),
                          "*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (print_args), 0,
                          print_args, NULL);

  picman_plugin_menu_register (PRINT_PROC_NAME, "<Image>/File/Send");
  picman_plugin_icon_register (PRINT_PROC_NAME, PICMAN_ICON_TYPE_STOCK_ID,
                             (const guint8 *) GTK_STOCK_PRINT);

#ifndef EMBED_PAGE_SETUP
  picman_install_procedure (PAGE_SETUP_PROC_NAME,
                          N_("Adjust page size and orientation for printing"),
                          "Adjust page size and orientation for printing the "
                          "image using the GTK+ Print API.",
                          "Bill Skaggs, Sven Neumann, Stefan Röllin",
                          "Sven Neumann <sven@picman.org>",
                          "2008",
                          N_("Page Set_up"),
                          "*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (print_args), 0,
                          print_args, NULL);

  picman_plugin_menu_register (PAGE_SETUP_PROC_NAME, "<Image>/File/Send");
  picman_plugin_icon_register (PAGE_SETUP_PROC_NAME, PICMAN_ICON_TYPE_STOCK_ID,
                             (const guint8 *) GTK_STOCK_PAGE_SETUP);
#endif
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[2];
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status;
  gint32             image_ID;
  GError            *error = NULL;

  INIT_I18N ();
  gegl_init (NULL, NULL);

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

  image_ID = param[1].data.d_int32;

  if (strcmp (name, PRINT_PROC_NAME) == 0)
    {
      status = print_image (image_ID, run_mode == PICMAN_RUN_INTERACTIVE, &error);

      if (error && run_mode == PICMAN_RUN_INTERACTIVE)
        {
          print_show_error (error->message);
        }
    }
#ifndef EMBED_PAGE_SETUP
  else if (strcmp (name, PAGE_SETUP_PROC_NAME) == 0)
    {
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        {
          status = page_setup (image_ID);
        }
      else
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
    }
#endif
  else
    {
      status = PICMAN_PDB_CALLING_ERROR;
    }

  if (status != PICMAN_PDB_SUCCESS && error)
    {
      *nreturn_vals = 2;
      values[1].type          = PICMAN_PDB_STRING;
      values[1].data.d_string = error->message;
    }

  values[0].data.d_status = status;
}

static PicmanPDBStatusType
print_image (gint32     image_ID,
             gboolean   interactive,
             GError   **error)
{
  GtkPrintOperation       *operation;
  GtkPrintOperationResult  result;
  gint32                   layer;
  PrintData                data;
#ifndef EMBED_PAGE_SETUP
  gchar                   *temp_proc;
#endif

  /*  create a print layer from the projection  */
  layer = picman_layer_new_from_visible (image_ID, image_ID, PRINT_PROC_NAME);

  operation = gtk_print_operation_new ();

  gtk_print_operation_set_n_pages (operation, 1);
  print_operation_set_name (operation, image_ID);

  print_page_setup_load (operation, image_ID);

  /* fill in the PrintData struct */
  data.image_id        = image_ID;
  data.drawable_id     = layer;
  data.unit            = picman_get_default_unit ();
  data.image_unit      = picman_image_get_unit (image_ID);
  data.offset_x        = 0;
  data.offset_y        = 0;
  data.center          = CENTER_BOTH;
  data.use_full_page   = FALSE;
  data.draw_crop_marks = FALSE;
  data.operation       = operation;

  picman_image_get_resolution (image_ID, &data.xres, &data.yres);

  print_settings_load (&data);

  gtk_print_operation_set_unit (operation, GTK_UNIT_PIXEL);

  g_signal_connect (operation, "begin-print",
                    G_CALLBACK (begin_print),
                    &data);
  g_signal_connect (operation, "draw-page",
                    G_CALLBACK (draw_page),
                    &data);
  g_signal_connect (operation, "end-print",
                    G_CALLBACK (end_print),
                    &layer);

#ifndef EMBED_PAGE_SETUP
  print_operation = operation;
  temp_proc = print_temp_proc_install (image_ID);
  picman_extension_enable ();
#endif

  if (interactive)
    {
      picman_ui_init (PLUG_IN_BINARY, FALSE);

      g_signal_connect_swapped (operation, "end-print",
                                G_CALLBACK (print_settings_save),
                                &data);

      g_signal_connect (operation, "create-custom-widget",
                        G_CALLBACK (create_custom_widget),
                        &data);

      gtk_print_operation_set_custom_tab_label (operation, _("Image Settings"));

#ifdef EMBED_PAGE_SETUP
      gtk_print_operation_set_embed_page_setup (operation, TRUE);
#endif

      result = gtk_print_operation_run (operation,
                                        GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                        NULL, error);

      if (result == GTK_PRINT_OPERATION_RESULT_APPLY ||
          result == GTK_PRINT_OPERATION_RESULT_IN_PROGRESS)
        {
          print_page_setup_save (operation, image_ID);
        }
    }
  else
    {
      result = gtk_print_operation_run (operation,
                                        GTK_PRINT_OPERATION_ACTION_PRINT,
                                        NULL, error);
    }

#ifndef EMBED_PAGE_SETUP
  picman_uninstall_temp_proc (temp_proc);
  g_free (temp_proc);
  print_operation = NULL;
#endif

  g_object_unref (operation);

  if (picman_item_is_valid (layer))
    picman_item_delete (layer);

  switch (result)
    {
    case GTK_PRINT_OPERATION_RESULT_APPLY:
    case GTK_PRINT_OPERATION_RESULT_IN_PROGRESS:
      return PICMAN_PDB_SUCCESS;

    case GTK_PRINT_OPERATION_RESULT_CANCEL:
      return PICMAN_PDB_CANCEL;

    case GTK_PRINT_OPERATION_RESULT_ERROR:
      return PICMAN_PDB_EXECUTION_ERROR;
    }

  return PICMAN_PDB_EXECUTION_ERROR;
}

#ifndef EMBED_PAGE_SETUP
static PicmanPDBStatusType
page_setup (gint32 image_ID)
{
  GtkPrintOperation  *operation;
  PicmanParam          *return_vals;
  gchar              *name;
  gint                n_return_vals;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  operation = gtk_print_operation_new ();

  print_page_setup_load (operation, image_ID);
  print_page_setup_dialog (operation);
  print_page_setup_save (operation, image_ID);

  g_object_unref (operation);

  /* now notify a running print procedure about this change */
  name = print_temp_proc_name (image_ID);

  /* we don't want the core to show an error message if the
   * temporary procedure does not exist
   */
  picman_plugin_set_pdb_error_handler (PICMAN_PDB_ERROR_HANDLER_PLUGIN);

  return_vals = picman_run_procedure (name,
                                    &n_return_vals,
                                    PICMAN_PDB_IMAGE, image_ID,
                                    PICMAN_PDB_END);
  picman_destroy_params (return_vals, n_return_vals);

  g_free (name);

  return PICMAN_PDB_SUCCESS;
}
#endif

static void
print_show_error (const gchar *message)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (NULL, 0,
                                   GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_OK,
				   "%s",
                                   _("An error occurred while trying to print:"));

  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                            "%s", message);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

static void
print_operation_set_name (GtkPrintOperation *operation,
                          gint               image_ID)
{
  gchar *name = picman_image_get_name (image_ID);

  gtk_print_operation_set_job_name (operation, name);

  g_free (name);
}

static void
begin_print (GtkPrintOperation *operation,
             GtkPrintContext   *context,
             PrintData         *data)
{
  gtk_print_operation_set_use_full_page (operation, data->use_full_page);

  picman_progress_init (_("Printing"));
}

static void
end_print (GtkPrintOperation *operation,
           GtkPrintContext   *context,
           gint32            *layer_ID)
{
  /* we don't need the print layer any longer, delete it */
  if (picman_item_is_valid (*layer_ID))
    {
      picman_item_delete (*layer_ID);
      *layer_ID = -1;
    }

  picman_progress_end ();

  /* generate events to solve the problems described in bug #466928 */
  g_timeout_add_seconds (1, (GSourceFunc) gtk_true, NULL);
}

static void
draw_page (GtkPrintOperation *operation,
           GtkPrintContext   *context,
           gint               page_nr,
           PrintData         *data)
{
  print_draw_page (context, data);

  picman_progress_update (1.0);
}

/*
 * This callback creates a "custom" widget that gets inserted into the
 * print operation dialog.
 */
static GtkWidget *
create_custom_widget (GtkPrintOperation *operation,
                      PrintData         *data)
{
  return print_page_layout_gui (data, PRINT_PROC_NAME);
}

#ifndef EMBED_PAGE_SETUP
static void
print_temp_proc_run (const gchar      *name,
                     gint              nparams,
                     const PicmanParam  *param,
                     gint             *nreturn_vals,
                     PicmanParam       **return_vals)
{
  static PicmanParam  values[1];

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_SUCCESS;

  *nreturn_vals = 1;
  *return_vals  = values;

  if (print_operation && nparams == 1)
    print_page_setup_load (print_operation, param[0].data.d_int32);
}

static gchar *
print_temp_proc_name (gint32 image_ID)
{
  return g_strdup_printf (PRINT_TEMP_PROC_NAME "-%d", image_ID);
}

static gchar *
print_temp_proc_install (gint32  image_ID)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_IMAGE, "image", "Image to print" }
  };

  gchar *name = print_temp_proc_name (image_ID);

  picman_install_temp_proc (name,
                          "DON'T USE THIS ONE",
                          "Temporary procedure to notify the Print plug-in "
                          "about changes to the Page Setup.",
                         "Sven Neumann",
                         "Sven Neumann",
                         "2008",
                          NULL,
                          "",
                          PICMAN_TEMPORARY,
                          G_N_ELEMENTS (args), 0, args, NULL,
                          print_temp_proc_run);

  return name;
}
#endif
