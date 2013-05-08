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

#ifndef __APP_PICMAN_UTILS_H__
#define __APP_PICMAN_UTILS_H__


#define PICMAN_TIMER_START() \
  { GTimer *_timer = g_timer_new ();

#define PICMAN_TIMER_END(message) \
  g_printerr ("%s: %s took %0.4f seconds\n", \
              G_STRFUNC, message, g_timer_elapsed (_timer, NULL)); \
  g_timer_destroy (_timer); }


#define MIN4(a,b,c,d) MIN (MIN ((a), (b)), MIN ((c), (d)))
#define MAX4(a,b,c,d) MAX (MAX ((a), (b)), MAX ((c), (d)))


gint64       picman_g_type_instance_get_memsize      (GTypeInstance   *instance);
gint64       picman_g_object_get_memsize             (GObject         *object);
gint64       picman_g_hash_table_get_memsize         (GHashTable      *hash,
                                                    gint64           data_size);
gint64       picman_g_hash_table_get_memsize_foreach (GHashTable      *hash,
                                                    PicmanMemsizeFunc  func,
                                                    gint64          *gui_size);
gint64       picman_g_slist_get_memsize              (GSList          *slist,
                                                    gint64           data_size);
gint64       picman_g_slist_get_memsize_foreach      (GSList          *slist,
                                                    PicmanMemsizeFunc  func,
                                                    gint64          *gui_size);
gint64       picman_g_list_get_memsize               (GList           *list,
                                                    gint64           data_size);
gint64       picman_g_list_get_memsize_foreach       (GList            *slist,
                                                    PicmanMemsizeFunc  func,
                                                    gint64          *gui_size);
gint64       picman_g_value_get_memsize              (GValue          *value);
gint64       picman_g_param_spec_get_memsize         (GParamSpec      *pspec);

gint64       picman_gegl_buffer_get_memsize          (GeglBuffer      *buffer);

gint64       picman_string_get_memsize               (const gchar     *string);
gint64       picman_parasite_get_memsize             (PicmanParasite    *parasite,
                                                    gint64          *gui_size);

gint         picman_get_pid                          (void);
gint         picman_get_number_of_processors         (void);
guint64      picman_get_physical_memory_size         (void);
gchar      * picman_get_backtrace                    (void);
gchar      * picman_get_default_language             (const gchar     *category);
PicmanUnit     picman_get_default_unit                 (void);

GParameter * picman_parameters_append                (GType            object_type,
                                                    GParameter      *params,
                                                    gint            *n_params,
                                                    ...) G_GNUC_NULL_TERMINATED;
GParameter * picman_parameters_append_valist         (GType            object_type,
                                                    GParameter      *params,
                                                    gint            *n_params,
                                                    va_list          args);
void         picman_parameters_free                  (GParameter      *params,
                                                    gint             n_params);

gchar      * picman_markup_extract_text              (const gchar     *markup);

const gchar* picman_enum_get_value_name              (GType            enum_type,
                                                    gint             value);

/* Common values for the n_snap_lines parameter of
 * picman_constrain_line.
 */
#define PICMAN_CONSTRAIN_LINE_90_DEGREES 2
#define PICMAN_CONSTRAIN_LINE_45_DEGREES 4
#define PICMAN_CONSTRAIN_LINE_15_DEGREES 12

void         picman_constrain_line                   (gdouble          start_x,
                                                    gdouble          start_y,
                                                    gdouble         *end_x,
                                                    gdouble         *end_y,
                                                    gint             n_snap_lines);

void         picman_create_image_from_buffer         (Picman            *picman,
                                                    GeglBuffer      *buffer);


#endif /* __APP_PICMAN_UTILS_H__ */
