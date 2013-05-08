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

#ifndef __PICMAN_PROCEDURE_H__
#define __PICMAN_PROCEDURE_H__


#include "core/picmanobject.h"


typedef PicmanValueArray * (* PicmanMarshalFunc) (PicmanProcedure         *procedure,
                                              Picman                  *picman,
                                              PicmanContext           *context,
                                              PicmanProgress          *progress,
                                              const PicmanValueArray  *args,
                                              GError               **error);


#define PICMAN_TYPE_PROCEDURE            (picman_procedure_get_type ())
#define PICMAN_PROCEDURE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PROCEDURE, PicmanProcedure))
#define PICMAN_PROCEDURE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PROCEDURE, PicmanProcedureClass))
#define PICMAN_IS_PROCEDURE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PROCEDURE))
#define PICMAN_IS_PROCEDURE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PROCEDURE))
#define PICMAN_PROCEDURE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PROCEDURE, PicmanProcedureClass))


typedef struct _PicmanProcedureClass PicmanProcedureClass;

struct _PicmanProcedure
{
  PicmanObject        parent_instance;

  PicmanPDBProcType   proc_type;      /* Type of procedure              */

  gboolean          static_strings; /* Are the strings allocated?     */

  gchar            *original_name;  /* Uncanonicalized procedure name */
  gchar            *blurb;          /* Short procedure description    */
  gchar            *help;           /* Detailed help instructions     */
  gchar            *author;         /* Author field                   */
  gchar            *copyright;      /* Copyright field                */
  gchar            *date;           /* Date field                     */
  gchar            *deprecated;     /* Replacement if deprecated      */

  gint32            num_args;       /* Number of procedure arguments  */
  GParamSpec      **args;           /* Array of procedure arguments   */

  gint32            num_values;     /* Number of return values        */
  GParamSpec      **values;         /* Array of return values         */

  PicmanMarshalFunc   marshal_func;   /* Marshaller for internal procs  */
};

struct _PicmanProcedureClass
{
  PicmanObjectClass parent_class;

  PicmanValueArray * (* execute)       (PicmanProcedure   *procedure,
                                      Picman            *picman,
                                      PicmanContext     *context,
                                      PicmanProgress    *progress,
                                      PicmanValueArray  *args,
                                      GError         **error);
  void             (* execute_async) (PicmanProcedure   *procedure,
                                      Picman            *picman,
                                      PicmanContext     *context,
                                      PicmanProgress    *progress,
                                      PicmanValueArray  *args,
                                      PicmanObject      *display);
};


GType            picman_procedure_get_type           (void) G_GNUC_CONST;

PicmanProcedure  * picman_procedure_new                (PicmanMarshalFunc   marshal_func);

void             picman_procedure_set_strings        (PicmanProcedure    *procedure,
                                                    const gchar      *original_name,
                                                    const gchar      *blurb,
                                                    const gchar      *help,
                                                    const gchar      *author,
                                                    const gchar      *copyright,
                                                    const gchar      *date,
                                                    const gchar      *deprecated);
void             picman_procedure_set_static_strings (PicmanProcedure    *procedure,
                                                    const gchar      *original_name,
                                                    const gchar      *blurb,
                                                    const gchar      *help,
                                                    const gchar      *author,
                                                    const gchar      *copyright,
                                                    const gchar      *date,
                                                    const gchar      *deprecated);
void             picman_procedure_take_strings       (PicmanProcedure    *procedure,
                                                    gchar            *original_name,
                                                    gchar            *blurb,
                                                    gchar            *help,
                                                    gchar            *author,
                                                    gchar            *copyright,
                                                    gchar            *date,
                                                    gchar            *deprecated);

void             picman_procedure_add_argument       (PicmanProcedure    *procedure,
                                                    GParamSpec       *pspec);
void             picman_procedure_add_return_value   (PicmanProcedure    *procedure,
                                                    GParamSpec       *pspec);

PicmanValueArray * picman_procedure_get_arguments      (PicmanProcedure    *procedure);
PicmanValueArray * picman_procedure_get_return_values  (PicmanProcedure    *procedure,
                                                    gboolean          success,
                                                    const GError     *error);

PicmanProcedure  * picman_procedure_create_override    (PicmanProcedure    *procedure,
                                                    PicmanMarshalFunc   new_marshal_func);

PicmanValueArray * picman_procedure_execute            (PicmanProcedure    *procedure,
                                                    Picman             *picman,
                                                    PicmanContext      *context,
                                                    PicmanProgress     *progress,
                                                    PicmanValueArray   *args,
                                                    GError          **error);
void             picman_procedure_execute_async      (PicmanProcedure    *procedure,
                                                    Picman             *picman,
                                                    PicmanContext      *context,
                                                    PicmanProgress     *progress,
                                                    PicmanValueArray   *args,
                                                    PicmanObject       *display,
                                                    GError          **error);

gint             picman_procedure_name_compare       (PicmanProcedure    *proc1,
                                                    PicmanProcedure    *proc2);



#endif  /*  __PICMAN_PROCEDURE_H__  */
