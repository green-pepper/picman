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

#include "config.h"

#include <glib-object.h>

#include "core-types.h"

#include "core/picmansubprogress.h"
#include "core/picmanprogress.h"


static void           picman_sub_progress_iface_init    (PicmanProgressInterface *iface);

static void           picman_sub_progress_finalize      (GObject             *object);

static PicmanProgress * picman_sub_progress_start         (PicmanProgress        *progress,
                                                       const gchar         *message,
                                                       gboolean             cancelable);
static void           picman_sub_progress_end           (PicmanProgress        *progress);
static gboolean       picman_sub_progress_is_active     (PicmanProgress        *progress);
static void           picman_sub_progress_set_text      (PicmanProgress        *progress,
                                                       const gchar         *message);
static void           picman_sub_progress_set_value     (PicmanProgress        *progress,
                                                       gdouble              percentage);
static gdouble        picman_sub_progress_get_value     (PicmanProgress        *progress);
static void           picman_sub_progress_pulse         (PicmanProgress        *progress);
static guint32        picman_sub_progress_get_window_id (PicmanProgress        *progress);
static gboolean       picman_sub_progress_message       (PicmanProgress        *progress,
                                                       Picman                *picman,
                                                       PicmanMessageSeverity  severity,
                                                       const gchar         *domain,
                                                       const gchar         *message);


G_DEFINE_TYPE_WITH_CODE (PicmanSubProgress, picman_sub_progress, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROGRESS,
                                                picman_sub_progress_iface_init))

#define parent_class picman_sub_progress_parent_class


static void
picman_sub_progress_class_init (PicmanSubProgressClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = picman_sub_progress_finalize;
}

static void
picman_sub_progress_init (PicmanSubProgress *sub)
{
  sub->progress = NULL;
  sub->start    = 0.0;
  sub->end      = 1.0;
}

static void
picman_sub_progress_finalize (GObject *object)
{
  PicmanSubProgress *sub = PICMAN_SUB_PROGRESS (object);

  if (sub->progress)
    {
      g_object_unref (sub->progress);
      sub->progress = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_sub_progress_iface_init (PicmanProgressInterface *iface)
{
  iface->start         = picman_sub_progress_start;
  iface->end           = picman_sub_progress_end;
  iface->is_active     = picman_sub_progress_is_active;
  iface->set_text      = picman_sub_progress_set_text;
  iface->set_value     = picman_sub_progress_set_value;
  iface->get_value     = picman_sub_progress_get_value;
  iface->pulse         = picman_sub_progress_pulse;
  iface->get_window_id = picman_sub_progress_get_window_id;
  iface->message       = picman_sub_progress_message;
}

static PicmanProgress *
picman_sub_progress_start (PicmanProgress *progress,
                         const gchar  *message,
                         gboolean      cancelable)
{
  /* does nothing */
  return NULL;
}

static void
picman_sub_progress_end (PicmanProgress *progress)
{
  /* does nothing */
}

static gboolean
picman_sub_progress_is_active (PicmanProgress *progress)
{
  PicmanSubProgress *sub = PICMAN_SUB_PROGRESS (progress);

  if (sub->progress)
    return picman_progress_is_active (sub->progress);

  return FALSE;
}

static void
picman_sub_progress_set_text (PicmanProgress *progress,
                            const gchar  *message)
{
  /* does nothing */
}

static void
picman_sub_progress_set_value (PicmanProgress *progress,
                             gdouble       percentage)
{
  PicmanSubProgress *sub = PICMAN_SUB_PROGRESS (progress);

  if (sub->progress)
    picman_progress_set_value (sub->progress,
                             sub->start + percentage * (sub->end - sub->start));
}

static gdouble
picman_sub_progress_get_value (PicmanProgress *progress)
{
  PicmanSubProgress *sub = PICMAN_SUB_PROGRESS (progress);

  if (sub->progress)
    return picman_progress_get_value (sub->progress);

  return 0.0;
}

static void
picman_sub_progress_pulse (PicmanProgress *progress)
{
  PicmanSubProgress *sub = PICMAN_SUB_PROGRESS (progress);

  if (sub->progress)
    picman_progress_pulse (sub->progress);
}

static guint32
picman_sub_progress_get_window_id (PicmanProgress *progress)
{
  PicmanSubProgress *sub = PICMAN_SUB_PROGRESS (progress);

  if (sub->progress)
    return picman_progress_get_window_id (sub->progress);

  return 0;
}

static gboolean
picman_sub_progress_message (PicmanProgress        *progress,
                           Picman                *picman,
                           PicmanMessageSeverity  severity,
                           const gchar         *domain,
                           const gchar         *message)
{
  PicmanSubProgress *sub = PICMAN_SUB_PROGRESS (progress);

  if (sub->progress)
    return picman_progress_message (sub->progress,
                                  picman, severity, domain, message);

  return FALSE;
}

/**
 * picman_sub_progress_new:
 * @progress: parent progress or %NULL
 *
 * PicmanSubProgress implements the PicmanProgress interface and can be
 * used wherever a PicmanProgress is needed. It maps progress
 * information to a sub-range of its parent @progress. This is useful
 * when an action breaks down into multiple sub-actions that itself
 * need a #PicmanProgress pointer. See picman_image_scale() for an example.
 *
 * Return value: a new #PicmanProgress object
 */
PicmanProgress *
picman_sub_progress_new (PicmanProgress *progress)
{
  PicmanSubProgress *sub;

  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);

  sub = g_object_new (PICMAN_TYPE_SUB_PROGRESS, NULL);

  if (progress)
    sub->progress = g_object_ref (progress);

  return PICMAN_PROGRESS (sub);
}

/**
 * picman_sub_progress_set_range:
 * @start: start value of range on the parent process
 * @end:   end value of range on the parent process
 *
 * Sets a range on the parent progress that this @progress should be
 * mapped to.
 */
void
picman_sub_progress_set_range (PicmanSubProgress *progress,
                             gdouble          start,
                             gdouble          end)
{
  g_return_if_fail (PICMAN_IS_SUB_PROGRESS (progress));
  g_return_if_fail (start < end);

  progress->start = start;
  progress->end   = end;
}

/**
 * picman_sub_progress_set_step:
 * @index:     step index
 * @num_steps: number of steps
 *
 * A more convenient form of picman_sub_progress_set_range().
 */
void
picman_sub_progress_set_step (PicmanSubProgress *progress,
                            gint             index,
                            gint             num_steps)
{
  g_return_if_fail (PICMAN_IS_SUB_PROGRESS (progress));
  g_return_if_fail (index < num_steps && num_steps > 0);

  progress->start = (gdouble) index / num_steps;
  progress->end   = (gdouble) (index + 1) / num_steps;
}
