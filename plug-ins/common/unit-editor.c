/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This is a plug-in for PICMAN.
 *
 * Copyright (C) 2000 Michael Natterer <mitch@picman.org>
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

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC     "plug-in-unit-editor"
#define PLUG_IN_BINARY   "unit-editor"
#define PLUG_IN_ROLE     "picman-unit-editor"
#define RESPONSE_REFRESH 1

enum
{
  SAVE,
  IDENTIFIER,
  FACTOR,
  DIGITS,
  SYMBOL,
  ABBREVIATION,
  SINGULAR,
  PLURAL,
  UNIT,
  USER_UNIT,
  BG_COLOR,
  NUM_COLUMNS
};

typedef struct
{
  const gchar *title;
  const gchar *help;

} UnitColumn;


static void     query                  (void);
static void     run                    (const gchar           *name,
                                        gint                   n_params,
                                        const PicmanParam       *param,
                                        gint                  *n_return_vals,
                                        PicmanParam            **return_vals);

static PicmanUnit new_unit_dialog        (GtkWidget             *main_dialog,
                                        PicmanUnit               template);
static void     unit_editor_dialog     (void);
static void     unit_editor_response   (GtkWidget             *widget,
                                        gint                   response_id,
                                        gpointer               data);
static void     new_callback           (GtkAction             *action,
                                        GtkTreeView           *tv);
static void     duplicate_callback     (GtkAction             *action,
                                        GtkTreeView           *tv);
static void     saved_toggled_callback (GtkCellRendererToggle *celltoggle,
                                        gchar                 *path_string,
                                        GtkListStore          *list_store);
static void     unit_list_init         (GtkTreeView           *tv);


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static const UnitColumn columns[] =
{
  { N_("Saved"),        N_("A unit definition will only be saved before "
                           "PICMAN exits if this column is checked.")         },
  { N_("ID"),           N_("This string will be used to identify a "
                           "unit in PICMAN's configuration files.")           },
  { N_("Factor"),       N_("How many units make up an inch.")               },
  { N_("Digits"),       N_("This field is a hint for numerical input "
                           "fields. It specifies how many decimal digits "
                           "the input field should provide to get "
                           "approximately the same accuracy as an "
                           "\"inch\" input field with two decimal digits.") },
  { N_("Symbol"),       N_("The unit's symbol if it has one (e.g. \" "
                           "for inches). The unit's abbreviation is used "
                           "if doesn't have a symbol.")                     },
  { N_("Abbreviation"), N_("The unit's abbreviation (e.g. \"cm\" for "
                           "centimeters).")                                 },
  { N_("Singular"),     N_("The unit's singular form.")                     },
  { N_("Plural"),       N_("The unit's plural form.")                       }
};

static GtkActionEntry actions[] =
{
  { "unit-editor-toolbar", NULL,
    "Unit Editor Toolbar", NULL, NULL, NULL
  },

  { "unit-editor-new", GTK_STOCK_NEW,
    NULL, "<control>N",
    N_("Create a new unit from scratch"),
    G_CALLBACK (new_callback)
  },

  { "unit-editor-duplicate", PICMAN_STOCK_DUPLICATE,
    NULL,  "<control>D",
    N_("Create a new unit using the currently selected unit as template"),
    G_CALLBACK (duplicate_callback)
  }
};


MAIN ()


static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32, "run-mode", "The run mode { RUN-INTERACTIVE (0) }" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Create or alter units used in PICMAN"),
                          "The PICMAN unit editor",
                          "Michael Natterer <mitch@picman.org>",
                          "Michael Natterer <mitch@picman.org>",
                          "2000",
                          N_("U_nits"),
                          "",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Edit/Preferences");
  picman_plugin_icon_register (PLUG_IN_PROC, PICMAN_ICON_TYPE_STOCK_ID,
                             (const guint8 *) PICMAN_STOCK_TOOL_MEASURE);
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam values[2];

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_CALLING_ERROR;

  if (strcmp (name, PLUG_IN_PROC) == 0)
    {
      values[0].data.d_status = PICMAN_PDB_SUCCESS;

      unit_editor_dialog ();
    }
}

static PicmanUnit
new_unit_dialog (GtkWidget *main_dialog,
                 PicmanUnit   template)
{
  GtkWidget *dialog;
  GtkWidget *table;
  GtkWidget *entry;
  GtkWidget *spinbutton;

  GtkWidget *identifier_entry;
  GtkObject *factor_adj;
  GtkObject *digits_adj;
  GtkWidget *symbol_entry;
  GtkWidget *abbreviation_entry;
  GtkWidget *singular_entry;
  GtkWidget *plural_entry;

  PicmanUnit   unit = PICMAN_UNIT_PIXEL;

  dialog = picman_dialog_new (_("Add a New Unit"), PLUG_IN_ROLE,
                            main_dialog, GTK_DIALOG_MODAL,
                            picman_standard_help_func, PLUG_IN_PROC,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_ADD,    GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  table = gtk_table_new (7, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_container_set_border_width (GTK_CONTAINER (table), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  entry = identifier_entry = gtk_entry_new ();
  if (template != PICMAN_UNIT_PIXEL)
    {
      gtk_entry_set_text (GTK_ENTRY (entry),
                          picman_unit_get_identifier (template));
    }
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("_ID:"), 0.0, 0.5,
                             entry, 1, FALSE);

  picman_help_set_help_data (entry, gettext (columns[IDENTIFIER].help), NULL);

  spinbutton = picman_spin_button_new (&factor_adj,
                                     (template != PICMAN_UNIT_PIXEL) ?
                                     picman_unit_get_factor (template) : 1.0,
                                     PICMAN_MIN_RESOLUTION, PICMAN_MAX_RESOLUTION,
                                     0.01, 0.1, 0.0, 0.01, 5);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             _("_Factor:"), 0.0, 0.5,
                             spinbutton, 1, TRUE);

  picman_help_set_help_data (spinbutton, gettext (columns[FACTOR].help), NULL);

  spinbutton = picman_spin_button_new (&digits_adj,
                                     (template != PICMAN_UNIT_PIXEL) ?
                                     picman_unit_get_digits (template) : 2.0,
                                     0, 5, 1, 1, 0, 1, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 2,
                             _("_Digits:"), 0.0, 0.5,
                             spinbutton, 1, TRUE);

  picman_help_set_help_data (spinbutton, gettext (columns[DIGITS].help), NULL);

  entry = symbol_entry = gtk_entry_new ();
  if (template != PICMAN_UNIT_PIXEL)
    {
      gtk_entry_set_text (GTK_ENTRY (entry),
                          picman_unit_get_symbol (template));
    }
  picman_table_attach_aligned (GTK_TABLE (table), 0, 3,
                             _("_Symbol:"), 0.0, 0.5,
                             entry, 1, FALSE);

  picman_help_set_help_data (entry, gettext (columns[SYMBOL].help), NULL);

  entry = abbreviation_entry = gtk_entry_new ();
  if (template != PICMAN_UNIT_PIXEL)
    {
      gtk_entry_set_text (GTK_ENTRY (entry),
                          picman_unit_get_abbreviation (template));
    }
  picman_table_attach_aligned (GTK_TABLE (table), 0, 4,
                             _("_Abbreviation:"), 0.0, 0.5,
                             entry, 1, FALSE);

  picman_help_set_help_data (entry, gettext (columns[ABBREVIATION].help), NULL);

  entry = singular_entry = gtk_entry_new ();
  if (template != PICMAN_UNIT_PIXEL)
    {
      gtk_entry_set_text (GTK_ENTRY (entry),
                          picman_unit_get_singular (template));
    }
  picman_table_attach_aligned (GTK_TABLE (table), 0, 5,
                             _("Si_ngular:"), 0.0, 0.5,
                             entry, 1, FALSE);

  picman_help_set_help_data (entry, gettext (columns[SINGULAR].help), NULL);

  entry = plural_entry = gtk_entry_new ();
  if (template != PICMAN_UNIT_PIXEL)
    {
      gtk_entry_set_text (GTK_ENTRY (entry),
                          picman_unit_get_plural (template));
    }
  picman_table_attach_aligned (GTK_TABLE (table), 0, 6,
                             _("_Plural:"), 0.0, 0.5,
                             entry, 1, FALSE);

  picman_help_set_help_data (entry, gettext (columns[PLURAL].help), NULL);

  gtk_widget_show (dialog);

  while (TRUE)
    {
      gchar   *identifier;
      gdouble  factor;
      gint     digits;
      gchar   *symbol;
      gchar   *abbreviation;
      gchar   *singular;
      gchar   *plural;

      if (picman_dialog_run (PICMAN_DIALOG (dialog)) != GTK_RESPONSE_OK)
        break;

      identifier   = g_strdup (gtk_entry_get_text (GTK_ENTRY (identifier_entry)));
      factor       = gtk_adjustment_get_value (GTK_ADJUSTMENT (factor_adj));
      digits       = gtk_adjustment_get_value (GTK_ADJUSTMENT (digits_adj));
      symbol       = g_strdup (gtk_entry_get_text (GTK_ENTRY (symbol_entry)));
      abbreviation = g_strdup (gtk_entry_get_text (GTK_ENTRY (abbreviation_entry)));
      singular     = g_strdup (gtk_entry_get_text (GTK_ENTRY (singular_entry)));
      plural       = g_strdup (gtk_entry_get_text (GTK_ENTRY (plural_entry)));

      identifier   = g_strstrip (identifier);
      symbol       = g_strstrip (symbol);
      abbreviation = g_strstrip (abbreviation);
      singular     = g_strstrip (singular);
      plural       = g_strstrip (plural);

      if (!strlen (identifier) |
          !strlen (symbol) |
          !strlen (abbreviation) |
          !strlen (singular) |
          !strlen (plural))
        {
          GtkWidget *msg = gtk_message_dialog_new (GTK_WINDOW (dialog), 0,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_OK,
                                                   _("Incomplete input"));

          gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (msg),
                                                    _("Please fill in all text fields."));
          gtk_dialog_run (GTK_DIALOG (msg));
          gtk_widget_destroy (msg);

          continue;
        }

      unit = picman_unit_new (identifier,
                            factor, digits,
                            symbol, abbreviation, singular, plural);

      g_free (identifier);
      g_free (symbol);
      g_free (abbreviation);
      g_free (singular);
      g_free (plural);

      break;
    }

  gtk_widget_destroy (dialog);

  return unit;
}

static void
unit_editor_dialog (void)
{
  GtkWidget         *dialog;
  GtkWidget         *scrolled_win;
  GtkUIManager      *ui_manager;
  GtkActionGroup    *group;
  GtkWidget         *toolbar;
  GtkListStore      *list_store;
  GtkWidget         *tv;
  GtkTreeViewColumn *col;
  GtkWidget         *col_widget;
  GtkWidget         *button;
  GtkCellRenderer   *rend;
  gint               i;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  list_store = gtk_list_store_new (NUM_COLUMNS,
                                   G_TYPE_BOOLEAN,   /*  SAVE          */
                                   G_TYPE_STRING,    /*  IDENTIFIER    */
                                   G_TYPE_DOUBLE,    /*  FACTOR        */
                                   G_TYPE_INT,       /*  DIGITS        */
                                   G_TYPE_STRING,    /*  SYMBOL        */
                                   G_TYPE_STRING,    /*  ABBREVIATION  */
                                   G_TYPE_STRING,    /*  SINGULAR      */
                                   G_TYPE_STRING,    /*  PLURAL        */
                                   PICMAN_TYPE_UNIT,   /*  UNIT          */
                                   G_TYPE_BOOLEAN,   /*  USER_UNIT     */
                                   GDK_TYPE_COLOR);  /*  BG_COLOR      */

  tv = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  g_object_unref (list_store);

  dialog = picman_dialog_new (_("Unit Editor"), PLUG_IN_ROLE,
                            NULL, 0,
                            picman_standard_help_func, PLUG_IN_PROC,

                            GTK_STOCK_REFRESH, RESPONSE_REFRESH,
                            GTK_STOCK_CLOSE,   GTK_RESPONSE_CLOSE,

                            NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (unit_editor_response),
                    tv);
  g_signal_connect (dialog, "destroy",
                    G_CALLBACK (gtk_main_quit),
                    NULL);

  /*  the toolbar  */
  ui_manager = gtk_ui_manager_new ();

  group = gtk_action_group_new ("unit-editor");

  gtk_action_group_set_translation_domain (group, NULL);
  gtk_action_group_add_actions (group, actions, G_N_ELEMENTS (actions), tv);

  gtk_window_add_accel_group (GTK_WINDOW (dialog),
                              gtk_ui_manager_get_accel_group (ui_manager));
  gtk_accel_group_lock (gtk_ui_manager_get_accel_group (ui_manager));

  gtk_ui_manager_insert_action_group (ui_manager, group, -1);
  g_object_unref (group);

  gtk_ui_manager_add_ui_from_string
    (ui_manager,
     "<ui>\n"
     "  <toolbar action=\"unit-editor-toolbar\">\n"
     "    <toolitem action=\"unit-editor-new\" />\n"
     "    <toolitem action=\"unit-editor-duplicate\" />\n"
     "  </toolbar>\n"
     "</ui>\n",
     -1, NULL);

  toolbar = gtk_ui_manager_get_widget (ui_manager, "/unit-editor-toolbar");
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      toolbar, FALSE, FALSE, 0);
  gtk_widget_show (toolbar);

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_win),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                  GTK_POLICY_NEVER,
                                  GTK_POLICY_ALWAYS);
  gtk_container_set_border_width (GTK_CONTAINER (scrolled_win), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      scrolled_win, TRUE, TRUE, 0);
  gtk_widget_show (scrolled_win);

  gtk_widget_set_size_request (tv, -1, 220);
  gtk_container_add (GTK_CONTAINER (scrolled_win), tv);
  gtk_widget_show (tv);

  rend = gtk_cell_renderer_toggle_new ();
  col =
    gtk_tree_view_column_new_with_attributes (gettext (columns[SAVE].title),
                                              rend,
                                              "active",              SAVE,
                                              "activatable",         USER_UNIT,
                                              "cell-background-gdk", BG_COLOR,
                                              NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);

  col_widget = gtk_tree_view_column_get_widget (col);
  if (col_widget)
    {
      button = gtk_widget_get_ancestor (col_widget, GTK_TYPE_BUTTON);

      if (button)
        picman_help_set_help_data (button,
                                 gettext (columns[SAVE].help), NULL);
    }

  g_signal_connect (rend, "toggled",
                    G_CALLBACK (saved_toggled_callback),
                    list_store);

  for (i = 0; i < G_N_ELEMENTS (columns); i++)
    {
      if (i == SAVE)
        continue;

      col =
        gtk_tree_view_column_new_with_attributes (gettext (columns[i].title),
                                                  gtk_cell_renderer_text_new (),
                                                  "text",                i,
                                                  "cell-background-gdk", BG_COLOR,
                                                  NULL);

      gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);

      col_widget = gtk_tree_view_column_get_widget (col);
      if (col_widget)
        {
          button = gtk_widget_get_ancestor (col_widget, GTK_TYPE_BUTTON);

          if (button)
            picman_help_set_help_data (button, gettext (columns[i].help), NULL);
        }
    }

  unit_list_init (GTK_TREE_VIEW (tv));

  gtk_widget_show (dialog);

  gtk_main ();
}

static void
unit_editor_response (GtkWidget *widget,
                      gint       response_id,
                      gpointer   data)
{
  switch (response_id)
    {
    case RESPONSE_REFRESH:
      unit_list_init (GTK_TREE_VIEW (data));
      break;

    default:
      gtk_widget_destroy (widget);
      break;
    }
}

static void
new_callback (GtkAction   *action,
              GtkTreeView *tv)
{
  PicmanUnit  unit;

  unit = new_unit_dialog (gtk_widget_get_toplevel (GTK_WIDGET (tv)),
                          PICMAN_UNIT_PIXEL);

  if (unit != PICMAN_UNIT_PIXEL)
    {
      GtkTreeModel *model;
      GtkTreeIter   iter;

      unit_list_init (tv);

      model = gtk_tree_view_get_model (tv);

      if (gtk_tree_model_get_iter_first (model, &iter) &&
          gtk_tree_model_iter_nth_child (model, &iter,
                                         NULL, unit - PICMAN_UNIT_INCH))
        {
          GtkAdjustment *adj;

          gtk_tree_selection_select_iter (gtk_tree_view_get_selection (tv),
                                          &iter);

          adj = gtk_tree_view_get_vadjustment (tv);
          gtk_adjustment_set_value (adj, gtk_adjustment_get_upper (adj));
        }
    }
}

static void
duplicate_callback (GtkAction   *action,
                    GtkTreeView *tv)
{
  GtkTreeModel     *model;
  GtkTreeSelection *sel;
  GtkTreeIter       iter;

  model = gtk_tree_view_get_model (tv);
  sel   = gtk_tree_view_get_selection (tv);

  if (gtk_tree_selection_get_selected (sel, NULL, &iter))
    {
      PicmanUnit unit;

      gtk_tree_model_get (model, &iter,
                          UNIT, &unit,
                          -1);

      unit = new_unit_dialog (gtk_widget_get_toplevel (GTK_WIDGET (tv)),
                              unit);

      if (unit != PICMAN_UNIT_PIXEL)
        {
          GtkTreeIter iter;

          unit_list_init (tv);

          if (gtk_tree_model_get_iter_first (model, &iter) &&
              gtk_tree_model_iter_nth_child (model, &iter,
                                             NULL, unit - PICMAN_UNIT_INCH))
            {
              GtkAdjustment *adj;

              gtk_tree_selection_select_iter (sel, &iter);

              adj = gtk_tree_view_get_vadjustment (tv);
              gtk_adjustment_set_value (adj, gtk_adjustment_get_upper (adj));
            }
        }
    }
}

static void
saved_toggled_callback (GtkCellRendererToggle *celltoggle,
                        gchar                 *path_string,
                        GtkListStore          *list_store)
{
  GtkTreePath *path;
  GtkTreeIter  iter;
  gboolean     saved;
  PicmanUnit     unit;

  path = gtk_tree_path_new_from_string (path_string);

  if (! gtk_tree_model_get_iter (GTK_TREE_MODEL (list_store), &iter, path))
    {
      g_warning ("%s: bad tree path?", G_STRLOC);
      return;
    }
  gtk_tree_path_free (path);

  gtk_tree_model_get (GTK_TREE_MODEL (list_store), &iter,
                      SAVE, &saved,
                      UNIT, &unit,
                      -1);

  if (unit >= picman_unit_get_number_of_built_in_units ())
    {
      picman_unit_set_deletion_flag (unit, saved);
      gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
                          SAVE, ! saved,
                          -1);
    }
}

static void
unit_list_init (GtkTreeView *tv)
{
  GtkListStore *list_store;
  GtkTreeIter   iter;
  gint          num_units;
  PicmanUnit      unit;
  GdkColor      color;

  list_store = GTK_LIST_STORE (gtk_tree_view_get_model (tv));

  gtk_list_store_clear (list_store);

  num_units = picman_unit_get_number_of_units ();

  color.red   = 0xdddd;
  color.green = 0xdddd;
  color.blue  = 0xffff;

  for (unit = PICMAN_UNIT_INCH; unit < num_units; unit++)
    {
      gboolean user_unit = (unit >= picman_unit_get_number_of_built_in_units ());

      gtk_list_store_append (list_store, &iter);
      gtk_list_store_set (list_store, &iter,
                          SAVE,         ! picman_unit_get_deletion_flag (unit),
                          IDENTIFIER,   picman_unit_get_identifier (unit),
                          FACTOR,       picman_unit_get_factor (unit),
                          DIGITS,       picman_unit_get_digits (unit),
                          SYMBOL,       picman_unit_get_symbol (unit),
                          ABBREVIATION, picman_unit_get_abbreviation (unit),
                          SINGULAR,     picman_unit_get_singular (unit),
                          PLURAL,       picman_unit_get_plural (unit),
                          UNIT,         unit,
                          USER_UNIT,    user_unit,

                          user_unit ? -1 : BG_COLOR, &color,

                          -1);
    }

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter))
    gtk_tree_selection_select_iter (gtk_tree_view_get_selection (tv), &iter);
}
