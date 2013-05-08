/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2002 Spencer Kimball, Peter Mattis, and others
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

#include <string.h> /* strlen */

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "config/picmanrc.h"

#include "pdb/picmanpdb.h"
#include "pdb/picman-pdb-compat.h"
#include "pdb/internal-procs.h"

#include "plug-in/picmanpluginmanager.h"
#include "plug-in/picmanpluginmanager-restore.h"

#include "paint/picman-paint.h"

#include "text/picman-fonts.h"

#include "xcf/xcf.h"

#include "picman.h"
#include "picman-contexts.h"
#include "picman-gradients.h"
#include "picman-modules.h"
#include "picman-parasites.h"
#include "picman-templates.h"
#include "picman-units.h"
#include "picman-utils.h"
#include "picmanbrush-load.h"
#include "picmanbrush.h"
#include "picmanbrushclipboard.h"
#include "picmanbrushgenerated-load.h"
#include "picmanbrushpipe-load.h"
#include "picmanbuffer.h"
#include "picmancontext.h"
#include "picmandatafactory.h"
#include "picmandynamics.h"
#include "picmandynamics-load.h"
#include "picmandocumentlist.h"
#include "picmangradient-load.h"
#include "picmangradient.h"
#include "picmanidtable.h"
#include "picmanimage.h"
#include "picmanimagefile.h"
#include "picmanlist.h"
#include "picmanmarshal.h"
#include "picmanpalette-load.h"
#include "picmanpalette.h"
#include "picmanparasitelist.h"
#include "picmanpattern-load.h"
#include "picmanpattern.h"
#include "picmanpatternclipboard.h"
#include "picmantagcache.h"
#include "picmantemplate.h"
#include "picmantoolinfo.h"
#include "picmantoolpreset.h"
#include "picmantoolpreset-load.h"

#include "picman-intl.h"


enum
{
  INITIALIZE,
  RESTORE,
  EXIT,
  BUFFER_CHANGED,
  IMAGE_OPENED,
  LAST_SIGNAL
};


static void      picman_dispose              (GObject           *object);
static void      picman_finalize             (GObject           *object);

static gint64    picman_get_memsize          (PicmanObject        *object,
                                            gint64            *gui_size);

static void      picman_real_initialize      (Picman              *picman,
                                            PicmanInitStatusFunc status_callback);
static void      picman_real_restore         (Picman              *picman,
                                            PicmanInitStatusFunc status_callback);
static gboolean  picman_real_exit            (Picman              *picman,
                                            gboolean           force);

static void      picman_global_config_notify (GObject           *global_config,
                                            GParamSpec        *param_spec,
                                            GObject           *edit_config);
static void      picman_edit_config_notify   (GObject           *edit_config,
                                            GParamSpec        *param_spec,
                                            GObject           *global_config);


G_DEFINE_TYPE (Picman, picman, PICMAN_TYPE_OBJECT)

#define parent_class picman_parent_class

static guint picman_signals[LAST_SIGNAL] = { 0, };


static void
picman_class_init (PicmanClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  picman_signals[INITIALIZE] =
    g_signal_new ("initialize",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanClass, initialize),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  picman_signals[RESTORE] =
    g_signal_new ("restore",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanClass, restore),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  picman_signals[EXIT] =
    g_signal_new ("exit",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanClass, exit),
                  g_signal_accumulator_true_handled, NULL,
                  picman_marshal_BOOLEAN__BOOLEAN,
                  G_TYPE_BOOLEAN, 1,
                  G_TYPE_BOOLEAN);

  picman_signals[BUFFER_CHANGED] =
    g_signal_new ("buffer-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanClass, buffer_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_signals[IMAGE_OPENED] =
    g_signal_new ("image-opened",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanClass, image_opened),
                  NULL, NULL,
                  picman_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);

  object_class->dispose          = picman_dispose;
  object_class->finalize         = picman_finalize;

  picman_object_class->get_memsize = picman_get_memsize;

  klass->initialize              = picman_real_initialize;
  klass->restore                 = picman_real_restore;
  klass->exit                    = picman_real_exit;
  klass->buffer_changed          = NULL;
}

static void
picman_init (Picman *picman)
{
  picman->config           = NULL;
  picman->session_name     = NULL;
  picman->default_folder   = NULL;

  picman->be_verbose       = FALSE;
  picman->no_data          = FALSE;
  picman->no_interface     = FALSE;
  picman->show_gui         = TRUE;
  picman->use_shm          = FALSE;
  picman->message_handler  = PICMAN_CONSOLE;
  picman->stack_trace_mode = PICMAN_STACK_TRACE_NEVER;
  picman->pdb_compat_mode  = PICMAN_PDB_COMPAT_OFF;

  picman->restored         = FALSE;

  picman_gui_init (picman);

  picman->busy                = 0;
  picman->busy_idle_id        = 0;

  picman_units_init (picman);

  picman->parasites           = picman_parasite_list_new ();

  picman_modules_init (picman);

  picman->plug_in_manager     = picman_plug_in_manager_new (picman);

  picman->images              = picman_list_new_weak (PICMAN_TYPE_IMAGE, FALSE);
  picman_object_set_static_name (PICMAN_OBJECT (picman->images), "images");

  picman->next_guide_ID        = 1;
  picman->next_sample_point_ID = 1;
  picman->image_table          = picman_id_table_new ();

  picman->item_table          = picman_id_table_new ();

  picman->displays            = g_object_new (PICMAN_TYPE_LIST,
                                            "children-type", PICMAN_TYPE_OBJECT,
                                            "policy",        PICMAN_CONTAINER_POLICY_WEAK,
                                            "append",        TRUE,
                                            NULL);
  picman_object_set_static_name (PICMAN_OBJECT (picman->displays), "displays");

  picman->next_display_ID     = 1;

  picman->image_windows       = NULL;

  picman->global_buffer       = NULL;
  picman->named_buffers       = picman_list_new (PICMAN_TYPE_BUFFER, TRUE);
  picman_object_set_static_name (PICMAN_OBJECT (picman->named_buffers),
                               "named buffers");

  picman->fonts               = NULL;
  picman->brush_factory       = NULL;
  picman->dynamics_factory    = NULL;
  picman->pattern_factory     = NULL;
  picman->gradient_factory    = NULL;
  picman->palette_factory     = NULL;
  picman->tool_preset_factory = NULL;

  picman->tag_cache           = NULL;

  picman->pdb                 = picman_pdb_new (picman);

  xcf_init (picman);

  picman->tool_info_list      = picman_list_new (PICMAN_TYPE_TOOL_INFO, FALSE);
  picman_object_set_static_name (PICMAN_OBJECT (picman->tool_info_list),
                               "tool infos");

  picman->standard_tool_info  = NULL;

  picman->documents           = picman_document_list_new (picman);

  picman->templates           = picman_list_new (PICMAN_TYPE_TEMPLATE, TRUE);
  picman_object_set_static_name (PICMAN_OBJECT (picman->templates), "templates");

  picman->image_new_last_template = NULL;

  picman->context_list        = NULL;
  picman->default_context     = NULL;
  picman->user_context        = NULL;
}

static void
picman_dispose (GObject *object)
{
  Picman *picman = PICMAN (object);

  if (picman->be_verbose)
    g_print ("EXIT: %s\n", G_STRFUNC);

  if (picman->brush_factory)
    picman_data_factory_data_free (picman->brush_factory);

  if (picman->dynamics_factory)
    picman_data_factory_data_free (picman->dynamics_factory);

  if (picman->pattern_factory)
    picman_data_factory_data_free (picman->pattern_factory);

  if (picman->gradient_factory)
    picman_data_factory_data_free (picman->gradient_factory);

  if (picman->palette_factory)
    picman_data_factory_data_free (picman->palette_factory);

  if (picman->tool_preset_factory)
    picman_data_factory_data_free (picman->tool_preset_factory);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_finalize (GObject *object)
{
  Picman *picman = PICMAN (object);

  if (picman->be_verbose)
    g_print ("EXIT: %s\n", G_STRFUNC);

  picman_contexts_exit (picman);

  if (picman->image_new_last_template)
    {
      g_object_unref (picman->image_new_last_template);
      picman->image_new_last_template = NULL;
    }

  if (picman->templates)
    {
      g_object_unref (picman->templates);
      picman->templates = NULL;
    }

  if (picman->documents)
    {
      g_object_unref (picman->documents);
      picman->documents = NULL;
    }

  picman_tool_info_set_standard (picman, NULL);

  if (picman->tool_info_list)
    {
      picman_container_foreach (picman->tool_info_list,
                              (GFunc) g_object_run_dispose, NULL);
      g_object_unref (picman->tool_info_list);
      picman->tool_info_list = NULL;
    }

  xcf_exit (picman);

  if (picman->pdb)
    {
      g_object_unref (picman->pdb);
      picman->pdb = NULL;
    }

  if (picman->brush_factory)
    {
      g_object_unref (picman->brush_factory);
      picman->brush_factory = NULL;
    }

  if (picman->dynamics_factory)
    {
      g_object_unref (picman->dynamics_factory);
      picman->dynamics_factory = NULL;
    }

  if (picman->pattern_factory)
    {
      g_object_unref (picman->pattern_factory);
      picman->pattern_factory = NULL;
    }

  if (picman->gradient_factory)
    {
      g_object_unref (picman->gradient_factory);
      picman->gradient_factory = NULL;
    }

  if (picman->palette_factory)
    {
      g_object_unref (picman->palette_factory);
      picman->palette_factory = NULL;
    }

  if (picman->tool_preset_factory)
    {
      g_object_unref (picman->tool_preset_factory);
      picman->tool_preset_factory = NULL;
    }

  if (picman->tag_cache)
    {
      g_object_unref (picman->tag_cache);
      picman->tag_cache = NULL;
    }

  if (picman->fonts)
    {
      g_object_unref (picman->fonts);
      picman->fonts = NULL;
    }

  if (picman->named_buffers)
    {
      g_object_unref (picman->named_buffers);
      picman->named_buffers = NULL;
    }

  if (picman->global_buffer)
    {
      g_object_unref (picman->global_buffer);
      picman->global_buffer = NULL;
    }

  if (picman->displays)
    {
      g_object_unref (picman->displays);
      picman->displays = NULL;
    }

  if (picman->item_table)
    {
      g_object_unref (picman->item_table);
      picman->item_table = NULL;
    }

  if (picman->image_table)
    {
      g_object_unref (picman->image_table);
      picman->image_table = NULL;
    }

  if (picman->images)
    {
      g_object_unref (picman->images);
      picman->images = NULL;
    }

  if (picman->plug_in_manager)
    {
      g_object_unref (picman->plug_in_manager);
      picman->plug_in_manager = NULL;
    }

  if (picman->module_db)
    picman_modules_exit (picman);

  picman_paint_exit (picman);

  if (picman->parasites)
    {
      g_object_unref (picman->parasites);
      picman->parasites = NULL;
    }

  if (picman->edit_config)
    {
      g_object_unref (picman->edit_config);
      picman->edit_config = NULL;
    }

  if (picman->default_folder)
    {
      g_free (picman->default_folder);
      picman->default_folder = NULL;
    }

  if (picman->session_name)
    {
      g_free (picman->session_name);
      picman->session_name = NULL;
    }

  if (picman->context_list)
    {
      g_list_free (picman->context_list);
      picman->context_list = NULL;
    }

  picman_units_exit (picman);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_get_memsize (PicmanObject *object,
                  gint64     *gui_size)
{
  Picman   *picman    = PICMAN (object);
  gint64  memsize = 0;

  memsize += picman_g_list_get_memsize (picman->user_units, 0 /* FIXME */);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->parasites),
                                      gui_size);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->paint_info_list),
                                      gui_size);

  memsize += picman_g_object_get_memsize (G_OBJECT (picman->module_db));
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->plug_in_manager),
                                      gui_size);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->image_table), 0);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->item_table),  0);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->displays), gui_size);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->global_buffer),
                                      gui_size);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->named_buffers),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->fonts),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->brush_factory),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->dynamics_factory),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->pattern_factory),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->gradient_factory),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->palette_factory),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->tool_preset_factory),
                                      gui_size);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->tag_cache),
                                      gui_size);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->pdb), gui_size);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->tool_info_list),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->standard_tool_info),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->documents),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->templates),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->image_new_last_template),
                                      gui_size);

  memsize += picman_g_list_get_memsize (picman->context_list, 0);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->default_context),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (picman->user_context),
                                      gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_real_initialize (Picman               *picman,
                      PicmanInitStatusFunc  status_callback)
{
  static const PicmanDataFactoryLoaderEntry brush_loader_entries[] =
  {
    { picman_brush_load,           PICMAN_BRUSH_FILE_EXTENSION,           FALSE },
    { picman_brush_load,           PICMAN_BRUSH_PIXMAP_FILE_EXTENSION,    FALSE },
    { picman_brush_load_abr,       PICMAN_BRUSH_PS_FILE_EXTENSION,        FALSE },
    { picman_brush_load_abr,       PICMAN_BRUSH_PSP_FILE_EXTENSION,       FALSE },
    { picman_brush_generated_load, PICMAN_BRUSH_GENERATED_FILE_EXTENSION, TRUE  },
    { picman_brush_pipe_load,      PICMAN_BRUSH_PIPE_FILE_EXTENSION,      FALSE }
  };

  static const PicmanDataFactoryLoaderEntry dynamics_loader_entries[] =
  {
    { picman_dynamics_load,        PICMAN_DYNAMICS_FILE_EXTENSION,        TRUE  }
  };

  static const PicmanDataFactoryLoaderEntry pattern_loader_entries[] =
  {
    { picman_pattern_load,         PICMAN_PATTERN_FILE_EXTENSION,         FALSE },
    { picman_pattern_load_pixbuf,  NULL,                                FALSE }
  };

  static const PicmanDataFactoryLoaderEntry gradient_loader_entries[] =
  {
    { picman_gradient_load,        PICMAN_GRADIENT_FILE_EXTENSION,        TRUE  },
    { picman_gradient_load_svg,    PICMAN_GRADIENT_SVG_FILE_EXTENSION,    FALSE },
    { picman_gradient_load,        NULL /* legacy loader */,            TRUE  }
  };

  static const PicmanDataFactoryLoaderEntry palette_loader_entries[] =
  {
    { picman_palette_load,         PICMAN_PALETTE_FILE_EXTENSION,         TRUE  },
    { picman_palette_load,         NULL /* legacy loader */,            TRUE  }
  };

  static const PicmanDataFactoryLoaderEntry tool_preset_loader_entries[] =
  {
    { picman_tool_preset_load,     PICMAN_TOOL_PRESET_FILE_EXTENSION,     TRUE  }
  };

  PicmanData *clipboard_brush;
  PicmanData *clipboard_pattern;

  if (picman->be_verbose)
    g_print ("INIT: %s\n", G_STRFUNC);

  status_callback (_("Initialization"), NULL, 0.0);

  picman_fonts_init (picman);

  picman->brush_factory =
    picman_data_factory_new (picman,
                           PICMAN_TYPE_BRUSH,
                           "brush-path", "brush-path-writable",
                           brush_loader_entries,
                           G_N_ELEMENTS (brush_loader_entries),
                           picman_brush_new,
                           picman_brush_get_standard);
  picman_object_set_static_name (PICMAN_OBJECT (picman->brush_factory),
                               "brush factory");

  picman->dynamics_factory =
    picman_data_factory_new (picman,
                           PICMAN_TYPE_DYNAMICS,
                           "dynamics-path", "dynamics-path-writable",
                           dynamics_loader_entries,
                           G_N_ELEMENTS (dynamics_loader_entries),
                           picman_dynamics_new,
                           picman_dynamics_get_standard);
  picman_object_set_static_name (PICMAN_OBJECT (picman->dynamics_factory),
                               "dynamics factory");

  picman->pattern_factory =
    picman_data_factory_new (picman,
                           PICMAN_TYPE_PATTERN,
                           "pattern-path", "pattern-path-writable",
                           pattern_loader_entries,
                           G_N_ELEMENTS (pattern_loader_entries),
                           NULL,
                           picman_pattern_get_standard);
  picman_object_set_static_name (PICMAN_OBJECT (picman->pattern_factory),
                               "pattern factory");

  picman->gradient_factory =
    picman_data_factory_new (picman,
                           PICMAN_TYPE_GRADIENT,
                           "gradient-path", "gradient-path-writable",
                           gradient_loader_entries,
                           G_N_ELEMENTS (gradient_loader_entries),
                           picman_gradient_new,
                           picman_gradient_get_standard);
  picman_object_set_static_name (PICMAN_OBJECT (picman->gradient_factory),
                               "gradient factory");

  picman->palette_factory =
    picman_data_factory_new (picman,
                           PICMAN_TYPE_PALETTE,
                           "palette-path", "palette-path-writable",
                           palette_loader_entries,
                           G_N_ELEMENTS (palette_loader_entries),
                           picman_palette_new,
                           picman_palette_get_standard);
  picman_object_set_static_name (PICMAN_OBJECT (picman->palette_factory),
                               "palette factory");

  picman->tool_preset_factory =
    picman_data_factory_new (picman,
                           PICMAN_TYPE_TOOL_PRESET,
                           "tool-preset-path", "tool-preset-path-writable",
                           tool_preset_loader_entries,
                           G_N_ELEMENTS (tool_preset_loader_entries),
                           picman_tool_preset_new,
                           NULL);
  picman_object_set_static_name (PICMAN_OBJECT (picman->tool_preset_factory),
                               "tool preset factory");

  picman->tag_cache = picman_tag_cache_new ();

  picman_paint_init (picman);

  /* Set the last values used to default values. */
  picman->image_new_last_template =
    picman_config_duplicate (PICMAN_CONFIG (picman->config->default_image));

  /*  create user and default context  */
  picman_contexts_init (picman);

  /*  add the builtin FG -> BG etc. gradients  */
  picman_gradients_init (picman);

  /*  add the clipboard brush  */
  clipboard_brush = picman_brush_clipboard_new (picman);
  picman_data_make_internal (PICMAN_DATA (clipboard_brush),
                           "picman-brush-clipboard");
  picman_container_add (picman_data_factory_get_container (picman->brush_factory),
                      PICMAN_OBJECT (clipboard_brush));
  g_object_unref (clipboard_brush);

  /*  add the clipboard pattern  */
  clipboard_pattern = picman_pattern_clipboard_new (picman);
  picman_data_make_internal (PICMAN_DATA (clipboard_pattern),
                           "picman-pattern-clipboard");
  picman_container_add (picman_data_factory_get_container (picman->pattern_factory),
                      PICMAN_OBJECT (clipboard_pattern));
  g_object_unref (clipboard_pattern);

  /*  register all internal procedures  */
  status_callback (NULL, _("Internal Procedures"), 0.2);
  internal_procs_init (picman->pdb);
  picman_pdb_compat_procs_register (picman->pdb, picman->pdb_compat_mode);

  picman_plug_in_manager_initialize (picman->plug_in_manager, status_callback);

  status_callback (NULL, "", 1.0);
}

static void
picman_real_restore (Picman               *picman,
                   PicmanInitStatusFunc  status_callback)
{
  if (picman->be_verbose)
    g_print ("INIT: %s\n", G_STRFUNC);

  picman_plug_in_manager_restore (picman->plug_in_manager,
                                picman_get_user_context (picman), status_callback);

  picman->restored = TRUE;
}

static gboolean
picman_real_exit (Picman     *picman,
                gboolean  force)
{
  if (picman->be_verbose)
    g_print ("EXIT: %s\n", G_STRFUNC);

  picman_plug_in_manager_exit (picman->plug_in_manager);
  picman_modules_unload (picman);

  picman_tag_cache_save (picman->tag_cache);

  picman_data_factory_data_save (picman->brush_factory);
  picman_data_factory_data_save (picman->dynamics_factory);
  picman_data_factory_data_save (picman->pattern_factory);
  picman_data_factory_data_save (picman->gradient_factory);
  picman_data_factory_data_save (picman->palette_factory);
  picman_data_factory_data_save (picman->tool_preset_factory);

  picman_fonts_reset (picman);

  picman_templates_save (picman);
  picman_parasiterc_save (picman);
  picman_unitrc_save (picman);

  return FALSE; /* continue exiting */
}

Picman *
picman_new (const gchar       *name,
          const gchar       *session_name,
          const gchar       *default_folder,
          gboolean           be_verbose,
          gboolean           no_data,
          gboolean           no_fonts,
          gboolean           no_interface,
          gboolean           use_shm,
          gboolean           console_messages,
          PicmanStackTraceMode stack_trace_mode,
          PicmanPDBCompatMode  pdb_compat_mode)
{
  Picman *picman;

  g_return_val_if_fail (name != NULL, NULL);

  picman = g_object_new (PICMAN_TYPE_PICMAN,
                       "name", name,
                       NULL);

  picman->session_name     = g_strdup (session_name);
  picman->default_folder   = g_strdup (default_folder);
  picman->be_verbose       = be_verbose       ? TRUE : FALSE;
  picman->no_data          = no_data          ? TRUE : FALSE;
  picman->no_fonts         = no_fonts         ? TRUE : FALSE;
  picman->no_interface     = no_interface     ? TRUE : FALSE;
  picman->use_shm          = use_shm          ? TRUE : FALSE;
  picman->console_messages = console_messages ? TRUE : FALSE;
  picman->stack_trace_mode = stack_trace_mode;
  picman->pdb_compat_mode  = pdb_compat_mode;

  return picman;
}

/**
 * picman_set_show_gui:
 * @picman:
 * @show:
 *
 * Test cases that tests the UI typically don't want any windows to be
 * presented during the test run. Allow them to set this.
 **/
void
picman_set_show_gui (Picman     *picman,
                   gboolean  show_gui)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman->show_gui = show_gui;
}

/**
 * picman_get_show_gui:
 * @picman:
 *
 * Returns: %TRUE if the GUI should be shown, %FALSE otherwise.
 **/
gboolean
picman_get_show_gui (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  return picman->show_gui;
}

static void
picman_global_config_notify (GObject    *global_config,
                           GParamSpec *param_spec,
                           GObject    *edit_config)
{
  GValue global_value = { 0, };
  GValue edit_value   = { 0, };

  g_value_init (&global_value, param_spec->value_type);
  g_value_init (&edit_value,   param_spec->value_type);

  g_object_get_property (global_config, param_spec->name, &global_value);
  g_object_get_property (edit_config,   param_spec->name, &edit_value);

  if (g_param_values_cmp (param_spec, &global_value, &edit_value))
    {
      g_signal_handlers_block_by_func (edit_config,
                                       picman_edit_config_notify,
                                       global_config);

      g_object_set_property (edit_config, param_spec->name, &global_value);

      g_signal_handlers_unblock_by_func (edit_config,
                                         picman_edit_config_notify,
                                         global_config);
    }

  g_value_unset (&global_value);
  g_value_unset (&edit_value);
}

static void
picman_edit_config_notify (GObject    *edit_config,
                         GParamSpec *param_spec,
                         GObject    *global_config)
{
  GValue edit_value   = { 0, };
  GValue global_value = { 0, };

  g_value_init (&edit_value,   param_spec->value_type);
  g_value_init (&global_value, param_spec->value_type);

  g_object_get_property (edit_config,   param_spec->name, &edit_value);
  g_object_get_property (global_config, param_spec->name, &global_value);

  if (g_param_values_cmp (param_spec, &edit_value, &global_value))
    {
      if (param_spec->flags & PICMAN_CONFIG_PARAM_RESTART)
        {
#ifdef PICMAN_CONFIG_DEBUG
          g_print ("NOT Applying edit_config change of '%s' to global_config "
                   "because it needs restart\n",
                   param_spec->name);
#endif
        }
      else
        {
#ifdef PICMAN_CONFIG_DEBUG
          g_print ("Applying edit_config change of '%s' to global_config\n",
                   param_spec->name);
#endif
          g_signal_handlers_block_by_func (global_config,
                                           picman_global_config_notify,
                                           edit_config);

          g_object_set_property (global_config, param_spec->name, &edit_value);

          g_signal_handlers_unblock_by_func (global_config,
                                             picman_global_config_notify,
                                             edit_config);
        }
    }

  g_value_unset (&edit_value);
  g_value_unset (&global_value);
}

void
picman_load_config (Picman        *picman,
                  const gchar *alternate_system_picmanrc,
                  const gchar *alternate_picmanrc)
{
  PicmanRc *picmanrc;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (picman->config == NULL);
  g_return_if_fail (picman->edit_config == NULL);

  if (picman->be_verbose)
    g_print ("INIT: %s\n", G_STRFUNC);

  /*  this needs to be done before picmanrc loading because picmanrc can
   *  use user defined units
   */
  picman_unitrc_load (picman);

  picmanrc = picman_rc_new (alternate_system_picmanrc,
                        alternate_picmanrc,
                        picman->be_verbose);

  picman->config = PICMAN_CORE_CONFIG (picmanrc);

  picman->edit_config = picman_config_duplicate (PICMAN_CONFIG (picman->config));

  g_signal_connect_object (picman->config, "notify",
                           G_CALLBACK (picman_global_config_notify),
                           picman->edit_config, 0);
  g_signal_connect_object (picman->edit_config, "notify",
                           G_CALLBACK (picman_edit_config_notify),
                           picman->config, 0);
}

void
picman_initialize (Picman               *picman,
                 PicmanInitStatusFunc  status_callback)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (status_callback != NULL);
  g_return_if_fail (PICMAN_IS_CORE_CONFIG (picman->config));

  if (picman->be_verbose)
    g_print ("INIT: %s\n", G_STRFUNC);

  g_signal_emit (picman, picman_signals[INITIALIZE], 0, status_callback);
}

void
picman_restore (Picman               *picman,
              PicmanInitStatusFunc  status_callback)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (status_callback != NULL);

  if (picman->be_verbose)
    g_print ("INIT: %s\n", G_STRFUNC);

  /*  initialize  the global parasite table  */
  status_callback (_("Looking for data files"), _("Parasites"), 0.0);
  picman_parasiterc_load (picman);

  /*  initialize the list of picman brushes    */
  status_callback (NULL, _("Brushes"), 0.1);
  picman_data_factory_data_init (picman->brush_factory, picman->user_context,
                               picman->no_data);

  /*  initialize the list of picman dynamics   */
  status_callback (NULL, _("Dynamics"), 0.2);
  picman_data_factory_data_init (picman->dynamics_factory, picman->user_context,
                               picman->no_data);

  /*  initialize the list of picman patterns   */
  status_callback (NULL, _("Patterns"), 0.3);
  picman_data_factory_data_init (picman->pattern_factory, picman->user_context,
                               picman->no_data);

  /*  initialize the list of picman palettes   */
  status_callback (NULL, _("Palettes"), 0.4);
  picman_data_factory_data_init (picman->palette_factory, picman->user_context,
                               picman->no_data);

  /*  initialize the list of picman gradients  */
  status_callback (NULL, _("Gradients"), 0.5);
  picman_data_factory_data_init (picman->gradient_factory, picman->user_context,
                               picman->no_data);

  /*  initialize the list of fonts  */
  status_callback (NULL, _("Fonts (this may take a while)"), 0.6);
  if (! picman->no_fonts)
    picman_fonts_load (picman);

  /*  initialize the list of picman tool presets if we have a GUI  */
  if (! picman->no_interface)
    {
      status_callback (NULL, _("Tool Presets"), 0.65);
      picman_data_factory_data_init (picman->tool_preset_factory, picman->user_context,
                                   picman->no_data);
    }

  /*  initialize the template list  */
  status_callback (NULL, _("Templates"), 0.7);
  picman_templates_load (picman);

  /*  initialize the module list  */
  status_callback (NULL, _("Modules"), 0.8);
  picman_modules_load (picman);

  /* update tag cache */
  status_callback (NULL, _("Updating tag cache"), 0.9);
  picman_tag_cache_load (picman->tag_cache);
  picman_tag_cache_add_container (picman->tag_cache,
                                picman_data_factory_get_container (picman->brush_factory));
  picman_tag_cache_add_container (picman->tag_cache,
                                picman_data_factory_get_container (picman->dynamics_factory));
  picman_tag_cache_add_container (picman->tag_cache,
                                picman_data_factory_get_container (picman->pattern_factory));
  picman_tag_cache_add_container (picman->tag_cache,
                                picman_data_factory_get_container (picman->gradient_factory));
  picman_tag_cache_add_container (picman->tag_cache,
                                picman_data_factory_get_container (picman->palette_factory));
  picman_tag_cache_add_container (picman->tag_cache,
                                picman_data_factory_get_container (picman->tool_preset_factory));

  g_signal_emit (picman, picman_signals[RESTORE], 0, status_callback);
}

/**
 * picman_is_restored:
 * @picman: a #Picman object
 *
 * Return value: %TRUE if PICMAN is completely started, %FALSE otherwise.
 **/
gboolean
picman_is_restored (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  return picman->restored;
}


/**
 * picman_exit:
 * @picman: a #Picman object
 * @force: whether to force the application to quit
 *
 * Exit this PICMAN session. Unless @force is %TRUE, the user is queried
 * whether unsaved images should be saved and can cancel the operation.
 **/
void
picman_exit (Picman     *picman,
           gboolean  force)
{
  gboolean handled;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (picman->be_verbose)
    g_print ("EXIT: %s\n", G_STRFUNC);

  g_signal_emit (picman, picman_signals[EXIT], 0,
                 force ? TRUE : FALSE,
                 &handled);
}

GList *
picman_get_image_iter (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return PICMAN_LIST (picman->images)->list;
}

GList *
picman_get_display_iter (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return PICMAN_LIST (picman->displays)->list;
}

GList *
picman_get_image_windows (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return g_list_copy (picman->image_windows);
}

GList *
picman_get_paint_info_iter (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return PICMAN_LIST (picman->paint_info_list)->list;
}

GList *
picman_get_tool_info_iter (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return PICMAN_LIST (picman->tool_info_list)->list;
}

void
picman_set_global_buffer (Picman       *picman,
                        PicmanBuffer *buffer)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (buffer == NULL || PICMAN_IS_BUFFER (buffer));

  if (buffer == picman->global_buffer)
    return;

  if (picman->global_buffer)
    g_object_unref (picman->global_buffer);

  picman->global_buffer = buffer;

  if (picman->global_buffer)
    g_object_ref (picman->global_buffer);

  g_signal_emit (picman, picman_signals[BUFFER_CHANGED], 0);
}

PicmanImage *
picman_create_image (Picman              *picman,
                   gint               width,
                   gint               height,
                   PicmanImageBaseType  type,
                   PicmanPrecision      precision,
                   gboolean           attach_comment)
{
  PicmanImage *image;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  image = picman_image_new (picman, width, height, type, precision);

  if (attach_comment)
    {
      const gchar *comment;

      comment = picman_template_get_comment (picman->config->default_image);

      if (comment)
        {
          PicmanParasite *parasite = picman_parasite_new ("picman-comment",
                                                      PICMAN_PARASITE_PERSISTENT,
                                                      strlen (comment) + 1,
                                                      comment);
          picman_image_parasite_attach (image, parasite);
          picman_parasite_free (parasite);
        }
    }

  return image;
}

void
picman_set_default_context (Picman        *picman,
                          PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  if (context != picman->default_context)
    {
      if (picman->default_context)
        g_object_unref (picman->default_context);

      picman->default_context = context;

      if (picman->default_context)
        g_object_ref (picman->default_context);
    }
}

PicmanContext *
picman_get_default_context (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return picman->default_context;
}

void
picman_set_user_context (Picman        *picman,
                       PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  if (context != picman->user_context)
    {
      if (picman->user_context)
        g_object_unref (picman->user_context);

      picman->user_context = context;

      if (picman->user_context)
        g_object_ref (picman->user_context);
    }
}

PicmanContext *
picman_get_user_context (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return picman->user_context;
}

PicmanToolInfo *
picman_get_tool_info (Picman        *picman,
                    const gchar *tool_id)
{
  gpointer info;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (tool_id != NULL, NULL);

  info = picman_container_get_child_by_name (picman->tool_info_list, tool_id);

  return (PicmanToolInfo *) info;
}

/**
 * picman_message:
 * @picman:     a pointer to the %Picman object
 * @handler:  either a %PicmanProgress or a %GtkWidget pointer
 * @severity: severity of the message
 * @format:   printf-like format string
 * @...:      arguments to use with @format
 *
 * Present a message to the user. How exactly the message is displayed
 * depends on the @severity, the @handler object and user preferences.
 **/
void
picman_message (Picman                *picman,
              GObject             *handler,
              PicmanMessageSeverity  severity,
              const gchar         *format,
              ...)
{
  va_list args;

  va_start (args, format);

  picman_message_valist (picman, handler, severity, format, args);

  va_end (args);
}

/**
 * picman_message_valist:
 * @picman:     a pointer to the %Picman object
 * @handler:  either a %PicmanProgress or a %GtkWidget pointer
 * @severity: severity of the message
 * @format:   printf-like format string
 * @args:     arguments to use with @format
 *
 * See documentation for picman_message().
 **/
void
picman_message_valist (Picman                *picman,
                     GObject             *handler,
                     PicmanMessageSeverity  severity,
                     const gchar         *format,
                     va_list              args)
{
  gchar *message;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (handler == NULL || G_IS_OBJECT (handler));
  g_return_if_fail (format != NULL);

  message = g_strdup_vprintf (format, args);

  picman_show_message (picman, handler, severity, NULL, message);

  g_free (message);
}

void
picman_message_literal (Picman                *picman,
		      GObject             *handler,
		      PicmanMessageSeverity  severity,
		      const gchar         *message)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (handler == NULL || G_IS_OBJECT (handler));
  g_return_if_fail (message != NULL);

  picman_show_message (picman, handler, severity, NULL, message);
}

void
picman_image_opened (Picman        *picman,
		   const gchar *uri)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (uri != NULL);

  g_signal_emit (picman, picman_signals[IMAGE_OPENED], 0, uri);
}

gchar *
picman_get_temp_filename (Picman        *picman,
                        const gchar *extension)
{
  static gint  id = 0;
  static gint  pid;
  gchar       *filename;
  gchar       *basename;
  gchar       *path;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (id == 0)
    pid = picman_get_pid ();

  if (extension)
    basename = g_strdup_printf ("picman-temp-%d%d.%s", pid, id++, extension);
  else
    basename = g_strdup_printf ("picman-temp-%d%d", pid, id++);

  path = picman_config_path_expand (PICMAN_GEGL_CONFIG (picman->config)->temp_path,
                                  TRUE, NULL);

  filename = g_build_filename (path, basename, NULL);

  g_free (path);
  g_free (basename);

  return filename;
}
