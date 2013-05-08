/* picmanxmpmodelentry.c - custom entry widget linked to the xmp model
 *
 * Copyright (C) 2009, Róman Joost <romanofski@picman.org>
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

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "xmp-schemas.h"
#include "xmp-model.h"

#include "picmanxmpmodelwidget.h"
#include "picmanxmpmodelentry.h"


static void   picman_xmp_model_entry_iface_init  (PicmanXmpModelWidgetInterface *iface);

static void   picman_xmp_model_entry_constructed (GObject            *object);

static void   picman_xmp_model_entry_set_text    (PicmanXmpModelWidget *widget,
                                                const gchar        *tree_value);

static void   picman_xmp_model_entry_changed     (PicmanXmpModelEntry  *entry);


G_DEFINE_TYPE_WITH_CODE (PicmanXmpModelEntry, picman_xmp_model_entry,
                         GTK_TYPE_ENTRY,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_XMP_MODEL_WIDGET,
                                                picman_xmp_model_entry_iface_init))


#define parent_class picman_xmp_model_entry_parent_class


static void
picman_xmp_model_entry_class_init (PicmanXmpModelEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_xmp_model_entry_constructed;
  object_class->set_property = picman_xmp_model_widget_set_property;
  object_class->get_property = picman_xmp_model_widget_get_property;

  picman_xmp_model_widget_install_properties (object_class);
}

static void
picman_xmp_model_entry_iface_init (PicmanXmpModelWidgetInterface *iface)
{
  iface->widget_set_text = picman_xmp_model_entry_set_text;
}

static void
picman_xmp_model_entry_init (PicmanXmpModelEntry *entry)
{
  g_signal_connect (entry, "changed",
                    G_CALLBACK (picman_xmp_model_entry_changed),
                    NULL);
}

static void
picman_xmp_model_entry_constructed (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_xmp_model_widget_constructor (object);
}

static void
picman_xmp_model_entry_set_text (PicmanXmpModelWidget *widget,
                               const gchar        *tree_value)
{
  gtk_entry_set_text (GTK_ENTRY (widget), tree_value);
}

static void
picman_xmp_model_entry_changed (PicmanXmpModelEntry *entry)
{
  const gchar *value = gtk_entry_get_text (GTK_ENTRY (entry));

  picman_xmp_model_widget_changed (PICMAN_XMP_MODEL_WIDGET (entry), value);
}
