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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmanimage.h"
#include "core/picmanviewable.h"

#include "file/file-utils.h"
#include "file/picman-file.h"

#include "plug-in/picmanpluginmanager-file.h"

#include "widgets/picmanaction.h"
#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "actions.h"
#include "file-actions.h"
#include "file-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void    file_actions_last_opened_update  (PicmanContainer   *container,
                                                 PicmanImagefile   *unused,
                                                 PicmanActionGroup *group);
static void    file_actions_last_opened_reorder (PicmanContainer   *container,
                                                 PicmanImagefile   *unused1,
                                                 gint             unused2,
                                                 PicmanActionGroup *group);
static void    file_actions_close_all_update    (PicmanContainer   *images,
                                                 PicmanObject      *unused,
                                                 PicmanActionGroup *group);
static gchar * file_actions_create_label        (const gchar     *format,
                                                 const gchar     *uri);


static const PicmanActionEntry file_actions[] =
{
  { "file-menu",             NULL, NC_("file-action", "_File")        },
  { "file-create-menu",      NULL, NC_("file-action", "Crea_te")      },
  { "file-open-recent-menu", NULL, NC_("file-action", "Open _Recent") },

  { "file-open", GTK_STOCK_OPEN,
    NC_("file-action", "_Open..."), NULL,
    NC_("file-action", "Open an image file"),
    G_CALLBACK (file_open_cmd_callback),
    PICMAN_HELP_FILE_OPEN },

  { "file-open-as-layers", PICMAN_STOCK_LAYER,
    NC_("file-action", "Op_en as Layers..."), "<primary><alt>O",
    NC_("file-action", "Open an image file as layers"),
    G_CALLBACK (file_open_as_layers_cmd_callback),
    PICMAN_HELP_FILE_OPEN_AS_LAYER },

  { "file-open-location", PICMAN_STOCK_WEB,
    NC_("file-action", "Open _Location..."), NULL,
    NC_("file-action", "Open an image file from a specified location"),
    G_CALLBACK (file_open_location_cmd_callback),
    PICMAN_HELP_FILE_OPEN_LOCATION },

  { "file-create-template", NULL,
    NC_("file-action", "Create Template..."), NULL,
    NC_("file-action", "Create a new template from this image"),
    G_CALLBACK (file_create_template_cmd_callback),
    PICMAN_HELP_FILE_CREATE_TEMPLATE },

  { "file-revert", GTK_STOCK_REVERT_TO_SAVED,
    NC_("file-action", "Re_vert"), NULL,
    NC_("file-action", "Reload the image file from disk"),
    G_CALLBACK (file_revert_cmd_callback),
    PICMAN_HELP_FILE_REVERT },

  { "file-close-all", GTK_STOCK_CLOSE,
    NC_("file-action", "Close all"), "<primary><shift>W",
    NC_("file-action", "Close all opened images"),
    G_CALLBACK (file_close_all_cmd_callback),
    PICMAN_HELP_FILE_CLOSE_ALL },

  { "file-quit", GTK_STOCK_QUIT,
    NC_("file-action", "_Quit"), "<primary>Q",
    NC_("file-action", "Quit the GNU Image Manipulation Program"),
    G_CALLBACK (file_quit_cmd_callback),
    PICMAN_HELP_FILE_QUIT }
};

static const PicmanEnumActionEntry file_save_actions[] =
{
  { "file-save", GTK_STOCK_SAVE,
    NC_("file-action", "_Save"), "<primary>S",
    NC_("file-action", "Save this image"),
    PICMAN_SAVE_MODE_SAVE, FALSE,
    PICMAN_HELP_FILE_SAVE },

  { "file-save-as", GTK_STOCK_SAVE_AS,
    NC_("file-action", "Save _As..."), "<primary><shift>S",
    NC_("file-action", "Save this image with a different name"),
    PICMAN_SAVE_MODE_SAVE_AS, FALSE,
    PICMAN_HELP_FILE_SAVE_AS },

  { "file-save-a-copy", NULL,
    NC_("file-action", "Save a Cop_y..."), NULL,
    NC_("file-action",
        "Save a copy of this image, without affecting the source file (if any) or the current state of the image"),
    PICMAN_SAVE_MODE_SAVE_A_COPY, FALSE,
    PICMAN_HELP_FILE_SAVE_A_COPY },

  { "file-save-and-close", NULL,
    NC_("file-action", "Save and Close..."), NULL,
    NC_("file-action", "Save this image and close its window"),
    PICMAN_SAVE_MODE_SAVE_AND_CLOSE, FALSE,
    PICMAN_HELP_FILE_SAVE },

  { "file-export-to", NULL,
    NC_("file-action", "Export to"), "<primary>E",
    NC_("file-action", "Export the image again"),
    PICMAN_SAVE_MODE_EXPORT_TO, FALSE,
    PICMAN_HELP_FILE_EXPORT_TO },

  { "file-overwrite", NULL,
    NC_("file-action", "Over_write"), "",
    NC_("file-action", "Export the image back to the imported file in the import format"),
    PICMAN_SAVE_MODE_OVERWRITE, FALSE,
    PICMAN_HELP_FILE_OVERWRITE },

  { "file-export", NULL,
    NC_("file-action", "Export..."), "<primary><shift>E",
    NC_("file-action", "Export the image to various file formats such as PNG or JPEG"),
    PICMAN_SAVE_MODE_EXPORT, FALSE,
    PICMAN_HELP_FILE_EXPORT }
};

void
file_actions_setup (PicmanActionGroup *group)
{
  PicmanEnumActionEntry *entries;
  gint                 n_entries;
  gint                 i;

  picman_action_group_add_actions (group, "file-action",
                                 file_actions,
                                 G_N_ELEMENTS (file_actions));

  picman_action_group_add_enum_actions (group, "file-action",
                                      file_save_actions,
                                      G_N_ELEMENTS (file_save_actions),
                                      G_CALLBACK (file_save_cmd_callback));

  n_entries = PICMAN_GUI_CONFIG (group->picman->config)->last_opened_size;

  entries = g_new0 (PicmanEnumActionEntry, n_entries);

  for (i = 0; i < n_entries; i++)
    {
      entries[i].name           = g_strdup_printf ("file-open-recent-%02d",
                                                   i + 1);
      entries[i].stock_id       = GTK_STOCK_OPEN;
      entries[i].label          = entries[i].name;
      entries[i].tooltip        = NULL;
      entries[i].value          = i;
      entries[i].value_variable = FALSE;

      if (i < 9)
        entries[i].accelerator = g_strdup_printf ("<primary>%d", i + 1);
      else if (i == 9)
        entries[i].accelerator = "<primary>0";
      else
        entries[i].accelerator = "";
    }

  picman_action_group_add_enum_actions (group, NULL, entries, n_entries,
                                      G_CALLBACK (file_open_recent_cmd_callback));

  for (i = 0; i < n_entries; i++)
    {
      GtkAction *action;

      picman_action_group_set_action_visible (group, entries[i].name, FALSE);
      picman_action_group_set_action_always_show_image (group, entries[i].name,
                                                      TRUE);

      action = gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                            entries[i].name);
      g_object_set (action,
                    "context", picman_get_user_context (group->picman),
                    NULL);

      g_free ((gchar *) entries[i].name);
      if (i < 9)
        g_free ((gchar *) entries[i].accelerator);
    }

  g_free (entries);

  g_signal_connect_object (group->picman->documents, "add",
                           G_CALLBACK (file_actions_last_opened_update),
                           group, 0);
  g_signal_connect_object (group->picman->documents, "remove",
                           G_CALLBACK (file_actions_last_opened_update),
                           group, 0);
  g_signal_connect_object (group->picman->documents, "reorder",
                           G_CALLBACK (file_actions_last_opened_reorder),
                           group, 0);

  file_actions_last_opened_update (group->picman->documents, NULL, group);

  /*  also listen to image adding/removal so we catch the case where
   *  the last image is closed but its display stays open.
   */
  g_signal_connect_object (group->picman->images, "add",
                           G_CALLBACK (file_actions_close_all_update),
                           group, 0);
  g_signal_connect_object (group->picman->images, "remove",
                           G_CALLBACK (file_actions_close_all_update),
                           group, 0);

  file_actions_close_all_update (group->picman->displays, NULL, group);
}

void
file_actions_update (PicmanActionGroup *group,
                     gpointer         data)
{
  Picman         *picman           = action_data_get_picman (data);
  PicmanImage    *image          = action_data_get_image (data);
  PicmanDrawable *drawable       = NULL;
  const gchar  *source         = NULL;
  const gchar  *export         = NULL;
  gboolean      show_overwrite = FALSE;

  if (image)
    {
      drawable = picman_image_get_active_drawable (image);
      source   = picman_image_get_imported_uri (image);
      export   = picman_image_get_exported_uri (image);
    }

  show_overwrite =
    (source &&
     picman_plug_in_manager_uri_has_exporter (picman->plug_in_manager,
                                            source));

#define SET_VISIBLE(action,condition) \
        picman_action_group_set_action_visible (group, action, (condition) != 0)
#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)

  SET_SENSITIVE ("file-save",            drawable);
  SET_SENSITIVE ("file-save-as",         drawable);
  SET_SENSITIVE ("file-save-a-copy",     drawable);
  SET_SENSITIVE ("file-revert",          image && (picman_image_get_uri (image) || source));
  SET_SENSITIVE ("file-export-to",       drawable);
  SET_VISIBLE   ("file-export-to",       ! show_overwrite);
  SET_SENSITIVE ("file-overwrite",       show_overwrite);
  SET_VISIBLE   ("file-overwrite",       show_overwrite);
  SET_SENSITIVE ("file-export",          drawable);
  SET_SENSITIVE ("file-create-template", image);

  if (export)
    {
      gchar *label = file_actions_create_label (_("Export to %s"), export);
      picman_action_group_set_action_label (group, "file-export-to", label);
      g_free (label);
    }
  else if (show_overwrite)
    {
      gchar *label = file_actions_create_label (_("Over_write %s"), source);
      picman_action_group_set_action_label (group, "file-overwrite", label);
      g_free (label);

    }
  else
    {
      picman_action_group_set_action_label (group,
                                          "file-export-to", _("Export to"));
    }

  /*  needed for the empty display  */
  SET_SENSITIVE ("file-close-all", image);

#undef SET_SENSITIVE
}


/*  private functions  */

static void
file_actions_last_opened_update (PicmanContainer   *container,
                                 PicmanImagefile   *unused,
                                 PicmanActionGroup *group)
{
  gint num_documents;
  gint i;
  gint n = PICMAN_GUI_CONFIG (group->picman->config)->last_opened_size;

  num_documents = picman_container_get_n_children (container);

  for (i = 0; i < n; i++)
    {
      GtkAction *action;
      gchar     *name = g_strdup_printf ("file-open-recent-%02d", i + 1);

      action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), name);

      if (i < num_documents)
        {
          PicmanImagefile *imagefile = (PicmanImagefile *)
            picman_container_get_child_by_index (container, i);

          if (PICMAN_ACTION (action)->viewable != (PicmanViewable *) imagefile)
            {
              const gchar *uri;
              gchar       *filename;
              gchar       *basename;
              gchar       *escaped;

              uri = picman_object_get_name (imagefile);

              filename = file_utils_uri_display_name (uri);
              basename = file_utils_uri_display_basename (uri);

              escaped = picman_escape_uline (basename);

              g_free (basename);

              g_object_set (action,
                            "label",    escaped,
                            "tooltip",  filename,
                            "visible",  TRUE,
                            "viewable", imagefile,
                            NULL);

              g_free (filename);
              g_free (escaped);
            }
        }
      else
        {
          g_object_set (action,
                        "label",    name,
                        "tooltip",  NULL,
                        "visible",  FALSE,
                        "viewable", NULL,
                        NULL);
        }

      g_free (name);
    }
}

static void
file_actions_last_opened_reorder (PicmanContainer   *container,
                                  PicmanImagefile   *unused1,
                                  gint             unused2,
                                  PicmanActionGroup *group)
{
  file_actions_last_opened_update (container, unused1, group);
}

static void
file_actions_close_all_update (PicmanContainer   *images,
                               PicmanObject      *unused,
                               PicmanActionGroup *group)
{
  PicmanContainer *container  = group->picman->displays;
  gint           n_displays = picman_container_get_n_children (container);
  gboolean       sensitive  = (n_displays > 0);

  if (n_displays == 1)
    {
      PicmanDisplay *display;

      display = PICMAN_DISPLAY (picman_container_get_first_child (container));

      if (! picman_display_get_image (display))
        sensitive = FALSE;
    }

  picman_action_group_set_action_sensitive (group, "file-close-all", sensitive);
}

static gchar *
file_actions_create_label (const gchar *format,
                           const gchar *uri)
{
  gchar *basename         = file_utils_uri_display_basename (uri);
  gchar *escaped_basename = picman_escape_uline (basename);
  gchar *label            = g_strdup_printf (format, escaped_basename);

  g_free (escaped_basename);
  g_free (basename);

  return label;
}
