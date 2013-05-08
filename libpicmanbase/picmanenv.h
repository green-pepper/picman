/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanenv.h
 * Copyright (C) 1999 Tor Lillqvist <tml@iki.fi>
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

#if !defined (__PICMAN_BASE_H_INSIDE__) && !defined (PICMAN_BASE_COMPILATION)
#error "Only <libpicmanbase/picmanbase.h> can be included directly."
#endif

#ifndef __PICMAN_ENV_H__
#define __PICMAN_ENV_H__


G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#ifdef G_OS_WIN32
#  ifdef __PICMAN_ENV_C__
#    define PICMANVAR __declspec(dllexport)
#  else  /* !__PICMAN_ENV_C__ */
#    define PICMANVAR extern __declspec(dllimport)
#  endif /* !__PICMAN_ENV_C__ */
#else  /* !G_OS_WIN32 */
#  define PICMANVAR extern
#endif

PICMANVAR const guint picman_major_version;
PICMANVAR const guint picman_minor_version;
PICMANVAR const guint picman_micro_version;


const gchar * picman_directory                  (void) G_GNUC_CONST;
const gchar * picman_installation_directory     (void) G_GNUC_CONST;
const gchar * picman_data_directory             (void) G_GNUC_CONST;
const gchar * picman_locale_directory           (void) G_GNUC_CONST;
const gchar * picman_sysconf_directory          (void) G_GNUC_CONST;
const gchar * picman_plug_in_directory          (void) G_GNUC_CONST;

#ifndef PICMAN_DISABLE_DEPRECATED
PICMAN_DEPRECATED_FOR(g_get_user_special_dir)
const gchar * picman_user_directory             (PicmanUserDirectory   type) G_GNUC_CONST;
#endif /* !PICMAN_DISABLE_DEPRECATED */

const gchar * picman_gtkrc                      (void) G_GNUC_CONST;
gchar       * picman_personal_rc_file           (const gchar        *basename) G_GNUC_MALLOC;

GList       * picman_path_parse                 (const gchar        *path,
                                               gint                max_paths,
                                               gboolean            check,
                                               GList             **check_failed);
gchar       * picman_path_to_str                (GList              *path) G_GNUC_MALLOC;
void          picman_path_free                  (GList              *path);

gchar       * picman_path_get_user_writable_dir (GList              *path) G_GNUC_MALLOC;


/*  should be considered private, don't use!  */
void          picman_env_init                   (gboolean            plug_in);


G_END_DECLS

#endif  /*  __PICMAN_ENV_H__  */
