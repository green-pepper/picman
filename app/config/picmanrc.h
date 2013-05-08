/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanRc
 * Copyright (C) 2001  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_RC_H__
#define __PICMAN_RC_H__

#include "config/picmanpluginconfig.h"


#define PICMAN_TYPE_RC            (picman_rc_get_type ())
#define PICMAN_RC(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_RC, PicmanRc))
#define PICMAN_RC_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_RC, PicmanRcClass))
#define PICMAN_IS_RC(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_RC))
#define PICMAN_IS_RC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_RC))


typedef struct _PicmanRcClass PicmanRcClass;

struct _PicmanRc
{
  PicmanPluginConfig  parent_instance;

  gchar            *user_picmanrc;
  gchar            *system_picmanrc;
  gboolean          verbose;
  gboolean          autosave;
  guint             save_idle_id;
};

struct _PicmanRcClass
{
  PicmanPluginConfigClass  parent_class;
};


GType     picman_rc_get_type          (void) G_GNUC_CONST;
PicmanRc  * picman_rc_new               (const gchar *system_picmanrc,
                                     const gchar *user_picmanrc,
                                     gboolean     verbose);
void      picman_rc_set_autosave      (PicmanRc      *picmanrc,
                                     gboolean     autosave);
void      picman_rc_save              (PicmanRc      *picmanrc);
gchar   * picman_rc_query             (PicmanRc      *rc,
                                     const gchar *key);
void      picman_rc_set_unknown_token (PicmanRc      *rc,
                                     const gchar *token,
                                     const gchar *value);

void      picman_rc_migrate           (PicmanRc      *rc);


#endif /* PICMAN_RC_H__ */
