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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "operations/picmanposterizeconfig.h"

#include "core/picmandrawable.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"

#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "picmanimagemapoptions.h"
#include "picmanposterizetool.h"

#include "picman-intl.h"


#define SLIDER_WIDTH 200


static gboolean   picman_posterize_tool_initialize     (PicmanTool          *tool,
                                                      PicmanDisplay       *display,
                                                      GError           **error);

static GeglNode * picman_posterize_tool_get_operation  (PicmanImageMapTool  *im_tool,
                                                      GObject          **config,
                                                      gchar            **undo_desc);
static void       picman_posterize_tool_dialog         (PicmanImageMapTool  *im_tool);

static void       picman_posterize_tool_config_notify  (GObject           *object,
                                                      GParamSpec        *pspec,
                                                      PicmanPosterizeTool *posterize_tool);

static void       picman_posterize_tool_levels_changed (GtkAdjustment     *adjustment,
                                                      PicmanPosterizeTool *posterize_tool);


G_DEFINE_TYPE (PicmanPosterizeTool, picman_posterize_tool,
               PICMAN_TYPE_IMAGE_MAP_TOOL)

#define parent_class picman_posterize_tool_parent_class


void
picman_posterize_tool_register (PicmanToolRegisterCallback  callback,
                              gpointer                  data)
{
  (* callback) (PICMAN_TYPE_POSTERIZE_TOOL,
                PICMAN_TYPE_IMAGE_MAP_OPTIONS, NULL,
                0,
                "picman-posterize-tool",
                _("Posterize"),
                _("Posterize Tool: Reduce to a limited set of colors"),
                N_("_Posterize..."), NULL,
                NULL, PICMAN_HELP_TOOL_POSTERIZE,
                PICMAN_STOCK_TOOL_POSTERIZE,
                data);
}

static void
picman_posterize_tool_class_init (PicmanPosterizeToolClass *klass)
{
  PicmanToolClass         *tool_class    = PICMAN_TOOL_CLASS (klass);
  PicmanImageMapToolClass *im_tool_class = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  tool_class->initialize       = picman_posterize_tool_initialize;

  im_tool_class->dialog_desc   = _("Posterize (Reduce Number of Colors)");

  im_tool_class->get_operation = picman_posterize_tool_get_operation;
  im_tool_class->dialog        = picman_posterize_tool_dialog;
}

static void
picman_posterize_tool_init (PicmanPosterizeTool *posterize_tool)
{
}

static gboolean
picman_posterize_tool_initialize (PicmanTool     *tool,
                                PicmanDisplay  *display,
                                GError      **error)
{
  PicmanPosterizeTool *posterize_tool = PICMAN_POSTERIZE_TOOL (tool);

  if (! PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error))
    {
      return FALSE;
    }

  gtk_adjustment_set_value (posterize_tool->levels_data,
                            posterize_tool->config->levels);

  return TRUE;
}

static GeglNode *
picman_posterize_tool_get_operation (PicmanImageMapTool  *image_map_tool,
                                   GObject          **config,
                                   gchar            **undo_desc)
{
  PicmanPosterizeTool *posterize_tool = PICMAN_POSTERIZE_TOOL (image_map_tool);

  posterize_tool->config = g_object_new (PICMAN_TYPE_POSTERIZE_CONFIG, NULL);

  g_signal_connect_object (posterize_tool->config, "notify",
                           G_CALLBACK (picman_posterize_tool_config_notify),
                           G_OBJECT (posterize_tool), 0);

  *config = G_OBJECT (posterize_tool->config);

  return gegl_node_new_child (NULL,
                              "operation", "picman:posterize",
                              "config",    posterize_tool->config,
                              NULL);
}


/**********************/
/*  Posterize dialog  */
/**********************/

static void
picman_posterize_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanPosterizeTool *posterize_tool = PICMAN_POSTERIZE_TOOL (image_map_tool);
  GtkWidget         *main_vbox;
  GtkWidget         *table;
  GtkObject         *data;

  main_vbox = picman_image_map_tool_dialog_get_vbox (image_map_tool);

  /*  The table containing sliders  */
  table = gtk_table_new (1, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  data = picman_scale_entry_new (GTK_TABLE (table), 0, 0,
                               _("Posterize _levels:"), SLIDER_WIDTH, -1,
                               posterize_tool->config->levels,
                               2.0, 256.0, 1.0, 10.0, 0,
                               TRUE, 0.0, 0.0,
                               NULL, NULL);

  picman_scale_entry_set_logarithmic (data, TRUE);

  posterize_tool->levels_data = GTK_ADJUSTMENT (data);

  g_signal_connect (posterize_tool->levels_data, "value-changed",
                    G_CALLBACK (picman_posterize_tool_levels_changed),
                    posterize_tool);
}

static void
picman_posterize_tool_config_notify (GObject           *object,
                                   GParamSpec        *pspec,
                                   PicmanPosterizeTool *posterize_tool)
{
  PicmanPosterizeConfig *config = PICMAN_POSTERIZE_CONFIG (object);

  if (! posterize_tool->levels_data)
    return;

  gtk_adjustment_set_value (posterize_tool->levels_data, config->levels);
}

static void
picman_posterize_tool_levels_changed (GtkAdjustment     *adjustment,
                                    PicmanPosterizeTool *posterize_tool)
{
  gint value = ROUND (gtk_adjustment_get_value (adjustment));

  if (posterize_tool->config->levels != value)
    {
      g_object_set (posterize_tool->config,
                    "levels", value,
                    NULL);
    }
}
