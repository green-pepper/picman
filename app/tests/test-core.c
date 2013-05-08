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

#include <gegl.h>
#include <gtk/gtk.h>

#include "widgets/widgets-types.h"

#include "widgets/picmanuimanager.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanlayer.h"

#include "operations/picmanlevelsconfig.h"

#include "tests.h"

#include "picman-app-test-utils.h"


#define PICMAN_TEST_IMAGE_SIZE 100

#define ADD_IMAGE_TEST(function) \
  g_test_add ("/picman-core/" #function, \
              PicmanTestFixture, \
              picman, \
              picman_test_image_setup, \
              function, \
              picman_test_image_teardown);

#define ADD_TEST(function) \
  g_test_add ("/picman-core/" #function, \
              PicmanTestFixture, \
              picman, \
              NULL, \
              function, \
              NULL);


typedef struct
{
  PicmanImage *image;
} PicmanTestFixture;


static void picman_test_image_setup    (PicmanTestFixture *fixture,
                                      gconstpointer    data);
static void picman_test_image_teardown (PicmanTestFixture *fixture,
                                      gconstpointer    data);


/**
 * picman_test_image_setup:
 * @fixture:
 * @data:
 *
 * Test fixture setup for a single image.
 **/
static void
picman_test_image_setup (PicmanTestFixture *fixture,
                       gconstpointer    data)
{
  Picman *picman = PICMAN (data);

  fixture->image = picman_image_new (picman,
                                   PICMAN_TEST_IMAGE_SIZE,
                                   PICMAN_TEST_IMAGE_SIZE,
                                   PICMAN_RGB,
                                   PICMAN_PRECISION_FLOAT);
}

/**
 * picman_test_image_teardown:
 * @fixture:
 * @data:
 *
 * Test fixture teardown for a single image.
 **/
static void
picman_test_image_teardown (PicmanTestFixture *fixture,
                          gconstpointer    data)
{
  g_object_unref (fixture->image);
}

/**
 * rotate_non_overlapping:
 * @fixture:
 * @data:
 *
 * Super basic test that makes sure we can add a layer
 * and call picman_item_rotate with center at (0, -10)
 * without triggering a failed assertion .
 **/
static void
rotate_non_overlapping (PicmanTestFixture *fixture,
                        gconstpointer    data)
{
  Picman        *picman    = PICMAN (data);
  PicmanImage   *image   = fixture->image;
  PicmanLayer   *layer;
  PicmanContext *context = picman_context_new (picman, "Test", NULL /*template*/);
  gboolean     result;

  g_assert_cmpint (picman_image_get_n_layers (image), ==, 0);

  layer = picman_layer_new (image,
                          PICMAN_TEST_IMAGE_SIZE,
                          PICMAN_TEST_IMAGE_SIZE,
                          babl_format ("R'G'B'A u8"),
                          "Test Layer",
                          1.0,
                          PICMAN_NORMAL_MODE);

  g_assert_cmpint (PICMAN_IS_LAYER (layer), ==, TRUE);

  result = picman_image_add_layer (image,
                                 layer,
                                 PICMAN_IMAGE_ACTIVE_PARENT,
                                 0,
                                 FALSE);

  picman_item_rotate (PICMAN_ITEM (layer), context, PICMAN_ROTATE_90, 0., -10., TRUE);

  g_assert_cmpint (result, ==, TRUE);
  g_assert_cmpint (picman_image_get_n_layers (image), ==, 1);
  g_object_unref (context);
}

/**
 * add_layer:
 * @fixture:
 * @data:
 *
 * Super basic test that makes sure we can add a layer.
 **/
static void
add_layer (PicmanTestFixture *fixture,
           gconstpointer    data)
{
  PicmanImage *image = fixture->image;
  PicmanLayer *layer;
  gboolean   result;

  g_assert_cmpint (picman_image_get_n_layers (image), ==, 0);

  layer = picman_layer_new (image,
                          PICMAN_TEST_IMAGE_SIZE,
                          PICMAN_TEST_IMAGE_SIZE,
                          babl_format ("R'G'B'A u8"),
                          "Test Layer",
                          1.0,
                          PICMAN_NORMAL_MODE);

  g_assert_cmpint (PICMAN_IS_LAYER (layer), ==, TRUE);

  result = picman_image_add_layer (image,
                                 layer,
                                 PICMAN_IMAGE_ACTIVE_PARENT,
                                 0,
                                 FALSE);

  g_assert_cmpint (result, ==, TRUE);
  g_assert_cmpint (picman_image_get_n_layers (image), ==, 1);
}

/**
 * remove_layer:
 * @fixture:
 * @data:
 *
 * Super basic test that makes sure we can remove a layer.
 **/
static void
remove_layer (PicmanTestFixture *fixture,
              gconstpointer    data)
{
  PicmanImage *image = fixture->image;
  PicmanLayer *layer;
  gboolean   result;

  g_assert_cmpint (picman_image_get_n_layers (image), ==, 0);

  layer = picman_layer_new (image,
                          PICMAN_TEST_IMAGE_SIZE,
                          PICMAN_TEST_IMAGE_SIZE,
                          babl_format ("R'G'B'A u8"),
                          "Test Layer",
                          1.0,
                          PICMAN_NORMAL_MODE);

  g_assert_cmpint (PICMAN_IS_LAYER (layer), ==, TRUE);

  result = picman_image_add_layer (image,
                                 layer,
                                 PICMAN_IMAGE_ACTIVE_PARENT,
                                 0,
                                 FALSE);

  g_assert_cmpint (result, ==, TRUE);
  g_assert_cmpint (picman_image_get_n_layers (image), ==, 1);

  picman_image_remove_layer (image,
                           layer,
                           FALSE,
                           NULL);

  g_assert_cmpint (picman_image_get_n_layers (image), ==, 0);
}

/**
 * white_graypoint_in_red_levels:
 * @fixture:
 * @data:
 *
 * Makes sure the levels algorithm can handle when the graypoint is
 * white. It's easy to get a divide by zero problem when trying to
 * calculate what gamma will give a white graypoint.
 **/
static void
white_graypoint_in_red_levels (PicmanTestFixture *fixture,
                               gconstpointer    data)
{
  PicmanRGB              black   = { 0, 0, 0, 0 };
  PicmanRGB              gray    = { 1, 1, 1, 1 };
  PicmanRGB              white   = { 1, 1, 1, 1 };
  PicmanHistogramChannel channel = PICMAN_HISTOGRAM_RED;
  PicmanLevelsConfig    *config;

  config = g_object_new (PICMAN_TYPE_LEVELS_CONFIG, NULL);

  picman_levels_config_adjust_by_colors (config,
                                       channel,
                                       &black,
                                       &gray,
                                       &white);

  /* Make sure we didn't end up with an invalid gamma value */
  g_object_set (config,
                "gamma", config->gamma[channel],
                NULL);
}

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

  /* We share the same application instance across all tests */
  picman = picman_init_for_testing ();

  /* Add tests */
  ADD_IMAGE_TEST (add_layer);
  ADD_IMAGE_TEST (remove_layer);
  ADD_IMAGE_TEST (rotate_non_overlapping);
  ADD_TEST (white_graypoint_in_red_levels);

  /* Run the tests */
  result = g_test_run ();

  /* Don't write files to the source dir */
  picman_test_utils_set_picman2_directory ("PICMAN_TESTING_ABS_TOP_BUILDDIR",
                                       "app/tests/picmandir-output");

  /* Exit so we don't break script-fu plug-in wire */
  picman_exit (picman, TRUE);

  return result;
}
