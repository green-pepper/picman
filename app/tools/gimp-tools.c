/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis and others
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "widgets/picmanwidgets-utils.h"

#include "core/picman.h"
#include "core/picman-contexts.h"
#include "core/picmanlist.h"
#include "core/picmantoolinfo.h"
#include "core/picmantooloptions.h"

#include "picman-tools.h"
#include "picmantooloptions-gui.h"
#include "tool_manager.h"

#include "picmanairbrushtool.h"
#include "picmanaligntool.h"
#include "picmanblendtool.h"
#include "picmanbrightnesscontrasttool.h"
#include "picmanbucketfilltool.h"
#include "picmanbycolorselecttool.h"
#include "picmancagetool.h"
#include "picmanclonetool.h"
#include "picmancolorbalancetool.h"
#include "picmancolorizetool.h"
#include "picmancolorpickertool.h"
#include "picmanconvolvetool.h"
#include "picmancroptool.h"
#include "picmancurvestool.h"
#include "picmandesaturatetool.h"
#include "picmandodgeburntool.h"
#include "picmanellipseselecttool.h"
#include "picmanerasertool.h"
#include "picmanfliptool.h"
#include "picmanfreeselecttool.h"
#include "picmanforegroundselecttool.h"
#include "picmanfuzzyselecttool.h"
#include "picmangegltool.h"
#include "picmanhealtool.h"
#include "picmanhuesaturationtool.h"
#include "picmaninktool.h"
#include "picmaniscissorstool.h"
#include "picmanlevelstool.h"
#include "picmanoperationtool.h"
#include "picmanmagnifytool.h"
#include "picmanmeasuretool.h"
#include "picmanmovetool.h"
#include "picmanpaintbrushtool.h"
#include "picmanpenciltool.h"
#include "picmanperspectiveclonetool.h"
#include "picmanperspectivetool.h"
#include "picmanposterizetool.h"
#include "picmanthresholdtool.h"
#include "picmanrectangleselecttool.h"
#include "picmanrotatetool.h"
#include "picmanscaletool.h"
#include "picmansheartool.h"
#include "picmansmudgetool.h"
#include "picmantexttool.h"
#include "picmanunifiedtransformtool.h"
#include "picmanvectortool.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   picman_tools_register (GType                   tool_type,
                                   GType                   tool_options_type,
                                   PicmanToolOptionsGUIFunc  options_gui_func,
                                   PicmanContextPropMask     context_props,
                                   const gchar            *identifier,
                                   const gchar            *blurb,
                                   const gchar            *help,
                                   const gchar            *menu_label,
                                   const gchar            *menu_accel,
                                   const gchar            *help_domain,
                                   const gchar            *help_data,
                                   const gchar            *stock_id,
                                   gpointer                data);


/*  private variables  */

static gboolean   tool_options_deleted = FALSE;


/*  public functions  */

void
picman_tools_init (Picman *picman)
{
  PicmanToolRegisterFunc register_funcs[] =
  {
    /*  register tools in reverse order  */

    /*  color tools  */
    picman_operation_tool_register,
    picman_gegl_tool_register,
    picman_posterize_tool_register,
    picman_curves_tool_register,
    picman_levels_tool_register,
    picman_threshold_tool_register,
    picman_brightness_contrast_tool_register,
    picman_colorize_tool_register,
    picman_hue_saturation_tool_register,
    picman_color_balance_tool_register,
    picman_desaturate_tool_register,

    /*  paint tools  */

    picman_dodge_burn_tool_register,
    picman_smudge_tool_register,
    picman_convolve_tool_register,
    picman_perspective_clone_tool_register,
    picman_heal_tool_register,
    picman_clone_tool_register,
    picman_ink_tool_register,
    picman_airbrush_tool_register,
    picman_eraser_tool_register,
    picman_paintbrush_tool_register,
    picman_pencil_tool_register,
    picman_blend_tool_register,
    picman_bucket_fill_tool_register,
    picman_text_tool_register,

    /*  transform tools  */

    picman_cage_tool_register,
    picman_flip_tool_register,
    picman_perspective_tool_register,
    picman_shear_tool_register,
    picman_scale_tool_register,
    picman_rotate_tool_register,
    picman_unified_transform_tool_register,
    picman_crop_tool_register,
    picman_align_tool_register,
    picman_move_tool_register,

    /*  non-modifying tools  */

    picman_measure_tool_register,
    picman_magnify_tool_register,
    picman_color_picker_tool_register,

    /*  path tool */

    picman_vector_tool_register,

    /*  selection tools */

    picman_foreground_select_tool_register,
#if 0
    picman_iscissors_tool_register,
#endif
    picman_by_color_select_tool_register,
    picman_fuzzy_select_tool_register,
    picman_free_select_tool_register,
    picman_ellipse_select_tool_register,
    picman_rectangle_select_tool_register
  };

  GList *default_order = NULL;
  GList *list;
  gint   i;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman_tool_options_create_folder ();

  tool_manager_init (picman);

  picman_container_freeze (picman->tool_info_list);

  for (i = 0; i < G_N_ELEMENTS (register_funcs); i++)
    {
      register_funcs[i] (picman_tools_register, picman);
    }

  picman_container_thaw (picman->tool_info_list);

  for (list = picman_get_tool_info_iter (picman);
       list;
       list = g_list_next (list))
    {
      const gchar *identifier = picman_object_get_name (list->data);

      default_order = g_list_prepend (default_order, g_strdup (identifier));
    }

  default_order = g_list_reverse (default_order);

  g_object_set_data (G_OBJECT (picman),
                     "picman-tools-default-order", default_order);
}

void
picman_tools_exit (Picman *picman)
{
  GList *default_order;
  GList *list;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  default_order = g_object_get_data (G_OBJECT (picman),
                                     "picman-tools-default-order");

  g_list_free_full (default_order, (GDestroyNotify) g_free);

  g_object_set_data (G_OBJECT (picman), "picman-tools-default-order", NULL);

  for (list = picman_get_tool_info_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanToolInfo *tool_info = list->data;
      GtkWidget    *options_gui;

      options_gui = picman_tools_get_tool_options_gui (tool_info->tool_options);
      gtk_widget_destroy (options_gui);
      picman_tools_set_tool_options_gui (tool_info->tool_options, NULL);
    }

  tool_manager_exit (picman);
}

void
picman_tools_restore (Picman *picman)
{
  PicmanContainer *picman_list;
  gchar         *filename;
  GList         *list;
  GError        *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman_list = picman_list_new (PICMAN_TYPE_TOOL_INFO, FALSE);

  filename = picman_personal_rc_file ("toolrc");

  if (picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  if (picman_config_deserialize_file (PICMAN_CONFIG (picman_list), filename,
                                    NULL, NULL))
    {
      gint n = picman_container_get_n_children (picman->tool_info_list);
      gint i;

      picman_list_reverse (PICMAN_LIST (picman_list));

      for (list = PICMAN_LIST (picman_list)->list, i = 0;
           list;
           list = g_list_next (list), i++)
        {
          const gchar *name;
          PicmanObject  *object;

          name = picman_object_get_name (list->data);

          object = picman_container_get_child_by_name (picman->tool_info_list,
                                                     name);

          if (object)
            {
              g_object_set (object,
                            "visible", PICMAN_TOOL_INFO (list->data)->visible,
                            NULL);

              picman_container_reorder (picman->tool_info_list,
                                      object, MIN (i, n - 1));
            }
        }
    }

  g_free (filename);
  g_object_unref (picman_list);

  for (list = picman_get_tool_info_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanToolInfo *tool_info = PICMAN_TOOL_INFO (list->data);

      /*  get default values from prefs (see bug #120832)  */
      picman_tool_options_reset (tool_info->tool_options);
    }

  if (! picman_contexts_load (picman, &error))
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_WARNING, error->message);
      g_clear_error (&error);
    }

  for (list = picman_get_tool_info_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanToolInfo           *tool_info = PICMAN_TOOL_INFO (list->data);
      PicmanToolOptionsGUIFunc  options_gui_func;
      GtkWidget              *options_gui;

      /*  copy all context properties except those the tool actually
       *  uses, because the subsequent deserialize() on the tool
       *  options will only set the properties that were set to
       *  non-default values at the time of saving, and we want to
       *  keep these default values as if they have been saved.
       * (see bug #541586).
       */
      picman_context_copy_properties (picman_get_user_context (picman),
                                    PICMAN_CONTEXT (tool_info->tool_options),
                                    PICMAN_CONTEXT_ALL_PROPS_MASK &~
                                    (tool_info->context_props |
                                     PICMAN_CONTEXT_TOOL_MASK   |
                                     PICMAN_CONTEXT_PAINT_INFO_MASK));

      picman_tool_options_deserialize (tool_info->tool_options, NULL);

      options_gui_func = g_object_get_data (G_OBJECT (tool_info),
                                            "picman-tool-options-gui-func");

      if (options_gui_func)
        {
          options_gui = (* options_gui_func) (tool_info->tool_options);
        }
      else
        {
          GtkWidget *label;

          options_gui = picman_tool_options_gui (tool_info->tool_options);

          label = gtk_label_new (_("This tool has\nno options."));
          gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
          picman_label_set_attributes (GTK_LABEL (label),
                                     PANGO_ATTR_STYLE, PANGO_STYLE_ITALIC,
                                     -1);
          gtk_box_pack_start (GTK_BOX (options_gui), label, FALSE, FALSE, 6);
          gtk_widget_show (label);
        }

      picman_tools_set_tool_options_gui (tool_info->tool_options,
                                       g_object_ref_sink (options_gui));
    }
}

void
picman_tools_save (Picman     *picman,
                 gboolean  save_tool_options,
                 gboolean  always_save)
{
  gchar *filename;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (save_tool_options && (! tool_options_deleted || always_save))
    {
      GList  *list;
      GError *error = NULL;

      if (! picman_contexts_save (picman, &error))
        {
          picman_message_literal (picman, NULL, PICMAN_MESSAGE_WARNING,
				error->message);
          g_clear_error (&error);
        }

      picman_tool_options_create_folder ();

      for (list = picman_get_tool_info_iter (picman);
           list;
           list = g_list_next (list))
        {
          PicmanToolInfo *tool_info = PICMAN_TOOL_INFO (list->data);

          picman_tool_options_serialize (tool_info->tool_options, NULL);
        }
    }

  filename = picman_personal_rc_file ("toolrc");

  if (picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  picman_config_serialize_to_file (PICMAN_CONFIG (picman->tool_info_list),
                                 filename,
                                 "PICMAN toolrc",
                                 "end of toolrc",
                                 NULL, NULL);
  g_free (filename);
}

gboolean
picman_tools_clear (Picman    *picman,
                  GError **error)
{
  GList    *list;
  gboolean  success = TRUE;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  for (list = picman_get_tool_info_iter (picman);
       list && success;
       list = g_list_next (list))
    {
      PicmanToolInfo *tool_info = PICMAN_TOOL_INFO (list->data);

      success = picman_tool_options_delete (tool_info->tool_options, NULL);
    }

  if (success)
    success = picman_contexts_clear (picman, error);

  if (success)
    tool_options_deleted = TRUE;

  return success;
}

GList *
picman_tools_get_default_order (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return g_object_get_data (G_OBJECT (picman),
                            "picman-tools-default-order");
}


/*  private functions  */

static void
picman_tools_register (GType                   tool_type,
                     GType                   tool_options_type,
                     PicmanToolOptionsGUIFunc  options_gui_func,
                     PicmanContextPropMask     context_props,
                     const gchar            *identifier,
                     const gchar            *blurb,
                     const gchar            *help,
                     const gchar            *menu_label,
                     const gchar            *menu_accel,
                     const gchar            *help_domain,
                     const gchar            *help_data,
                     const gchar            *stock_id,
                     gpointer                data)
{
  Picman         *picman = (Picman *) data;
  PicmanToolInfo *tool_info;
  const gchar  *paint_core_name;
  gboolean      visible;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (g_type_is_a (tool_type, PICMAN_TYPE_TOOL));
  g_return_if_fail (tool_options_type == G_TYPE_NONE ||
                    g_type_is_a (tool_options_type, PICMAN_TYPE_TOOL_OPTIONS));

  if (tool_options_type == G_TYPE_NONE)
    tool_options_type = PICMAN_TYPE_TOOL_OPTIONS;

  if (tool_type == PICMAN_TYPE_PENCIL_TOOL)
    {
      paint_core_name = "picman-pencil";
    }
  else if (tool_type == PICMAN_TYPE_PAINTBRUSH_TOOL)
    {
      paint_core_name = "picman-paintbrush";
    }
  else if (tool_type == PICMAN_TYPE_ERASER_TOOL)
    {
      paint_core_name = "picman-eraser";
    }
  else if (tool_type == PICMAN_TYPE_AIRBRUSH_TOOL)
    {
      paint_core_name = "picman-airbrush";
    }
  else if (tool_type == PICMAN_TYPE_CLONE_TOOL)
    {
      paint_core_name = "picman-clone";
    }
  else if (tool_type == PICMAN_TYPE_HEAL_TOOL)
    {
      paint_core_name = "picman-heal";
    }
  else if (tool_type == PICMAN_TYPE_PERSPECTIVE_CLONE_TOOL)
    {
      paint_core_name = "picman-perspective-clone";
    }
  else if (tool_type == PICMAN_TYPE_CONVOLVE_TOOL)
    {
      paint_core_name = "picman-convolve";
    }
  else if (tool_type == PICMAN_TYPE_SMUDGE_TOOL)
    {
      paint_core_name = "picman-smudge";
    }
  else if (tool_type == PICMAN_TYPE_DODGE_BURN_TOOL)
    {
      paint_core_name = "picman-dodge-burn";
    }
  else if (tool_type == PICMAN_TYPE_INK_TOOL)
    {
      paint_core_name = "picman-ink";
    }
  else
    {
      paint_core_name = "picman-paintbrush";
    }

  tool_info = picman_tool_info_new (picman,
                                  tool_type,
                                  tool_options_type,
                                  context_props,
                                  identifier,
                                  blurb,
                                  help,
                                  menu_label,
                                  menu_accel,
                                  help_domain,
                                  help_data,
                                  paint_core_name,
                                  stock_id);

  visible = (! g_type_is_a (tool_type, PICMAN_TYPE_IMAGE_MAP_TOOL));

  g_object_set (tool_info, "visible", visible, NULL);
  g_object_set_data (G_OBJECT (tool_info), "picman-tool-default-visible",
                     GINT_TO_POINTER (visible));

  g_object_set_data (G_OBJECT (tool_info), "picman-tool-options-gui-func",
                     options_gui_func);

  picman_container_add (picman->tool_info_list, PICMAN_OBJECT (tool_info));
  g_object_unref (tool_info);

  if (tool_type == PICMAN_TYPE_PAINTBRUSH_TOOL)
    picman_tool_info_set_standard (picman, tool_info);
}
