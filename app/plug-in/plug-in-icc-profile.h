/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * plug-in-icc-profile.h
 * Copyright (C) 2006  Sven Neumann <sven@picman.org>
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

#ifndef __PLUG_IN_ICC_PROFILE_H__
#define __PLUG_IN_ICC_PROFILE_H__


gboolean  plug_in_icc_profile_apply_rgb (PicmanImage     *image,
                                         PicmanContext   *context,
                                         PicmanProgress  *progress,
                                         PicmanRunMode    run_mode,
                                         GError       **error);

gboolean  plug_in_icc_profile_info      (PicmanImage     *image,
                                         PicmanContext   *context,
                                         PicmanProgress  *progress,
                                         gchar        **name,
                                         gchar        **desc,
                                         gchar        **info,
                                         GError       **error);

gboolean  plug_in_icc_profile_file_info (Picman          *picman,
                                         PicmanContext   *context,
                                         PicmanProgress  *progress,
                                         const gchar   *filename,
                                         gchar        **name,
                                         gchar        **desc,
                                         gchar        **info,
                                         GError       **error);


#endif /* __PLUG_IN_ICC_PROFILE_H__ */
