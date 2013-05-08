/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanparasite.h
 * Copyright (C) 1998 Jay Cox <jaycox@picman.org>
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

#ifndef __PICMAN_PARASITE_H__
#define __PICMAN_PARASITE_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*
 * PICMAN_TYPE_PARASITE
 */

#define PICMAN_TYPE_PARASITE               (picman_parasite_get_type ())
#define PICMAN_VALUE_HOLDS_PARASITE(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_PARASITE))

GType   picman_parasite_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_PARASITE
 */

#define PICMAN_TYPE_PARAM_PARASITE           (picman_param_parasite_get_type ())
#define PICMAN_IS_PARAM_SPEC_PARASITE(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_PARASITE))

GType        picman_param_parasite_get_type  (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_parasite      (const gchar  *name,
                                            const gchar  *nick,
                                            const gchar  *blurb,
                                            GParamFlags   flags);


#define PICMAN_PARASITE_PERSISTENT 1
#define PICMAN_PARASITE_UNDOABLE   2

#define PICMAN_PARASITE_ATTACH_PARENT     (0x80 << 8)
#define PICMAN_PARASITE_PARENT_PERSISTENT (PICMAN_PARASITE_PERSISTENT << 8)
#define PICMAN_PARASITE_PARENT_UNDOABLE   (PICMAN_PARASITE_UNDOABLE << 8)

#define PICMAN_PARASITE_ATTACH_GRANDPARENT     (0x80 << 16)
#define PICMAN_PARASITE_GRANDPARENT_PERSISTENT (PICMAN_PARASITE_PERSISTENT << 16)
#define PICMAN_PARASITE_GRANDPARENT_UNDOABLE   (PICMAN_PARASITE_UNDOABLE << 16)


struct _PicmanParasite
{
  gchar    *name;   /* The name of the parasite. USE A UNIQUE PREFIX! */
  guint32   flags;  /* save Parasite in XCF file, etc.                */
  guint32   size;   /* amount of data                                 */
  gpointer  data;   /* a pointer to the data.  plugin is              *
                     * responsible for tracking byte order            */
};


PicmanParasite * picman_parasite_new           (const gchar        *name,
                                            guint32             flags,
                                            guint32             size,
                                            gconstpointer       data);
void           picman_parasite_free          (PicmanParasite       *parasite);

PicmanParasite * picman_parasite_copy          (const PicmanParasite *parasite);

gboolean       picman_parasite_compare       (const PicmanParasite *a,
                                            const PicmanParasite *b);

gboolean       picman_parasite_is_type       (const PicmanParasite *parasite,
                                            const gchar        *name);
gboolean       picman_parasite_is_persistent (const PicmanParasite *parasite);
gboolean       picman_parasite_is_undoable   (const PicmanParasite *parasite);
gboolean       picman_parasite_has_flag      (const PicmanParasite *parasite,
                                            gulong              flag);
gulong         picman_parasite_flags         (const PicmanParasite *parasite);
const gchar  * picman_parasite_name          (const PicmanParasite *parasite);
gconstpointer  picman_parasite_data          (const PicmanParasite *parasite);
glong          picman_parasite_data_size     (const PicmanParasite *parasite);


G_END_DECLS

#endif /* __PICMAN_PARASITE_H__ */
