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

#ifndef __PICMAN_PARAM_SPECS_H__
#define __PICMAN_PARAM_SPECS_H__


/*
 * Keep in sync with libpicmanconfig/picmanconfig-params.h
 */
#define PICMAN_PARAM_NO_VALIDATE (1 << (6 + G_PARAM_USER_SHIFT))


/*
 * PICMAN_TYPE_INT32
 */

#define PICMAN_TYPE_INT32               (picman_int32_get_type ())
#define PICMAN_VALUE_HOLDS_INT32(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                       PICMAN_TYPE_INT32))

GType   picman_int32_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_INT32
 */

#define PICMAN_TYPE_PARAM_INT32           (picman_param_int32_get_type ())
#define PICMAN_PARAM_SPEC_INT32(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_INT32, PicmanParamSpecInt32))
#define PICMAN_IS_PARAM_SPEC_INT32(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_INT32))

typedef struct _PicmanParamSpecInt32 PicmanParamSpecInt32;

struct _PicmanParamSpecInt32
{
  GParamSpecInt parent_instance;
};

GType        picman_param_int32_get_type (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_int32     (const gchar *name,
                                        const gchar *nick,
                                        const gchar *blurb,
                                        gint         minimum,
                                        gint         maximum,
                                        gint         default_value,
                                        GParamFlags  flags);


/*
 * PICMAN_TYPE_INT16
 */

#define PICMAN_TYPE_INT16               (picman_int16_get_type ())
#define PICMAN_VALUE_HOLDS_INT16(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                       PICMAN_TYPE_INT16))

GType   picman_int16_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_INT16
 */

#define PICMAN_TYPE_PARAM_INT16           (picman_param_int16_get_type ())
#define PICMAN_PARAM_SPEC_INT16(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_INT16, PicmanParamSpecInt16))
#define PICMAN_IS_PARAM_SPEC_INT16(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_INT16))

typedef struct _PicmanParamSpecInt16 PicmanParamSpecInt16;

struct _PicmanParamSpecInt16
{
  GParamSpecInt parent_instance;
};

GType        picman_param_int16_get_type (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_int16     (const gchar *name,
                                        const gchar *nick,
                                        const gchar *blurb,
                                        gint         minimum,
                                        gint         maximum,
                                        gint         default_value,
                                        GParamFlags  flags);


/*
 * PICMAN_TYPE_INT8
 */

#define PICMAN_TYPE_INT8               (picman_int8_get_type ())
#define PICMAN_VALUE_HOLDS_INT8(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                      PICMAN_TYPE_INT8))

GType   picman_int8_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_INT8
 */

#define PICMAN_TYPE_PARAM_INT8           (picman_param_int8_get_type ())
#define PICMAN_PARAM_SPEC_INT8(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_INT8, PicmanParamSpecInt8))
#define PICMAN_IS_PARAM_SPEC_INT8(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_INT8))

typedef struct _PicmanParamSpecInt8 PicmanParamSpecInt8;

struct _PicmanParamSpecInt8
{
  GParamSpecUInt parent_instance;
};

GType        picman_param_int8_get_type (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_int8     (const gchar *name,
                                       const gchar *nick,
                                       const gchar *blurb,
                                       guint        minimum,
                                       guint        maximum,
                                       guint        default_value,
                                       GParamFlags  flags);


/*
 * PICMAN_TYPE_PARAM_STRING
 */

#define PICMAN_TYPE_PARAM_STRING           (picman_param_string_get_type ())
#define PICMAN_PARAM_SPEC_STRING(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_STRING, PicmanParamSpecString))
#define PICMAN_IS_PARAM_SPEC_STRING(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_STRING))

typedef struct _PicmanParamSpecString PicmanParamSpecString;

struct _PicmanParamSpecString
{
  GParamSpecString parent_instance;

  guint            allow_non_utf8 : 1;
  guint            non_empty      : 1;
};

GType        picman_param_string_get_type (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_string     (const gchar *name,
                                         const gchar *nick,
                                         const gchar *blurb,
                                         gboolean     allow_non_utf8,
                                         gboolean     null_ok,
                                         gboolean     non_empty,
                                         const gchar *default_value,
                                         GParamFlags  flags);


/*
 * PICMAN_TYPE_PARAM_ENUM
 */

#define PICMAN_TYPE_PARAM_ENUM           (picman_param_enum_get_type ())
#define PICMAN_PARAM_SPEC_ENUM(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_ENUM, PicmanParamSpecEnum))

#define PICMAN_IS_PARAM_SPEC_ENUM(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_ENUM))

typedef struct _PicmanParamSpecEnum PicmanParamSpecEnum;

struct _PicmanParamSpecEnum
{
  GParamSpecEnum  parent_instance;

  GSList         *excluded_values;
};

GType        picman_param_enum_get_type     (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_enum         (const gchar       *name,
                                           const gchar       *nick,
                                           const gchar       *blurb,
                                           GType              enum_type,
                                           gint               default_value,
                                           GParamFlags        flags);

void   picman_param_spec_enum_exclude_value (PicmanParamSpecEnum *espec,
                                           gint               value);


/*
 * PICMAN_TYPE_IMAGE_ID
 */

#define PICMAN_TYPE_IMAGE_ID               (picman_image_id_get_type ())
#define PICMAN_VALUE_HOLDS_IMAGE_ID(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                          PICMAN_TYPE_IMAGE_ID))

GType   picman_image_id_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_IMAGE_ID
 */

#define PICMAN_TYPE_PARAM_IMAGE_ID           (picman_param_image_id_get_type ())
#define PICMAN_PARAM_SPEC_IMAGE_ID(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_IMAGE_ID, PicmanParamSpecImageID))
#define PICMAN_IS_PARAM_SPEC_IMAGE_ID(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_IMAGE_ID))

typedef struct _PicmanParamSpecImageID PicmanParamSpecImageID;

struct _PicmanParamSpecImageID
{
  GParamSpecInt  parent_instance;

  Picman          *picman;
  gboolean       none_ok;
};

GType        picman_param_image_id_get_type (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_image_id     (const gchar  *name,
                                           const gchar  *nick,
                                           const gchar  *blurb,
                                           Picman         *picman,
                                           gboolean      none_ok,
                                           GParamFlags   flags);

PicmanImage  * picman_value_get_image         (const GValue *value,
                                           Picman         *picman);
void         picman_value_set_image         (GValue       *value,
                                           PicmanImage    *image);



/*
 * PICMAN_TYPE_ITEM_ID
 */

#define PICMAN_TYPE_ITEM_ID               (picman_item_id_get_type ())
#define PICMAN_VALUE_HOLDS_ITEM_ID(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                         PICMAN_TYPE_ITEM_ID))

GType   picman_item_id_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_ITEM_ID
 */

#define PICMAN_TYPE_PARAM_ITEM_ID           (picman_param_item_id_get_type ())
#define PICMAN_PARAM_SPEC_ITEM_ID(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_ITEM_ID, PicmanParamSpecItemID))
#define PICMAN_IS_PARAM_SPEC_ITEM_ID(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_ITEM_ID))

typedef struct _PicmanParamSpecItemID PicmanParamSpecItemID;

struct _PicmanParamSpecItemID
{
  GParamSpecInt  parent_instance;

  Picman          *picman;
  GType          item_type;
  gboolean       none_ok;
};

GType        picman_param_item_id_get_type (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_item_id     (const gchar  *name,
                                          const gchar  *nick,
                                          const gchar  *blurb,
                                          Picman         *picman,
                                          gboolean      none_ok,
                                          GParamFlags   flags);

PicmanItem   * picman_value_get_item         (const GValue *value,
                                          Picman         *picman);
void         picman_value_set_item         (GValue       *value,
                                          PicmanItem     *item);


/*
 * PICMAN_TYPE_DRAWABLE_ID
 */

#define PICMAN_TYPE_DRAWABLE_ID               (picman_drawable_id_get_type ())
#define PICMAN_VALUE_HOLDS_DRAWABLE_ID(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                             PICMAN_TYPE_DRAWABLE_ID))

GType   picman_drawable_id_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_DRAWABLE_ID
 */

#define PICMAN_TYPE_PARAM_DRAWABLE_ID           (picman_param_drawable_id_get_type ())
#define PICMAN_PARAM_SPEC_DRAWABLE_ID(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_DRAWABLE_ID, PicmanParamSpecDrawableID))
#define PICMAN_IS_PARAM_SPEC_DRAWABLE_ID(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_DRAWABLE_ID))

typedef struct _PicmanParamSpecDrawableID PicmanParamSpecDrawableID;

struct _PicmanParamSpecDrawableID
{
  PicmanParamSpecItemID parent_instance;
};

GType         picman_param_drawable_id_get_type (void) G_GNUC_CONST;

GParamSpec  * picman_param_spec_drawable_id     (const gchar  *name,
                                               const gchar  *nick,
                                               const gchar  *blurb,
                                               Picman         *picman,
                                               gboolean      none_ok,
                                               GParamFlags   flags);

PicmanDrawable * picman_value_get_drawable        (const GValue *value,
                                               Picman         *picman);
void           picman_value_set_drawable        (GValue       *value,
                                               PicmanDrawable *drawable);


/*
 * PICMAN_TYPE_LAYER_ID
 */

#define PICMAN_TYPE_LAYER_ID               (picman_layer_id_get_type ())
#define PICMAN_VALUE_HOLDS_LAYER_ID(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                          PICMAN_TYPE_LAYER_ID))

GType   picman_layer_id_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_LAYER_ID
 */

#define PICMAN_TYPE_PARAM_LAYER_ID           (picman_param_layer_id_get_type ())
#define PICMAN_PARAM_SPEC_LAYER_ID(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_LAYER_ID, PicmanParamSpecLayerID))
#define PICMAN_IS_PARAM_SPEC_LAYER_ID(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_LAYER_ID))

typedef struct _PicmanParamSpecLayerID PicmanParamSpecLayerID;

struct _PicmanParamSpecLayerID
{
  PicmanParamSpecDrawableID parent_instance;
};

GType        picman_param_layer_id_get_type (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_layer_id     (const gchar  *name,
                                           const gchar  *nick,
                                           const gchar  *blurb,
                                           Picman         *picman,
                                           gboolean      none_ok,
                                           GParamFlags   flags);

PicmanLayer  * picman_value_get_layer         (const GValue *value,
                                           Picman         *picman);
void         picman_value_set_layer         (GValue       *value,
                                           PicmanLayer    *layer);


/*
 * PICMAN_TYPE_CHANNEL_ID
 */

#define PICMAN_TYPE_CHANNEL_ID               (picman_channel_id_get_type ())
#define PICMAN_VALUE_HOLDS_CHANNEL_ID(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                            PICMAN_TYPE_CHANNEL_ID))

GType   picman_channel_id_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_CHANNEL_ID
 */

#define PICMAN_TYPE_PARAM_CHANNEL_ID           (picman_param_channel_id_get_type ())
#define PICMAN_PARAM_SPEC_CHANNEL_ID(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_CHANNEL_ID, PicmanParamSpecChannelID))
#define PICMAN_IS_PARAM_SPEC_CHANNEL_ID(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_CHANNEL_ID))

typedef struct _PicmanParamSpecChannelID PicmanParamSpecChannelID;

struct _PicmanParamSpecChannelID
{
  PicmanParamSpecDrawableID parent_instance;
};

GType         picman_param_channel_id_get_type (void) G_GNUC_CONST;

GParamSpec  * picman_param_spec_channel_id     (const gchar  *name,
                                              const gchar  *nick,
                                              const gchar  *blurb,
                                              Picman         *picman,
                                              gboolean      none_ok,
                                              GParamFlags   flags);

PicmanChannel * picman_value_get_channel         (const GValue *value,
                                              Picman         *picman);
void          picman_value_set_channel         (GValue       *value,
                                              PicmanChannel  *channel);


/*
 * PICMAN_TYPE_LAYER_MASK_ID
 */

#define PICMAN_TYPE_LAYER_MASK_ID               (picman_layer_mask_id_get_type ())
#define PICMAN_VALUE_HOLDS_LAYER_MASK_ID(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                               PICMAN_TYPE_LAYER_MASK_ID))

GType   picman_layer_mask_id_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_LAYER_MASK_ID
 */

#define PICMAN_TYPE_PARAM_LAYER_MASK_ID           (picman_param_layer_mask_id_get_type ())
#define PICMAN_PARAM_SPEC_LAYER_MASK_ID(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_LAYER_MASK_ID, PicmanParamSpecLayerMaskID))
#define PICMAN_IS_PARAM_SPEC_LAYER_MASK_ID(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_LAYER_MASK_ID))

typedef struct _PicmanParamSpecLayerMaskID PicmanParamSpecLayerMaskID;

struct _PicmanParamSpecLayerMaskID
{
  PicmanParamSpecChannelID parent_instance;
};

GType           picman_param_layer_mask_id_get_type (void) G_GNUC_CONST;

GParamSpec    * picman_param_spec_layer_mask_id     (const gchar   *name,
                                                   const gchar   *nick,
                                                   const gchar   *blurb,
                                                   Picman          *picman,
                                                   gboolean       none_ok,
                                                   GParamFlags    flags);

PicmanLayerMask * picman_value_get_layer_mask         (const GValue  *value,
                                                   Picman          *picman);
void            picman_value_set_layer_mask         (GValue        *value,
                                                   PicmanLayerMask *layer_mask);


/*
 * PICMAN_TYPE_SELECTION_ID
 */

#define PICMAN_TYPE_SELECTION_ID               (picman_selection_id_get_type ())
#define PICMAN_VALUE_HOLDS_SELECTION_ID(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                              PICMAN_TYPE_SELECTION_ID))

GType   picman_selection_id_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_SELECTION_ID
 */

#define PICMAN_TYPE_PARAM_SELECTION_ID           (picman_param_selection_id_get_type ())
#define PICMAN_PARAM_SPEC_SELECTION_ID(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_SELECTION_ID, PicmanParamSpecSelectionID))
#define PICMAN_IS_PARAM_SPEC_SELECTION_ID(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_SELECTION_ID))

typedef struct _PicmanParamSpecSelectionID PicmanParamSpecSelectionID;

struct _PicmanParamSpecSelectionID
{
  PicmanParamSpecChannelID parent_instance;
};

GType           picman_param_selection_id_get_type (void) G_GNUC_CONST;

GParamSpec    * picman_param_spec_selection_id     (const gchar   *name,
                                                  const gchar   *nick,
                                                  const gchar   *blurb,
                                                  Picman          *picman,
                                                  gboolean       none_ok,
                                                  GParamFlags    flags);

PicmanSelection * picman_value_get_selection         (const GValue  *value,
                                                  Picman          *picman);
void            picman_value_set_selection         (GValue        *value,
                                                  PicmanSelection *selection);


/*
 * PICMAN_TYPE_VECTORS_ID
 */

#define PICMAN_TYPE_VECTORS_ID               (picman_vectors_id_get_type ())
#define PICMAN_VALUE_HOLDS_VECTORS_ID(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                            PICMAN_TYPE_VECTORS_ID))

GType   picman_vectors_id_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_VECTORS_ID
 */

#define PICMAN_TYPE_PARAM_VECTORS_ID           (picman_param_vectors_id_get_type ())
#define PICMAN_PARAM_SPEC_VECTORS_ID(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_VECTORS_ID, PicmanParamSpecVectorsID))
#define PICMAN_IS_PARAM_SPEC_VECTORS_ID(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_VECTORS_ID))

typedef struct _PicmanParamSpecVectorsID PicmanParamSpecVectorsID;

struct _PicmanParamSpecVectorsID
{
  PicmanParamSpecItemID parent_instance;
};

GType         picman_param_vectors_id_get_type (void) G_GNUC_CONST;

GParamSpec  * picman_param_spec_vectors_id     (const gchar  *name,
                                              const gchar  *nick,
                                              const gchar  *blurb,
                                              Picman         *picman,
                                              gboolean      none_ok,
                                              GParamFlags   flags);

PicmanVectors * picman_value_get_vectors         (const GValue *value,
                                              Picman         *picman);
void          picman_value_set_vectors         (GValue       *value,
                                              PicmanVectors  *vectors);


/*
 * PICMAN_TYPE_DISPLAY_ID
 */

#define PICMAN_TYPE_DISPLAY_ID               (picman_display_id_get_type ())
#define PICMAN_VALUE_HOLDS_DISPLAY_ID(value) (G_TYPE_CHECK_VALUE_TYPE ((value),\
                                            PICMAN_TYPE_DISPLAY_ID))

GType   picman_display_id_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_DISPLAY_ID
 */

#define PICMAN_TYPE_PARAM_DISPLAY_ID           (picman_param_display_id_get_type ())
#define PICMAN_PARAM_SPEC_DISPLAY_ID(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_DISPLAY_ID, PicmanParamSpecDisplayID))
#define PICMAN_IS_PARAM_SPEC_DISPLAY_ID(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_DISPLAY_ID))

typedef struct _PicmanParamSpecDisplayID PicmanParamSpecDisplayID;

struct _PicmanParamSpecDisplayID
{
  GParamSpecInt  parent_instance;

  Picman          *picman;
  gboolean       none_ok;
};

GType        picman_param_display_id_get_type (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_display_id     (const gchar  *name,
                                             const gchar  *nick,
                                             const gchar  *blurb,
                                             Picman         *picman,
                                             gboolean      none_ok,
                                             GParamFlags   flags);

PicmanObject * picman_value_get_display         (const GValue *value,
                                             Picman         *picman);
void         picman_value_set_display         (GValue       *value,
                                             PicmanObject   *display);


/*
 * PICMAN_TYPE_ARRAY
 */

typedef struct _PicmanArray PicmanArray;

struct _PicmanArray
{
  guint8   *data;
  gsize     length;
  gboolean  static_data;
};

PicmanArray * picman_array_new  (const guint8    *data,
                             gsize            length,
                             gboolean         static_data);
PicmanArray * picman_array_copy (const PicmanArray *array);
void        picman_array_free (PicmanArray       *array);

#define PICMAN_TYPE_ARRAY               (picman_array_get_type ())
#define PICMAN_VALUE_HOLDS_ARRAY(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_ARRAY))

GType   picman_array_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_ARRAY
 */

#define PICMAN_TYPE_PARAM_ARRAY           (picman_param_array_get_type ())
#define PICMAN_PARAM_SPEC_ARRAY(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_ARRAY, PicmanParamSpecArray))
#define PICMAN_IS_PARAM_SPEC_ARRAY(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_ARRAY))

typedef struct _PicmanParamSpecArray PicmanParamSpecArray;

struct _PicmanParamSpecArray
{
  GParamSpecBoxed parent_instance;
};

GType        picman_param_array_get_type (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_array     (const gchar  *name,
                                        const gchar  *nick,
                                        const gchar  *blurb,
                                        GParamFlags   flags);


/*
 * PICMAN_TYPE_INT8_ARRAY
 */

#define PICMAN_TYPE_INT8_ARRAY               (picman_int8_array_get_type ())
#define PICMAN_VALUE_HOLDS_INT8_ARRAY(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_INT8_ARRAY))

GType   picman_int8_array_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_INT8_ARRAY
 */

#define PICMAN_TYPE_PARAM_INT8_ARRAY           (picman_param_int8_array_get_type ())
#define PICMAN_PARAM_SPEC_INT8_ARRAY(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_INT8_ARRAY, PicmanParamSpecInt8Array))
#define PICMAN_IS_PARAM_SPEC_INT8_ARRAY(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_INT8_ARRAY))

typedef struct _PicmanParamSpecInt8Array PicmanParamSpecInt8Array;

struct _PicmanParamSpecInt8Array
{
  PicmanParamSpecArray parent_instance;
};

GType          picman_param_int8_array_get_type  (void) G_GNUC_CONST;

GParamSpec   * picman_param_spec_int8_array      (const gchar  *name,
                                                const gchar  *nick,
                                                const gchar  *blurb,
                                                GParamFlags   flags);

const guint8 * picman_value_get_int8array        (const GValue *value);
guint8       * picman_value_dup_int8array        (const GValue *value);
void           picman_value_set_int8array        (GValue       *value,
                                                const guint8 *array,
                                                gsize         length);
void           picman_value_set_static_int8array (GValue       *value,
                                                const guint8 *array,
                                                gsize         length);
void           picman_value_take_int8array       (GValue       *value,
                                                guint8       *array,
                                                gsize         length);


/*
 * PICMAN_TYPE_INT16_ARRAY
 */

#define PICMAN_TYPE_INT16_ARRAY               (picman_int16_array_get_type ())
#define PICMAN_VALUE_HOLDS_INT16_ARRAY(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_INT16_ARRAY))

GType   picman_int16_array_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_INT16_ARRAY
 */

#define PICMAN_TYPE_PARAM_INT16_ARRAY           (picman_param_int16_array_get_type ())
#define PICMAN_PARAM_SPEC_INT16_ARRAY(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_INT16_ARRAY, PicmanParamSpecInt16Array))
#define PICMAN_IS_PARAM_SPEC_INT16_ARRAY(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_INT16_ARRAY))

typedef struct _PicmanParamSpecInt16Array PicmanParamSpecInt16Array;

struct _PicmanParamSpecInt16Array
{
  PicmanParamSpecArray parent_instance;
};

GType          picman_param_int16_array_get_type  (void) G_GNUC_CONST;

GParamSpec   * picman_param_spec_int16_array      (const gchar  *name,
                                                 const gchar  *nick,
                                                 const gchar  *blurb,
                                                 GParamFlags   flags);

const gint16 * picman_value_get_int16array        (const GValue *value);
gint16       * picman_value_dup_int16array        (const GValue *value);
void           picman_value_set_int16array        (GValue       *value,
                                                 const gint16 *array,
                                                 gsize         length);
void           picman_value_set_static_int16array (GValue       *value,
                                                 const gint16 *array,
                                                 gsize         length);
void           picman_value_take_int16array       (GValue       *value,
                                                 gint16       *array,
                                                 gsize         length);


/*
 * PICMAN_TYPE_INT32_ARRAY
 */

#define PICMAN_TYPE_INT32_ARRAY               (picman_int32_array_get_type ())
#define PICMAN_VALUE_HOLDS_INT32_ARRAY(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_INT32_ARRAY))

GType   picman_int32_array_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_INT32_ARRAY
 */

#define PICMAN_TYPE_PARAM_INT32_ARRAY           (picman_param_int32_array_get_type ())
#define PICMAN_PARAM_SPEC_INT32_ARRAY(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_INT32_ARRAY, PicmanParamSpecInt32Array))
#define PICMAN_IS_PARAM_SPEC_INT32_ARRAY(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_INT32_ARRAY))

typedef struct _PicmanParamSpecInt32Array PicmanParamSpecInt32Array;

struct _PicmanParamSpecInt32Array
{
  PicmanParamSpecArray parent_instance;
};

GType          picman_param_int32_array_get_type  (void) G_GNUC_CONST;

GParamSpec   * picman_param_spec_int32_array      (const gchar  *name,
                                                 const gchar  *nick,
                                                 const gchar  *blurb,
                                                 GParamFlags   flags);

const gint32 * picman_value_get_int32array        (const GValue *value);
gint32       * picman_value_dup_int32array        (const GValue *value);
void           picman_value_set_int32array        (GValue       *value,
                                                 const gint32 *array,
                                                 gsize         length);
void           picman_value_set_static_int32array (GValue       *value,
                                                 const gint32 *array,
                                                 gsize         length);
void           picman_value_take_int32array       (GValue       *value,
                                                 gint32       *array,
                                                 gsize         length);


/*
 * PICMAN_TYPE_FLOAT_ARRAY
 */

#define PICMAN_TYPE_FLOAT_ARRAY               (picman_float_array_get_type ())
#define PICMAN_VALUE_HOLDS_FLOAT_ARRAY(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_FLOAT_ARRAY))

GType   picman_float_array_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_FLOAT_ARRAY
 */

#define PICMAN_TYPE_PARAM_FLOAT_ARRAY           (picman_param_float_array_get_type ())
#define PICMAN_PARAM_SPEC_FLOAT_ARRAY(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_FLOAT_ARRAY, PicmanParamSpecFloatArray))
#define PICMAN_IS_PARAM_SPEC_FLOAT_ARRAY(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_FLOAT_ARRAY))

typedef struct _PicmanParamSpecFloatArray PicmanParamSpecFloatArray;

struct _PicmanParamSpecFloatArray
{
  PicmanParamSpecArray parent_instance;
};

GType           picman_param_float_array_get_type  (void) G_GNUC_CONST;

GParamSpec    * picman_param_spec_float_array      (const gchar  *name,
                                                  const gchar  *nick,
                                                  const gchar  *blurb,
                                                  GParamFlags   flags);

const gdouble * picman_value_get_floatarray        (const GValue  *value);
gdouble       * picman_value_dup_floatarray        (const GValue  *value);
void            picman_value_set_floatarray        (GValue        *value,
                                                  const gdouble *array,
                                                  gsize         length);
void            picman_value_set_static_floatarray (GValue        *value,
                                                  const gdouble *array,
                                                  gsize         length);
void            picman_value_take_floatarray       (GValue        *value,
                                                  gdouble       *array,
                                                  gsize         length);


/*
 * PICMAN_TYPE_STRING_ARRAY
 */

PicmanArray * picman_string_array_new  (const gchar     **data,
                                    gsize             length,
                                    gboolean          static_data);
PicmanArray * picman_string_array_copy (const PicmanArray  *array);
void        picman_string_array_free (PicmanArray        *array);

#define PICMAN_TYPE_STRING_ARRAY               (picman_string_array_get_type ())
#define PICMAN_VALUE_HOLDS_STRING_ARRAY(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_STRING_ARRAY))

GType   picman_string_array_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_STRING_ARRAY
 */

#define PICMAN_TYPE_PARAM_STRING_ARRAY           (picman_param_string_array_get_type ())
#define PICMAN_PARAM_SPEC_STRING_ARRAY(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_STRING_ARRAY, PicmanParamSpecStringArray))
#define PICMAN_IS_PARAM_SPEC_STRING_ARRAY(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_STRING_ARRAY))

typedef struct _PicmanParamSpecStringArray PicmanParamSpecStringArray;

struct _PicmanParamSpecStringArray
{
  GParamSpecBoxed parent_instance;
};

GType          picman_param_string_array_get_type  (void) G_GNUC_CONST;

GParamSpec   * picman_param_spec_string_array      (const gchar  *name,
                                                  const gchar  *nick,
                                                  const gchar  *blurb,
                                                  GParamFlags   flags);

const gchar ** picman_value_get_stringarray        (const GValue *value);
gchar       ** picman_value_dup_stringarray        (const GValue *value);
void           picman_value_set_stringarray        (GValue       *value,
                                                  const gchar **array,
                                                  gsize         length);
void           picman_value_set_static_stringarray (GValue       *value,
                                                  const gchar **array,
                                                  gsize         length);
void           picman_value_take_stringarray       (GValue       *value,
                                                  gchar       **array,
                                                  gsize         length);


/*
 * PICMAN_TYPE_COLOR_ARRAY
 */

#define PICMAN_TYPE_COLOR_ARRAY               (picman_color_array_get_type ())
#define PICMAN_VALUE_HOLDS_COLOR_ARRAY(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_COLOR_ARRAY))

GType   picman_color_array_get_type           (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_COLOR_ARRAY
 */

#define PICMAN_TYPE_PARAM_COLOR_ARRAY           (picman_param_color_array_get_type ())
#define PICMAN_PARAM_SPEC_COLOR_ARRAY(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_COLOR_ARRAY, PicmanParamSpecColorArray))
#define PICMAN_IS_PARAM_SPEC_COLOR_ARRAY(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_COLOR_ARRAY))

typedef struct _PicmanParamSpecColorArray PicmanParamSpecColorArray;

struct _PicmanParamSpecColorArray
{
  GParamSpecBoxed parent_instance;
};

GType           picman_param_color_array_get_type  (void) G_GNUC_CONST;

GParamSpec    * picman_param_spec_color_array      (const gchar   *name,
                                                  const gchar   *nick,
                                                  const gchar   *blurb,
                                                  GParamFlags    flags);

const PicmanRGB * picman_value_get_colorarray        (const GValue  *value);
PicmanRGB       * picman_value_dup_colorarray        (const GValue  *value);
void            picman_value_set_colorarray        (GValue        *value,
                                                  const PicmanRGB *array,
                                                  gsize          length);
void            picman_value_set_static_colorarray (GValue        *value,
                                                  const PicmanRGB *array,
                                                  gsize          length);
void            picman_value_take_colorarray       (GValue        *value,
                                                  PicmanRGB       *array,
                                                  gsize          length);


#endif  /*  __PICMAN_PARAM_SPECS_H__  */
