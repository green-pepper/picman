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

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmantemplate.h"

#include "widgets/picmantemplateeditor.h"
#include "widgets/picmanviewabledialog.h"

#include "template-options-dialog.h"

#include "picman-intl.h"


static void  template_options_dialog_free (TemplateOptionsDialog *dialog);


TemplateOptionsDialog *
template_options_dialog_new (PicmanTemplate *template,
                             PicmanContext  *context,
                             GtkWidget    *parent,
                             const gchar  *title,
                             const gchar  *role,
                             const gchar  *stock_id,
                             const gchar  *desc,
                             const gchar  *help_id)
{
  TemplateOptionsDialog *options;
  PicmanViewable          *viewable = NULL;
  GtkWidget             *vbox;

  g_return_val_if_fail (template == NULL || PICMAN_IS_TEMPLATE (template), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (parent), NULL);
  g_return_val_if_fail (title != NULL, NULL);
  g_return_val_if_fail (role != NULL, NULL);
  g_return_val_if_fail (stock_id != NULL, NULL);
  g_return_val_if_fail (desc != NULL, NULL);
  g_return_val_if_fail (help_id != NULL, NULL);

  options = g_slice_new0 (TemplateOptionsDialog);

  options->picman     = context->picman;
  options->template = template;

  if (template)
    {
      viewable = PICMAN_VIEWABLE (template);
      template = picman_config_duplicate (PICMAN_CONFIG (template));
    }
  else
    {
      template =
        picman_config_duplicate (PICMAN_CONFIG (options->picman->config->default_image));
      viewable = PICMAN_VIEWABLE (template);

      picman_object_set_static_name (PICMAN_OBJECT (template), _("Unnamed"));
    }

  options->dialog =
    picman_viewable_dialog_new (viewable, context,
                              title, role, stock_id, desc,
                              parent,
                              picman_standard_help_func, help_id,

                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              GTK_STOCK_OK,     GTK_RESPONSE_OK,

                              NULL);

  gtk_window_set_resizable (GTK_WINDOW (options->dialog), FALSE);
  gtk_dialog_set_alternative_button_order (GTK_DIALOG (options->dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_object_weak_ref (G_OBJECT (options->dialog),
                     (GWeakNotify) template_options_dialog_free, options);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (options->dialog))),
                      vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  options->editor = picman_template_editor_new (template, options->picman, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), options->editor, FALSE, FALSE, 0);
  gtk_widget_show (options->editor);

  g_object_unref (template);

  return options;
}

static void
template_options_dialog_free (TemplateOptionsDialog *dialog)
{
  g_slice_free (TemplateOptionsDialog, dialog);
}
