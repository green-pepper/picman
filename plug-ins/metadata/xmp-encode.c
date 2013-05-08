/* xmp-encode.c - generate XMP metadata from the tree model
 *
 * Copyright (C) 2005, Raphaël Quinet <raphael@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include <libpicman/picman.h>

#include "libpicman/stdplugins-intl.h"

#include "xmp-encode.h"
#include "xmp-schemas.h"


static void  gen_element (GString     *buffer,
                          gint         indent,
                          const gchar *prefix,
                          const gchar *name,
                          const gchar *value,
                          ...) G_GNUC_NULL_TERMINATED;


static void
gen_schema_start (GString         *buffer,
                  const XMPSchema *schema)
{
  g_string_append_printf (buffer, " <rdf:Description xmlns:%s='%s'>\n",
                          schema->prefix, schema->uri);
}

static void
gen_schema_end (GString *buffer)
{
  g_string_append (buffer, " </rdf:Description>\n\n");
}

static void
gen_element (GString     *buffer,
             gint         indent,
             const gchar *prefix,
             const gchar *name,
             const gchar *value,
             ...)
{
  const gchar *attr_name;
  const gchar *attr_value;
  gchar       *utf8 = NULL;
  va_list      args;

  while (indent--)
    g_string_append_c (buffer, ' ');

  g_string_append_printf (buffer, "<%s:%s", prefix, name);

  va_start (args, value);

  while ((attr_name  = va_arg (args, const gchar *)) != NULL &&
         (attr_value = va_arg (args, const gchar *)) != NULL)
    {
      if (*attr_name && *attr_value)
        g_string_append_printf (buffer, " %s='%s'", attr_name, attr_value);
    }

  va_end (args);

  if (value && *value)
    utf8 = picman_any_to_utf8 (value, -1,  NULL);

  if (utf8 && *utf8)
    {
      gchar *escaped_value = g_markup_escape_text (utf8, -1);

      g_string_append_printf (buffer, ">%s</%s:%s>\n",
                              escaped_value, prefix, name);
      g_free (escaped_value);
    }
  else
    {
      g_string_append (buffer, " />\n");
    }

  g_free (utf8);
}

static void
gen_property (GString            *buffer,
              const XMPSchema    *schema,
              const XMPProperty  *property,
              const gchar       **value_array)
{
  gint         i;
  const gchar *ns_prefix;

  switch (property->type)
    {
    case XMP_TYPE_BOOLEAN:
    case XMP_TYPE_DATE:
    case XMP_TYPE_INTEGER:
    case XMP_TYPE_REAL:
    case XMP_TYPE_MIME_TYPE:
    case XMP_TYPE_TEXT:
    case XMP_TYPE_RATIONAL:
      gen_element (buffer, 2,
                   schema->prefix, property->name, value_array[0],
                   NULL);
      break;

    case XMP_TYPE_LOCALE_BAG:
    case XMP_TYPE_TEXT_BAG:
    case XMP_TYPE_XPATH_BAG:
    case XMP_TYPE_JOB_BAG:
      g_string_append_printf (buffer, "  <%s:%s>\n   <rdf:Bag>\n",
                              schema->prefix, property->name);
      for (i = 0; value_array[i] != NULL; i++)
        {
          gen_element (buffer, 4,
                       "rdf", "li", value_array[i],
                       NULL);
        }
      g_string_append_printf (buffer, "   </rdf:Bag>\n  </%s:%s>\n",
                              schema->prefix, property->name);
      break;

    case XMP_TYPE_INTEGER_SEQ:
    case XMP_TYPE_TEXT_SEQ:
    case XMP_TYPE_RESOURCE_EVENT_SEQ:
    case XMP_TYPE_RATIONAL_SEQ:
      g_string_append_printf (buffer, "  <%s:%s>\n   <rdf:Seq>\n",
                              schema->prefix, property->name);
      for (i = 0; value_array[i] != NULL; i++)
        {
          gen_element (buffer, 4,
                       "rdf", "li", value_array[i],
                       NULL);
        }
      g_string_append_printf (buffer, "   </rdf:Seq>\n  </%s:%s>\n",
                              schema->prefix, property->name);
      break;

    case XMP_TYPE_LANG_ALT:
      g_string_append_printf (buffer, "  <%s:%s>\n   <rdf:Alt>\n",
                              schema->prefix, property->name);
      gen_element (buffer, 4,
                   "rdf", "li", value_array[0],
                   "xml:lang", "x-default",
                   NULL);
      g_string_append_printf (buffer, "   </rdf:Alt>\n  </%s:%s>\n",
                              schema->prefix, property->name);
      break;

    case XMP_TYPE_URI:
      gen_element (buffer, 2,
                   schema->prefix, property->name, NULL,
                   "rdf:resource", value_array[0],
                   NULL);
      break;

    case XMP_TYPE_GPS_COORDINATE:
      gen_element (buffer, 2,
                   schema->prefix, property->name, value_array[0],
                   NULL);
      break;

    case XMP_TYPE_RESOURCE_REF:
    case XMP_TYPE_DIMENSIONS:
    case XMP_TYPE_FLASH:
    case XMP_TYPE_OECF_SFR:
    case XMP_TYPE_CFA_PATTERN:
    case XMP_TYPE_DEVICE_SETTINGS:
    case XMP_TYPE_CONTACT_INFO:
    case XMP_TYPE_GENERIC_STRUCTURE:
      if (value_array[0] && value_array[1]
          && !! strcmp (value_array[1], schema->uri))
        {
          g_string_append_printf (buffer,
                                  "  <%s:%s rdf:parseType='Resource'\n"
                                  "   xmlns:%s='%s'>\n",
                                  schema->prefix, property->name,
                                  value_array[0], value_array[1]);
          ns_prefix = value_array[0];
        }
      else
        {
          g_string_append_printf (buffer,
                                  "  <%s:%s rdf:parseType='Resource'>\n",
                                  schema->prefix, property->name);
          ns_prefix = schema->prefix;
        }
      if (value_array[0] && value_array[1])
        for (i = 2; value_array[i] != NULL; i += 2)
          {
            gen_element (buffer, 3,
                         ns_prefix, value_array[i], value_array[i + 1],
                         NULL);
          }
      g_string_append_printf (buffer, "  </%s:%s>\n",
                              schema->prefix, property->name);
      break;

    case XMP_TYPE_THUMBNAIL_ALT:
      g_printerr ("FIXME: output not implemented yet (%s:%s)",
                  schema->prefix, property->name);
      break;

    case XMP_TYPE_UNKNOWN:
      g_printerr ("Unknown property type for %s", property->name);
      break;
    }
}

/**
 * xmp_generate_block:
 * @xmp_model: An #XMPModel
 * @buffer: A #GString in which the generated XMP packet will be stored.
 *
 * Generate XMP packet from xmp_model and store it in the supplied buffer.
 *
 */
void
xmp_generate_packet (XMPModel *xmp_model,
                     GString  *buffer)
{
  GtkTreeModel    *model;
  GtkTreeIter      iter;
  GtkTreeIter      child;
  const XMPSchema *schema;
  gpointer         saved_ref;

  g_return_if_fail (xmp_model != NULL);
  g_return_if_fail (buffer != NULL);
  model = xmp_model_get_tree_model (xmp_model);
  g_return_if_fail (model != NULL);
  if (! buffer)
    buffer = g_string_new (NULL);
  buffer = g_string_append (buffer,
    "<?xpacket begin='\357\273\277' id='W5M0MpCehiHzreSzNTczkc9d'?>\n"
    "<x:xmpmeta xmlns:x='adobe:ns:meta/'>\n"
    "<rdf:RDF xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>\n"
    "\n");
  /* generate the contents of the XMP packet */
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
      do
        {
          gtk_tree_model_get (model, &iter,
                              COL_XMP_TYPE_XREF, &schema,
                              -1);
          gen_schema_start (buffer, schema);
          if (gtk_tree_model_iter_children (model, &child, &iter))
            {
              saved_ref = NULL;
              do
                {
                  const XMPProperty  *property;
                  const gchar       **value_array;

                  gtk_tree_model_get (model, &child,
                                      COL_XMP_TYPE_XREF, &property,
                                      COL_XMP_VALUE_RAW, &value_array,
                                      -1);
                  /* do not process structured types multiple times */
                  if (saved_ref != value_array)
                    {
                      saved_ref = value_array;
                      g_return_if_fail (property->name != NULL);
                      gen_property (buffer, schema, property, value_array);
                    }
                }
              while (gtk_tree_model_iter_next (model, &child));
            }
          gen_schema_end (buffer);
        }
      while (gtk_tree_model_iter_next (model, &iter));
    }
  g_string_append (buffer, "</rdf:RDF>\n</x:xmpmeta>\n<?xpacket end='r'?>\n");
}
