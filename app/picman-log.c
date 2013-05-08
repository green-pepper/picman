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

#include "glib-object.h"

#include "picman-debug.h"
#include "picman-log.h"


static const GDebugKey log_keys[] =
{
  { "tool-events",        PICMAN_LOG_TOOL_EVENTS        },
  { "tool-focus",         PICMAN_LOG_TOOL_FOCUS         },
  { "dnd",                PICMAN_LOG_DND                },
  { "help",               PICMAN_LOG_HELP               },
  { "dialog-factory",     PICMAN_LOG_DIALOG_FACTORY     },
  { "menus",              PICMAN_LOG_MENUS              },
  { "save-dialog",        PICMAN_LOG_SAVE_DIALOG        },
  { "image-scale",        PICMAN_LOG_IMAGE_SCALE        },
  { "shadow-tiles",       PICMAN_LOG_SHADOW_TILES       },
  { "scale",              PICMAN_LOG_SCALE              },
  { "wm",                 PICMAN_LOG_WM                 },
  { "floating-selection", PICMAN_LOG_FLOATING_SELECTION },
  { "shm",                PICMAN_LOG_SHM                },
  { "text-editing",       PICMAN_LOG_TEXT_EDITING       },
  { "key-events",         PICMAN_LOG_KEY_EVENTS         },
  { "auto-tab-style",     PICMAN_LOG_AUTO_TAB_STYLE     },
  { "instances",          PICMAN_LOG_INSTANCES          },
  { "rectangle-tool",     PICMAN_LOG_RECTANGLE_TOOL     },
  { "brush-cache",        PICMAN_LOG_BRUSH_CACHE        }
};


PicmanLogFlags picman_log_flags = 0;


void
picman_log_init (void)
{
  const gchar *env_log_val = g_getenv ("PICMAN_LOG");

  if (! env_log_val)
    env_log_val = g_getenv ("PICMAN_DEBUG");

  if (env_log_val)
    g_setenv ("G_MESSAGES_DEBUG", env_log_val, TRUE);

  if (env_log_val)
    {
      /*  g_parse_debug_string() has special treatment of the string 'help',
       *  but we want to use it for the PICMAN_LOG_HELP domain
       */
      if (g_ascii_strcasecmp (env_log_val, "help") == 0)
        picman_log_flags = PICMAN_LOG_HELP;
      else
        picman_log_flags = g_parse_debug_string (env_log_val,
                                               log_keys,
                                               G_N_ELEMENTS (log_keys));

      if (picman_log_flags & PICMAN_LOG_INSTANCES)
        picman_debug_enable_instances ();
    }
}

void
picman_log (PicmanLogFlags  flags,
          const gchar  *function,
          gint          line,
          const gchar  *format,
          ...)
{
  va_list args;

  va_start (args, format);
  picman_logv (flags, function, line, format, args);
  va_end (args);
}

void
picman_logv (PicmanLogFlags  flags,
           const gchar  *function,
           gint          line,
           const gchar  *format,
           va_list       args)
{
  const gchar *domain = "unknown";
  gchar       *message;
  gint         i;

  for (i = 0; i < G_N_ELEMENTS (log_keys); i++)
    if (log_keys[i].value == flags)
      {
        domain = log_keys[i].key;
        break;
      }

  if (format)
    message = g_strdup_vprintf (format, args);
  else
    message = g_strdup ("called");

  g_log (domain, G_LOG_LEVEL_DEBUG,
         "%s(%d): %s", function, line, message);

  g_free (message);
}
