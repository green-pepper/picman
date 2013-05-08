/* unit tests for the color parsing routines in picmanrgb-parse.c
 */

#include "config.h"

#include <stdlib.h>

#include <babl/babl.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <glib-object.h>
#include <cairo.h>

#include "picmancolor.h"


#define DBL(c) ((gdouble)(c) / 255.0)


typedef struct
{
  const gchar   *str;
  gboolean       alpha;
  gboolean       fail;
  const gdouble  r;
  const gdouble  g;
  const gdouble  b;
  const gdouble  a;
} ColorSample;

static const ColorSample samples[] =
{
  /* sample                  alpha  fail   red       green     blue     alpha */

  { "#000000",               FALSE, FALSE, 0.0,      0.0,      0.0,      0.0 },
  { "#FFff00",               FALSE, FALSE, 1.0,      1.0,      0.0,      0.0 },
  { "#6495ed",               FALSE, FALSE, DBL(100), DBL(149), DBL(237), 0.0 },
  { "#fff",                  FALSE, FALSE, 1.0,      1.0,      1.0,      0.0 },
  { "#64649595eded",         FALSE, FALSE, 1.0,      1.0,      0.0,      0.0 },
  { "rgb(0,0,0)",            FALSE, FALSE, 0.0,      0.0,      0.0,      0.0 },
  { "rgb(100,149,237)",      FALSE, FALSE, DBL(100), DBL(149), DBL(237), 0.0 },
  { "rgba(100%,0,100%,0.5)", TRUE,  FALSE, 255.0,    0.0,      255.0,    0.5 },
  { "rgba(100%,0,100%,0.5)", FALSE, TRUE,  255.0,    0.0,      255.0,    0.5 },
  { "rgb(100%,149,20%)",     FALSE, FALSE, 1.0,      DBL(149), 0.2,      0.0 },
  { "rgb(100%,149,20%)",     TRUE,  TRUE,  1.0,      DBL(149), 0.2,      0.0 },
  { "rgb(foobar)",           FALSE, TRUE,  0.0,      0.0,      0.0,      0.0 },
  { "rgb(100,149,237",       FALSE, TRUE,  0.0,      0.0,      0.0,      0.0 },
  { "rED",                   FALSE, FALSE, 1.0,      0.0,      0.0,      0.0 },
  { "cornflowerblue",        FALSE, FALSE, DBL(100), DBL(149), DBL(237), 0.0 },
  { "    red",               FALSE, FALSE, 1.0,      0.0,      0.0,      0.0 },
  { "red      ",             FALSE, FALSE, 1.0,      0.0,      0.0,      0.0 },
  { "red",                   TRUE,  TRUE,  1.0,      0.0,      0.0,      0.0 },
  { "red  blue",             FALSE, TRUE,  0.0,      0.0,      0.0,      0.0 },
  { "transparent",           FALSE, TRUE,  0.0,      0.0,      0.0,      0.0 },
  { "transparent",           TRUE,  FALSE, 0.0,      0.0,      0.0,      0.0 },
  { "23foobar",              FALSE, TRUE,  0.0,      0.0,      0.0,      0.0 },
  { "",                      FALSE, TRUE,  0.0,      0.0,      0.0,      0.0 }
};


static gint
check_failure (const ColorSample *sample,
               gboolean           success,
               PicmanRGB           *rgb)
{
  if (success && sample->fail)
    {
      g_print ("Parser succeeded for sample \"%s\" but should have failed!\n"
               "  parsed color: (%g, %g, %g, %g)\n",
               sample->str, rgb->r, rgb->g, rgb->b, rgb->a);
      return 1;
    }

  if (!success && !sample->fail)
    {
      g_print ("Parser failed for sample \"%s\" but should have succeeded!\n"
               "  parsed color: (%g, %g, %g, %g)\n",
               sample->str, rgb->r, rgb->g, rgb->b, rgb->a);
      return 1;
    }

  return 0;
}

int
main (void)
{
  gint failures = 0;
  gint i;

  g_print ("\nTesting the PICMAN color parser ...\n");

  for (i = 0; i < G_N_ELEMENTS (samples); i++)
    {
      PicmanRGB   rgb = { 0.0, 0.0, 0.0, 0.0 };
      gboolean  success;

      if (samples[i].alpha)
        success = picman_rgba_parse_css (&rgb, samples[i].str, -1);
      else
        success = picman_rgb_parse_css (&rgb, samples[i].str, -1);

      failures += check_failure (samples + i, success, &rgb);
    }

  if (failures)
    {
      g_print ("%d out of %d samples failed!\n\n",
               failures, (int)G_N_ELEMENTS (samples));
      return EXIT_FAILURE;
    }
  else
    {
      g_print ("All %d samples passed.\n\n", (int)G_N_ELEMENTS (samples));
      return EXIT_SUCCESS;
    }
}

