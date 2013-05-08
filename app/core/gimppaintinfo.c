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

#include <gegl.h>

#include "core-types.h"

#include "paint/picmanpaintoptions.h"

#include "picman.h"
#include "picmanpaintinfo.h"


static void    picman_paint_info_dispose         (GObject       *object);
static void    picman_paint_info_finalize        (GObject       *object);
static gchar * picman_paint_info_get_description (PicmanViewable  *viewable,
                                                gchar        **tooltip);


G_DEFINE_TYPE (PicmanPaintInfo, picman_paint_info, PICMAN_TYPE_VIEWABLE)

#define parent_class picman_paint_info_parent_class


static void
picman_paint_info_class_init (PicmanPaintInfoClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);

  object_class->dispose           = picman_paint_info_dispose;
  object_class->finalize          = picman_paint_info_finalize;

  viewable_class->get_description = picman_paint_info_get_description;
}

static void
picman_paint_info_init (PicmanPaintInfo *paint_info)
{
  paint_info->picman          = NULL;
  paint_info->paint_type    = G_TYPE_NONE;
  paint_info->blurb         = NULL;
  paint_info->paint_options = NULL;
}

static void
picman_paint_info_dispose (GObject *object)
{
  PicmanPaintInfo *paint_info = PICMAN_PAINT_INFO (object);

  if (paint_info->paint_options)
    {
      g_object_run_dispose (G_OBJECT (paint_info->paint_options));
      g_object_unref (paint_info->paint_options);
      paint_info->paint_options = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_paint_info_finalize (GObject *object)
{
  PicmanPaintInfo *paint_info = PICMAN_PAINT_INFO (object);

  if (paint_info->blurb)
    {
      g_free (paint_info->blurb);
      paint_info->blurb = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gchar *
picman_paint_info_get_description (PicmanViewable  *viewable,
                                 gchar        **tooltip)
{
  PicmanPaintInfo *paint_info = PICMAN_PAINT_INFO (viewable);

  return g_strdup (paint_info->blurb);
}

PicmanPaintInfo *
picman_paint_info_new (Picman        *picman,
                     GType        paint_type,
                     GType        paint_options_type,
                     const gchar *identifier,
                     const gchar *blurb,
                     const gchar *stock_id)
{
  PicmanPaintInfo *paint_info;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (identifier != NULL, NULL);
  g_return_val_if_fail (blurb != NULL, NULL);
  g_return_val_if_fail (stock_id != NULL, NULL);

  paint_info = g_object_new (PICMAN_TYPE_PAINT_INFO,
                             "name",     identifier,
                             "stock-id", stock_id,
                             NULL);

  paint_info->picman               = picman;
  paint_info->paint_type         = paint_type;
  paint_info->paint_options_type = paint_options_type;
  paint_info->blurb              = g_strdup (blurb);

  paint_info->paint_options      = picman_paint_options_new (paint_info);

  return paint_info;
}

void
picman_paint_info_set_standard (Picman          *picman,
                              PicmanPaintInfo *paint_info)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (! paint_info || PICMAN_IS_PAINT_INFO (paint_info));

  if (paint_info != picman->standard_paint_info)
    {
      if (picman->standard_paint_info)
        g_object_unref (picman->standard_paint_info);

      picman->standard_paint_info = paint_info;

      if (picman->standard_paint_info)
        g_object_ref (picman->standard_paint_info);
    }
}

PicmanPaintInfo *
picman_paint_info_get_standard (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return picman->standard_paint_info;
}
