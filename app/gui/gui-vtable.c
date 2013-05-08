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
#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#include "libpicmanwidgets/picmanwidgets.h"

#include "gui-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmanbrush.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmangradient.h"
#include "core/picmanimage.h"
#include "core/picmanimagefile.h"
#include "core/picmanlist.h"
#include "core/picmanpalette.h"
#include "core/picmanpattern.h"
#include "core/picmanprogress.h"

#include "text/picmanfont.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanbrushselect.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmandocked.h"
#include "widgets/picmanfontselect.h"
#include "widgets/picmangradientselect.h"
#include "widgets/picmanhelp.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmenufactory.h"
#include "widgets/picmanpaletteselect.h"
#include "widgets/picmanpatternselect.h"
#include "widgets/picmanprogressdialog.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"
#include "display/picmandisplay-foreach.h"
#include "display/picmandisplayshell.h"
#include "display/picmansinglewindowstrategy.h"
#include "display/picmanmultiwindowstrategy.h"

#include "actions/plug-in-actions.h"

#include "menus/menus.h"

#include "gui-message.h"
#include "gui-vtable.h"
#include "themes.h"


/*  local function prototypes  */

static void           gui_ungrab                (Picman                *picman);

static void           gui_threads_enter         (Picman                *picman);
static void           gui_threads_leave         (Picman                *picman);
static void           gui_set_busy              (Picman                *picman);
static void           gui_unset_busy            (Picman                *picman);
static void           gui_help                  (Picman                *picman,
                                                 PicmanProgress        *progress,
                                                 const gchar         *help_domain,
                                                 const gchar         *help_id);
static const gchar  * gui_get_program_class     (Picman                *picman);
static gchar        * gui_get_display_name      (Picman                *picman,
                                                 gint                 display_ID,
                                                 gint                *monitor_number);
static guint32        gui_get_user_time         (Picman                *picman);
static const gchar  * gui_get_theme_dir         (Picman                *picman);
static PicmanObject   * gui_get_window_strategy   (Picman                *picman);
static PicmanObject   * gui_get_empty_display     (Picman                *picman);
static PicmanObject   * gui_display_get_by_ID     (Picman                *picman,
                                                 gint                 ID);
static gint           gui_display_get_ID        (PicmanObject          *display);
static guint32        gui_display_get_window_id (PicmanObject          *display);
static PicmanObject   * gui_display_create        (Picman                *picman,
                                                 PicmanImage           *image,
                                                 PicmanUnit             unit,
                                                 gdouble              scale);
static void           gui_display_delete        (PicmanObject          *display);
static void           gui_displays_reconnect    (Picman                *picman,
                                                 PicmanImage           *old_image,
                                                 PicmanImage           *new_image);
static PicmanProgress * gui_new_progress          (Picman                *picman,
                                                 PicmanObject          *display);
static void           gui_free_progress         (Picman                *picman,
                                                 PicmanProgress        *progress);
static gboolean       gui_pdb_dialog_new        (Picman                *picman,
                                                 PicmanContext         *context,
                                                 PicmanProgress        *progress,
                                                 PicmanContainer       *container,
                                                 const gchar         *title,
                                                 const gchar         *callback_name,
                                                 const gchar         *object_name,
                                                 va_list              args);
static gboolean       gui_pdb_dialog_set        (Picman                *picman,
                                                 PicmanContainer       *container,
                                                 const gchar         *callback_name,
                                                 const gchar         *object_name,
                                                 va_list              args);
static gboolean       gui_pdb_dialog_close      (Picman                *picman,
                                                 PicmanContainer       *container,
                                                 const gchar         *callback_name);
static gboolean       gui_recent_list_add_uri   (Picman                *picman,
                                                 const gchar         *uri,
                                                 const gchar         *mime_type);
static void           gui_recent_list_load      (Picman                *picman);


/*  public functions  */

void
gui_vtable_init (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman->gui.ungrab                = gui_ungrab;
  picman->gui.threads_enter         = gui_threads_enter;
  picman->gui.threads_leave         = gui_threads_leave;
  picman->gui.set_busy              = gui_set_busy;
  picman->gui.unset_busy            = gui_unset_busy;
  picman->gui.show_message          = gui_message;
  picman->gui.help                  = gui_help;
  picman->gui.get_program_class     = gui_get_program_class;
  picman->gui.get_display_name      = gui_get_display_name;
  picman->gui.get_user_time         = gui_get_user_time;
  picman->gui.get_theme_dir         = gui_get_theme_dir;
  picman->gui.get_window_strategy   = gui_get_window_strategy;
  picman->gui.get_empty_display     = gui_get_empty_display;
  picman->gui.display_get_by_id     = gui_display_get_by_ID;
  picman->gui.display_get_id        = gui_display_get_ID;
  picman->gui.display_get_window_id = gui_display_get_window_id;
  picman->gui.display_create        = gui_display_create;
  picman->gui.display_delete        = gui_display_delete;
  picman->gui.displays_reconnect    = gui_displays_reconnect;
  picman->gui.progress_new          = gui_new_progress;
  picman->gui.progress_free         = gui_free_progress;
  picman->gui.pdb_dialog_new        = gui_pdb_dialog_new;
  picman->gui.pdb_dialog_set        = gui_pdb_dialog_set;
  picman->gui.pdb_dialog_close      = gui_pdb_dialog_close;
  picman->gui.recent_list_add_uri   = gui_recent_list_add_uri;
  picman->gui.recent_list_load      = gui_recent_list_load;
}


/*  private functions  */

static void
gui_ungrab (Picman *picman)
{
  GdkDisplay *display = gdk_display_get_default ();

  if (display)
    {
      gdk_display_pointer_ungrab (display, GDK_CURRENT_TIME);
      gdk_display_keyboard_ungrab (display, GDK_CURRENT_TIME);
    }
}

static void
gui_threads_enter (Picman *picman)
{
  GDK_THREADS_ENTER ();
}

static void
gui_threads_leave (Picman *picman)
{
  GDK_THREADS_LEAVE ();
}

static void
gui_set_busy (Picman *picman)
{
  picman_displays_set_busy (picman);
  picman_dialog_factory_set_busy (picman_dialog_factory_get_singleton ());

  gdk_flush ();
}

static void
gui_unset_busy (Picman *picman)
{
  picman_displays_unset_busy (picman);
  picman_dialog_factory_unset_busy (picman_dialog_factory_get_singleton ());

  gdk_flush ();
}

static void
gui_help (Picman         *picman,
          PicmanProgress *progress,
          const gchar  *help_domain,
          const gchar  *help_id)
{
  picman_help_show (picman, progress, help_domain, help_id);
}

static const gchar *
gui_get_program_class (Picman *picman)
{
  return gdk_get_program_class ();
}

static gchar *
gui_get_display_name (Picman *picman,
                      gint  display_ID,
                      gint *monitor_number)
{
  PicmanDisplay *display = NULL;
  GdkScreen   *screen;
  gint         monitor;

  if (display_ID > 0)
    display = picman_display_get_by_ID (picman, display_ID);

  if (display)
    {
      PicmanDisplayShell *shell  = picman_display_get_shell (display);
      GdkWindow        *window = gtk_widget_get_window (GTK_WIDGET (shell));

      screen  = gtk_widget_get_screen (GTK_WIDGET (shell));
      monitor = gdk_screen_get_monitor_at_window (screen, window);
    }
  else
    {
      gint x, y;

      gdk_display_get_pointer (gdk_display_get_default (),
                               &screen, &x, &y, NULL);
      monitor = gdk_screen_get_monitor_at_point (screen, x, y);
    }

  *monitor_number = monitor;

  if (screen)
    return gdk_screen_make_display_name (screen);

  return NULL;
}

static guint32
gui_get_user_time (Picman *picman)
{
#ifdef GDK_WINDOWING_X11
  return gdk_x11_display_get_user_time (gdk_display_get_default ());
#endif
  return 0;
}

static const gchar *
gui_get_theme_dir (Picman *picman)
{
  return themes_get_theme_dir (picman, PICMAN_GUI_CONFIG (picman->config)->theme);
}

static PicmanObject *
gui_get_window_strategy (Picman *picman)
{
  if (PICMAN_GUI_CONFIG (picman->config)->single_window_mode)
    return picman_single_window_strategy_get_singleton ();
  else
    return picman_multi_window_strategy_get_singleton ();
}

static PicmanObject *
gui_get_empty_display (Picman *picman)
{
  PicmanObject *display = NULL;

  if (picman_container_get_n_children (picman->displays) == 1)
    {
      display = picman_container_get_first_child (picman->displays);

      if (picman_display_get_image (PICMAN_DISPLAY (display)))
        {
          /* The display was not empty */
          display = NULL;
        }
    }

  return display;
}

static PicmanObject *
gui_display_get_by_ID (Picman *picman,
                       gint  ID)
{
  return (PicmanObject *) picman_display_get_by_ID (picman, ID);
}

static gint
gui_display_get_ID (PicmanObject *display)
{
  return picman_display_get_ID (PICMAN_DISPLAY (display));
}

static guint32
gui_display_get_window_id (PicmanObject *display)
{
  PicmanDisplay      *disp  = PICMAN_DISPLAY (display);
  PicmanDisplayShell *shell = picman_display_get_shell (disp);

  if (shell)
    {
      GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (shell));

      if (GTK_IS_WINDOW (toplevel))
        return picman_window_get_native_id (GTK_WINDOW (toplevel));
    }

  return 0;
}

static PicmanObject *
gui_display_create (Picman      *picman,
                    PicmanImage *image,
                    PicmanUnit   unit,
                    gdouble    scale)
{
  PicmanContext *context = picman_get_user_context (picman);
  PicmanDisplay *display = PICMAN_DISPLAY (gui_get_empty_display (picman));

  if (display)
    {
      picman_display_fill (display, image, unit, scale);
    }
  else
    {
      GList *image_managers = picman_ui_managers_from_name ("<Image>");

      g_return_val_if_fail (image_managers != NULL, NULL);

      display = picman_display_new (picman, image, unit, scale,
                                  global_menu_factory,
                                  image_managers->data,
                                  picman_dialog_factory_get_singleton ());
   }

  if (picman_context_get_display (context) == display)
    {
      picman_context_set_image (context, image);
      picman_context_display_changed (context);
    }
  else
    {
      picman_context_set_display (context, display);
    }

  return PICMAN_OBJECT (display);
}

static void
gui_display_delete (PicmanObject *display)
{
  picman_display_close (PICMAN_DISPLAY (display));
}

static void
gui_displays_reconnect (Picman      *picman,
                        PicmanImage *old_image,
                        PicmanImage *new_image)
{
  picman_displays_reconnect (picman, old_image, new_image);
}

static PicmanProgress *
gui_new_progress (Picman       *picman,
                  PicmanObject *display)
{
  g_return_val_if_fail (display == NULL || PICMAN_IS_DISPLAY (display), NULL);

  if (display)
    return PICMAN_PROGRESS (display);

  return PICMAN_PROGRESS (picman_progress_dialog_new ());
}

static void
gui_free_progress (Picman          *picman,
                   PicmanProgress  *progress)
{
  g_return_if_fail (PICMAN_IS_PROGRESS_DIALOG (progress));

  if (PICMAN_IS_PROGRESS_DIALOG (progress))
    gtk_widget_destroy (GTK_WIDGET (progress));
}

static gboolean
gui_pdb_dialog_present (GtkWindow *window)
{
  gtk_window_present (window);

  return FALSE;
}

static gboolean
gui_pdb_dialog_new (Picman          *picman,
                    PicmanContext   *context,
                    PicmanProgress  *progress,
                    PicmanContainer *container,
                    const gchar   *title,
                    const gchar   *callback_name,
                    const gchar   *object_name,
                    va_list        args)
{
  GType        dialog_type = G_TYPE_NONE;
  const gchar *dialog_role = NULL;
  const gchar *help_id     = NULL;

  if (picman_container_get_children_type (container) == PICMAN_TYPE_BRUSH)
    {
      dialog_type = PICMAN_TYPE_BRUSH_SELECT;
      dialog_role = "picman-brush-selection";
      help_id     = PICMAN_HELP_BRUSH_DIALOG;
    }
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_FONT)
    {
      dialog_type = PICMAN_TYPE_FONT_SELECT;
      dialog_role = "picman-font-selection";
      help_id     = PICMAN_HELP_FONT_DIALOG;
    }
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_GRADIENT)
    {
      dialog_type = PICMAN_TYPE_GRADIENT_SELECT;
      dialog_role = "picman-gradient-selection";
      help_id     = PICMAN_HELP_GRADIENT_DIALOG;
    }
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_PALETTE)
    {
      dialog_type = PICMAN_TYPE_PALETTE_SELECT;
      dialog_role = "picman-palette-selection";
      help_id     = PICMAN_HELP_PALETTE_DIALOG;
    }
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_PATTERN)
    {
      dialog_type = PICMAN_TYPE_PATTERN_SELECT;
      dialog_role = "picman-pattern-selection";
      help_id     = PICMAN_HELP_PATTERN_DIALOG;
    }

  if (dialog_type != G_TYPE_NONE)
    {
      PicmanObject *object = NULL;

      if (object_name && strlen (object_name))
        object = picman_container_get_child_by_name (container, object_name);

      if (! object)
        object = picman_context_get_by_type (context,
                                           picman_container_get_children_type (container));

      if (object)
        {
          GParameter *params   = NULL;
          gint        n_params = 0;
          GtkWidget  *dialog;
          GtkWidget  *view;

          params = picman_parameters_append (dialog_type, params, &n_params,
                                           "title",          title,
                                           "role",           dialog_role,
                                           "help-func",      picman_standard_help_func,
                                           "help-id",        help_id,
                                           "pdb",            picman->pdb,
                                           "context",        context,
                                           "select-type",    picman_container_get_children_type (container),
                                           "initial-object", object,
                                           "callback-name",  callback_name,
                                           "menu-factory",   global_menu_factory,
                                           NULL);

          params = picman_parameters_append_valist (dialog_type,
                                                  params, &n_params,
                                                  args);

          dialog = g_object_newv (dialog_type, n_params, params);

          picman_parameters_free (params, n_params);

          view = PICMAN_PDB_DIALOG (dialog)->view;
          if (view)
            picman_docked_set_show_button_bar (PICMAN_DOCKED (view), FALSE);

          if (progress)
            {
              guint32 window_id = picman_progress_get_window_id (progress);

              if (window_id)
                picman_window_set_transient_for (GTK_WINDOW (dialog), window_id);
            }

          gtk_widget_show (dialog);

          /*  workaround for bug #360106  */
          {
            GSource  *source = g_timeout_source_new (100);
            GClosure *closure;

            closure = g_cclosure_new_object (G_CALLBACK (gui_pdb_dialog_present),
                                             G_OBJECT (dialog));

            g_source_set_closure (source, closure);
            g_source_attach (source, NULL);
            g_source_unref (source);
          }

          return TRUE;
        }
    }

  return FALSE;
}

static gboolean
gui_pdb_dialog_set (Picman          *picman,
                    PicmanContainer *container,
                    const gchar   *callback_name,
                    const gchar   *object_name,
                    va_list        args)
{
  PicmanPdbDialogClass *klass = NULL;

  if (picman_container_get_children_type (container) == PICMAN_TYPE_BRUSH)
    klass = g_type_class_peek (PICMAN_TYPE_BRUSH_SELECT);
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_FONT)
    klass = g_type_class_peek (PICMAN_TYPE_FONT_SELECT);
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_GRADIENT)
    klass = g_type_class_peek (PICMAN_TYPE_GRADIENT_SELECT);
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_PALETTE)
    klass = g_type_class_peek (PICMAN_TYPE_PALETTE_SELECT);
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_PATTERN)
    klass = g_type_class_peek (PICMAN_TYPE_PATTERN_SELECT);

  if (klass)
    {
      PicmanPdbDialog *dialog;

      dialog = picman_pdb_dialog_get_by_callback (klass, callback_name);

      if (dialog && dialog->select_type == picman_container_get_children_type (container))
        {
          PicmanObject *object;

          object = picman_container_get_child_by_name (container, object_name);

          if (object)
            {
              const gchar *prop_name = va_arg (args, const gchar *);

              picman_context_set_by_type (dialog->context, dialog->select_type,
                                        object);

              if (prop_name)
                g_object_set_valist (G_OBJECT (dialog), prop_name, args);

              gtk_window_present (GTK_WINDOW (dialog));

              return TRUE;
            }
        }
    }

  return FALSE;
}

static gboolean
gui_pdb_dialog_close (Picman          *picman,
                      PicmanContainer *container,
                      const gchar   *callback_name)
{
  PicmanPdbDialogClass *klass = NULL;

  if (picman_container_get_children_type (container) == PICMAN_TYPE_BRUSH)
    klass = g_type_class_peek (PICMAN_TYPE_BRUSH_SELECT);
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_FONT)
    klass = g_type_class_peek (PICMAN_TYPE_FONT_SELECT);
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_GRADIENT)
    klass = g_type_class_peek (PICMAN_TYPE_GRADIENT_SELECT);
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_PALETTE)
    klass = g_type_class_peek (PICMAN_TYPE_PALETTE_SELECT);
  else if (picman_container_get_children_type (container) == PICMAN_TYPE_PATTERN)
    klass = g_type_class_peek (PICMAN_TYPE_PATTERN_SELECT);

  if (klass)
    {
      PicmanPdbDialog *dialog;

      dialog = picman_pdb_dialog_get_by_callback (klass, callback_name);

      if (dialog && dialog->select_type == picman_container_get_children_type (container))
        {
          gtk_widget_destroy (GTK_WIDGET (dialog));
          return TRUE;
        }
    }

  return FALSE;
}

static gboolean
gui_recent_list_add_uri (Picman        *picman,
                         const gchar *uri,
                         const gchar *mime_type)
{
  GtkRecentData  recent;
  const gchar   *groups[2] = { "Graphics", NULL };

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);

  /* use last part of the URI */
  recent.display_name = NULL;

  /* no special description */
  recent.description  = NULL;
  recent.mime_type    = (mime_type ?
                         (gchar *) mime_type : "application/octet-stream");
  recent.app_name     = "GNU Image Manipulation Program";
  recent.app_exec     = PICMAN_COMMAND " %u";
  recent.groups       = (gchar **) groups;
  recent.is_private   = FALSE;

  return gtk_recent_manager_add_full (gtk_recent_manager_get_default (),
                                      uri, &recent);
}

static gint
gui_recent_list_compare (gconstpointer a,
                         gconstpointer b)
{
  return (gtk_recent_info_get_modified ((GtkRecentInfo *) a) -
          gtk_recent_info_get_modified ((GtkRecentInfo *) b));
}

static void
gui_recent_list_load (Picman *picman)
{
  GList *items;
  GList *list;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman_container_freeze (picman->documents);
  picman_container_clear (picman->documents);

  items = gtk_recent_manager_get_items (gtk_recent_manager_get_default ());

  items = g_list_sort (items, gui_recent_list_compare);

  for (list = items; list; list = list->next)
    {
      GtkRecentInfo *info = list->data;

      if (gtk_recent_info_has_application (info,
                                           "GNU Image Manipulation Program"))
        {
          PicmanImagefile *imagefile;

          imagefile = picman_imagefile_new (picman,
                                          gtk_recent_info_get_uri (info));

          picman_imagefile_set_mime_type (imagefile,
                                        gtk_recent_info_get_mime_type (info));

          picman_container_add (picman->documents, PICMAN_OBJECT (imagefile));
          g_object_unref (imagefile);
        }

      gtk_recent_info_unref (info);
    }

  g_list_free (items);

  picman_container_thaw (picman->documents);
}
