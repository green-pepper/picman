/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanmemsizeentry.c
 * Copyright (C) 2000-2003  Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gtk/gtk.h>

#include "picmanwidgetstypes.h"

#include "picmanmemsizeentry.h"
#include "picmanwidgets.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmanmemsizeentry
 * @title: PicmanMemSizeEntry
 * @short_description: A composite widget to enter a memory size.
 *
 * Similar to a #PicmanSizeEntry but instead of lengths, this widget is
 * used to let the user enter memory sizes. A combo box allows one to
 * switch between Kilobytes, Megabytes and Gigabytes. Used in the PICMAN
 * preferences dialog.
 **/


enum
{
  VALUE_CHANGED,
  LAST_SIGNAL
};


static void  picman_memsize_entry_finalize      (GObject          *object);

static void  picman_memsize_entry_adj_callback  (GtkAdjustment    *adj,
                                               PicmanMemsizeEntry *entry);
static void  picman_memsize_entry_unit_callback (GtkWidget        *widget,
                                               PicmanMemsizeEntry *entry);


G_DEFINE_TYPE (PicmanMemsizeEntry, picman_memsize_entry, GTK_TYPE_BOX)

#define parent_class picman_memsize_entry_parent_class

static guint picman_memsize_entry_signals[LAST_SIGNAL] = { 0 };


static void
picman_memsize_entry_class_init (PicmanMemsizeEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = picman_memsize_entry_finalize;

  klass->value_changed   = NULL;

  picman_memsize_entry_signals[VALUE_CHANGED] =
    g_signal_new ("value-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanMemsizeEntryClass, value_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
picman_memsize_entry_init (PicmanMemsizeEntry *entry)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (entry),
                                  GTK_ORIENTATION_HORIZONTAL);

  gtk_box_set_spacing (GTK_BOX (entry), 4);

  entry->value      = 0;
  entry->lower      = 0;
  entry->upper      = 0;
  entry->shift      = 0;
  entry->adjustment = NULL;
  entry->menu       = NULL;
}

static void
picman_memsize_entry_finalize (GObject *object)
{
  PicmanMemsizeEntry *entry = (PicmanMemsizeEntry *) object;

  if (entry->adjustment)
    {
      g_object_unref (entry->adjustment);
      entry->adjustment = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_memsize_entry_adj_callback (GtkAdjustment    *adj,
                                 PicmanMemsizeEntry *entry)
{
  guint64 size = gtk_adjustment_get_value (adj);

  entry->value = size << entry->shift;

  g_signal_emit (entry, picman_memsize_entry_signals[VALUE_CHANGED], 0);
}

static void
picman_memsize_entry_unit_callback (GtkWidget        *widget,
                                  PicmanMemsizeEntry *entry)
{
  guint  shift;

  picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (widget), (gint *) &shift);

#if _MSC_VER < 1300
#  define CAST (gint64)
#else
#  define CAST
#endif

  if (shift != entry->shift)
    {
      entry->shift = shift;

      gtk_adjustment_configure (entry->adjustment,
                                CAST entry->value >> shift,
                                CAST entry->lower >> shift,
                                CAST entry->upper >> shift,
                                gtk_adjustment_get_step_increment (entry->adjustment),
                                gtk_adjustment_get_page_increment (entry->adjustment),
                                gtk_adjustment_get_page_size (entry->adjustment));
    }

#undef CAST
}


/**
 * picman_memsize_entry_new:
 * @value: the initial value (in Bytes)
 * @lower: the lower limit for the value (in Bytes)
 * @upper: the upper limit for the value (in Bytes)
 *
 * Creates a new #PicmanMemsizeEntry which is a #GtkHBox with a #GtkSpinButton
 * and a #GtkOptionMenu all setup to allow the user to enter memory sizes.
 *
 * Returns: Pointer to the new #PicmanMemsizeEntry.
 **/
GtkWidget *
picman_memsize_entry_new (guint64  value,
                        guint64  lower,
                        guint64  upper)
{
  PicmanMemsizeEntry *entry;
  GtkObject        *adj;
  guint             shift;

#if _MSC_VER < 1300
#  define CAST (gint64)
#else
#  define CAST
#endif

  g_return_val_if_fail (value >= lower && value <= upper, NULL);

  entry = g_object_new (PICMAN_TYPE_MEMSIZE_ENTRY, NULL);

  for (shift = 30; shift > 10; shift -= 10)
    {
      if (value > (G_GUINT64_CONSTANT (1) << shift) &&
          value % (G_GUINT64_CONSTANT (1) << shift) == 0)
        break;
    }

  entry->value = value;
  entry->lower = lower;
  entry->upper = upper;
  entry->shift = shift;

  entry->spinbutton = picman_spin_button_new (&adj,
                                            CAST (value >> shift),
                                            CAST (lower >> shift),
                                            CAST (upper >> shift),
                                            1, 8, 0, 1, 0);

#undef CAST

  entry->adjustment = GTK_ADJUSTMENT (adj);
  g_object_ref_sink (entry->adjustment);

  gtk_entry_set_width_chars (GTK_ENTRY (entry->spinbutton), 7);
  gtk_box_pack_start (GTK_BOX (entry), entry->spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (entry->spinbutton);

  g_signal_connect (entry->adjustment, "value-changed",
                    G_CALLBACK (picman_memsize_entry_adj_callback),
                    entry);

  entry->menu = picman_int_combo_box_new (_("Kilobytes"), 10,
                                        _("Megabytes"), 20,
                                        _("Gigabytes"), 30,
                                        NULL);

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (entry->menu), shift);

  g_signal_connect (entry->menu, "changed",
                    G_CALLBACK (picman_memsize_entry_unit_callback),
                    entry);

  gtk_box_pack_start (GTK_BOX (entry), entry->menu, FALSE, FALSE, 0);
  gtk_widget_show (entry->menu);

  return GTK_WIDGET (entry);
}

/**
 * picman_memsize_entry_set_value:
 * @entry: a #PicmanMemsizeEntry
 * @value: the new value (in Bytes)
 *
 * Sets the @entry's value. Please note that the #PicmanMemsizeEntry rounds
 * the value to full Kilobytes.
 **/
void
picman_memsize_entry_set_value (PicmanMemsizeEntry *entry,
                              guint64           value)
{
  guint shift;

  g_return_if_fail (PICMAN_IS_MEMSIZE_ENTRY (entry));
  g_return_if_fail (value >= entry->lower && value <= entry->upper);

  for (shift = 30; shift > 10; shift -= 10)
    {
      if (value > (G_GUINT64_CONSTANT (1) << shift) &&
          value % (G_GUINT64_CONSTANT (1) << shift) == 0)
        break;
    }

  if (shift != entry->shift)
    {
      entry->shift = shift;
      entry->value = value;

      picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (entry->menu), shift);
    }

#if _MSC_VER < 1300
#  define CAST (gint64)
#else
#  define CAST
#endif

  gtk_adjustment_set_value (entry->adjustment, CAST (value >> shift));

#undef CASE
}

/**
 * picman_memsize_entry_get_value:
 * @entry: a #PicmanMemsizeEntry
 *
 * Retrieves the current value from a #PicmanMemsizeEntry.
 *
 * Returns: the current value of @entry (in Bytes).
 **/
guint64
picman_memsize_entry_get_value (PicmanMemsizeEntry *entry)
{
  g_return_val_if_fail (PICMAN_IS_MEMSIZE_ENTRY (entry), 0);

  return entry->value;
}
