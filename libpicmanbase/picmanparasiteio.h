/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanparasiteio.h
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

#ifndef __PICMAN_PARASITE_IO_H__
#define __PICMAN_PARASITE_IO_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*  Data structures for various standard parasites used by plug-ins and
 *  the PICMAN core, and functions to build and parse their string
 *  representations.
 */

/*
 *  Pixmap brush pipes.
 */

#define PICMAN_PIXPIPE_MAXDIM 4

typedef struct _PicmanPixPipeParams PicmanPixPipeParams;

struct _PicmanPixPipeParams
{
  gint      step;
  gint      ncells;
  gint      dim;
  gint      cols;
  gint      rows;
  gint      cellwidth;
  gint      cellheight;
  gchar    *placement;
  gboolean  free_placement_string;
  gint      rank[PICMAN_PIXPIPE_MAXDIM];
  gchar    *selection[PICMAN_PIXPIPE_MAXDIM];
  gboolean  free_selection_string;
};

/* Initialize with dummy values */
void    picman_pixpipe_params_init  (PicmanPixPipeParams *params);

/* Parse a string into a PicmanPixPipeParams */
void    picman_pixpipe_params_parse (const gchar       *parameters,
                                   PicmanPixPipeParams *params);

/* Build a string representation of PicmanPixPipeParams */
gchar * picman_pixpipe_params_build (PicmanPixPipeParams *params) G_GNUC_MALLOC;


G_END_DECLS

#endif /* __PICMAN_PARASITE_IO_H__ */
