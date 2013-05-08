/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpdbprogress.c
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#include "config.h"

#include <string.h>

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "core/picmancontext.h"

#include "pdb/picmanpdb.h"

#include "picman.h"
#include "picmanparamspecs.h"
#include "picmanpdbprogress.h"
#include "picmanprogress.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_PDB,
  PROP_CONTEXT,
  PROP_CALLBACK_NAME
};


static void      picman_pdb_progress_class_init     (PicmanPdbProgressClass *klass);
static void      picman_pdb_progress_init           (PicmanPdbProgress      *progress,
                                                   PicmanPdbProgressClass *klass);
static void picman_pdb_progress_progress_iface_init (PicmanProgressInterface *iface);

static void      picman_pdb_progress_constructed    (GObject            *object);
static void      picman_pdb_progress_dispose        (GObject            *object);
static void      picman_pdb_progress_finalize       (GObject            *object);
static void      picman_pdb_progress_set_property   (GObject            *object,
                                                   guint               property_id,
                                                   const GValue       *value,
                                                   GParamSpec         *pspec);

static PicmanProgress * picman_pdb_progress_progress_start   (PicmanProgress *progress,
                                                          const gchar  *message,
                                                          gboolean      cancelable);
static void     picman_pdb_progress_progress_end           (PicmanProgress *progress);
static gboolean picman_pdb_progress_progress_is_active     (PicmanProgress *progress);
static void     picman_pdb_progress_progress_set_text      (PicmanProgress *progress,
                                                          const gchar  *message);
static void     picman_pdb_progress_progress_set_value     (PicmanProgress *progress,
                                                          gdouble       percentage);
static gdouble  picman_pdb_progress_progress_get_value     (PicmanProgress *progress);
static void     picman_pdb_progress_progress_pulse         (PicmanProgress *progress);
static guint32  picman_pdb_progress_progress_get_window_id (PicmanProgress *progress);


static GObjectClass *parent_class = NULL;


GType
picman_pdb_progress_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (PicmanPdbProgressClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) picman_pdb_progress_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (PicmanPdbProgress),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) picman_pdb_progress_init,
      };

      const GInterfaceInfo progress_iface_info =
      {
        (GInterfaceInitFunc) picman_pdb_progress_progress_iface_init,
        NULL,           /* iface_finalize */
        NULL            /* iface_data     */
      };

      type = g_type_register_static (G_TYPE_OBJECT,
                                     "PicmanPdbProgress",
                                     &info, 0);

      g_type_add_interface_static (type, PICMAN_TYPE_PROGRESS,
                                   &progress_iface_info);
    }

  return type;
}

static void
picman_pdb_progress_class_init (PicmanPdbProgressClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->constructed  = picman_pdb_progress_constructed;
  object_class->dispose      = picman_pdb_progress_dispose;
  object_class->finalize     = picman_pdb_progress_finalize;
  object_class->set_property = picman_pdb_progress_set_property;

  g_object_class_install_property (object_class, PROP_PDB,
                                   g_param_spec_object ("pdb", NULL, NULL,
                                                        PICMAN_TYPE_PDB,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context", NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CALLBACK_NAME,
                                   g_param_spec_string ("callback-name",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_pdb_progress_init (PicmanPdbProgress      *progress,
                        PicmanPdbProgressClass *klass)
{
  klass->progresses = g_list_prepend (klass->progresses, progress);
}

static void
picman_pdb_progress_progress_iface_init (PicmanProgressInterface *iface)
{
  iface->start         = picman_pdb_progress_progress_start;
  iface->end           = picman_pdb_progress_progress_end;
  iface->is_active     = picman_pdb_progress_progress_is_active;
  iface->set_text      = picman_pdb_progress_progress_set_text;
  iface->set_value     = picman_pdb_progress_progress_set_value;
  iface->get_value     = picman_pdb_progress_progress_get_value;
  iface->pulse         = picman_pdb_progress_progress_pulse;
  iface->get_window_id = picman_pdb_progress_progress_get_window_id;
}

static void
picman_pdb_progress_constructed (GObject *object)
{
  PicmanPdbProgress *progress = PICMAN_PDB_PROGRESS (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PDB (progress->pdb));
  g_assert (PICMAN_IS_CONTEXT (progress->context));
}

static void
picman_pdb_progress_dispose (GObject *object)
{
  PicmanPdbProgressClass *klass = PICMAN_PDB_PROGRESS_GET_CLASS (object);

  klass->progresses = g_list_remove (klass->progresses, object);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_pdb_progress_finalize (GObject *object)
{
  PicmanPdbProgress *progress = PICMAN_PDB_PROGRESS (object);

  if (progress->pdb)
    {
      g_object_unref (progress->pdb);
      progress->pdb = NULL;
    }

  if (progress->context)
    {
      g_object_unref (progress->context);
      progress->context = NULL;
    }

  if (progress->callback_name)
    {
      g_free (progress->callback_name);
      progress->callback_name = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_pdb_progress_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanPdbProgress *progress = PICMAN_PDB_PROGRESS (object);

  switch (property_id)
    {
    case PROP_PDB:
      if (progress->pdb)
        g_object_unref (progress->pdb);
      progress->pdb = g_value_dup_object (value);
      break;

    case PROP_CONTEXT:
      if (progress->context)
        g_object_unref (progress->context);
      progress->context = g_value_dup_object (value);
      break;

    case PROP_CALLBACK_NAME:
      if (progress->callback_name)
        g_free (progress->callback_name);
      progress->callback_name = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gdouble
picman_pdb_progress_run_callback (PicmanPdbProgress     *progress,
                                PicmanProgressCommand  command,
                                const gchar         *text,
                                gdouble              value)
{
  gdouble retval = 0;

  if (progress->callback_name && ! progress->callback_busy)
    {
      PicmanValueArray *return_vals;

      progress->callback_busy = TRUE;

      return_vals =
        picman_pdb_execute_procedure_by_name (progress->pdb,
                                            progress->context,
                                            NULL, NULL,
                                            progress->callback_name,
                                            PICMAN_TYPE_INT32, command,
                                            G_TYPE_STRING,   text,
                                            G_TYPE_DOUBLE,   value,
                                            G_TYPE_NONE);

      if (g_value_get_enum (picman_value_array_index (return_vals, 0)) !=
          PICMAN_PDB_SUCCESS)
        {
          picman_message (progress->context->picman, NULL, PICMAN_MESSAGE_ERROR,
                        _("Unable to run %s callback. "
                          "The corresponding plug-in may have crashed."),
                        g_type_name (G_TYPE_FROM_INSTANCE (progress)));
        }
      else if (picman_value_array_length (return_vals) >= 2 &&
               G_VALUE_HOLDS_DOUBLE (picman_value_array_index (return_vals, 1)))
        {
          retval = g_value_get_double (picman_value_array_index (return_vals, 1));
        }

      picman_value_array_unref (return_vals);

      progress->callback_busy = FALSE;
    }

  return retval;
}

static PicmanProgress *
picman_pdb_progress_progress_start (PicmanProgress *progress,
                                  const gchar  *message,
                                  gboolean      cancelable)
{
  PicmanPdbProgress *pdb_progress = PICMAN_PDB_PROGRESS (progress);

  if (! pdb_progress->active)
    {
      picman_pdb_progress_run_callback (pdb_progress,
                                      PICMAN_PROGRESS_COMMAND_START,
                                      message, 0.0);

      pdb_progress->active = TRUE;
      pdb_progress->value  = 0.0;

      return progress;
    }

  return NULL;
}

static void
picman_pdb_progress_progress_end (PicmanProgress *progress)
{
  PicmanPdbProgress *pdb_progress = PICMAN_PDB_PROGRESS (progress);

  if (pdb_progress->active)
    {
      picman_pdb_progress_run_callback (pdb_progress,
                                      PICMAN_PROGRESS_COMMAND_END,
                                      NULL, 0.0);

      pdb_progress->active = FALSE;
      pdb_progress->value  = 0.0;
    }
}

static gboolean
picman_pdb_progress_progress_is_active (PicmanProgress *progress)
{
  PicmanPdbProgress *pdb_progress = PICMAN_PDB_PROGRESS (progress);

  return pdb_progress->active;
}

static void
picman_pdb_progress_progress_set_text (PicmanProgress *progress,
                                     const gchar  *message)
{
  PicmanPdbProgress *pdb_progress = PICMAN_PDB_PROGRESS (progress);

  if (pdb_progress->active)
    picman_pdb_progress_run_callback (pdb_progress,
                                    PICMAN_PROGRESS_COMMAND_SET_TEXT,
                                    message, 0.0);
}

static void
picman_pdb_progress_progress_set_value (PicmanProgress *progress,
                                      gdouble       percentage)
{
  PicmanPdbProgress *pdb_progress = PICMAN_PDB_PROGRESS (progress);

  if (pdb_progress->active)
    {
      picman_pdb_progress_run_callback (pdb_progress,
                                      PICMAN_PROGRESS_COMMAND_SET_VALUE,
                                      NULL, percentage);
      pdb_progress->value = percentage;
    }
}

static gdouble
picman_pdb_progress_progress_get_value (PicmanProgress *progress)
{
  PicmanPdbProgress *pdb_progress = PICMAN_PDB_PROGRESS (progress);

  return pdb_progress->value;

}

static void
picman_pdb_progress_progress_pulse (PicmanProgress *progress)
{
  PicmanPdbProgress *pdb_progress = PICMAN_PDB_PROGRESS (progress);

  if (pdb_progress->active)
    picman_pdb_progress_run_callback (pdb_progress,
                                    PICMAN_PROGRESS_COMMAND_PULSE,
                                    NULL, 0.0);
}

static guint32
picman_pdb_progress_progress_get_window_id (PicmanProgress *progress)
{
  PicmanPdbProgress *pdb_progress = PICMAN_PDB_PROGRESS (progress);

  return (guint32)
    picman_pdb_progress_run_callback (pdb_progress,
                                    PICMAN_PROGRESS_COMMAND_GET_WINDOW,
                                    NULL, 0.0);
}

PicmanPdbProgress *
picman_pdb_progress_get_by_callback (PicmanPdbProgressClass *klass,
                                   const gchar          *callback_name)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_PDB_PROGRESS_CLASS (klass), NULL);
  g_return_val_if_fail (callback_name != NULL, NULL);

  for (list = klass->progresses; list; list = g_list_next (list))
    {
      PicmanPdbProgress *progress = list->data;

      if (! g_strcmp0 (callback_name, progress->callback_name))
        return progress;
    }

  return NULL;
}
