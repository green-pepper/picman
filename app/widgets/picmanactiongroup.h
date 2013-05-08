/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanactiongroup.h
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

#ifndef __PICMAN_ACTION_GROUP_H__
#define __PICMAN_ACTION_GROUP_H__


#define PICMAN_TYPE_ACTION_GROUP              (picman_action_group_get_type ())
#define PICMAN_ACTION_GROUP(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ACTION_GROUP, PicmanActionGroup))
#define PICMAN_ACTION_GROUP_CLASS(vtable)     (G_TYPE_CHECK_CLASS_CAST ((vtable), PICMAN_TYPE_ACTION_GROUP, PicmanActionGroupClass))
#define PICMAN_IS_ACTION_GROUP(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ACTION_GROUP))
#define PICMAN_IS_ACTION_GROUP_CLASS(vtable)  (G_TYPE_CHECK_CLASS_TYPE ((vtable), PICMAN_TYPE_ACTION_GROUP))
#define PICMAN_ACTION_GROUP_GET_CLASS(inst)   (G_TYPE_INSTANCE_GET_CLASS ((inst), PICMAN_TYPE_ACTION_GROUP, PicmanActionGroupClass))


typedef struct _PicmanActionGroupClass PicmanActionGroupClass;

struct _PicmanActionGroup
{
  GtkActionGroup             parent_instance;

  Picman                      *picman;
  gchar                     *label;
  gchar                     *stock_id;

  gpointer                   user_data;

  PicmanActionGroupUpdateFunc  update_func;
};

struct _PicmanActionGroupClass
{
  GtkActionGroupClass  parent_class;

  GHashTable          *groups;
};

struct _PicmanActionEntry
{
  const gchar *name;
  const gchar *stock_id;
  const gchar *label;
  const gchar *accelerator;
  const gchar *tooltip;
  GCallback    callback;

  const gchar *help_id;
};

struct _PicmanToggleActionEntry
{
  const gchar *name;
  const gchar *stock_id;
  const gchar *label;
  const gchar *accelerator;
  const gchar *tooltip;
  GCallback    callback;
  gboolean     is_active;

  const gchar *help_id;
};

struct _PicmanRadioActionEntry
{
  const gchar *name;
  const gchar *stock_id;
  const gchar *label;
  const gchar *accelerator;
  const gchar *tooltip;
  gint         value;

  const gchar *help_id;
};

struct _PicmanEnumActionEntry
{
  const gchar *name;
  const gchar *stock_id;
  const gchar *label;
  const gchar *accelerator;
  const gchar *tooltip;
  gint         value;
  gboolean     value_variable;

  const gchar *help_id;
};

struct _PicmanStringActionEntry
{
  const gchar *name;
  const gchar *stock_id;
  const gchar *label;
  const gchar *accelerator;
  const gchar *tooltip;
  const gchar *value;

  const gchar *help_id;
};

struct _PicmanPlugInActionEntry
{
  const gchar         *name;
  const gchar         *stock_id;
  const gchar         *label;
  const gchar         *accelerator;
  const gchar         *tooltip;
  PicmanPlugInProcedure *procedure;

  const gchar         *help_id;
};


GType            picman_action_group_get_type   (void) G_GNUC_CONST;

PicmanActionGroup *picman_action_group_new        (Picman                  *picman,
                                               const gchar           *name,
                                               const gchar           *label,
                                               const gchar           *stock_id,
                                               gpointer               user_data,
                                               PicmanActionGroupUpdateFunc update_func);

GList *picman_action_groups_from_name           (const gchar           *name);

void   picman_action_group_update               (PicmanActionGroup       *group,
                                               gpointer               update_data);

void   picman_action_group_add_actions          (PicmanActionGroup             *group,
					       const gchar                 *msg_context,
                                               const PicmanActionEntry       *entries,
                                               guint                        n_entries);
void   picman_action_group_add_toggle_actions   (PicmanActionGroup             *group,
					       const gchar                 *msg_context,
                                               const PicmanToggleActionEntry *entries,
                                               guint                        n_entries);
GSList *picman_action_group_add_radio_actions   (PicmanActionGroup             *group,
					       const gchar                 *msg_context,
                                               const PicmanRadioActionEntry  *entries,
                                               guint                        n_entries,
                                               GSList                      *radio_group,
                                               gint                         value,
                                               GCallback                    callback);
void   picman_action_group_add_enum_actions     (PicmanActionGroup             *group,
					       const gchar                 *msg_context,
                                               const PicmanEnumActionEntry   *entries,
                                               guint                        n_entries,
                                               GCallback                    callback);
void   picman_action_group_add_string_actions   (PicmanActionGroup             *group,
					       const gchar                 *msg_context,
                                               const PicmanStringActionEntry *entries,
                                               guint                        n_entries,
                                               GCallback                    callback);
void   picman_action_group_add_plug_in_actions  (PicmanActionGroup             *group,
                                               const PicmanPlugInActionEntry *entries,
                                               guint                        n_entries,
                                               GCallback                    callback);

void          picman_action_group_activate_action       (PicmanActionGroup *group,
                                                       const gchar     *action_name);
void          picman_action_group_set_action_visible    (PicmanActionGroup *group,
                                                       const gchar     *action_name,
                                                       gboolean         visible);
void          picman_action_group_set_action_sensitive  (PicmanActionGroup *group,
                                                       const gchar     *action_name,
                                                       gboolean         sensitive);
void          picman_action_group_set_action_active     (PicmanActionGroup *group,
                                                       const gchar     *action_name,
                                                       gboolean         active);
void          picman_action_group_set_action_label      (PicmanActionGroup *group,
                                                       const gchar     *action_name,
                                                       const gchar     *label);
void          picman_action_group_set_action_tooltip    (PicmanActionGroup *group,
                                                       const gchar     *action_name,
                                                       const gchar     *tooltip);
const gchar * picman_action_group_get_action_tooltip    (PicmanActionGroup *group,
                                                       const gchar     *action_name);
void          picman_action_group_set_action_color      (PicmanActionGroup *group,
                                                       const gchar     *action_name,
                                                       const PicmanRGB   *color,
                                                       gboolean         set_label);
void          picman_action_group_set_action_viewable   (PicmanActionGroup *group,
                                                       const gchar     *action_name,
                                                       PicmanViewable    *viewable);
void          picman_action_group_set_action_hide_empty (PicmanActionGroup *group,
                                                       const gchar     *action_name,
                                                       gboolean         hide_empty);
void   picman_action_group_set_action_always_show_image (PicmanActionGroup *group,
                                                       const gchar     *action_name,
                                                       gboolean         always_show_image);



#endif  /* __PICMAN_ACTION_GROUP_H__ */
