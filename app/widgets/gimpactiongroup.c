/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanactiongroup.c
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

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanviewable.h"

#include "picmanactiongroup.h"
#include "picmanaction.h"
#include "picmanenumaction.h"
#include "picmanpluginaction.h"
#include "picmanradioaction.h"
#include "picmanstringaction.h"
#include "picmantoggleaction.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_PICMAN,
  PROP_LABEL,
  PROP_STOCK_ID
};


static void   picman_action_group_constructed   (GObject      *object);
static void   picman_action_group_dispose       (GObject      *object);
static void   picman_action_group_finalize      (GObject      *object);
static void   picman_action_group_set_property  (GObject      *object,
                                               guint         prop_id,
                                               const GValue *value,
                                               GParamSpec   *pspec);
static void   picman_action_group_get_property  (GObject      *object,
                                               guint         prop_id,
                                               GValue       *value,
                                               GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanActionGroup, picman_action_group, GTK_TYPE_ACTION_GROUP)

#define parent_class picman_action_group_parent_class


static void
picman_action_group_class_init (PicmanActionGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_action_group_constructed;
  object_class->dispose      = picman_action_group_dispose;
  object_class->finalize     = picman_action_group_finalize;
  object_class->set_property = picman_action_group_set_property;
  object_class->get_property = picman_action_group_get_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_LABEL,
                                   g_param_spec_string ("label",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_STOCK_ID,
                                   g_param_spec_string ("stock-id",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  klass->groups = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}

static void
picman_action_group_init (PicmanActionGroup *group)
{
}

static void
picman_action_group_constructed (GObject *object)
{
  PicmanActionGroup *group = PICMAN_ACTION_GROUP (object);
  const gchar     *name;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (group->picman));

  name = gtk_action_group_get_name (GTK_ACTION_GROUP (object));

  if (name)
    {
      PicmanActionGroupClass *group_class;
      GList                *list;

      group_class = PICMAN_ACTION_GROUP_GET_CLASS (object);

      list = g_hash_table_lookup (group_class->groups, name);

      list = g_list_append (list, object);

      g_hash_table_replace (group_class->groups,
                            g_strdup (name), list);
    }
}

static void
picman_action_group_dispose (GObject *object)
{
  const gchar *name = gtk_action_group_get_name (GTK_ACTION_GROUP (object));

  if (name)
    {
      PicmanActionGroupClass *group_class;
      GList                *list;

      group_class = PICMAN_ACTION_GROUP_GET_CLASS (object);

      list = g_hash_table_lookup (group_class->groups, name);

      if (list)
        {
          list = g_list_remove (list, object);

          if (list)
            g_hash_table_replace (group_class->groups,
                                  g_strdup (name), list);
          else
            g_hash_table_remove (group_class->groups, name);
        }
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_action_group_finalize (GObject *object)
{
  PicmanActionGroup *group = PICMAN_ACTION_GROUP (object);

  if (group->label)
    {
      g_free (group->label);
      group->label = NULL;
    }

  if (group->stock_id)
    {
      g_free (group->stock_id);
      group->stock_id = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_action_group_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanActionGroup *group = PICMAN_ACTION_GROUP (object);

  switch (prop_id)
    {
    case PROP_PICMAN:
      group->picman = g_value_get_object (value);
      break;
    case PROP_LABEL:
      group->label = g_value_dup_string (value);
      break;
    case PROP_STOCK_ID:
      group->stock_id = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_action_group_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanActionGroup *group = PICMAN_ACTION_GROUP (object);

  switch (prop_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, group->picman);
      break;
    case PROP_LABEL:
      g_value_set_string (value, group->label);
      break;
    case PROP_STOCK_ID:
      g_value_set_string (value, group->stock_id);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static gboolean
picman_action_group_check_unique_action (PicmanActionGroup *group,
				       const gchar     *action_name)
{
  if (G_UNLIKELY (gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                               action_name)))
    {
      g_warning ("Refusing to add non-unique action '%s' to action group '%s'",
	 	 action_name,
                 gtk_action_group_get_name (GTK_ACTION_GROUP (group)));
      return FALSE;
    }

  return TRUE;

}

/**
 * picman_action_group_new:
 * @picman:        the @Picman instance this action group belongs to
 * @name:        the name of the action group.
 * @label:       the user visible label of the action group.
 * @stock_id:    the icon of the action group.
 * @user_data:   the user_data for #GtkAction callbacks.
 * @update_func: the function that will be called on
 *               picman_action_group_update().
 *
 * Creates a new #PicmanActionGroup object. The name of the action group
 * is used when associating <link linkend="Action-Accel">keybindings</link>
 * with the actions.
 *
 * Returns: the new #PicmanActionGroup
 */
PicmanActionGroup *
picman_action_group_new (Picman                      *picman,
                       const gchar               *name,
                       const gchar               *label,
                       const gchar               *stock_id,
                       gpointer                   user_data,
                       PicmanActionGroupUpdateFunc  update_func)
{
  PicmanActionGroup *group;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  group = g_object_new (PICMAN_TYPE_ACTION_GROUP,
                        "picman",      picman,
                        "name",      name,
                        "label",     label,
                        "stock-id",  stock_id,
                        NULL);

  group->user_data   = user_data;
  group->update_func = update_func;

  return group;
}

GList *
picman_action_groups_from_name (const gchar *name)
{
  PicmanActionGroupClass *group_class;
  GList                *list;

  g_return_val_if_fail (name != NULL, NULL);

  group_class = g_type_class_ref (PICMAN_TYPE_ACTION_GROUP);

  list = g_hash_table_lookup (group_class->groups, name);

  g_type_class_unref (group_class);

  return list;
}

void
picman_action_group_update (PicmanActionGroup *group,
                          gpointer         update_data)
{
  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));

  if (group->update_func)
    group->update_func (group, update_data);
}

void
picman_action_group_add_actions (PicmanActionGroup       *group,
			       const gchar           *msg_context,
                               const PicmanActionEntry *entries,
                               guint                  n_entries)
{
  gint i;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));

  for (i = 0; i < n_entries; i++)
    {
      PicmanAction  *action;
      const gchar *label;
      const gchar *tooltip = NULL;

      if (! picman_action_group_check_unique_action (group, entries[i].name))
        continue;

      if (msg_context)
        {
          label = g_dpgettext2 (NULL, msg_context, entries[i].label);

          if (entries[i].tooltip)
            tooltip = g_dpgettext2 (NULL, msg_context, entries[i].tooltip);
        }
      else
        {
          label   = gettext (entries[i].label);
          tooltip = gettext (entries[i].tooltip);
        }

      action = picman_action_new (entries[i].name, label, tooltip,
                                entries[i].stock_id);

      if (entries[i].callback)
        g_signal_connect (action, "activate",
                          entries[i].callback,
                          group->user_data);

      gtk_action_group_add_action_with_accel (GTK_ACTION_GROUP (group),
                                              GTK_ACTION (action),
                                              entries[i].accelerator);

      if (entries[i].help_id)
        g_object_set_qdata_full (G_OBJECT (action), PICMAN_HELP_ID,
                                 g_strdup (entries[i].help_id),
                                 (GDestroyNotify) g_free);

      g_object_unref (action);
    }
}

void
picman_action_group_add_toggle_actions (PicmanActionGroup             *group,
                                      const gchar                 *msg_context,
                                      const PicmanToggleActionEntry *entries,
                                      guint                        n_entries)
{
  gint i;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));

  for (i = 0; i < n_entries; i++)
    {
      GtkToggleAction *action;
      const gchar     *label;
      const gchar     *tooltip = NULL;

      if (! picman_action_group_check_unique_action (group, entries[i].name))
        continue;

      if (msg_context)
        {
          label = g_dpgettext2 (NULL, msg_context, entries[i].label);

          if (entries[i].tooltip)
            tooltip = g_dpgettext2 (NULL, msg_context, entries[i].tooltip);
        }
      else
        {
          label   = gettext (entries[i].label);
          tooltip = gettext (entries[i].tooltip);
        }

      action = picman_toggle_action_new (entries[i].name, label, tooltip,
                                       entries[i].stock_id);

      gtk_toggle_action_set_active (action, entries[i].is_active);

      if (entries[i].callback)
        g_signal_connect (action, "toggled",
                          entries[i].callback,
                          group->user_data);

      gtk_action_group_add_action_with_accel (GTK_ACTION_GROUP (group),
                                              GTK_ACTION (action),
                                              entries[i].accelerator);

      if (entries[i].help_id)
        g_object_set_qdata_full (G_OBJECT (action), PICMAN_HELP_ID,
                                 g_strdup (entries[i].help_id),
                                 (GDestroyNotify) g_free);

      g_object_unref (action);
    }
}

GSList *
picman_action_group_add_radio_actions (PicmanActionGroup            *group,
                                     const gchar                *msg_context,
                                     const PicmanRadioActionEntry *entries,
                                     guint                       n_entries,
                                     GSList                     *radio_group,
                                     gint                        value,
                                     GCallback                   callback)
{
  GtkRadioAction *first_action = NULL;
  gint            i;

  g_return_val_if_fail (PICMAN_IS_ACTION_GROUP (group), NULL);

  for (i = 0; i < n_entries; i++)
    {
      GtkRadioAction *action;
      const gchar    *label;
      const gchar    *tooltip = NULL;

      if (! picman_action_group_check_unique_action (group, entries[i].name))
        continue;

      if (msg_context)
        {
          label = g_dpgettext2 (NULL, msg_context, entries[i].label);

          if (entries[i].tooltip)
            tooltip = g_dpgettext2 (NULL, msg_context, entries[i].tooltip);
        }
      else
        {
          label   = gettext (entries[i].label);
          tooltip = gettext (entries[i].tooltip);
        }

      action = picman_radio_action_new (entries[i].name, label, tooltip,
                                      entries[i].stock_id,
                                      entries[i].value);

      if (i == 0)
        first_action = action;

      gtk_radio_action_set_group (action, radio_group);
      radio_group = gtk_radio_action_get_group (action);

      if (value == entries[i].value)
        gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), TRUE);

      gtk_action_group_add_action_with_accel (GTK_ACTION_GROUP (group),
                                              GTK_ACTION (action),
                                              entries[i].accelerator);

      if (entries[i].help_id)
        g_object_set_qdata_full (G_OBJECT (action), PICMAN_HELP_ID,
                                 g_strdup (entries[i].help_id),
                                 (GDestroyNotify) g_free);

      g_object_unref (action);
    }

  if (callback && first_action)
    g_signal_connect (first_action, "changed",
                      callback,
                      group->user_data);

  return radio_group;
}

void
picman_action_group_add_enum_actions (PicmanActionGroup           *group,
                                    const gchar               *msg_context,
                                    const PicmanEnumActionEntry *entries,
                                    guint                      n_entries,
                                    GCallback                  callback)
{
  gint i;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));

  for (i = 0; i < n_entries; i++)
    {
      PicmanEnumAction *action;
      const gchar    *label;
      const gchar    *tooltip = NULL;

      if (! picman_action_group_check_unique_action (group, entries[i].name))
        continue;

      if (msg_context)
        {
          label = g_dpgettext2 (NULL, msg_context, entries[i].label);

          if (entries[i].tooltip)
            tooltip = g_dpgettext2 (NULL, msg_context, entries[i].tooltip);
        }
      else
        {
          label   = gettext (entries[i].label);
          tooltip = gettext (entries[i].tooltip);
        }

      action = picman_enum_action_new (entries[i].name, label, tooltip,
                                     entries[i].stock_id,
                                     entries[i].value,
                                     entries[i].value_variable);

      if (callback)
        g_signal_connect (action, "selected",
                          callback,
                          group->user_data);

      gtk_action_group_add_action_with_accel (GTK_ACTION_GROUP (group),
                                              GTK_ACTION (action),
                                              entries[i].accelerator);

      if (entries[i].help_id)
        g_object_set_qdata_full (G_OBJECT (action), PICMAN_HELP_ID,
                                 g_strdup (entries[i].help_id),
                                 (GDestroyNotify) g_free);

      g_object_unref (action);
    }
}

void
picman_action_group_add_string_actions (PicmanActionGroup             *group,
                                      const gchar                 *msg_context,
                                      const PicmanStringActionEntry *entries,
                                      guint                        n_entries,
                                      GCallback                    callback)
{
  gint i;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));

  for (i = 0; i < n_entries; i++)
    {
      PicmanStringAction *action;
      const gchar      *label;
      const gchar      *tooltip = NULL;

      if (! picman_action_group_check_unique_action (group, entries[i].name))
        continue;

      if (msg_context)
        {
          label = g_dpgettext2 (NULL, msg_context, entries[i].label);

          if (entries[i].tooltip)
            tooltip = g_dpgettext2 (NULL, msg_context, entries[i].tooltip);
        }
      else
        {
          label   = gettext (entries[i].label);
          tooltip = gettext (entries[i].tooltip);
        }

      action = picman_string_action_new (entries[i].name, label, tooltip,
                                       entries[i].stock_id,
                                       entries[i].value);

      if (callback)
        g_signal_connect (action, "selected",
                          callback,
                          group->user_data);

      gtk_action_group_add_action_with_accel (GTK_ACTION_GROUP (group),
                                              GTK_ACTION (action),
                                              entries[i].accelerator);

      if (entries[i].help_id)
        g_object_set_qdata_full (G_OBJECT (action), PICMAN_HELP_ID,
                                 g_strdup (entries[i].help_id),
                                 (GDestroyNotify) g_free);

      g_object_unref (action);
    }
}

void
picman_action_group_add_plug_in_actions (PicmanActionGroup             *group,
                                       const PicmanPlugInActionEntry *entries,
                                       guint                        n_entries,
                                       GCallback                    callback)
{
  gint i;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));

  for (i = 0; i < n_entries; i++)
    {
      PicmanPlugInAction *action;

      if (! picman_action_group_check_unique_action (group, entries[i].name))
        continue;

      action = picman_plug_in_action_new (entries[i].name,
                                        entries[i].label,
                                        entries[i].tooltip,
                                        entries[i].stock_id,
                                        entries[i].procedure);

      if (callback)
        g_signal_connect (action, "selected",
                          callback,
                          group->user_data);

      gtk_action_group_add_action_with_accel (GTK_ACTION_GROUP (group),
                                              GTK_ACTION (action),
                                              entries[i].accelerator);

      if (entries[i].help_id)
        g_object_set_qdata_full (G_OBJECT (action), PICMAN_HELP_ID,
                                 g_strdup (entries[i].help_id),
                                 (GDestroyNotify) g_free);

      g_object_unref (action);
    }
}

void
picman_action_group_activate_action (PicmanActionGroup *group,
                                   const gchar     *action_name)
{
  GtkAction *action;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));
  g_return_if_fail (action_name != NULL);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to activate action which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return;
    }

  gtk_action_activate (action);
}

void
picman_action_group_set_action_visible (PicmanActionGroup *group,
                                      const gchar     *action_name,
                                      gboolean         visible)
{
  GtkAction *action;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));
  g_return_if_fail (action_name != NULL);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to set visibility of action "
                 "which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return;
    }

  gtk_action_set_visible (action, visible);
}

void
picman_action_group_set_action_sensitive (PicmanActionGroup *group,
                                        const gchar     *action_name,
                                        gboolean         sensitive)
{
  GtkAction *action;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));
  g_return_if_fail (action_name != NULL);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to set sensitivity of action "
                 "which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return;
    }

  gtk_action_set_sensitive (action, sensitive);
}

void
picman_action_group_set_action_active (PicmanActionGroup *group,
                                     const gchar     *action_name,
                                     gboolean         active)
{
  GtkAction *action;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));
  g_return_if_fail (action_name != NULL);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to set \"active\" of action "
                 "which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return;
    }

  if (! GTK_IS_TOGGLE_ACTION (action))
    {
      g_warning ("%s: Unable to set \"active\" of action "
                 "which is not a GtkToggleAction: %s",
                 G_STRFUNC, action_name);
      return;
    }

  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
                                active ? TRUE : FALSE);
}

void
picman_action_group_set_action_label (PicmanActionGroup *group,
                                    const gchar     *action_name,
                                    const gchar     *label)
{
  GtkAction *action;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));
  g_return_if_fail (action_name != NULL);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to set label of action "
                 "which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return;
    }

  gtk_action_set_label (action, label);
}

void
picman_action_group_set_action_tooltip (PicmanActionGroup     *group,
                                      const gchar         *action_name,
                                      const gchar         *tooltip)
{
  GtkAction *action;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));
  g_return_if_fail (action_name != NULL);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to set tooltip of action "
                 "which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return;
    }

  gtk_action_set_tooltip (action, tooltip);
}

const gchar *
picman_action_group_get_action_tooltip (PicmanActionGroup     *group,
                                      const gchar         *action_name)
{
  GtkAction *action;

  g_return_val_if_fail (PICMAN_IS_ACTION_GROUP (group), NULL);
  g_return_val_if_fail (action_name != NULL, NULL);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to get tooltip of action "
                 "which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return NULL;
    }

  return gtk_action_get_tooltip (action);
}

void
picman_action_group_set_action_color (PicmanActionGroup *group,
                                    const gchar     *action_name,
                                    const PicmanRGB   *color,
                                    gboolean         set_label)
{
  GtkAction *action;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));
  g_return_if_fail (action_name != NULL);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to set color of action "
                 "which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return;
    }

  if (! PICMAN_IS_ACTION (action))
    {
      g_warning ("%s: Unable to set \"color\" of action "
                 "which is not a PicmanAction: %s",
                 G_STRFUNC, action_name);
      return;
    }

  if (set_label)
    {
      gchar *label;

      if (color)
        label = g_strdup_printf (_("RGBA (%0.3f, %0.3f, %0.3f, %0.3f)"),
                                 color->r, color->g, color->b, color->a);
      else
        label = g_strdup (_("(none)"));

      g_object_set (action,
                    "color", color,
                    "label", label,
                    NULL);
      g_free (label);
    }
  else
    {
      g_object_set (action, "color", color, NULL);
    }
}

void
picman_action_group_set_action_viewable (PicmanActionGroup *group,
                                       const gchar     *action_name,
                                       PicmanViewable    *viewable)
{
  GtkAction *action;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));
  g_return_if_fail (action_name != NULL);
  g_return_if_fail (viewable == NULL || PICMAN_IS_VIEWABLE (viewable));

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to set viewable of action "
                 "which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return;
    }

  if (! PICMAN_IS_ACTION (action))
    {
      g_warning ("%s: Unable to set \"viewable\" of action "
                 "which is not a PicmanAction: %s",
                 G_STRFUNC, action_name);
      return;
    }

  g_object_set (action, "viewable", viewable, NULL);
}

void
picman_action_group_set_action_hide_empty (PicmanActionGroup *group,
                                         const gchar     *action_name,
                                         gboolean         hide_empty)
{
  GtkAction *action;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));
  g_return_if_fail (action_name != NULL);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to set \"hide-if-empty\" of action "
                 "which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return;
    }

  g_object_set (action, "hide-if-empty", hide_empty ? TRUE : FALSE, NULL);
}

void
picman_action_group_set_action_always_show_image (PicmanActionGroup *group,
                                                const gchar     *action_name,
                                                gboolean         always_show_image)
{
  GtkAction *action;

  g_return_if_fail (PICMAN_IS_ACTION_GROUP (group));
  g_return_if_fail (action_name != NULL);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (! action)
    {
      g_warning ("%s: Unable to set \"always-show-image\" of action "
                 "which doesn't exist: %s",
                 G_STRFUNC, action_name);
      return;
    }

  gtk_action_set_always_show_image (action, always_show_image);
}
