/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 2009 Martin Nordholts
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

#include <string.h>

#include <glib/gstdio.h>

#include <gegl.h>

#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"

#include "widgets/widgets-types.h"

#include "widgets/picmanuimanager.h"

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmanchannel-select.h"
#include "core/picmandrawable.h"
#include "core/picmangrid.h"
#include "core/picmangrouplayer.h"
#include "core/picmanguide.h"
#include "core/picmanimage.h"
#include "core/picmanimage-grid.h"
#include "core/picmanimage-guides.h"
#include "core/picmanimage-sample-points.h"
#include "core/picmanlayer.h"
#include "core/picmansamplepoint.h"
#include "core/picmanselection.h"

#include "vectors/picmananchor.h"
#include "vectors/picmanbezierstroke.h"
#include "vectors/picmanvectors.h"

#include "file/file-open.h"
#include "file/file-procedure.h"
#include "file/file-save.h"

#include "plug-in/picmanpluginmanager.h"

#include "tests.h"

#include "picman-app-test-utils.h"


#define PICMAN_MAINIMAGE_WIDTH            100
#define PICMAN_MAINIMAGE_HEIGHT           90
#define PICMAN_MAINIMAGE_TYPE             PICMAN_RGB
#define PICMAN_MAINIMAGE_PRECISION        PICMAN_PRECISION_U8

#define PICMAN_MAINIMAGE_LAYER1_NAME      "layer1"
#define PICMAN_MAINIMAGE_LAYER1_WIDTH     50
#define PICMAN_MAINIMAGE_LAYER1_HEIGHT    51
#define PICMAN_MAINIMAGE_LAYER1_FORMAT    babl_format ("R'G'B'A u8")
#define PICMAN_MAINIMAGE_LAYER1_OPACITY   1.0
#define PICMAN_MAINIMAGE_LAYER1_MODE      PICMAN_NORMAL_MODE

#define PICMAN_MAINIMAGE_LAYER2_NAME      "layer2"
#define PICMAN_MAINIMAGE_LAYER2_WIDTH     25
#define PICMAN_MAINIMAGE_LAYER2_HEIGHT    251
#define PICMAN_MAINIMAGE_LAYER2_FORMAT    babl_format ("R'G'B' u8")
#define PICMAN_MAINIMAGE_LAYER2_OPACITY   0.0
#define PICMAN_MAINIMAGE_LAYER2_MODE      PICMAN_MULTIPLY_MODE

#define PICMAN_MAINIMAGE_GROUP1_NAME      "group1"

#define PICMAN_MAINIMAGE_LAYER3_NAME      "layer3"

#define PICMAN_MAINIMAGE_LAYER4_NAME      "layer4"

#define PICMAN_MAINIMAGE_GROUP2_NAME      "group2"

#define PICMAN_MAINIMAGE_LAYER5_NAME      "layer5"

#define PICMAN_MAINIMAGE_VGUIDE1_POS      42
#define PICMAN_MAINIMAGE_VGUIDE2_POS      82
#define PICMAN_MAINIMAGE_HGUIDE1_POS      3
#define PICMAN_MAINIMAGE_HGUIDE2_POS      4

#define PICMAN_MAINIMAGE_SAMPLEPOINT1_X   10
#define PICMAN_MAINIMAGE_SAMPLEPOINT1_Y   12
#define PICMAN_MAINIMAGE_SAMPLEPOINT2_X   41
#define PICMAN_MAINIMAGE_SAMPLEPOINT2_Y   49

#define PICMAN_MAINIMAGE_RESOLUTIONX      400
#define PICMAN_MAINIMAGE_RESOLUTIONY      410

#define PICMAN_MAINIMAGE_PARASITE_NAME    "test-parasite"
#define PICMAN_MAINIMAGE_PARASITE_DATA    "foo"
#define PICMAN_MAINIMAGE_PARASITE_SIZE    4                /* 'f' 'o' 'o' '\0' */

#define PICMAN_MAINIMAGE_COMMENT          "Created with code from "\
                                        "app/tests/test-xcf.c in the PICMAN "\
                                        "source tree, i.e. it was not created "\
                                        "manually and may thus look weird if "\
                                        "opened and inspected in PICMAN."

#define PICMAN_MAINIMAGE_UNIT             PICMAN_UNIT_PICA

#define PICMAN_MAINIMAGE_GRIDXSPACING     25.0
#define PICMAN_MAINIMAGE_GRIDYSPACING     27.0

#define PICMAN_MAINIMAGE_CHANNEL1_NAME    "channel1"
#define PICMAN_MAINIMAGE_CHANNEL1_WIDTH   PICMAN_MAINIMAGE_WIDTH
#define PICMAN_MAINIMAGE_CHANNEL1_HEIGHT  PICMAN_MAINIMAGE_HEIGHT
#define PICMAN_MAINIMAGE_CHANNEL1_COLOR   { 1.0, 0.0, 1.0, 1.0 }

#define PICMAN_MAINIMAGE_SELECTION_X      5
#define PICMAN_MAINIMAGE_SELECTION_Y      6
#define PICMAN_MAINIMAGE_SELECTION_W      7
#define PICMAN_MAINIMAGE_SELECTION_H      8

#define PICMAN_MAINIMAGE_VECTORS1_NAME    "vectors1"
#define PICMAN_MAINIMAGE_VECTORS1_COORDS  { { 11.0, 12.0, /* pad zeroes */ },\
                                          { 21.0, 22.0, /* pad zeroes */ },\
                                          { 31.0, 32.0, /* pad zeroes */ }, }

#define PICMAN_MAINIMAGE_VECTORS2_NAME    "vectors2"
#define PICMAN_MAINIMAGE_VECTORS2_COORDS  { { 911.0, 912.0, /* pad zeroes */ },\
                                          { 921.0, 922.0, /* pad zeroes */ },\
                                          { 931.0, 932.0, /* pad zeroes */ }, }

#define ADD_TEST(function) \
  g_test_add_data_func ("/picman-xcf/" #function, picman, function);


PicmanImage        * picman_test_load_image                        (Picman            *picman,
                                                                const gchar     *uri);
static void        picman_write_and_read_file                    (Picman            *picman,
                                                                gboolean         with_unusual_stuff,
                                                                gboolean         compat_paths,
                                                                gboolean         use_picman_2_8_features);
static PicmanImage * picman_create_mainimage                       (Picman            *picman,
                                                                gboolean         with_unusual_stuff,
                                                                gboolean         compat_paths,
                                                                gboolean         use_picman_2_8_features);
static void        picman_assert_mainimage                       (PicmanImage       *image,
                                                                gboolean         with_unusual_stuff,
                                                                gboolean         compat_paths,
                                                                gboolean         use_picman_2_8_features);


/**
 * write_and_read_picman_2_6_format:
 * @data:
 *
 * Do a write and read test on a file that could as well be
 * constructed with PICMAN 2.6.
 **/
static void
write_and_read_picman_2_6_format (gconstpointer data)
{
  Picman *picman = PICMAN (data);

  picman_write_and_read_file (picman,
                            FALSE /*with_unusual_stuff*/,
                            FALSE /*compat_paths*/,
                            FALSE /*use_picman_2_8_features*/);
}

/**
 * write_and_read_picman_2_6_format_unusual:
 * @data:
 *
 * Do a write and read test on a file that could as well be
 * constructed with PICMAN 2.6, and make it unusual, like compatible
 * vectors and with a floating selection.
 **/
static void
write_and_read_picman_2_6_format_unusual (gconstpointer data)
{
  Picman *picman = PICMAN (data);

  picman_write_and_read_file (picman,
                            TRUE /*with_unusual_stuff*/,
                            TRUE /*compat_paths*/,
                            FALSE /*use_picman_2_8_features*/);
}

/**
 * load_picman_2_6_file:
 * @data:
 *
 * Loads a file created with PICMAN 2.6 and makes sure it loaded as
 * expected.
 **/
static void
load_picman_2_6_file (gconstpointer data)
{
  Picman      *picman  = PICMAN (data);
  PicmanImage *image = NULL;
  gchar     *uri   = NULL;

  uri = g_build_filename (g_getenv ("PICMAN_TESTING_ABS_TOP_SRCDIR"),
                          "app/tests/files/picman-2-6-file.xcf",
                          NULL);

  image = picman_test_load_image (picman, uri);

  /* The image file was constructed by running
   * picman_write_and_read_file (FALSE, FALSE) in PICMAN 2.6 by
   * copy-pasting the code to PICMAN 2.6 and adapting it to changes in
   * the core API, so we can use picman_assert_mainimage() to make sure
   * the file was loaded successfully.
   */
  picman_assert_mainimage (image,
                         FALSE /*with_unusual_stuff*/,
                         FALSE /*compat_paths*/,
                         FALSE /*use_picman_2_8_features*/);
}

/**
 * write_and_read_picman_2_8_format:
 * @data:
 *
 * Writes an XCF file that uses PICMAN 2.8 features such as layer
 * groups, then reads the file and make sure no relevant information
 * was lost.
 **/
static void
write_and_read_picman_2_8_format (gconstpointer data)
{
  Picman *picman = PICMAN (data);

  picman_write_and_read_file (picman,
                            FALSE /*with_unusual_stuff*/,
                            FALSE /*compat_paths*/,
                            TRUE /*use_picman_2_8_features*/);
}

PicmanImage *
picman_test_load_image (Picman        *picman,
                      const gchar *uri)
{
  PicmanPlugInProcedure *proc     = NULL;
  PicmanImage           *image    = NULL;
  PicmanPDBStatusType    not_used = 0;

  proc = file_procedure_find (picman->plug_in_manager->load_procs,
                              uri,
                              NULL /*error*/);
  image = file_open_image (picman,
                           picman_get_user_context (picman),
                           NULL /*progress*/,
                           uri,
                           "irrelevant" /*entered_filename*/,
                           FALSE /*as_new*/,
                           proc,
                           PICMAN_RUN_NONINTERACTIVE,
                           &not_used /*status*/,
                           NULL /*mime_type*/,
                           NULL /*error*/);

  return image;
}

/**
 * picman_write_and_read_file:
 *
 * Constructs the main test image and asserts its state, writes it to
 * a file, reads the image from the file, and asserts the state of the
 * loaded file. The function takes various parameters so the same
 * function can be used for different formats.
 **/
static void
picman_write_and_read_file (Picman     *picman,
                          gboolean  with_unusual_stuff,
                          gboolean  compat_paths,
                          gboolean  use_picman_2_8_features)
{
  PicmanImage           *image        = NULL;
  PicmanImage           *loaded_image = NULL;
  PicmanPlugInProcedure *proc         = NULL;
  gchar               *uri          = NULL;

  /* Create the image */
  image = picman_create_mainimage (picman,
                                 with_unusual_stuff,
                                 compat_paths,
                                 use_picman_2_8_features);

  /* Assert valid state */
  picman_assert_mainimage (image,
                         with_unusual_stuff,
                         compat_paths,
                         use_picman_2_8_features);

  /* Write to file */
  uri  = g_build_filename (g_get_tmp_dir (), "picman-test.xcf", NULL);
  proc = file_procedure_find (image->picman->plug_in_manager->save_procs,
                              uri,
                              NULL /*error*/);
  file_save (picman,
             image,
             NULL /*progress*/,
             uri,
             proc,
             PICMAN_RUN_NONINTERACTIVE,
             FALSE /*change_saved_state*/,
             FALSE /*export_backward*/,
             FALSE /*export_forward*/,
             NULL /*error*/);

  /* Load from file */
  loaded_image = picman_test_load_image (image->picman, uri);

  /* Assert on the loaded file. If success, it means that there is no
   * significant information loss when we wrote the image to a file
   * and loaded it again
   */
  picman_assert_mainimage (loaded_image,
                         with_unusual_stuff,
                         compat_paths,
                         use_picman_2_8_features);

  g_unlink (uri);
  g_free (uri);
}

/**
 * picman_create_mainimage:
 *
 * Creates the main test image, i.e. the image that we use for most of
 * our XCF testing purposes.
 *
 * Returns: The #PicmanImage
 **/
static PicmanImage *
picman_create_mainimage (Picman     *picman,
                       gboolean  with_unusual_stuff,
                       gboolean  compat_paths,
                       gboolean  use_picman_2_8_features)
{
  PicmanImage     *image             = NULL;
  PicmanLayer     *layer             = NULL;
  PicmanParasite  *parasite          = NULL;
  PicmanGrid      *grid              = NULL;
  PicmanChannel   *channel           = NULL;
  PicmanRGB        channel_color     = PICMAN_MAINIMAGE_CHANNEL1_COLOR;
  PicmanChannel   *selection         = NULL;
  PicmanVectors   *vectors           = NULL;
  PicmanCoords     vectors1_coords[] = PICMAN_MAINIMAGE_VECTORS1_COORDS;
  PicmanCoords     vectors2_coords[] = PICMAN_MAINIMAGE_VECTORS2_COORDS;
  PicmanStroke    *stroke            = NULL;
  PicmanLayerMask *layer_mask        = NULL;

  /* Image size and type */
  image = picman_image_new (picman,
                          PICMAN_MAINIMAGE_WIDTH,
                          PICMAN_MAINIMAGE_HEIGHT,
                          PICMAN_MAINIMAGE_TYPE,
                          PICMAN_MAINIMAGE_PRECISION);

  /* Layers */
  layer = picman_layer_new (image,
                          PICMAN_MAINIMAGE_LAYER1_WIDTH,
                          PICMAN_MAINIMAGE_LAYER1_HEIGHT,
                          PICMAN_MAINIMAGE_LAYER1_FORMAT,
                          PICMAN_MAINIMAGE_LAYER1_NAME,
                          PICMAN_MAINIMAGE_LAYER1_OPACITY,
                          PICMAN_MAINIMAGE_LAYER1_MODE);
  picman_image_add_layer (image,
                        layer,
                        NULL,
                        0,
                        FALSE/*push_undo*/);
  layer = picman_layer_new (image,
                          PICMAN_MAINIMAGE_LAYER2_WIDTH,
                          PICMAN_MAINIMAGE_LAYER2_HEIGHT,
                          PICMAN_MAINIMAGE_LAYER2_FORMAT,
                          PICMAN_MAINIMAGE_LAYER2_NAME,
                          PICMAN_MAINIMAGE_LAYER2_OPACITY,
                          PICMAN_MAINIMAGE_LAYER2_MODE);
  picman_image_add_layer (image,
                        layer,
                        NULL,
                        0,
                        FALSE /*push_undo*/);

  /* Layer mask */
  layer_mask = picman_layer_create_mask (layer,
                                       PICMAN_ADD_BLACK_MASK,
                                       NULL /*channel*/);
  picman_layer_add_mask (layer,
                       layer_mask,
                       FALSE /*push_undo*/,
                       NULL /*error*/);

  /* Image compression type
   *
   * We don't do any explicit test, only implicit when we read tile
   * data in other tests
   */

  /* Guides, note we add them in reversed order */
  picman_image_add_hguide (image,
                         PICMAN_MAINIMAGE_HGUIDE2_POS,
                         FALSE /*push_undo*/);
  picman_image_add_hguide (image,
                         PICMAN_MAINIMAGE_HGUIDE1_POS,
                         FALSE /*push_undo*/);
  picman_image_add_vguide (image,
                         PICMAN_MAINIMAGE_VGUIDE2_POS,
                         FALSE /*push_undo*/);
  picman_image_add_vguide (image,
                         PICMAN_MAINIMAGE_VGUIDE1_POS,
                         FALSE /*push_undo*/);


  /* Sample points */
  picman_image_add_sample_point_at_pos (image,
                                      PICMAN_MAINIMAGE_SAMPLEPOINT1_X,
                                      PICMAN_MAINIMAGE_SAMPLEPOINT1_Y,
                                      FALSE /*push_undo*/);
  picman_image_add_sample_point_at_pos (image,
                                      PICMAN_MAINIMAGE_SAMPLEPOINT2_X,
                                      PICMAN_MAINIMAGE_SAMPLEPOINT2_Y,
                                      FALSE /*push_undo*/);

  /* Tatto
   * We don't bother testing this, not yet at least
   */

  /* Resolution */
  picman_image_set_resolution (image,
                             PICMAN_MAINIMAGE_RESOLUTIONX,
                             PICMAN_MAINIMAGE_RESOLUTIONY);


  /* Parasites */
  parasite = picman_parasite_new (PICMAN_MAINIMAGE_PARASITE_NAME,
                                PICMAN_PARASITE_PERSISTENT,
                                PICMAN_MAINIMAGE_PARASITE_SIZE,
                                PICMAN_MAINIMAGE_PARASITE_DATA);
  picman_image_parasite_attach (image,
                              parasite);
  picman_parasite_free (parasite);
  parasite = picman_parasite_new ("picman-comment",
                                PICMAN_PARASITE_PERSISTENT,
                                strlen (PICMAN_MAINIMAGE_COMMENT) + 1,
                                PICMAN_MAINIMAGE_COMMENT);
  picman_image_parasite_attach (image, parasite);
  picman_parasite_free (parasite);


  /* Unit */
  picman_image_set_unit (image,
                       PICMAN_MAINIMAGE_UNIT);

  /* Grid */
  grid = g_object_new (PICMAN_TYPE_GRID,
                       "xspacing", PICMAN_MAINIMAGE_GRIDXSPACING,
                       "yspacing", PICMAN_MAINIMAGE_GRIDYSPACING,
                       NULL);
  picman_image_set_grid (image,
                       grid,
                       FALSE /*push_undo*/);
  g_object_unref (grid);

  /* Channel */
  channel = picman_channel_new (image,
                              PICMAN_MAINIMAGE_CHANNEL1_WIDTH,
                              PICMAN_MAINIMAGE_CHANNEL1_HEIGHT,
                              PICMAN_MAINIMAGE_CHANNEL1_NAME,
                              &channel_color);
  picman_image_add_channel (image,
                          channel,
                          NULL,
                          -1,
                          FALSE /*push_undo*/);

  /* Selection */
  selection = picman_image_get_mask (image);
  picman_channel_select_rectangle (selection,
                                 PICMAN_MAINIMAGE_SELECTION_X,
                                 PICMAN_MAINIMAGE_SELECTION_Y,
                                 PICMAN_MAINIMAGE_SELECTION_W,
                                 PICMAN_MAINIMAGE_SELECTION_H,
                                 PICMAN_CHANNEL_OP_REPLACE,
                                 FALSE /*feather*/,
                                 0.0 /*feather_radius_x*/,
                                 0.0 /*feather_radius_y*/,
                                 FALSE /*push_undo*/);

  /* Vectors 1 */
  vectors = picman_vectors_new (image,
                              PICMAN_MAINIMAGE_VECTORS1_NAME);
  /* The XCF file can save vectors in two kind of ways, one old way
   * and a new way. Parameterize the way so we can test both variants,
   * i.e. picman_vectors_compat_is_compatible() must return both TRUE
   * and FALSE.
   */
  if (! compat_paths)
    {
      picman_item_set_visible (PICMAN_ITEM (vectors),
                             TRUE,
                             FALSE /*push_undo*/);
    }
  /* TODO: Add test for non-closed stroke. The order of the anchor
   * points changes for open strokes, so it's boring to test
   */
  stroke = picman_bezier_stroke_new_from_coords (vectors1_coords,
                                               G_N_ELEMENTS (vectors1_coords),
                                               TRUE /*closed*/);
  picman_vectors_stroke_add (vectors, stroke);
  picman_image_add_vectors (image,
                          vectors,
                          NULL /*parent*/,
                          -1 /*position*/,
                          FALSE /*push_undo*/);

  /* Vectors 2 */
  vectors = picman_vectors_new (image,
                              PICMAN_MAINIMAGE_VECTORS2_NAME);

  stroke = picman_bezier_stroke_new_from_coords (vectors2_coords,
                                               G_N_ELEMENTS (vectors2_coords),
                                               TRUE /*closed*/);
  picman_vectors_stroke_add (vectors, stroke);
  picman_image_add_vectors (image,
                          vectors,
                          NULL /*parent*/,
                          -1 /*position*/,
                          FALSE /*push_undo*/);

  /* Some of these things are pretty unusual, parameterize the
   * inclusion of this in the written file so we can do our test both
   * with and without
   */
  if (with_unusual_stuff)
    {
      /* Floating selection */
      picman_selection_float (PICMAN_SELECTION (picman_image_get_mask (image)),
                            picman_image_get_active_drawable (image),
                            picman_get_user_context (picman),
                            TRUE /*cut_image*/,
                            0 /*off_x*/,
                            0 /*off_y*/,
                            NULL /*error*/);
    }

  /* Adds stuff like layer groups */
  if (use_picman_2_8_features)
    {
      PicmanLayer *parent;

      /* Add a layer group and some layers:
       *
       *  group1
       *    layer3
       *    layer4
       *    group2
       *      layer5
       */

      /* group1 */
      layer = picman_group_layer_new (image);
      picman_object_set_name (PICMAN_OBJECT (layer), PICMAN_MAINIMAGE_GROUP1_NAME);
      picman_image_add_layer (image,
                            layer,
                            NULL /*parent*/,
                            -1 /*position*/,
                            FALSE /*push_undo*/);
      parent = layer;

      /* layer3 */
      layer = picman_layer_new (image,
                              PICMAN_MAINIMAGE_LAYER1_WIDTH,
                              PICMAN_MAINIMAGE_LAYER1_HEIGHT,
                              PICMAN_MAINIMAGE_LAYER1_FORMAT,
                              PICMAN_MAINIMAGE_LAYER3_NAME,
                              PICMAN_MAINIMAGE_LAYER1_OPACITY,
                              PICMAN_MAINIMAGE_LAYER1_MODE);
      picman_image_add_layer (image,
                            layer,
                            parent,
                            -1 /*position*/,
                            FALSE /*push_undo*/);

      /* layer4 */
      layer = picman_layer_new (image,
                              PICMAN_MAINIMAGE_LAYER1_WIDTH,
                              PICMAN_MAINIMAGE_LAYER1_HEIGHT,
                              PICMAN_MAINIMAGE_LAYER1_FORMAT,
                              PICMAN_MAINIMAGE_LAYER4_NAME,
                              PICMAN_MAINIMAGE_LAYER1_OPACITY,
                              PICMAN_MAINIMAGE_LAYER1_MODE);
      picman_image_add_layer (image,
                            layer,
                            parent,
                            -1 /*position*/,
                            FALSE /*push_undo*/);

      /* group2 */
      layer = picman_group_layer_new (image);
      picman_object_set_name (PICMAN_OBJECT (layer), PICMAN_MAINIMAGE_GROUP2_NAME);
      picman_image_add_layer (image,
                            layer,
                            parent,
                            -1 /*position*/,
                            FALSE /*push_undo*/);
      parent = layer;

      /* layer5 */
      layer = picman_layer_new (image,
                              PICMAN_MAINIMAGE_LAYER1_WIDTH,
                              PICMAN_MAINIMAGE_LAYER1_HEIGHT,
                              PICMAN_MAINIMAGE_LAYER1_FORMAT,
                              PICMAN_MAINIMAGE_LAYER5_NAME,
                              PICMAN_MAINIMAGE_LAYER1_OPACITY,
                              PICMAN_MAINIMAGE_LAYER1_MODE);
      picman_image_add_layer (image,
                            layer,
                            parent,
                            -1 /*position*/,
                            FALSE /*push_undo*/);
    }

  /* Todo, should be tested somehow:
   *
   * - Color maps
   * - Custom user units
   * - Text layers
   * - Layer parasites
   * - Channel parasites
   * - Different tile compression methods
   */

  return image;
}

static void
picman_assert_vectors (PicmanImage   *image,
                     const gchar *name,
                     PicmanCoords   coords[],
                     gsize        coords_size,
                     gboolean     visible)
{
  PicmanVectors *vectors        = NULL;
  PicmanStroke  *stroke         = NULL;
  GArray      *control_points = NULL;
  gboolean     closed         = FALSE;
  gint         i              = 0;

  vectors = picman_image_get_vectors_by_name (image, name);
  stroke = picman_vectors_stroke_get_next (vectors, NULL);
  g_assert (stroke != NULL);
  control_points = picman_stroke_control_points_get (stroke,
                                                   &closed);
  g_assert (closed);
  g_assert_cmpint (control_points->len,
                   ==,
                   coords_size);
  for (i = 0; i < control_points->len; i++)
    {
      g_assert_cmpint (coords[i].x,
                       ==,
                       g_array_index (control_points,
                                      PicmanAnchor,
                                      i).position.x);
      g_assert_cmpint (coords[i].y,
                       ==,
                       g_array_index (control_points,
                                      PicmanAnchor,
                                      i).position.y);
    }

  g_assert (picman_item_get_visible (PICMAN_ITEM (vectors)) ? TRUE : FALSE ==
            visible ? TRUE : FALSE);
}

/**
 * picman_assert_mainimage:
 * @image:
 *
 * Verifies that the passed #PicmanImage contains all the information
 * that was put in it by picman_create_mainimage().
 **/
static void
picman_assert_mainimage (PicmanImage *image,
                       gboolean   with_unusual_stuff,
                       gboolean   compat_paths,
                       gboolean   use_picman_2_8_features)
{
  const PicmanParasite *parasite               = NULL;
  PicmanLayer          *layer                  = NULL;
  GList              *iter                   = NULL;
  PicmanGuide          *guide                  = NULL;
  PicmanSamplePoint    *sample_point           = NULL;
  gdouble             xres                   = 0.0;
  gdouble             yres                   = 0.0;
  PicmanGrid           *grid                   = NULL;
  gdouble             xspacing               = 0.0;
  gdouble             yspacing               = 0.0;
  PicmanChannel        *channel                = NULL;
  PicmanRGB             expected_channel_color = PICMAN_MAINIMAGE_CHANNEL1_COLOR;
  PicmanRGB             actual_channel_color   = { 0, };
  PicmanChannel        *selection              = NULL;
  gint                x1                     = -1;
  gint                y1                     = -1;
  gint                x2                     = -1;
  gint                y2                     = -1;
  gint                w                      = -1;
  gint                h                      = -1;
  PicmanCoords          vectors1_coords[]      = PICMAN_MAINIMAGE_VECTORS1_COORDS;
  PicmanCoords          vectors2_coords[]      = PICMAN_MAINIMAGE_VECTORS2_COORDS;

  /* Image size and type */
  g_assert_cmpint (picman_image_get_width (image),
                   ==,
                   PICMAN_MAINIMAGE_WIDTH);
  g_assert_cmpint (picman_image_get_height (image),
                   ==,
                   PICMAN_MAINIMAGE_HEIGHT);
  g_assert_cmpint (picman_image_get_base_type (image),
                   ==,
                   PICMAN_MAINIMAGE_TYPE);

  /* Layers */
  layer = picman_image_get_layer_by_name (image,
                                        PICMAN_MAINIMAGE_LAYER1_NAME);
  g_assert_cmpint (picman_item_get_width (PICMAN_ITEM (layer)),
                   ==,
                   PICMAN_MAINIMAGE_LAYER1_WIDTH);
  g_assert_cmpint (picman_item_get_height (PICMAN_ITEM (layer)),
                   ==,
                   PICMAN_MAINIMAGE_LAYER1_HEIGHT);
  g_assert_cmpstr (babl_get_name (picman_drawable_get_format (PICMAN_DRAWABLE (layer))),
                   ==,
                   babl_get_name (PICMAN_MAINIMAGE_LAYER1_FORMAT));
  g_assert_cmpstr (picman_object_get_name (PICMAN_DRAWABLE (layer)),
                   ==,
                   PICMAN_MAINIMAGE_LAYER1_NAME);
  g_assert_cmpfloat (picman_layer_get_opacity (layer),
                     ==,
                     PICMAN_MAINIMAGE_LAYER1_OPACITY);
  g_assert_cmpint (picman_layer_get_mode (layer),
                   ==,
                   PICMAN_MAINIMAGE_LAYER1_MODE);
  layer = picman_image_get_layer_by_name (image,
                                        PICMAN_MAINIMAGE_LAYER2_NAME);
  g_assert_cmpint (picman_item_get_width (PICMAN_ITEM (layer)),
                   ==,
                   PICMAN_MAINIMAGE_LAYER2_WIDTH);
  g_assert_cmpint (picman_item_get_height (PICMAN_ITEM (layer)),
                   ==,
                   PICMAN_MAINIMAGE_LAYER2_HEIGHT);
  g_assert_cmpstr (babl_get_name (picman_drawable_get_format (PICMAN_DRAWABLE (layer))),
                   ==,
                   babl_get_name (PICMAN_MAINIMAGE_LAYER2_FORMAT));
  g_assert_cmpstr (picman_object_get_name (PICMAN_DRAWABLE (layer)),
                   ==,
                   PICMAN_MAINIMAGE_LAYER2_NAME);
  g_assert_cmpfloat (picman_layer_get_opacity (layer),
                     ==,
                     PICMAN_MAINIMAGE_LAYER2_OPACITY);
  g_assert_cmpint (picman_layer_get_mode (layer),
                   ==,
                   PICMAN_MAINIMAGE_LAYER2_MODE);

  /* Guides, note that we rely on internal ordering */
  iter = picman_image_get_guides (image);
  g_assert (iter != NULL);
  guide = PICMAN_GUIDE (iter->data);
  g_assert_cmpint (picman_guide_get_position (guide),
                   ==,
                   PICMAN_MAINIMAGE_VGUIDE1_POS);
  iter = g_list_next (iter);
  g_assert (iter != NULL);
  guide = PICMAN_GUIDE (iter->data);
  g_assert_cmpint (picman_guide_get_position (guide),
                   ==,
                   PICMAN_MAINIMAGE_VGUIDE2_POS);
  iter = g_list_next (iter);
  g_assert (iter != NULL);
  guide = PICMAN_GUIDE (iter->data);
  g_assert_cmpint (picman_guide_get_position (guide),
                   ==,
                   PICMAN_MAINIMAGE_HGUIDE1_POS);
  iter = g_list_next (iter);
  g_assert (iter != NULL);
  guide = PICMAN_GUIDE (iter->data);
  g_assert_cmpint (picman_guide_get_position (guide),
                   ==,
                   PICMAN_MAINIMAGE_HGUIDE2_POS);
  iter = g_list_next (iter);
  g_assert (iter == NULL);

  /* Sample points, we rely on the same ordering as when we added
   * them, although this ordering is not a necessaity
   */
  iter = picman_image_get_sample_points (image);
  g_assert (iter != NULL);
  sample_point = (PicmanSamplePoint *) iter->data;
  g_assert_cmpint (sample_point->x,
                   ==,
                   PICMAN_MAINIMAGE_SAMPLEPOINT1_X);
  g_assert_cmpint (sample_point->y,
                   ==,
                   PICMAN_MAINIMAGE_SAMPLEPOINT1_Y);
  iter = g_list_next (iter);
  g_assert (iter != NULL);
  sample_point = (PicmanSamplePoint *) iter->data;
  g_assert_cmpint (sample_point->x,
                   ==,
                   PICMAN_MAINIMAGE_SAMPLEPOINT2_X);
  g_assert_cmpint (sample_point->y,
                   ==,
                   PICMAN_MAINIMAGE_SAMPLEPOINT2_Y);
  iter = g_list_next (iter);
  g_assert (iter == NULL);

  /* Resolution */
  picman_image_get_resolution (image, &xres, &yres);
  g_assert_cmpint (xres,
                   ==,
                   PICMAN_MAINIMAGE_RESOLUTIONX);
  g_assert_cmpint (yres,
                   ==,
                   PICMAN_MAINIMAGE_RESOLUTIONY);

  /* Parasites */
  parasite = picman_image_parasite_find (image,
                                       PICMAN_MAINIMAGE_PARASITE_NAME);
  g_assert_cmpint (picman_parasite_data_size (parasite),
                   ==,
                   PICMAN_MAINIMAGE_PARASITE_SIZE);
  g_assert_cmpstr (picman_parasite_data (parasite),
                   ==,
                   PICMAN_MAINIMAGE_PARASITE_DATA);
  parasite = picman_image_parasite_find (image,
                                       "picman-comment");
  g_assert_cmpint (picman_parasite_data_size (parasite),
                   ==,
                   strlen (PICMAN_MAINIMAGE_COMMENT) + 1);
  g_assert_cmpstr (picman_parasite_data (parasite),
                   ==,
                   PICMAN_MAINIMAGE_COMMENT);

  /* Unit */
  g_assert_cmpint (picman_image_get_unit (image),
                   ==,
                   PICMAN_MAINIMAGE_UNIT);

  /* Grid */
  grid = picman_image_get_grid (image);
  g_object_get (grid,
                "xspacing", &xspacing,
                "yspacing", &yspacing,
                NULL);
  g_assert_cmpint (xspacing,
                   ==,
                   PICMAN_MAINIMAGE_GRIDXSPACING);
  g_assert_cmpint (yspacing,
                   ==,
                   PICMAN_MAINIMAGE_GRIDYSPACING);


  /* Channel */
  channel = picman_image_get_channel_by_name (image,
                                            PICMAN_MAINIMAGE_CHANNEL1_NAME);
  picman_channel_get_color (channel, &actual_channel_color);
  g_assert_cmpint (picman_item_get_width (PICMAN_ITEM (channel)),
                   ==,
                   PICMAN_MAINIMAGE_CHANNEL1_WIDTH);
  g_assert_cmpint (picman_item_get_height (PICMAN_ITEM (channel)),
                   ==,
                   PICMAN_MAINIMAGE_CHANNEL1_HEIGHT);
  g_assert (memcmp (&expected_channel_color,
                    &actual_channel_color,
                    sizeof (PicmanRGB)) == 0);

  /* Selection, if the image contains unusual stuff it contains a
   * floating select, and when floating a selection, the selection
   * mask is cleared, so don't test for the presence of the selection
   * mask in that case
   */
  if (! with_unusual_stuff)
    {
      selection = picman_image_get_mask (image);
      picman_channel_bounds (selection, &x1, &y1, &x2, &y2);
      w = x2 - x1;
      h = y2 - y1;
      g_assert_cmpint (x1,
                       ==,
                       PICMAN_MAINIMAGE_SELECTION_X);
      g_assert_cmpint (y1,
                       ==,
                       PICMAN_MAINIMAGE_SELECTION_Y);
      g_assert_cmpint (w,
                       ==,
                       PICMAN_MAINIMAGE_SELECTION_W);
      g_assert_cmpint (h,
                       ==,
                       PICMAN_MAINIMAGE_SELECTION_H);
    }

  /* Vectors 1 */
  picman_assert_vectors (image,
                       PICMAN_MAINIMAGE_VECTORS1_NAME,
                       vectors1_coords,
                       G_N_ELEMENTS (vectors1_coords),
                       ! compat_paths /*visible*/);

  /* Vectors 2 (always visible FALSE) */
  picman_assert_vectors (image,
                       PICMAN_MAINIMAGE_VECTORS2_NAME,
                       vectors2_coords,
                       G_N_ELEMENTS (vectors2_coords),
                       FALSE /*visible*/);

  if (with_unusual_stuff)
    g_assert (picman_image_get_floating_selection (image) != NULL);
  else /* if (! with_unusual_stuff) */
    g_assert (picman_image_get_floating_selection (image) == NULL);

  if (use_picman_2_8_features)
    {
      /* Only verify the parent relationships, the layer attributes
       * are tested above
       */
      PicmanItem *group1 = PICMAN_ITEM (picman_image_get_layer_by_name (image, PICMAN_MAINIMAGE_GROUP1_NAME));
      PicmanItem *layer3 = PICMAN_ITEM (picman_image_get_layer_by_name (image, PICMAN_MAINIMAGE_LAYER3_NAME));
      PicmanItem *layer4 = PICMAN_ITEM (picman_image_get_layer_by_name (image, PICMAN_MAINIMAGE_LAYER4_NAME));
      PicmanItem *group2 = PICMAN_ITEM (picman_image_get_layer_by_name (image, PICMAN_MAINIMAGE_GROUP2_NAME));
      PicmanItem *layer5 = PICMAN_ITEM (picman_image_get_layer_by_name (image, PICMAN_MAINIMAGE_LAYER5_NAME));

      g_assert (picman_item_get_parent (group1) == NULL);
      g_assert (picman_item_get_parent (layer3) == group1);
      g_assert (picman_item_get_parent (layer4) == group1);
      g_assert (picman_item_get_parent (group2) == group1);
      g_assert (picman_item_get_parent (layer5) == group2);
    }
}


/**
 * main:
 * @argc:
 * @argv:
 *
 * These tests intend to
 *
 *  - Make sure that we are backwards compatible with files created by
 *    older version of PICMAN, i.e. that we can load files from earlier
 *    version of PICMAN
 *
 *  - Make sure that the information put into a #PicmanImage is not lost
 *    when the #PicmanImage is written to a file and then read again
 **/
int
main (int    argc,
      char **argv)
{
  Picman *picman;
  int   result;

  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  picman_test_utils_set_picman2_directory ("PICMAN_TESTING_ABS_TOP_SRCDIR",
                                       "app/tests/picmandir");

  /* We share the same application instance across all tests. We need
   * the GUI variant for the file procs
   */
  picman = picman_init_for_testing ();

  /* Add tests */
  ADD_TEST (write_and_read_picman_2_6_format);
  ADD_TEST (write_and_read_picman_2_6_format_unusual);
  ADD_TEST (load_picman_2_6_file);
  ADD_TEST (write_and_read_picman_2_8_format);

  /* Don't write files to the source dir */
  picman_test_utils_set_picman2_directory ("PICMAN_TESTING_ABS_TOP_BUILDDIR",
                                       "app/tests/picmandir-output");

  /* Run the tests */
  result = g_test_run ();

  /* Exit so we don't break script-fu plug-in wire */
  picman_exit (picman, TRUE);

  return result;
}
