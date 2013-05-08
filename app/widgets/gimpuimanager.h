/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanuimanager.h
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_UI_MANAGER_H__
#define __PICMAN_UI_MANAGER_H__


typedef struct _PicmanUIManagerUIEntry PicmanUIManagerUIEntry;

struct _PicmanUIManagerUIEntry
{
  gchar                  *ui_path;
  gchar                  *basename;
  PicmanUIManagerSetupFunc  setup_func;
  guint                   merge_id;
  GtkWidget              *widget;
};


#define PICMAN_TYPE_UI_MANAGER              (picman_ui_manager_get_type ())
#define PICMAN_UI_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_UI_MANAGER, PicmanUIManager))
#define PICMAN_UI_MANAGER_CLASS(vtable)     (G_TYPE_CHECK_CLASS_CAST ((vtable), PICMAN_TYPE_UI_MANAGER, PicmanUIManagerClass))
#define PICMAN_IS_UI_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_UI_MANAGER))
#define PICMAN_IS_UI_MANAGER_CLASS(vtable)  (G_TYPE_CHECK_CLASS_TYPE ((vtable), PICMAN_TYPE_UI_MANAGER))
#define PICMAN_UI_MANAGER_GET_CLASS(inst)   (G_TYPE_INSTANCE_GET_CLASS ((inst), PICMAN_TYPE_UI_MANAGER, PicmanUIManagerClass))


typedef struct _PicmanUIManagerClass PicmanUIManagerClass;

/**
 * Among other things, is responsible for updating menu bars. A more
 * appropriate name would perhaps be PicmanMenubarManager.
 */
struct _PicmanUIManager
{
  GtkUIManager  parent_instance;

  gchar        *name;
  Picman         *picman;
  GList        *registered_uis;
};

struct _PicmanUIManagerClass
{
  GtkUIManagerClass  parent_class;

  GHashTable        *managers;

  void (* update)       (PicmanUIManager *manager,
                         gpointer       update_data);
  void (* show_tooltip) (PicmanUIManager *manager,
                         const gchar   *tooltip);
  void (* hide_tooltip) (PicmanUIManager *manager);
};


GType           picman_ui_manager_get_type    (void) G_GNUC_CONST;

PicmanUIManager * picman_ui_manager_new         (Picman                   *picman,
                                             const gchar            *name);

GList         * picman_ui_managers_from_name  (const gchar            *name);

void            picman_ui_manager_update      (PicmanUIManager          *manager,
                                             gpointer                update_data);
PicmanActionGroup * picman_ui_manager_get_action_group (PicmanUIManager   *manager,
                                                    const gchar     *name);

GtkAction     * picman_ui_manager_find_action     (PicmanUIManager      *manager,
                                                 const gchar        *group_name,
                                                 const gchar        *action_name);
gboolean        picman_ui_manager_activate_action (PicmanUIManager      *manager,
                                                 const gchar        *group_name,
                                                 const gchar        *action_name);

void            picman_ui_manager_ui_register (PicmanUIManager          *manager,
                                             const gchar            *ui_path,
                                             const gchar            *basename,
                                             PicmanUIManagerSetupFunc  setup_func);

void            picman_ui_manager_ui_popup    (PicmanUIManager          *manager,
                                             const gchar            *ui_path,
                                             GtkWidget              *parent,
                                             PicmanMenuPositionFunc    position_func,
                                             gpointer                position_data,
                                             GDestroyNotify          popdown_func,
                                             gpointer                popdown_data);


#endif  /* __PICMAN_UI_MANAGER_H__ */
