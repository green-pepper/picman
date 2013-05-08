/* picmanparasite.h: Copyright 1998 Jay Cox <jaycox@picman.org>
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

#ifndef __PICMAN_PARASITES_H__
#define __PICMAN_PARASITES_H__


/* some wrappers to access picman->parasites, mainly for the PDB */

void                  picman_parasite_attach       (Picman               *picman,
                                                  const PicmanParasite *parasite);
void                  picman_parasite_detach       (Picman               *picman,
                                                  const gchar        *name);
const PicmanParasite  * picman_parasite_find         (Picman               *picman,
                                                  const gchar        *name);
gchar              ** picman_parasite_list         (Picman               *picman,
                                                  gint               *count);

void                  picman_parasite_shift_parent (PicmanParasite       *parasite);

void                  picman_parasiterc_load       (Picman               *picman);
void                  picman_parasiterc_save       (Picman               *picman);


#endif  /*  __PICMAN_PARASITES_H__  */
