/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmantemplate.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

/* This file contains the definition of the image template objects.
 */

#include "config.h"

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "gegl/picman-babl.h"

#include "picmanimage.h"
#include "picmanprojection.h"
#include "picmantemplate.h"

#include "picman-intl.h"


#define DEFAULT_RESOLUTION    72.0


enum
{
  PROP_0,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_UNIT,
  PROP_XRESOLUTION,
  PROP_YRESOLUTION,
  PROP_RESOLUTION_UNIT,
  PROP_BASE_TYPE,
  PROP_PRECISION,
  PROP_FILL_TYPE,
  PROP_COMMENT,
  PROP_FILENAME
};


typedef struct _PicmanTemplatePrivate PicmanTemplatePrivate;

struct _PicmanTemplatePrivate
{
  gint               width;
  gint               height;
  PicmanUnit           unit;

  gdouble            xresolution;
  gdouble            yresolution;
  PicmanUnit           resolution_unit;

  PicmanImageBaseType  base_type;
  PicmanPrecision      precision;

  PicmanFillType       fill_type;

  gchar             *comment;
  gchar             *filename;

  guint64            initial_size;
};

#define GET_PRIVATE(template) G_TYPE_INSTANCE_GET_PRIVATE (template, \
                                                           PICMAN_TYPE_TEMPLATE, \
                                                           PicmanTemplatePrivate)


static void      picman_template_finalize     (GObject      *object);
static void      picman_template_set_property (GObject      *object,
                                             guint         property_id,
                                             const GValue *value,
                                             GParamSpec   *pspec);
static void      picman_template_get_property (GObject      *object,
                                             guint         property_id,
                                             GValue       *value,
                                             GParamSpec   *pspec);
static void      picman_template_notify       (GObject      *object,
                                             GParamSpec   *pspec);


G_DEFINE_TYPE_WITH_CODE (PicmanTemplate, picman_template, PICMAN_TYPE_VIEWABLE,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG, NULL))

#define parent_class picman_template_parent_class


static void
picman_template_class_init (PicmanTemplateClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);

  object_class->finalize     = picman_template_finalize;

  object_class->set_property = picman_template_set_property;
  object_class->get_property = picman_template_get_property;
  object_class->notify       = picman_template_notify;

  viewable_class->default_stock_id = "picman-template";

  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_WIDTH, "width",
                                NULL,
                                PICMAN_MIN_IMAGE_SIZE, PICMAN_MAX_IMAGE_SIZE,
                                PICMAN_DEFAULT_IMAGE_WIDTH,
                                PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_HEIGHT, "height",
                                NULL,
                                PICMAN_MIN_IMAGE_SIZE, PICMAN_MAX_IMAGE_SIZE,
                                PICMAN_DEFAULT_IMAGE_HEIGHT,
                                PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_UNIT (object_class, PROP_UNIT, "unit",
                                 N_("The unit used for coordinate display "
                                    "when not in dot-for-dot mode."),
                                 TRUE, FALSE, PICMAN_UNIT_PIXEL,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_RESOLUTION (object_class, PROP_XRESOLUTION,
                                       "xresolution",
                                       N_("The horizontal image resolution."),
                                       DEFAULT_RESOLUTION,
                                       PICMAN_PARAM_STATIC_STRINGS |
                                       PICMAN_TEMPLATE_PARAM_COPY_FIRST);
  PICMAN_CONFIG_INSTALL_PROP_RESOLUTION (object_class, PROP_YRESOLUTION,
                                       "yresolution",
                                       N_("The vertical image resolution."),
                                       DEFAULT_RESOLUTION,
                                       PICMAN_PARAM_STATIC_STRINGS |
                                       PICMAN_TEMPLATE_PARAM_COPY_FIRST);
  PICMAN_CONFIG_INSTALL_PROP_UNIT (object_class, PROP_RESOLUTION_UNIT,
                                 "resolution-unit",
                                 NULL,
                                 FALSE, FALSE, PICMAN_UNIT_INCH,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_BASE_TYPE,
                                 "image-type", /* serialized name */
                                 NULL,
                                 PICMAN_TYPE_IMAGE_BASE_TYPE, PICMAN_RGB,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_PRECISION,
                                 "precision",
                                 NULL,
                                 PICMAN_TYPE_PRECISION, PICMAN_PRECISION_U8,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_FILL_TYPE,
                                 "fill-type",
                                 NULL,
                                 PICMAN_TYPE_FILL_TYPE, PICMAN_BACKGROUND_FILL,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_STRING (object_class, PROP_COMMENT,
                                   "comment",
                                   NULL,
                                   NULL,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_STRING (object_class, PROP_FILENAME,
                                   "filename",
                                   NULL,
                                   NULL,
                                   PICMAN_PARAM_STATIC_STRINGS);

  g_type_class_add_private (klass, sizeof (PicmanTemplatePrivate));
}

static void
picman_template_init (PicmanTemplate *template)
{
}

static void
picman_template_finalize (GObject *object)
{
  PicmanTemplatePrivate *private = GET_PRIVATE (object);

  if (private->comment)
    {
      g_free (private->comment);
      private->comment = NULL;
    }

  if (private->filename)
    {
      g_free (private->filename);
      private->filename = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_template_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  PicmanTemplatePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_WIDTH:
      private->width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      private->height = g_value_get_int (value);
      break;
    case PROP_UNIT:
      private->unit = g_value_get_int (value);
      break;
    case PROP_XRESOLUTION:
      private->xresolution = g_value_get_double (value);
      break;
    case PROP_YRESOLUTION:
      private->yresolution = g_value_get_double (value);
      break;
    case PROP_RESOLUTION_UNIT:
      private->resolution_unit = g_value_get_int (value);
      break;
    case PROP_BASE_TYPE:
      private->base_type = g_value_get_enum (value);
      break;
    case PROP_PRECISION:
      private->precision = g_value_get_enum (value);
      break;
    case PROP_FILL_TYPE:
      private->fill_type = g_value_get_enum (value);
      break;
    case PROP_COMMENT:
      if (private->comment)
        g_free (private->comment);
      private->comment = g_value_dup_string (value);
      break;
    case PROP_FILENAME:
      if (private->filename)
        g_free (private->filename);
      private->filename = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_template_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  PicmanTemplatePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_WIDTH:
      g_value_set_int (value, private->width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, private->height);
      break;
    case PROP_UNIT:
      g_value_set_int (value, private->unit);
      break;
    case PROP_XRESOLUTION:
      g_value_set_double (value, private->xresolution);
      break;
    case PROP_YRESOLUTION:
      g_value_set_double (value, private->yresolution);
      break;
    case PROP_RESOLUTION_UNIT:
      g_value_set_int (value, private->resolution_unit);
      break;
    case PROP_BASE_TYPE:
      g_value_set_enum (value, private->base_type);
      break;
    case PROP_PRECISION:
      g_value_set_enum (value, private->precision);
      break;
    case PROP_FILL_TYPE:
      g_value_set_enum (value, private->fill_type);
      break;
    case PROP_COMMENT:
      g_value_set_string (value, private->comment);
      break;
    case PROP_FILENAME:
      g_value_set_string (value, private->filename);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_template_notify (GObject    *object,
                      GParamSpec *pspec)
{
  PicmanTemplatePrivate *private = GET_PRIVATE (object);
  const Babl          *format;
  gint                 bytes;

  if (G_OBJECT_CLASS (parent_class)->notify)
    G_OBJECT_CLASS (parent_class)->notify (object, pspec);

  /* the initial layer */
  format = picman_babl_format (private->base_type,
                             private->precision,
                             private->fill_type == PICMAN_TRANSPARENT_FILL);
  bytes = babl_format_get_bytes_per_pixel (format);

  /* the selection */
  format = picman_babl_format (PICMAN_GRAY,
                             private->precision,
                             FALSE);
  bytes += babl_format_get_bytes_per_pixel (format);

  private->initial_size = ((guint64) bytes          *
                           (guint64) private->width *
                           (guint64) private->height);

  private->initial_size +=
    picman_projection_estimate_memsize (private->base_type,
                                      private->precision,
                                      private->width, private->height);

  if (! strcmp (pspec->name, "stock-id"))
    picman_viewable_invalidate_preview (PICMAN_VIEWABLE (object));
}


/*  public functions  */

PicmanTemplate *
picman_template_new (const gchar *name)
{
  g_return_val_if_fail (name != NULL, NULL);

  return g_object_new (PICMAN_TYPE_TEMPLATE,
                       "name", name,
                       NULL);
}

void
picman_template_set_from_image (PicmanTemplate *template,
                              PicmanImage    *image)
{
  gdouble             xresolution;
  gdouble             yresolution;
  PicmanImageBaseType   base_type;
  const PicmanParasite *parasite;
  gchar              *comment = NULL;

  g_return_if_fail (PICMAN_IS_TEMPLATE (template));
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  picman_image_get_resolution (image, &xresolution, &yresolution);

  base_type = picman_image_get_base_type (image);

  if (base_type == PICMAN_INDEXED)
    base_type = PICMAN_RGB;

  parasite =  picman_image_parasite_find (image, "picman-comment");
  if (parasite)
    comment = g_strndup (picman_parasite_data (parasite),
                         picman_parasite_data_size (parasite));

  g_object_set (template,
                "width",           picman_image_get_width (image),
                "height",          picman_image_get_height (image),
                "xresolution",     xresolution,
                "yresolution",     yresolution,
                "resolution-unit", picman_image_get_unit (image),
                "image-type",      base_type,
                "precision",       picman_image_get_precision (image),
                "comment",         comment,
                NULL);

  if (comment)
    g_free (comment);
}

gint
picman_template_get_width (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), 0);

  return GET_PRIVATE (template)->width;
}

gint
picman_template_get_height (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), 0);

  return GET_PRIVATE (template)->height;
}

PicmanUnit
picman_template_get_unit (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), PICMAN_UNIT_INCH);

  return GET_PRIVATE (template)->unit;
}

gdouble
picman_template_get_resolution_x (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), 1.0);

  return GET_PRIVATE (template)->xresolution;
}

gdouble
picman_template_get_resolution_y (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), 1.0);

  return GET_PRIVATE (template)->yresolution;
}

PicmanUnit
picman_template_get_resolution_unit (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), PICMAN_UNIT_INCH);

  return GET_PRIVATE (template)->resolution_unit;
}

PicmanImageBaseType
picman_template_get_base_type (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), PICMAN_RGB);

  return GET_PRIVATE (template)->base_type;
}

PicmanPrecision
picman_template_get_precision (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), PICMAN_PRECISION_U8);

  return GET_PRIVATE (template)->precision;
}

PicmanFillType
picman_template_get_fill_type (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), PICMAN_NO_FILL);

  return GET_PRIVATE (template)->fill_type;
}

const gchar *
picman_template_get_comment (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), NULL);

  return GET_PRIVATE (template)->comment;
}

guint64
picman_template_get_initial_size (PicmanTemplate *template)
{
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), 0);

  return GET_PRIVATE (template)->initial_size;
}
