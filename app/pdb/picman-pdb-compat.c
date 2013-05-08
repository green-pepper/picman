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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"

#include "pdb-types.h"

#include "core/picman.h"
#include "core/picmanparamspecs.h"

#include "picmanpdb.h"
#include "picman-pdb-compat.h"


/*  public functions  */

GParamSpec *
picman_pdb_compat_param_spec (Picman           *picman,
                            PicmanPDBArgType  arg_type,
                            const gchar    *name,
                            const gchar    *desc)
{
  GParamSpec *pspec = NULL;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  switch (arg_type)
    {
    case PICMAN_PDB_INT32:
      pspec = picman_param_spec_int32 (name, name, desc,
                                     G_MININT32, G_MAXINT32, 0,
                                     G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_INT16:
      pspec = picman_param_spec_int16 (name, name, desc,
                                     G_MININT16, G_MAXINT16, 0,
                                     G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_INT8:
      pspec = picman_param_spec_int8 (name, name, desc,
                                    0, G_MAXUINT8, 0,
                                    G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_FLOAT:
      pspec = g_param_spec_double (name, name, desc,
                                   -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                                   G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_STRING:
      pspec = picman_param_spec_string (name, name, desc,
                                      TRUE, TRUE, FALSE,
                                      NULL,
                                      G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_INT32ARRAY:
      pspec = picman_param_spec_int32_array (name, name, desc,
                                           G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_INT16ARRAY:
      pspec = picman_param_spec_int16_array (name, name, desc,
                                           G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_INT8ARRAY:
      pspec = picman_param_spec_int8_array (name, name, desc,
                                          G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_FLOATARRAY:
      pspec = picman_param_spec_float_array (name, name, desc,
                                           G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_STRINGARRAY:
      pspec = picman_param_spec_string_array (name, name, desc,
                                            G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_COLOR:
      pspec = picman_param_spec_rgb (name, name, desc,
                                   TRUE, NULL,
                                   G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_ITEM:
      pspec = picman_param_spec_item_id (name, name, desc,
                                       picman, TRUE,
                                       G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_DISPLAY:
      pspec = picman_param_spec_display_id (name, name, desc,
                                          picman, TRUE,
                                          G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_IMAGE:
      pspec = picman_param_spec_image_id (name, name, desc,
                                        picman, TRUE,
                                        G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_LAYER:
      pspec = picman_param_spec_layer_id (name, name, desc,
                                        picman, TRUE,
                                        G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_CHANNEL:
      pspec = picman_param_spec_channel_id (name, name, desc,
                                          picman, TRUE,
                                          G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_DRAWABLE:
      pspec = picman_param_spec_drawable_id (name, name, desc,
                                           picman, TRUE,
                                           G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_SELECTION:
      pspec = picman_param_spec_selection_id (name, name, desc,
                                            picman, TRUE,
                                            G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_COLORARRAY:
      pspec = picman_param_spec_color_array (name, name, desc,
                                           G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_VECTORS:
      pspec = picman_param_spec_vectors_id (name, name, desc,
                                          picman, TRUE,
                                          G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_PARASITE:
      pspec = picman_param_spec_parasite (name, name, desc,
                                        G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_STATUS:
      pspec = g_param_spec_enum (name, name, desc,
                                 PICMAN_TYPE_PDB_STATUS_TYPE,
                                 PICMAN_PDB_EXECUTION_ERROR,
                                 G_PARAM_READWRITE);
      break;

    case PICMAN_PDB_END:
      break;
    }

  if (! pspec)
    g_warning ("%s: returning NULL for %s (%s)",
               G_STRFUNC, name, picman_pdb_compat_arg_type_to_string (arg_type));

  return pspec;
}

GType
picman_pdb_compat_arg_type_to_gtype (PicmanPDBArgType  type)
{

  switch (type)
    {
    case PICMAN_PDB_INT32:
      return PICMAN_TYPE_INT32;

    case PICMAN_PDB_INT16:
      return PICMAN_TYPE_INT16;

    case PICMAN_PDB_INT8:
      return PICMAN_TYPE_INT8;

    case PICMAN_PDB_FLOAT:
      return G_TYPE_DOUBLE;

    case PICMAN_PDB_STRING:
      return G_TYPE_STRING;

    case PICMAN_PDB_INT32ARRAY:
      return PICMAN_TYPE_INT32_ARRAY;

    case PICMAN_PDB_INT16ARRAY:
      return PICMAN_TYPE_INT16_ARRAY;

    case PICMAN_PDB_INT8ARRAY:
      return PICMAN_TYPE_INT8_ARRAY;

    case PICMAN_PDB_FLOATARRAY:
      return PICMAN_TYPE_FLOAT_ARRAY;

    case PICMAN_PDB_STRINGARRAY:
      return PICMAN_TYPE_STRING_ARRAY;

    case PICMAN_PDB_COLOR:
      return PICMAN_TYPE_RGB;

    case PICMAN_PDB_ITEM:
      return PICMAN_TYPE_ITEM_ID;

    case PICMAN_PDB_DISPLAY:
      return PICMAN_TYPE_DISPLAY_ID;

    case PICMAN_PDB_IMAGE:
      return PICMAN_TYPE_IMAGE_ID;

    case PICMAN_PDB_LAYER:
      return PICMAN_TYPE_LAYER_ID;

    case PICMAN_PDB_CHANNEL:
      return PICMAN_TYPE_CHANNEL_ID;

    case PICMAN_PDB_DRAWABLE:
      return PICMAN_TYPE_DRAWABLE_ID;

    case PICMAN_PDB_SELECTION:
      return PICMAN_TYPE_SELECTION_ID;

    case PICMAN_PDB_COLORARRAY:
      return PICMAN_TYPE_COLOR_ARRAY;

    case PICMAN_PDB_VECTORS:
      return PICMAN_TYPE_VECTORS_ID;

    case PICMAN_PDB_PARASITE:
      return PICMAN_TYPE_PARASITE;

    case PICMAN_PDB_STATUS:
      return PICMAN_TYPE_PDB_STATUS_TYPE;

    case PICMAN_PDB_END:
      break;
    }

  g_warning ("%s: returning G_TYPE_NONE for %d (%s)",
             G_STRFUNC, type, picman_pdb_compat_arg_type_to_string (type));

  return G_TYPE_NONE;
}

PicmanPDBArgType
picman_pdb_compat_arg_type_from_gtype (GType type)
{
  static GQuark  pdb_type_quark = 0;
  PicmanPDBArgType pdb_type;

  if (! pdb_type_quark)
    {
      struct
      {
        GType          g_type;
        PicmanPDBArgType pdb_type;
      }
      type_mapping[] =
      {
        { PICMAN_TYPE_INT32,           PICMAN_PDB_INT32       },
        { G_TYPE_INT,                PICMAN_PDB_INT32       },
        { G_TYPE_UINT,               PICMAN_PDB_INT32       },
        { G_TYPE_ENUM,               PICMAN_PDB_INT32       },
        { G_TYPE_BOOLEAN,            PICMAN_PDB_INT32       },

        { PICMAN_TYPE_INT16,           PICMAN_PDB_INT16       },
        { PICMAN_TYPE_INT8,            PICMAN_PDB_INT8        },
        { G_TYPE_DOUBLE,             PICMAN_PDB_FLOAT       },

        { G_TYPE_STRING,             PICMAN_PDB_STRING      },

        { PICMAN_TYPE_RGB,             PICMAN_PDB_COLOR       },

        { PICMAN_TYPE_INT32_ARRAY,     PICMAN_PDB_INT32ARRAY  },
        { PICMAN_TYPE_INT16_ARRAY,     PICMAN_PDB_INT16ARRAY  },
        { PICMAN_TYPE_INT8_ARRAY,      PICMAN_PDB_INT8ARRAY   },
        { PICMAN_TYPE_FLOAT_ARRAY,     PICMAN_PDB_FLOATARRAY  },
        { PICMAN_TYPE_STRING_ARRAY,    PICMAN_PDB_STRINGARRAY },
        { PICMAN_TYPE_COLOR_ARRAY,     PICMAN_PDB_COLORARRAY  },

        { PICMAN_TYPE_ITEM_ID,         PICMAN_PDB_ITEM        },
        { PICMAN_TYPE_DISPLAY_ID,      PICMAN_PDB_DISPLAY     },
        { PICMAN_TYPE_IMAGE_ID,        PICMAN_PDB_IMAGE       },
        { PICMAN_TYPE_LAYER_ID,        PICMAN_PDB_LAYER       },
        { PICMAN_TYPE_CHANNEL_ID,      PICMAN_PDB_CHANNEL     },
        { PICMAN_TYPE_DRAWABLE_ID,     PICMAN_PDB_DRAWABLE    },
        { PICMAN_TYPE_SELECTION_ID,    PICMAN_PDB_SELECTION   },
        { PICMAN_TYPE_LAYER_MASK_ID,   PICMAN_PDB_CHANNEL     },
        { PICMAN_TYPE_VECTORS_ID,      PICMAN_PDB_VECTORS     },

        { PICMAN_TYPE_PARASITE,        PICMAN_PDB_PARASITE    },

        { PICMAN_TYPE_PDB_STATUS_TYPE, PICMAN_PDB_STATUS      }
      };

      gint i;

      pdb_type_quark = g_quark_from_static_string ("picman-pdb-type");

      for (i = 0; i < G_N_ELEMENTS (type_mapping); i++)
        g_type_set_qdata (type_mapping[i].g_type, pdb_type_quark,
                          GINT_TO_POINTER (type_mapping[i].pdb_type));
    }

  pdb_type = GPOINTER_TO_INT (g_type_get_qdata (type, pdb_type_quark));

#if 0
  g_printerr ("%s: arg_type = %p (%s)  ->  %d (%s)\n",
              G_STRFUNC,
              (gpointer) type, g_type_name (type),
              pdb_type, picman_pdb_arg_type_to_string (pdb_type));
#endif

  return pdb_type;
}

gchar *
picman_pdb_compat_arg_type_to_string (PicmanPDBArgType type)
{
  const gchar *name;

  if (! picman_enum_get_value (PICMAN_TYPE_PDB_ARG_TYPE, type,
                             &name, NULL, NULL, NULL))
    {
      return  g_strdup_printf ("(PDB type %d unknown)", type);
    }

  return g_strdup (name);
}

void
picman_pdb_compat_procs_register (PicmanPDB           *pdb,
                                PicmanPDBCompatMode  compat_mode)
{
  static const struct
  {
    const gchar *old_name;
    const gchar *new_name;
  }
  compat_procs[] =
  {
    { "picman-blend",                         "picman-edit-blend"                 },
    { "picman-brushes-list",                  "picman-brushes-get-list"           },
    { "picman-bucket-fill",                   "picman-edit-bucket-fill"           },
    { "picman-channel-delete",                "picman-item-delete"                },
    { "picman-channel-get-name",              "picman-item-get-name"              },
    { "picman-channel-get-tattoo",            "picman-item-get-tattoo"            },
    { "picman-channel-get-visible",           "picman-item-get-visible"           },
    { "picman-channel-set-name",              "picman-item-set-name"              },
    { "picman-channel-set-tattoo",            "picman-item-set-tattoo"            },
    { "picman-channel-set-visible",           "picman-item-set-visible"           },
    { "picman-color-picker",                  "picman-image-pick-color"           },
    { "picman-convert-grayscale",             "picman-image-convert-grayscale"    },
    { "picman-convert-indexed",               "picman-image-convert-indexed"      },
    { "picman-convert-rgb",                   "picman-image-convert-rgb"          },
    { "picman-crop",                          "picman-image-crop"                 },
    { "picman-drawable-bytes",                "picman-drawable-bpp"               },
    { "picman-drawable-image",                "picman-drawable-get-image"         },
    { "picman-image-active-drawable",         "picman-image-get-active-drawable"  },
    { "picman-image-floating-selection",      "picman-image-get-floating-sel"     },
    { "picman-layer-delete",                  "picman-item-delete"                },
    { "picman-layer-get-linked",              "picman-item-get-linked"            },
    { "picman-layer-get-name",                "picman-item-get-name"              },
    { "picman-layer-get-tattoo",              "picman-item-get-tattoo"            },
    { "picman-layer-get-visible",             "picman-item-get-visible"           },
    { "picman-layer-mask",                    "picman-layer-get-mask"             },
    { "picman-layer-set-linked",              "picman-item-set-linked"            },
    { "picman-layer-set-name",                "picman-item-set-name"              },
    { "picman-layer-set-tattoo",              "picman-item-set-tattoo"            },
    { "picman-layer-set-visible",             "picman-item-set-visible"           },
    { "picman-palette-refresh",               "picman-palettes-refresh"           },
    { "picman-patterns-list",                 "picman-patterns-get-list"          },
    { "picman-temp-PDB-name",                 "picman-procedural-db-temp-name"    },
    { "picman-undo-push-group-end",           "picman-image-undo-group-end"       },
    { "picman-undo-push-group-start",         "picman-image-undo-group-start"     },

    /*  deprecations since 2.0  */
    { "picman-brushes-get-opacity",           "picman-context-get-opacity"        },
    { "picman-brushes-get-paint-mode",        "picman-context-get-paint-mode"     },
    { "picman-brushes-set-brush",             "picman-context-set-brush"          },
    { "picman-brushes-set-opacity",           "picman-context-set-opacity"        },
    { "picman-brushes-set-paint-mode",        "picman-context-set-paint-mode"     },
    { "picman-channel-ops-duplicate",         "picman-image-duplicate"            },
    { "picman-channel-ops-offset",            "picman-drawable-offset"            },
    { "picman-gradients-get-active",          "picman-context-get-gradient"       },
    { "picman-gradients-get-gradient",        "picman-context-get-gradient"       },
    { "picman-gradients-set-active",          "picman-context-set-gradient"       },
    { "picman-gradients-set-gradient",        "picman-context-set-gradient"       },
    { "picman-image-get-cmap",                "picman-image-get-colormap"         },
    { "picman-image-set-cmap",                "picman-image-set-colormap"         },
    { "picman-palette-get-background",        "picman-context-get-background"     },
    { "picman-palette-get-foreground",        "picman-context-get-foreground"     },
    { "picman-palette-set-background",        "picman-context-set-background"     },
    { "picman-palette-set-default-colors",    "picman-context-set-default-colors" },
    { "picman-palette-set-foreground",        "picman-context-set-foreground"     },
    { "picman-palette-swap-colors",           "picman-context-swap-colors"        },
    { "picman-palettes-set-palette",          "picman-context-set-palette"        },
    { "picman-patterns-set-pattern",          "picman-context-set-pattern"        },
    { "picman-selection-clear",               "picman-selection-none"             },

    /*  deprecations since 2.2  */
    { "picman-layer-get-preserve-trans",      "picman-layer-get-lock-alpha"       },
    { "picman-layer-set-preserve-trans",      "picman-layer-set-lock-alpha"       },

    /*  deprecations since 2.6  */
    { "picman-drawable-is-valid",             "picman-item-is-valid"              },
    { "picman-drawable-is-layer",             "picman-item-is-layer"              },
    { "picman-drawable-is-text-layer",        "picman-item-is-text-layer"         },
    { "picman-drawable-is-layer-mask",        "picman-item-is-layer-mask"         },
    { "picman-drawable-is-channel",           "picman-item-is-channel"            },
    { "picman-drawable-delete",               "picman-item-delete"                },
    { "picman-drawable-get-image",            "picman-item-get-image"             },
    { "picman-drawable-get-name",             "picman-item-get-name"              },
    { "picman-drawable-set-name",             "picman-item-set-name"              },
    { "picman-drawable-get-visible",          "picman-item-get-visible"           },
    { "picman-drawable-set-visible",          "picman-item-set-visible"           },
    { "picman-drawable-get-linked",           "picman-item-get-linked"            },
    { "picman-drawable-set-linked",           "picman-item-set-linked"            },
    { "picman-drawable-get-tattoo",           "picman-item-get-tattoo"            },
    { "picman-drawable-set-tattoo",           "picman-item-set-tattoo"            },
    { "picman-drawable-parasite-find",        "picman-item-get-parasite"          },
    { "picman-drawable-parasite-attach",      "picman-item-attach-parasite"       },
    { "picman-drawable-parasite-detach",      "picman-item-detach-parasite"       },
    { "picman-drawable-parasite-list",        "picman-item-get-parasite-list"     },
    { "picman-image-get-layer-position",      "picman-image-get-item-position"    },
    { "picman-image-raise-layer",             "picman-image-raise-item"           },
    { "picman-image-lower-layer",             "picman-image-lower-item"           },
    { "picman-image-raise-layer-to-top",      "picman-image-raise-item-to-top"    },
    { "picman-image-lower-layer-to-bottom",   "picman-image-lower-item-to-bottom" },
    { "picman-image-get-channel-position",    "picman-image-get-item-position"    },
    { "picman-image-raise-channel",           "picman-image-raise-item"           },
    { "picman-image-lower-channel",           "picman-image-lower-item"           },
    { "picman-image-get-vectors-position",    "picman-image-get-item-position"    },
    { "picman-image-raise-vectors",           "picman-image-raise-item"           },
    { "picman-image-lower-vectors",           "picman-image-lower-item"           },
    { "picman-image-raise-vectors-to-top",    "picman-image-raise-item-to-top"    },
    { "picman-image-lower-vectors-to-bottom", "picman-image-lower-item-to-bottom" },
    { "picman-vectors-is-valid",              "picman-item-is-valid"              },
    { "picman-vectors-get-image",             "picman-item-get-image"             },
    { "picman-vectors-get-name",              "picman-item-get-name"              },
    { "picman-vectors-set-name",              "picman-item-set-name"              },
    { "picman-vectors-get-visible",           "picman-item-get-visible"           },
    { "picman-vectors-set-visible",           "picman-item-set-visible"           },
    { "picman-vectors-get-linked",            "picman-item-get-linked"            },
    { "picman-vectors-set-linked",            "picman-item-set-linked"            },
    { "picman-vectors-get-tattoo",            "picman-item-get-tattoo"            },
    { "picman-vectors-set-tattoo",            "picman-item-set-tattoo"            },
    { "picman-vectors-parasite-find",         "picman-item-get-parasite"          },
    { "picman-vectors-parasite-attach",       "picman-item-attach-parasite"       },
    { "picman-vectors-parasite-detach",       "picman-item-detach-parasite"       },
    { "picman-vectors-parasite-list",         "picman-item-get-parasite-list"     },
    { "picman-image-parasite-find",           "picman-image-get-parasite"         },
    { "picman-image-parasite-attach",         "picman-image-attach-parasite"      },
    { "picman-image-parasite-detach",         "picman-image-detach-parasite"      },
    { "picman-image-parasite-list",           "picman-image-get-parasite-list"    },
    { "picman-parasite-find",                 "picman-get-parasite"               },
    { "picman-parasite-attach",               "picman-attach-parasite"            },
    { "picman-parasite-detach",               "picman-detach-parasite"            },
    { "picman-parasite-list",                 "picman-get-parasite-list"          }
  };

  g_return_if_fail (PICMAN_IS_PDB (pdb));

  if (compat_mode != PICMAN_PDB_COMPAT_OFF)
    {
      gint i;

      for (i = 0; i < G_N_ELEMENTS (compat_procs); i++)
        picman_pdb_register_compat_proc_name (pdb,
                                            compat_procs[i].old_name,
                                            compat_procs[i].new_name);
    }
}
