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

#include <gegl.h>
#include <gegl-plugin.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanpropwidgets.h"

#include "picmancoloroptions.h"
#include "picmangegltool.h"

#include "picman-intl.h"


enum
{
  COLUMN_NAME,
  COLUMN_LABEL,
  COLUMN_STOCK_ID,
  N_COLUMNS
};


/*  local function prototypes  */

static void   picman_gegl_tool_dialog            (PicmanImageMapTool  *im_tool);

static void   picman_gegl_tool_operation_changed (GtkWidget         *widget,
                                                PicmanGeglTool      *tool);


G_DEFINE_TYPE (PicmanGeglTool, picman_gegl_tool, PICMAN_TYPE_OPERATION_TOOL)

#define parent_class picman_gegl_tool_parent_class


void
picman_gegl_tool_register (PicmanToolRegisterCallback  callback,
                         gpointer                  data)
{
  (* callback) (PICMAN_TYPE_GEGL_TOOL,
                PICMAN_TYPE_COLOR_OPTIONS,
                picman_color_options_gui,
                0,
                "picman-gegl-tool",
                _("GEGL Operation"),
                _("GEGL Tool: Use an arbitrary GEGL operation"),
                N_("_GEGL Operation..."), NULL,
                NULL, PICMAN_HELP_TOOL_GEGL,
                PICMAN_STOCK_GEGL,
                data);
}

static void
picman_gegl_tool_class_init (PicmanGeglToolClass *klass)
{
  PicmanImageMapToolClass *im_tool_class = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  im_tool_class->dialog_desc = _("GEGL Operation");

  im_tool_class->dialog      = picman_gegl_tool_dialog;
}

static void
picman_gegl_tool_init (PicmanGeglTool *tool)
{
}

static gboolean
picman_gegl_tool_operation_blacklisted (const gchar *name,
                                      const gchar *categories_str)
{
  static const gchar * const category_blacklist[] =
  {
    "compositors",
    "core",
    "debug",
    "hidden",
    "input",
    "output",
    "programming",
    "transform",
    "video"
  };
  static const gchar * const name_blacklist[] =
  {
    "gegl:c2g", /* in picman */
    "gegl:cartoon", /* in picman */
    "gegl:checkerboard", /* in picman */
    "gegl:color", /* pointless */
    "gegl:color-reduction", /* in picman */
    "gegl:color-temperature", /* in picman */
    "gegl:color-to-alpha", /* in picman */
    "gegl:colorize", /* in picman */
    "gegl:contrast-curve",
    "gegl:convert-format",
    "gegl:difference-of-gaussians", /* in picman */
    "gegl:display",
    "gegl:dot", /* in picman */
    "gegl:edge-laplace", /* in picman */
    "gegl:edge-sobel", /* in picman */
    "gegl:fill-path",
    "gegl:gaussian-blur", /* in picman */
    "gegl:grey", /* in picman */
    "gegl:hstack",
    "gegl:introspect",
    "gegl:invert", /* in picman */
    "gegl:layer",
    "gegl:lens-correct",
    "gegl:lens-distortion", /* in picman */
    "gegl:matting-global", /* useless */
    "gegl:mono-mixer", /* in picman */
    "gegl:noise-CIE_lch", /* in picman */
    "gegl:noise-hsv", /* in picman */
    "gegl:noise-hurl", /* in picman */
    "gegl:noise-pick", /* in picman */
    "gegl:noise-rgb", /* in picman */
    "gegl:noise-slur", /* in picman */
    "gegl:opacity", /* pointless */
    "gegl:path",
    "gegl:photocopy", /* in picman */
    "gegl:pixelize", /* in picman */
    "gegl:polar-coordinates", /* in picman */
    "gegl:posterize", /* in picman */
    "gegl:ripple", /* in picman */
    "gegl:rotate", /* in picman */
    "gegl:scale", /* in picman */
    "gegl:sdl-display", /* useless */
    "gegl:seamless-clone",
    "gegl:shear", /* in picman */
    "gegl:text",
    "gegl:threshold", /* in picman */
    "gegl:tile", /* useless */
    "gegl:transform", /* in picman */
    "gegl:translate", /* pointless */
    "gegl:unsharp-mask", /* in picman */
    "gegl:value-invert", /* in picman */
    "gegl:vector-stroke",
    "gegl:vignette", /* in picman */
  };

  gchar **categories;
  gint    i;

  /* Operations with no name are abstract base classes */
  if (! name)
    return TRUE;

  if (g_str_has_prefix (name, "picman"))
    return TRUE;

  for (i = 0; i < G_N_ELEMENTS (name_blacklist); i++)
    {
      if (! strcmp (name, name_blacklist[i]))
        return TRUE;
    }

  if (! categories_str)
    return FALSE;

  categories = g_strsplit (categories_str, ":", 0);

  for (i = 0; i < G_N_ELEMENTS (category_blacklist); i++)
    {
      gint j;

      for (j = 0; categories[j]; j++)
        if (! strcmp (categories[j], category_blacklist[i]))
          {
            g_strfreev (categories);
            return TRUE;
          }
    }

  g_strfreev (categories);

  return FALSE;
}


/* Builds a GList of the class structures of all subtypes of type.
 */
static GList *
picman_get_subtype_classes (GType  type,
                          GList *classes)
{
  GeglOperationClass *klass;
  GType              *ops;
  const gchar        *categories;
  guint               n_ops;
  gint                i;

  if (! type)
    return classes;

  klass = GEGL_OPERATION_CLASS (g_type_class_ref (type));
  ops = g_type_children (type, &n_ops);

  categories = gegl_operation_class_get_key (klass, "categories");

  if (! picman_gegl_tool_operation_blacklisted (klass->name, categories))
    classes = g_list_prepend (classes, klass);

  for (i = 0; i < n_ops; i++)
    classes = picman_get_subtype_classes (ops[i], classes);

  if (ops)
    g_free (ops);

  return classes;
}

static gint
picman_gegl_tool_compare_operation_names (GeglOperationClass *a,
                                        GeglOperationClass *b)
{
  return strcmp (a->name, b->name);
}

static GList *
picman_get_geglopclasses (void)
{
  GList *opclasses;

  opclasses = picman_get_subtype_classes (GEGL_TYPE_OPERATION, NULL);

  opclasses = g_list_sort (opclasses,
                           (GCompareFunc)
                           picman_gegl_tool_compare_operation_names);

  return opclasses;
}


/*****************/
/*  Gegl dialog  */
/*****************/

static void
picman_gegl_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanGeglTool      *tool   = PICMAN_GEGL_TOOL (image_map_tool);
  PicmanOperationTool *o_tool = PICMAN_OPERATION_TOOL (image_map_tool);
  GtkListStore      *store;
  GtkCellRenderer   *cell;
  GtkWidget         *main_vbox;
  GtkWidget         *hbox;
  GtkWidget         *combo;
  GList             *opclasses;
  GList             *iter;

  PICMAN_IMAGE_MAP_TOOL_CLASS (parent_class)->dialog (image_map_tool);

  main_vbox = picman_image_map_tool_dialog_get_vbox (image_map_tool);

  /*  The operation combo box  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_box_reorder_child (GTK_BOX (main_vbox), hbox, 0);
  gtk_widget_show (hbox);

  store = gtk_list_store_new (N_COLUMNS,
                              G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

  opclasses = picman_get_geglopclasses ();

  for (iter = opclasses; iter; iter = iter->next)
    {
      GeglOperationClass *opclass = GEGL_OPERATION_CLASS (iter->data);
      const gchar        *stock_id;
      const gchar        *label;

      if (g_str_has_prefix (opclass->name, "gegl:"))
        {
          label    = opclass->name + strlen ("gegl:");
          stock_id = PICMAN_STOCK_GEGL;
        }
      else
        {
          label    = opclass->name;
          stock_id = NULL;
        }

      gtk_list_store_insert_with_values (store, NULL, -1,
                                         COLUMN_NAME,     opclass->name,
                                         COLUMN_LABEL,    label,
                                         COLUMN_STOCK_ID, stock_id,
                                         -1);
    }

  g_list_free (opclasses);

  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
  gtk_widget_show (combo);

  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), cell,
                                 "stock-id", COLUMN_STOCK_ID);

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), cell,
                                 "text", COLUMN_LABEL);

  g_signal_connect (combo, "changed",
                    G_CALLBACK (picman_gegl_tool_operation_changed),
                    tool);

  tool->operation_combo = combo;

  /*  The options vbox  */
  o_tool->options_table =
    gtk_label_new (_("Select an operation from the list above"));
  picman_label_set_attributes (GTK_LABEL (o_tool->options_table),
                             PANGO_ATTR_STYLE, PANGO_STYLE_ITALIC,
                             -1);
  gtk_misc_set_padding (GTK_MISC (o_tool->options_table), 0, 4);
  gtk_container_add (GTK_CONTAINER (o_tool->options_box),
                     o_tool->options_table);
  gtk_widget_show (o_tool->options_table);
}

static void
picman_gegl_tool_operation_changed (GtkWidget    *widget,
                                  PicmanGeglTool *tool)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gchar        *operation;

  if (! gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget), &iter))
    return;

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (widget));

  gtk_tree_model_get (model, &iter,
                      COLUMN_NAME, &operation,
                      -1);

  if (operation)
    {
      picman_operation_tool_set_operation (PICMAN_OPERATION_TOOL (tool),
                                         operation, NULL);
      g_free (operation);
    }
}
