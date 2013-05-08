/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanmodule.c
 * (C) 1999 Austin Donnelly <austin@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "picmanmodule.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmanmodule
 * @title: PicmanModule
 * @short_description: A #GTypeModule subclass which implements module
 *                     loading using #GModule.
 * @see_also: #GModule, #GTypeModule
 *
 * A #GTypeModule subclass which implements module loading using #GModule.
 **/


enum
{
  MODIFIED,
  LAST_SIGNAL
};


static void       picman_module_finalize       (GObject     *object);

static gboolean   picman_module_load           (GTypeModule *module);
static void       picman_module_unload         (GTypeModule *module);

static gboolean   picman_module_open           (PicmanModule  *module);
static gboolean   picman_module_close          (PicmanModule  *module);
static void       picman_module_set_last_error (PicmanModule  *module,
                                              const gchar *error_str);


G_DEFINE_TYPE (PicmanModule, picman_module, G_TYPE_TYPE_MODULE)

#define parent_class picman_module_parent_class

static guint module_signals[LAST_SIGNAL];


static void
picman_module_class_init (PicmanModuleClass *klass)
{
  GObjectClass     *object_class = G_OBJECT_CLASS (klass);
  GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS (klass);

  module_signals[MODIFIED] =
    g_signal_new ("modified",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanModuleClass, modified),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->finalize = picman_module_finalize;

  module_class->load     = picman_module_load;
  module_class->unload   = picman_module_unload;

  klass->modified        = NULL;
}

static void
picman_module_init (PicmanModule *module)
{
  module->filename          = NULL;
  module->verbose           = FALSE;
  module->state             = PICMAN_MODULE_STATE_ERROR;
  module->on_disk           = FALSE;
  module->load_inhibit      = FALSE;

  module->module            = NULL;
  module->info              = NULL;
  module->last_module_error = NULL;

  module->query_module      = NULL;
  module->register_module   = NULL;
}

static void
picman_module_finalize (GObject *object)
{
  PicmanModule *module = PICMAN_MODULE (object);

  if (module->info)
    {
      picman_module_info_free (module->info);
      module->info = NULL;
    }

  if (module->last_module_error)
    {
      g_free (module->last_module_error);
      module->last_module_error = NULL;
    }

  if (module->filename)
    {
      g_free (module->filename);
      module->filename = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
picman_module_load (GTypeModule *module)
{
  PicmanModule *picman_module = PICMAN_MODULE (module);
  gpointer    func;

  g_return_val_if_fail (picman_module->filename != NULL, FALSE);
  g_return_val_if_fail (picman_module->module == NULL, FALSE);

  if (picman_module->verbose)
    g_print ("Loading module '%s'\n",
             picman_filename_to_utf8 (picman_module->filename));

  if (! picman_module_open (picman_module))
    return FALSE;

  if (! picman_module_query_module (picman_module))
    return FALSE;

  /* find the picman_module_register symbol */
  if (! g_module_symbol (picman_module->module, "picman_module_register", &func))
    {
      picman_module_set_last_error (picman_module,
                                  "Missing picman_module_register() symbol");

      g_message (_("Module '%s' load error: %s"),
                 picman_filename_to_utf8 (picman_module->filename),
                 picman_module->last_module_error);

      picman_module_close (picman_module);

      picman_module->state = PICMAN_MODULE_STATE_ERROR;

      return FALSE;
    }

  picman_module->register_module = func;

  if (! picman_module->register_module (module))
    {
      picman_module_set_last_error (picman_module,
                                  "picman_module_register() returned FALSE");

      g_message (_("Module '%s' load error: %s"),
                 picman_filename_to_utf8 (picman_module->filename),
                 picman_module->last_module_error);

      picman_module_close (picman_module);

      picman_module->state = PICMAN_MODULE_STATE_LOAD_FAILED;

      return FALSE;
    }

  picman_module->state = PICMAN_MODULE_STATE_LOADED;

  return TRUE;
}

static void
picman_module_unload (GTypeModule *module)
{
  PicmanModule *picman_module = PICMAN_MODULE (module);

  g_return_if_fail (picman_module->module != NULL);

  if (picman_module->verbose)
    g_print ("Unloading module '%s'\n",
             picman_filename_to_utf8 (picman_module->filename));

  picman_module_close (picman_module);
}


/*  public functions  */

/**
 * picman_module_new:
 * @filename:     The filename of a loadable module.
 * @load_inhibit: Pass %TRUE to exclude this module from auto-loading.
 * @verbose:      Pass %TRUE to enable debugging output.
 *
 * Creates a new #PicmanModule instance.
 *
 * Return value: The new #PicmanModule object.
 **/
PicmanModule *
picman_module_new (const gchar *filename,
                 gboolean     load_inhibit,
                 gboolean     verbose)
{
  PicmanModule *module;

  g_return_val_if_fail (filename != NULL, NULL);

  module = g_object_new (PICMAN_TYPE_MODULE, NULL);

  module->filename     = g_strdup (filename);
  module->load_inhibit = load_inhibit ? TRUE : FALSE;
  module->verbose      = verbose ? TRUE : FALSE;
  module->on_disk      = TRUE;

  if (! module->load_inhibit)
    {
      if (picman_module_load (G_TYPE_MODULE (module)))
        picman_module_unload (G_TYPE_MODULE (module));
    }
  else
    {
      if (verbose)
        g_print ("Skipping module '%s'\n",
                 picman_filename_to_utf8 (filename));

      module->state = PICMAN_MODULE_STATE_NOT_LOADED;
    }

  return module;
}

/**
 * picman_module_query_module:
 * @module: A #PicmanModule.
 *
 * Queries the module without actually registering any of the types it
 * may implement. After successful query, the @info field of the
 * #PicmanModule struct will be available for further inspection.
 *
 * Return value: %TRUE on success.
 **/
gboolean
picman_module_query_module (PicmanModule *module)
{
  const PicmanModuleInfo *info;
  gboolean              close_module = FALSE;
  gpointer              func;

  g_return_val_if_fail (PICMAN_IS_MODULE (module), FALSE);

  if (! module->module)
    {
      if (! picman_module_open (module))
        return FALSE;

      close_module = TRUE;
    }

  /* find the picman_module_query symbol */
  if (! g_module_symbol (module->module, "picman_module_query", &func))
    {
      picman_module_set_last_error (module,
                                  "Missing picman_module_query() symbol");

      g_message (_("Module '%s' load error: %s"),
                 picman_filename_to_utf8 (module->filename),
                 module->last_module_error);

      picman_module_close (module);

      module->state = PICMAN_MODULE_STATE_ERROR;
      return FALSE;
    }

  module->query_module = func;

  if (module->info)
    {
      picman_module_info_free (module->info);
      module->info = NULL;
    }

  info = module->query_module (G_TYPE_MODULE (module));

  if (! info || info->abi_version != PICMAN_MODULE_ABI_VERSION)
    {
      picman_module_set_last_error (module,
                                  info ?
                                  "module ABI version does not match" :
                                  "picman_module_query() returned NULL");

      g_message (_("Module '%s' load error: %s"),
                 picman_filename_to_utf8 (module->filename),
                 module->last_module_error);

      picman_module_close (module);

      module->state = PICMAN_MODULE_STATE_ERROR;
      return FALSE;
    }

  module->info = picman_module_info_copy (info);

  if (close_module)
    return picman_module_close (module);

  return TRUE;
}

/**
 * picman_module_modified:
 * @module: A #PicmanModule.
 *
 * Emits the "modified" signal. Call it whenever you have modified the module
 * manually (which you shouldn't do).
 **/
void
picman_module_modified (PicmanModule *module)
{
  g_return_if_fail (PICMAN_IS_MODULE (module));

  g_signal_emit (module, module_signals[MODIFIED], 0);
}

/**
 * picman_module_set_load_inhibit:
 * @module:       A #PicmanModule.
 * @load_inhibit: Pass %TRUE to exclude this module from auto-loading.
 *
 * Sets the @load_inhibit property if the module. Emits "modified".
 **/
void
picman_module_set_load_inhibit (PicmanModule *module,
                              gboolean    load_inhibit)
{
  g_return_if_fail (PICMAN_IS_MODULE (module));

  if (load_inhibit != module->load_inhibit)
    {
      module->load_inhibit = load_inhibit ? TRUE : FALSE;

      picman_module_modified (module);
    }
}

/**
 * picman_module_state_name:
 * @state: A #PicmanModuleState.
 *
 * Returns the translated textual representation of a #PicmanModuleState.
 * The returned string must not be freed.
 *
 * Return value: The @state's name.
 **/
const gchar *
picman_module_state_name (PicmanModuleState state)
{
  static const gchar * const statenames[] =
  {
    N_("Module error"),
    N_("Loaded"),
    N_("Load failed"),
    N_("Not loaded")
  };

  g_return_val_if_fail (state >= PICMAN_MODULE_STATE_ERROR &&
                        state <= PICMAN_MODULE_STATE_NOT_LOADED, NULL);

  return gettext (statenames[state]);
}

/**
 * picman_module_register_enum:
 * @module:              a module
 * @name:                the name of the new enum type
 * @const_static_values: the enum values
 *
 * This function is deprecated! Use g_type_module_register_enum() instead.
 *
 * Return value: a new enum #GType
 **/
GType
picman_module_register_enum (GTypeModule      *module,
                           const gchar      *name,
                           const GEnumValue *const_static_values)
{
  return g_type_module_register_enum (module, name, const_static_values);
}

/**
 * picman_module_error_quark:
 *
 * This function is never called directly. Use PICMAN_MODULE_ERROR() instead.
 *
 * Return value: the #GQuark that defines the PICMAN module error domain.
 *
 * Since: PICMAN 2.8
 **/
GQuark
picman_module_error_quark (void)
{
  return g_quark_from_static_string ("picman-module-error-quark");
}


/*  private functions  */

static gboolean
picman_module_open (PicmanModule *module)
{
  module->module = g_module_open (module->filename, 0);

  if (! module->module)
    {
      module->state = PICMAN_MODULE_STATE_ERROR;
      picman_module_set_last_error (module, g_module_error ());

      g_message (_("Module '%s' load error: %s"),
                 picman_filename_to_utf8 (module->filename),
                 module->last_module_error);

      return FALSE;
    }

  return TRUE;
}

static gboolean
picman_module_close (PicmanModule *module)
{
  g_module_close (module->module); /* FIXME: error handling */
  module->module          = NULL;
  module->query_module    = NULL;
  module->register_module = NULL;

  module->state = PICMAN_MODULE_STATE_NOT_LOADED;

  return TRUE;
}

static void
picman_module_set_last_error (PicmanModule  *module,
                            const gchar *error_str)
{
  if (module->last_module_error)
    g_free (module->last_module_error);

  module->last_module_error = g_strdup (error_str);
}


/*  PicmanModuleInfo functions  */

/**
 * picman_module_info_new:
 * @abi_version: The #PICMAN_MODULE_ABI_VERSION the module was compiled against.
 * @purpose:     The module's general purpose.
 * @author:      The module's author.
 * @version:     The module's version.
 * @copyright:   The module's copyright.
 * @date:        The module's release date.
 *
 * Creates a newly allocated #PicmanModuleInfo struct.
 *
 * Return value: The new #PicmanModuleInfo struct.
 **/
PicmanModuleInfo *
picman_module_info_new (guint32      abi_version,
                      const gchar *purpose,
                      const gchar *author,
                      const gchar *version,
                      const gchar *copyright,
                      const gchar *date)
{
  PicmanModuleInfo *info = g_slice_new0 (PicmanModuleInfo);

  info->abi_version = abi_version;
  info->purpose     = g_strdup (purpose);
  info->author      = g_strdup (author);
  info->version     = g_strdup (version);
  info->copyright   = g_strdup (copyright);
  info->date        = g_strdup (date);

  return info;
}

/**
 * picman_module_info_copy:
 * @info: The #PicmanModuleInfo struct to copy.
 *
 * Copies a #PicmanModuleInfo struct.
 *
 * Return value: The new copy.
 **/
PicmanModuleInfo *
picman_module_info_copy (const PicmanModuleInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return picman_module_info_new (info->abi_version,
                               info->purpose,
                               info->author,
                               info->version,
                               info->copyright,
                               info->date);
}

/**
 * picman_module_info_free:
 * @info: The #PicmanModuleInfo struct to free
 *
 * Frees the passed #PicmanModuleInfo.
 **/
void
picman_module_info_free (PicmanModuleInfo *info)
{
  g_return_if_fail (info != NULL);

  g_free (info->purpose);
  g_free (info->author);
  g_free (info->version);
  g_free (info->copyright);
  g_free (info->date);

  g_slice_free (PicmanModuleInfo, info);
}
