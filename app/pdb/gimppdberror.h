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

#ifndef __PICMAN_PDB_ERROR_H__
#define __PICMAN_PDB_ERROR_H__


typedef enum
{
  PICMAN_PDB_ERROR_FAILED,  /* generic error condition */
  PICMAN_PDB_ERROR_CANCELLED,
  PICMAN_PDB_ERROR_PROCEDURE_NOT_FOUND,
  PICMAN_PDB_ERROR_INVALID_ARGUMENT,
  PICMAN_PDB_ERROR_INVALID_RETURN_VALUE,
  PICMAN_PDB_ERROR_INTERNAL_ERROR
} PicmanPdbErrorCode;


#define PICMAN_PDB_ERROR (picman_pdb_error_quark ())

GQuark  picman_pdb_error_quark (void) G_GNUC_CONST;


#endif /* __PICMAN_PDB_ERROR_H__ */
