/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanwidgets.h
 * Copyright (C) 2000 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_WIDGETS_H__
#define __PICMAN_WIDGETS_H__

#define __PICMAN_WIDGETS_H_INSIDE__

#include <libpicmanwidgets/picmanwidgetstypes.h>

#include <libpicmanwidgets/picmanbrowser.h>
#include <libpicmanwidgets/picmanbutton.h>
#include <libpicmanwidgets/picmancairo-utils.h>
#include <libpicmanwidgets/picmancellrenderercolor.h>
#include <libpicmanwidgets/picmancellrenderertoggle.h>
#include <libpicmanwidgets/picmanchainbutton.h>
#include <libpicmanwidgets/picmancolorarea.h>
#include <libpicmanwidgets/picmancolorbutton.h>
#include <libpicmanwidgets/picmancolordisplay.h>
#include <libpicmanwidgets/picmancolordisplaystack.h>
#include <libpicmanwidgets/picmancolorhexentry.h>
#include <libpicmanwidgets/picmancolornotebook.h>
#include <libpicmanwidgets/picmancolorprofilecombobox.h>
#include <libpicmanwidgets/picmancolorprofilestore.h>
#include <libpicmanwidgets/picmancolorscale.h>
#include <libpicmanwidgets/picmancolorscales.h>
#include <libpicmanwidgets/picmancolorselector.h>
#include <libpicmanwidgets/picmancolorselect.h>
#include <libpicmanwidgets/picmancolorselection.h>
#include <libpicmanwidgets/picmandialog.h>
#include <libpicmanwidgets/picmanenumcombobox.h>
#include <libpicmanwidgets/picmanenumlabel.h>
#include <libpicmanwidgets/picmanenumstore.h>
#include <libpicmanwidgets/picmanenumwidgets.h>
#include <libpicmanwidgets/picmanfileentry.h>
#include <libpicmanwidgets/picmanframe.h>
#include <libpicmanwidgets/picmanhelpui.h>
#include <libpicmanwidgets/picmanhintbox.h>
#include <libpicmanwidgets/picmanintcombobox.h>
#include <libpicmanwidgets/picmanintstore.h>
#include <libpicmanwidgets/picmanmemsizeentry.h>
#include <libpicmanwidgets/picmannumberpairentry.h>
#include <libpicmanwidgets/picmanoffsetarea.h>
#include <libpicmanwidgets/picmanpageselector.h>
#include <libpicmanwidgets/picmanpatheditor.h>
#include <libpicmanwidgets/picmanpickbutton.h>
#include <libpicmanwidgets/picmanpixmap.h>
#include <libpicmanwidgets/picmanpreview.h>
#include <libpicmanwidgets/picmanpreviewarea.h>
#include <libpicmanwidgets/picmanpropwidgets.h>
#include <libpicmanwidgets/picmanquerybox.h>
#include <libpicmanwidgets/picmanruler.h>
#include <libpicmanwidgets/picmanscaleentry.h>
#include <libpicmanwidgets/picmanscrolledpreview.h>
#include <libpicmanwidgets/picmansizeentry.h>
#include <libpicmanwidgets/picmanstock.h>
#include <libpicmanwidgets/picmanstringcombobox.h>
#include <libpicmanwidgets/picmanunitcombobox.h>
#include <libpicmanwidgets/picmanunitmenu.h>
#include <libpicmanwidgets/picmanunitstore.h>
#include <libpicmanwidgets/picmanwidgets-error.h>
#include <libpicmanwidgets/picmanzoommodel.h>

#include <libpicmanwidgets/picman3migration.h>
#include <libpicmanwidgets/picmanoldwidgets.h>

#undef __PICMAN_WIDGETS_H_INSIDE__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*
 *  Widget Constructors
 */

GtkWidget * picman_int_radio_group_new (gboolean          in_frame,
                                      const gchar      *frame_title,
                                      GCallback         radio_button_callback,
                                      gpointer          radio_button_callback_data,
                                      gint              initial, /* item_data */

                                      /* specify radio buttons as va_list:
                                       *  const gchar  *label,
                                       *  gint          item_data,
                                       *  GtkWidget   **widget_ptr,
                                       */

                                      ...) G_GNUC_NULL_TERMINATED;

void        picman_int_radio_group_set_active (GtkRadioButton *radio_button,
                                             gint            item_data);


GtkWidget * picman_radio_group_new   (gboolean            in_frame,
                                    const gchar        *frame_title,

                                    /* specify radio buttons as va_list:
                                     *  const gchar    *label,
                                     *  GCallback       callback,
                                     *  gpointer        callback_data,
                                     *  gpointer        item_data,
                                     *  GtkWidget     **widget_ptr,
                                     *  gboolean        active,
                                     */

                                    ...) G_GNUC_NULL_TERMINATED;
GtkWidget * picman_radio_group_new2  (gboolean            in_frame,
                                    const gchar        *frame_title,
                                    GCallback           radio_button_callback,
                                    gpointer            radio_button_callback_data,
                                    gpointer            initial, /* item_data */

                                    /* specify radio buttons as va_list:
                                     *  const gchar    *label,
                                     *  gpointer        item_data,
                                     *  GtkWidget     **widget_ptr,
                                     */

                                    ...) G_GNUC_NULL_TERMINATED;

void   picman_radio_group_set_active (GtkRadioButton     *radio_button,
                                    gpointer            item_data);


GtkWidget * picman_spin_button_new   (/* return value: */
                                    GtkObject         **adjustment,

                                    gdouble             value,
                                    gdouble             lower,
                                    gdouble             upper,
                                    gdouble             step_increment,
                                    gdouble             page_increment,
                                    gdouble             page_size,
                                    gdouble             climb_rate,
                                    guint               digits);

/**
 * PICMAN_RANDOM_SEED_SPINBUTTON:
 * @hbox: The #GtkHBox returned by picman_random_seed_new().
 *
 * Returns: the random_seed's #GtkSpinButton.
 **/
#define PICMAN_RANDOM_SEED_SPINBUTTON(hbox) \
        (g_object_get_data (G_OBJECT (hbox), "spinbutton"))

/**
 * PICMAN_RANDOM_SEED_SPINBUTTON_ADJ:
 * @hbox: The #GtkHBox returned by picman_random_seed_new().
 *
 * Returns: the #GtkAdjustment of the random_seed's #GtkSpinButton.
 **/
#define PICMAN_RANDOM_SEED_SPINBUTTON_ADJ(hbox)       \
        gtk_spin_button_get_adjustment \
        (GTK_SPIN_BUTTON (g_object_get_data (G_OBJECT (hbox), "spinbutton")))

/**
 * PICMAN_RANDOM_SEED_TOGGLE:
 * @hbox: The #GtkHBox returned by picman_random_seed_new().
 *
 * Returns: the random_seed's #GtkToggleButton.
 **/
#define PICMAN_RANDOM_SEED_TOGGLE(hbox) \
        (g_object_get_data (G_OBJECT(hbox), "toggle"))

GtkWidget * picman_random_seed_new   (guint32            *seed,
                                    gboolean           *random_seed);

/**
 * PICMAN_COORDINATES_CHAINBUTTON:
 * @sizeentry: The #PicmanSizeEntry returned by picman_coordinates_new().
 *
 * Returns: the #PicmanChainButton which is attached to the
 *          #PicmanSizeEntry.
 **/
#define PICMAN_COORDINATES_CHAINBUTTON(sizeentry) \
        (g_object_get_data (G_OBJECT (sizeentry), "chainbutton"))

GtkWidget * picman_coordinates_new   (PicmanUnit            unit,
                                    const gchar        *unit_format,
                                    gboolean            menu_show_pixels,
                                    gboolean            menu_show_percent,
                                    gint                spinbutton_width,
                                    PicmanSizeEntryUpdatePolicy  update_policy,

                                    gboolean            chainbutton_active,
                                    gboolean            chain_constrains_ratio,

                                    const gchar        *xlabel,
                                    gdouble             x,
                                    gdouble             xres,
                                    gdouble             lower_boundary_x,
                                    gdouble             upper_boundary_x,
                                    gdouble             xsize_0,   /* % */
                                    gdouble             xsize_100, /* % */

                                    const gchar        *ylabel,
                                    gdouble             y,
                                    gdouble             yres,
                                    gdouble             lower_boundary_y,
                                    gdouble             upper_boundary_y,
                                    gdouble             ysize_0,   /* % */
                                    gdouble             ysize_100  /* % */);


/*
 *  Standard Callbacks
 */

void picman_toggle_button_update           (GtkWidget       *widget,
                                          gpointer         data);

void picman_radio_button_update            (GtkWidget       *widget,
                                          gpointer         data);

void picman_int_adjustment_update          (GtkAdjustment   *adjustment,
                                          gpointer         data);

void picman_uint_adjustment_update         (GtkAdjustment   *adjustment,
                                          gpointer         data);

void picman_float_adjustment_update        (GtkAdjustment   *adjustment,
                                          gpointer         data);

void picman_double_adjustment_update       (GtkAdjustment   *adjustment,
                                          gpointer         data);


/*
 *  Helper Functions
 */

GtkWidget * picman_table_attach_aligned    (GtkTable        *table,
                                          gint             column,
                                          gint             row,
                                          const gchar     *label_text,
                                          gfloat           xalign,
                                          gfloat           yalign,
                                          GtkWidget       *widget,
                                          gint             colspan,
                                          gboolean         left_align);


void        picman_label_set_attributes    (GtkLabel        *label,
                                          ...);


G_END_DECLS

#endif /* __PICMAN_WIDGETS_H__ */
