/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gegl.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"
#ifdef G_OS_WIN32
#include "libpicmanbase/picmanwin32-io.h" /* For S_IRGRP etc */
#endif
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman.h"
#include "picmanerror.h"
#include "picmantoolinfo.h"
#include "picmantooloptions.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_TOOL,
  PROP_TOOL_INFO
};


static void   picman_tool_options_config_iface_init (PicmanConfigInterface *iface);

static void   picman_tool_options_dispose           (GObject         *object);
static void   picman_tool_options_set_property      (GObject         *object,
                                                   guint            property_id,
                                                   const GValue    *value,
                                                   GParamSpec      *pspec);
static void   picman_tool_options_get_property      (GObject         *object,
                                                   guint            property_id,
                                                   GValue          *value,
                                                   GParamSpec      *pspec);

static void   picman_tool_options_real_reset        (PicmanToolOptions *tool_options);

static void   picman_tool_options_config_reset      (PicmanConfig      *config);

static void   picman_tool_options_tool_notify       (PicmanToolOptions *options,
                                                   GParamSpec      *pspec);


G_DEFINE_TYPE_WITH_CODE (PicmanToolOptions, picman_tool_options, PICMAN_TYPE_CONTEXT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_tool_options_config_iface_init))

#define parent_class picman_tool_options_parent_class


static void
picman_tool_options_class_init (PicmanToolOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose      = picman_tool_options_dispose;
  object_class->set_property = picman_tool_options_set_property;
  object_class->get_property = picman_tool_options_get_property;

  klass->reset               = picman_tool_options_real_reset;

  g_object_class_override_property (object_class, PROP_TOOL, "tool");

  g_object_class_install_property (object_class, PROP_TOOL_INFO,
                                   g_param_spec_object ("tool-info",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_TOOL_INFO,
                                                        PICMAN_PARAM_READWRITE));

}

static void
picman_tool_options_init (PicmanToolOptions *options)
{
  options->tool_info = NULL;

  g_signal_connect (options, "notify::tool",
                    G_CALLBACK (picman_tool_options_tool_notify),
                    NULL);
}

static void
picman_tool_options_config_iface_init (PicmanConfigInterface *iface)
{
  iface->reset = picman_tool_options_config_reset;
}

static void
picman_tool_options_dispose (GObject *object)
{
  PicmanToolOptions *options = PICMAN_TOOL_OPTIONS (object);

  if (options->tool_info)
    {
      g_object_unref (options->tool_info);
      options->tool_info = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

/*  This is such a horrible hack, but necessary because we
 *  a) load an option's tool-info from disk in many cases
 *  b) screwed up in the past and saved the wrong tool-info in some cases
 */
static PicmanToolInfo *
picman_tool_options_check_tool_info (PicmanToolOptions *options,
                                   PicmanToolInfo    *tool_info,
                                   gboolean         warn)
{
  if (tool_info && G_OBJECT_TYPE (options) == tool_info->tool_options_type)
    {
      return tool_info;
    }
  else
    {
      GList *list;

      for (list = picman_get_tool_info_iter (PICMAN_CONTEXT (options)->picman);
           list;
           list = g_list_next (list))
        {
          PicmanToolInfo *new_info = list->data;

          if (G_OBJECT_TYPE (options) == new_info->tool_options_type)
            {
              if (warn)
                g_printerr ("%s: correcting bogus deserialized tool "
                            "type '%s' with right type '%s'\n",
                            g_type_name (G_OBJECT_TYPE (options)),
                            tool_info ? picman_object_get_name (tool_info) : "NULL",
                            picman_object_get_name (new_info));

              return new_info;
            }
        }

      g_return_val_if_reached (NULL);
    }
}

static void
picman_tool_options_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanToolOptions *options = PICMAN_TOOL_OPTIONS (object);

  switch (property_id)
    {
    case PROP_TOOL:
      {
        PicmanToolInfo *tool_info = g_value_get_object (value);
        PicmanToolInfo *context_tool;

        context_tool = picman_context_get_tool (PICMAN_CONTEXT (options));

        g_return_if_fail (context_tool == NULL ||
                          context_tool == tool_info);

        tool_info = picman_tool_options_check_tool_info (options, tool_info, TRUE);

        if (! context_tool)
          picman_context_set_tool (PICMAN_CONTEXT (options), tool_info);
      }
      break;

    case PROP_TOOL_INFO:
      {
        PicmanToolInfo *tool_info = g_value_get_object (value);

        g_return_if_fail (options->tool_info == NULL ||
                          options->tool_info == tool_info);

        tool_info = picman_tool_options_check_tool_info (options, tool_info, TRUE);

        if (! options->tool_info)
          {
            options->tool_info = g_object_ref (tool_info);

            if (tool_info->context_props)
              picman_context_define_properties (PICMAN_CONTEXT (options),
                                              tool_info->context_props, FALSE);

            picman_context_set_serialize_properties (PICMAN_CONTEXT (options),
                                                   tool_info->context_props);
          }
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_tool_options_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanToolOptions *options = PICMAN_TOOL_OPTIONS (object);

  switch (property_id)
    {
    case PROP_TOOL:
      g_value_set_object (value, picman_context_get_tool (PICMAN_CONTEXT (options)));
      break;

    case PROP_TOOL_INFO:
      g_value_set_object (value, options->tool_info);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_tool_options_real_reset (PicmanToolOptions *tool_options)
{
  picman_config_reset (PICMAN_CONFIG (tool_options));
}

static void
picman_tool_options_config_reset (PicmanConfig *config)
{
  gchar *name = g_strdup (picman_object_get_name (config));

  picman_config_reset_properties (G_OBJECT (config));

  picman_object_take_name (PICMAN_OBJECT (config), name);
}

static void
picman_tool_options_tool_notify (PicmanToolOptions *options,
                               GParamSpec      *pspec)
{
  PicmanToolInfo *tool_info = picman_context_get_tool (PICMAN_CONTEXT (options));
  PicmanToolInfo *new_info;

  new_info = picman_tool_options_check_tool_info (options, tool_info, FALSE);

  if (tool_info && new_info != tool_info)
    g_warning ("%s: 'tool' property on %s was set to bogus value "
               "'%s', it MUST be '%s'.",
               G_STRFUNC,
               g_type_name (G_OBJECT_TYPE (options)),
               picman_object_get_name (tool_info),
               picman_object_get_name (new_info));
}


/*  public functions  */

void
picman_tool_options_reset (PicmanToolOptions *tool_options)
{
  g_return_if_fail (PICMAN_IS_TOOL_OPTIONS (tool_options));

  PICMAN_TOOL_OPTIONS_GET_CLASS (tool_options)->reset (tool_options);
}

gboolean
picman_tool_options_serialize (PicmanToolOptions  *tool_options,
                             GError          **error)
{
  gchar    *filename;
  gchar    *header;
  gchar    *footer;
  gboolean  retval;

  g_return_val_if_fail (PICMAN_IS_TOOL_OPTIONS (tool_options), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  filename = picman_tool_info_build_options_filename (tool_options->tool_info,
                                                    NULL);

  if (tool_options->tool_info->picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  header = g_strdup_printf ("PICMAN %s options",
                            picman_object_get_name (tool_options->tool_info));
  footer = g_strdup_printf ("end of %s options",
                            picman_object_get_name (tool_options->tool_info));

  retval = picman_config_serialize_to_file (PICMAN_CONFIG (tool_options),
                                          filename,
                                          header, footer,
                                          NULL,
                                          error);

  g_free (filename);
  g_free (header);
  g_free (footer);

  return retval;
}

gboolean
picman_tool_options_deserialize (PicmanToolOptions  *tool_options,
                               GError          **error)
{
  gchar    *filename;
  gboolean  retval;

  g_return_val_if_fail (PICMAN_IS_TOOL_OPTIONS (tool_options), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  filename = picman_tool_info_build_options_filename (tool_options->tool_info,
                                                    NULL);

  if (tool_options->tool_info->picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  retval = picman_config_deserialize_file (PICMAN_CONFIG (tool_options),
                                         filename,
                                         NULL,
                                         error);

  g_free (filename);

  return retval;
}

gboolean
picman_tool_options_delete (PicmanToolOptions  *tool_options,
                          GError          **error)
{
  gchar    *filename;
  gboolean  retval = TRUE;

  g_return_val_if_fail (PICMAN_IS_TOOL_OPTIONS (tool_options), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  filename = picman_tool_info_build_options_filename (tool_options->tool_info,
                                                    NULL);

  if (g_unlink (filename) != 0 && errno != ENOENT)
    {
      retval = FALSE;
      g_set_error (error, PICMAN_ERROR, PICMAN_FAILED,
		   _("Deleting \"%s\" failed: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
    }

  g_free (filename);

  return retval;
}

void
picman_tool_options_create_folder (void)
{
  gchar *filename = g_build_filename (picman_directory (), "tool-options", NULL);

  g_mkdir (filename,
           S_IRUSR | S_IWUSR | S_IXUSR |
           S_IRGRP | S_IXGRP |
           S_IROTH | S_IXOTH);

  g_free (filename);
}
