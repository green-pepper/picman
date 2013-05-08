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

#include "paint-types.h"

#include "picmanpencil.h"
#include "picmanpenciloptions.h"

#include "picman-intl.h"


G_DEFINE_TYPE (PicmanPencil, picman_pencil, PICMAN_TYPE_PAINTBRUSH)


void
picman_pencil_register (Picman                      *picman,
                      PicmanPaintRegisterCallback  callback)
{
  (* callback) (picman,
                PICMAN_TYPE_PENCIL,
                PICMAN_TYPE_PENCIL_OPTIONS,
                "picman-pencil",
                _("Pencil"),
                "picman-tool-pencil");
}

static void
picman_pencil_class_init (PicmanPencilClass *klass)
{
}

static void
picman_pencil_init (PicmanPencil *pencil)
{
}
