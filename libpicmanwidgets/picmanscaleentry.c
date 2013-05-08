/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanscaleentry.c
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
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanbase/picmanbase.h"

#include "picmanwidgets.h"


static void picman_scale_entry_unconstrained_adjustment_callback (GtkAdjustment *adjustment,
                                                                GtkAdjustment *other_adj);
static void picman_scale_entry_exp_adjustment_callback           (GtkAdjustment *adjustment,
                                                                GtkAdjustment *other_adj);
static void picman_scale_entry_log_adjustment_callback           (GtkAdjustment *adjustment,
                                                                GtkAdjustment *other_adj);

static GtkObject * picman_scale_entry_new_internal               (gboolean       color_scale,
                                                                GtkTable      *table,
                                                                gint           column,
                                                                gint           row,
                                                                const gchar   *text,
                                                                gint           scale_width,
                                                                gint           spinbutton_width,
                                                                gdouble        value,
                                                                gdouble        lower,
                                                                gdouble        upper,
                                                                gdouble        step_increment,
                                                                gdouble        page_increment,
                                                                guint          digits,
                                                                gboolean       constrain,
                                                                gdouble        unconstrained_lower,
                                                                gdouble        unconstrained_upper,
                                                                const gchar   *tooltip,
                                                                const gchar   *help_id);


static void
picman_scale_entry_unconstrained_adjustment_callback (GtkAdjustment *adjustment,
                                                    GtkAdjustment *other_adj)
{
  g_signal_handlers_block_by_func (other_adj,
                                   picman_scale_entry_unconstrained_adjustment_callback,
                                   adjustment);

  gtk_adjustment_set_value (other_adj, gtk_adjustment_get_value (adjustment));

  g_signal_handlers_unblock_by_func (other_adj,
                                     picman_scale_entry_unconstrained_adjustment_callback,
                                     adjustment);
}

static void
picman_scale_entry_log_adjustment_callback (GtkAdjustment *adjustment,
                                          GtkAdjustment *other_adj)
{
  gdouble value;

  g_signal_handlers_block_by_func (other_adj,
                                   picman_scale_entry_exp_adjustment_callback,
                                   adjustment);

  if (gtk_adjustment_get_lower (adjustment) <= 0.0)
    value = log (gtk_adjustment_get_value (adjustment) -
                 gtk_adjustment_get_lower (adjustment) + 0.1);
  else
    value = log (gtk_adjustment_get_value (adjustment));

  gtk_adjustment_set_value (other_adj, value);

  g_signal_handlers_unblock_by_func (other_adj,
                                     picman_scale_entry_exp_adjustment_callback,
                                     adjustment);
}

static void
picman_scale_entry_exp_adjustment_callback (GtkAdjustment *adjustment,
                                          GtkAdjustment *other_adj)
{
  gdouble value;

  g_signal_handlers_block_by_func (other_adj,
                                   picman_scale_entry_log_adjustment_callback,
                                   adjustment);

  value = exp (gtk_adjustment_get_value (adjustment));
  if (gtk_adjustment_get_lower (other_adj) <= 0.0)
    value += gtk_adjustment_get_lower (other_adj) - 0.1;

  gtk_adjustment_set_value (other_adj, value);

  g_signal_handlers_unblock_by_func (other_adj,
                                     picman_scale_entry_log_adjustment_callback,
                                     adjustment);
}

static GtkObject *
picman_scale_entry_new_internal (gboolean     color_scale,
                               GtkTable    *table,
                               gint         column,
                               gint         row,
                               const gchar *text,
                               gint         scale_width,
                               gint         spinbutton_width,
                               gdouble      value,
                               gdouble      lower,
                               gdouble      upper,
                               gdouble      step_increment,
                               gdouble      page_increment,
                               guint        digits,
                               gboolean     constrain,
                               gdouble      unconstrained_lower,
                               gdouble      unconstrained_upper,
                               const gchar *tooltip,
                               const gchar *help_id)
{
  GtkWidget *label;
  GtkWidget *scale;
  GtkWidget *spinbutton;
  GtkObject *adjustment;
  GtkObject *return_adj;

  label = gtk_label_new_with_mnemonic (text);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label,
                    column, column + 1, row, row + 1,
                    GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  if (! constrain &&
           unconstrained_lower <= lower &&
           unconstrained_upper >= upper)
    {
      GtkObject *constrained_adj;

      constrained_adj = gtk_adjustment_new (value, lower, upper,
                                            step_increment, page_increment,
                                            0.0);

      spinbutton = picman_spin_button_new (&adjustment, value,
                                         unconstrained_lower,
                                         unconstrained_upper,
                                         step_increment, page_increment, 0.0,
                                         1.0, digits);

      g_signal_connect
        (G_OBJECT (constrained_adj), "value-changed",
         G_CALLBACK (picman_scale_entry_unconstrained_adjustment_callback),
         adjustment);

      g_signal_connect
        (G_OBJECT (adjustment), "value-changed",
         G_CALLBACK (picman_scale_entry_unconstrained_adjustment_callback),
         constrained_adj);

      return_adj = adjustment;

      adjustment = constrained_adj;
    }
  else
    {
      spinbutton = picman_spin_button_new (&adjustment, value, lower, upper,
                                         step_increment, page_increment, 0.0,
                                         1.0, digits);

      return_adj = adjustment;
    }

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), spinbutton);

  if (spinbutton_width > 0)
    {
      if (spinbutton_width < 17)
        gtk_entry_set_width_chars (GTK_ENTRY (spinbutton), spinbutton_width);
      else
        gtk_widget_set_size_request (spinbutton, spinbutton_width, -1);
    }

  if (color_scale)
    {
      scale = picman_color_scale_new (GTK_ORIENTATION_HORIZONTAL,
                                    PICMAN_COLOR_SELECTOR_VALUE);

      gtk_range_set_adjustment (GTK_RANGE (scale),
                                GTK_ADJUSTMENT (adjustment));
    }
  else
    {
      scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                             GTK_ADJUSTMENT (adjustment));
    }

  if (scale_width > 0)
    gtk_widget_set_size_request (scale, scale_width, -1);
  gtk_scale_set_digits (GTK_SCALE (scale), digits);
  gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);
  gtk_table_attach (GTK_TABLE (table), scale,
                    column + 1, column + 2, row, row + 1,
                    GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  gtk_widget_show (scale);

  gtk_table_attach (GTK_TABLE (table), spinbutton,
                    column + 2, column + 3, row, row + 1,
                    GTK_FILL | GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_widget_show (spinbutton);

  if (tooltip || help_id)
    {
      picman_help_set_help_data (label, tooltip, help_id);
      picman_help_set_help_data (scale, tooltip, help_id);
      picman_help_set_help_data (spinbutton, tooltip, help_id);
    }

  g_object_set_data (G_OBJECT (return_adj), "label",      label);
  g_object_set_data (G_OBJECT (return_adj), "scale",      scale);
  g_object_set_data (G_OBJECT (return_adj), "spinbutton", spinbutton);



  return return_adj;
}

/**
 * picman_scale_entry_new:
 * @table:               The #GtkTable the widgets will be attached to.
 * @column:              The column to start with.
 * @row:                 The row to attach the widgets.
 * @text:                The text for the #GtkLabel which will appear
 *                       left of the #GtkHScale.
 * @scale_width:         The minimum horizontal size of the #GtkHScale.
 * @spinbutton_width:    The minimum horizontal size of the #GtkSpinButton.
 * @value:               The initial value.
 * @lower:               The lower boundary.
 * @upper:               The upper boundary.
 * @step_increment:      The step increment.
 * @page_increment:      The page increment.
 * @digits:              The number of decimal digits.
 * @constrain:           %TRUE if the range of possible values of the
 *                       #GtkSpinButton should be the same as of the #GtkHScale.
 * @unconstrained_lower: The spinbutton's lower boundary
 *                       if @constrain == %FALSE.
 * @unconstrained_upper: The spinbutton's upper boundary
 *                       if @constrain == %FALSE.
 * @tooltip:             A tooltip message for the scale and the spinbutton.
 * @help_id:             The widgets' help_id (see picman_help_set_help_data()).
 *
 * This function creates a #GtkLabel, a #GtkHScale and a #GtkSpinButton and
 * attaches them to a 3-column #GtkTable.
 *
 * Returns: The #GtkSpinButton's #GtkAdjustment.
 **/
GtkObject *
picman_scale_entry_new (GtkTable    *table,
                      gint         column,
                      gint         row,
                      const gchar *text,
                      gint         scale_width,
                      gint         spinbutton_width,
                      gdouble      value,
                      gdouble      lower,
                      gdouble      upper,
                      gdouble      step_increment,
                      gdouble      page_increment,
                      guint        digits,
                      gboolean     constrain,
                      gdouble      unconstrained_lower,
                      gdouble      unconstrained_upper,
                      const gchar *tooltip,
                      const gchar *help_id)
{
  return picman_scale_entry_new_internal (FALSE,
                                        table, column, row,
                                        text, scale_width, spinbutton_width,
                                        value, lower, upper,
                                        step_increment, page_increment,
                                        digits,
                                        constrain,
                                        unconstrained_lower,
                                        unconstrained_upper,
                                        tooltip, help_id);
}

/**
 * picman_color_scale_entry_new:
 * @table:               The #GtkTable the widgets will be attached to.
 * @column:              The column to start with.
 * @row:                 The row to attach the widgets.
 * @text:                The text for the #GtkLabel which will appear
 *                       left of the #GtkHScale.
 * @scale_width:         The minimum horizontal size of the #GtkHScale.
 * @spinbutton_width:    The minimum horizontal size of the #GtkSpinButton.
 * @value:               The initial value.
 * @lower:               The lower boundary.
 * @upper:               The upper boundary.
 * @step_increment:      The step increment.
 * @page_increment:      The page increment.
 * @digits:              The number of decimal digits.
 * @tooltip:             A tooltip message for the scale and the spinbutton.
 * @help_id:             The widgets' help_id (see picman_help_set_help_data()).
 *
 * This function creates a #GtkLabel, a #PicmanColorScale and a
 * #GtkSpinButton and attaches them to a 3-column #GtkTable.
 *
 * Returns: The #GtkSpinButton's #GtkAdjustment.
 **/
GtkObject *
picman_color_scale_entry_new (GtkTable    *table,
                            gint         column,
                            gint         row,
                            const gchar *text,
                            gint         scale_width,
                            gint         spinbutton_width,
                            gdouble      value,
                            gdouble      lower,
                            gdouble      upper,
                            gdouble      step_increment,
                            gdouble      page_increment,
                            guint        digits,
                            const gchar *tooltip,
                            const gchar *help_id)
{
  return picman_scale_entry_new_internal (TRUE,
                                        table, column, row,
                                        text, scale_width, spinbutton_width,
                                        value, lower, upper,
                                        step_increment, page_increment,
                                        digits,
                                        TRUE, 0.0, 0.0,
                                        tooltip, help_id);
}

/**
 * picman_scale_entry_set_logarithmic:
 * @adjustment:  a  #GtkAdjustment as returned by picman_scale_entry_new()
 * @logarithmic: a boolean value to set or reset logarithmic behaviour
 *               of the scale widget
 *
 * Sets whether the scale_entry's scale widget will behave in a linear
 * or logharithmic fashion. Useful when an entry has to attend large
 * ranges, but smaller selections on that range require a finer
 * adjustment.
 *
 * Since: PICMAN 2.2
 **/
void
picman_scale_entry_set_logarithmic (GtkObject *adjustment,
                                  gboolean   logarithmic)
{
  GtkAdjustment *adj;
  GtkAdjustment *scale_adj;

  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  adj       = GTK_ADJUSTMENT (adjustment);
  scale_adj = PICMAN_SCALE_ENTRY_SCALE_ADJ (adjustment);

  if (logarithmic == picman_scale_entry_get_logarithmic (adjustment))
    return;

  if (logarithmic)
    {
      gdouble correction;
      gdouble log_value, log_lower, log_upper;
      gdouble log_step_increment, log_page_increment;

      correction = (gtk_adjustment_get_lower (scale_adj) > 0 ?
                    0 : 0.1 + - gtk_adjustment_get_lower (scale_adj));

      log_value = log (gtk_adjustment_get_value (scale_adj) + correction);
      log_lower = log (gtk_adjustment_get_lower (scale_adj) + correction);
      log_upper = log (gtk_adjustment_get_upper (scale_adj) + correction);
      log_step_increment =
        (log_upper - log_lower) / ((gtk_adjustment_get_upper (scale_adj) -
                                    gtk_adjustment_get_lower (scale_adj)) /
                                   gtk_adjustment_get_step_increment (scale_adj));
      log_page_increment =
        (log_upper - log_lower) / ((gtk_adjustment_get_upper (scale_adj) -
                                    gtk_adjustment_get_lower (scale_adj)) /
                                   gtk_adjustment_get_page_increment (scale_adj));

      if (scale_adj == adj)
        {
          GtkObject *new_adj;

          new_adj = gtk_adjustment_new (gtk_adjustment_get_value (scale_adj),
                                        gtk_adjustment_get_lower (scale_adj),
                                        gtk_adjustment_get_upper (scale_adj),
                                        gtk_adjustment_get_step_increment (scale_adj),
                                        gtk_adjustment_get_page_increment (scale_adj),
                                        0.0);
          gtk_range_set_adjustment (GTK_RANGE (PICMAN_SCALE_ENTRY_SCALE (adj)),
                                    GTK_ADJUSTMENT (new_adj));

          scale_adj = (GtkAdjustment *) new_adj;
        }
      else
        {
          g_signal_handlers_disconnect_by_func (adj,
                                                picman_scale_entry_unconstrained_adjustment_callback,
                                                scale_adj);

          g_signal_handlers_disconnect_by_func (scale_adj,
                                                picman_scale_entry_unconstrained_adjustment_callback,
                                                adj);
        }

      gtk_adjustment_configure (scale_adj,
                                log_value, log_lower, log_upper,
                                log_step_increment, log_page_increment, 0.0);

      g_signal_connect (scale_adj, "value-changed",
                        G_CALLBACK (picman_scale_entry_exp_adjustment_callback),
                        adj);

      g_signal_connect (adj, "value-changed",
                        G_CALLBACK (picman_scale_entry_log_adjustment_callback),
                        scale_adj);

      g_object_set_data (G_OBJECT (adjustment),
                         "logarithmic", GINT_TO_POINTER (TRUE));
    }
  else
    {
      gdouble lower, upper;

      g_signal_handlers_disconnect_by_func (adj,
                                            picman_scale_entry_log_adjustment_callback,
                                            scale_adj);

      g_signal_handlers_disconnect_by_func (scale_adj,
                                            picman_scale_entry_exp_adjustment_callback,
                                            adj);

      lower = exp (gtk_adjustment_get_lower (scale_adj));
      upper = exp (gtk_adjustment_get_upper (scale_adj));

      if (gtk_adjustment_get_lower (adj) <= 0.0)
        {
          lower += - 0.1 + gtk_adjustment_get_lower (adj);
          upper += - 0.1 + gtk_adjustment_get_lower (adj);
        }

      gtk_adjustment_configure (scale_adj,
                                gtk_adjustment_get_value (adj),
                                lower, upper,
                                gtk_adjustment_get_step_increment (adj),
                                gtk_adjustment_get_page_increment (adj),
                                0.0);

      g_signal_connect (scale_adj, "value-changed",
                        G_CALLBACK (picman_scale_entry_unconstrained_adjustment_callback),
                        adj);

      g_signal_connect (adj, "value-changed",
                        G_CALLBACK (picman_scale_entry_unconstrained_adjustment_callback),
                        scale_adj);

      g_object_set_data (G_OBJECT (adjustment),
                         "logarithmic", GINT_TO_POINTER (FALSE));
    }
}

/**
 * picman_scale_entry_get_logarithmic:
 * @adjustment: a  #GtkAdjustment as returned by picman_scale_entry_new()
 *
 * Return value: %TRUE if the the entry's scale widget will behave in
 *               logharithmic fashion, %FALSE for linear behaviour.
 *
 * Since: PICMAN 2.2
 **/
gboolean
picman_scale_entry_get_logarithmic (GtkObject *adjustment)
{
  return GPOINTER_TO_INT (g_object_get_data (G_OBJECT (adjustment),
                                             "logarithmic"));
}

/**
 * picman_scale_entry_set_sensitive:
 * @adjustment: a #GtkAdjustment returned by picman_scale_entry_new()
 * @sensitive:  a boolean value with the same semantics as the @sensitive
 *              parameter of gtk_widget_set_sensitive()
 *
 * Sets the sensitivity of the scale_entry's #GtkLabel, #GtkHScale and
 * #GtkSpinButton.
 **/
void
picman_scale_entry_set_sensitive (GtkObject *adjustment,
                                gboolean   sensitive)
{
  GtkWidget *widget;

  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  widget = PICMAN_SCALE_ENTRY_LABEL (adjustment);
  if (widget)
    gtk_widget_set_sensitive (widget, sensitive);

  widget = PICMAN_SCALE_ENTRY_SCALE (adjustment);
  if (widget)
    gtk_widget_set_sensitive (widget, sensitive);

  widget = PICMAN_SCALE_ENTRY_SPINBUTTON (adjustment);
  if (widget)
    gtk_widget_set_sensitive (widget, sensitive);
}
