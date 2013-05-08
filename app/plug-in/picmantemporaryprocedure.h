/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantemporaryprocedure.h
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

#ifndef __PICMAN_TEMPORARY_PROCEDURE_H__
#define __PICMAN_TEMPORARY_PROCEDURE_H__


#include "picmanpluginprocedure.h"


#define PICMAN_TYPE_TEMPORARY_PROCEDURE            (picman_temporary_procedure_get_type ())
#define PICMAN_TEMPORARY_PROCEDURE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEMPORARY_PROCEDURE, PicmanTemporaryProcedure))
#define PICMAN_TEMPORARY_PROCEDURE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEMPORARY_PROCEDURE, PicmanTemporaryProcedureClass))
#define PICMAN_IS_TEMPORARY_PROCEDURE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEMPORARY_PROCEDURE))
#define PICMAN_IS_TEMPORARY_PROCEDURE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEMPORARY_PROCEDURE))
#define PICMAN_TEMPORARY_PROCEDURE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TEMPORARY_PROCEDURE, PicmanTemporaryProcedureClass))


typedef struct _PicmanTemporaryProcedureClass PicmanTemporaryProcedureClass;

struct _PicmanTemporaryProcedure
{
  PicmanPlugInProcedure  parent_instance;

  PicmanPlugIn          *plug_in;
};

struct _PicmanTemporaryProcedureClass
{
  PicmanPlugInProcedureClass parent_class;
};


GType           picman_temporary_procedure_get_type (void) G_GNUC_CONST;

PicmanProcedure * picman_temporary_procedure_new      (PicmanPlugIn *plug_in);


#endif /* __PICMAN_TEMPORARY_PROCEDURE_H__ */
