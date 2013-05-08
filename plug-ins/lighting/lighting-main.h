#ifndef __LIGHTING_MAIN_H__
#define __LIGHTING_MAIN_H__

/* Defines and stuff */
/* ================= */

#define PLUG_IN_PROC   "plug-in-lighting"
#define PLUG_IN_BINARY "lighting"
#define PLUG_IN_ROLE   "picman-lighting"

#define TILE_CACHE_SIZE 16
#define NUM_LIGHTS      6

/* Typedefs */
/* ======== */

typedef enum
{
  POINT_LIGHT,
  DIRECTIONAL_LIGHT,
  SPOT_LIGHT,
  NO_LIGHT
} LightType;

enum
{
  LINEAR_MAP,
  LOGARITHMIC_MAP,
  SINUSOIDAL_MAP,
  SPHERICAL_MAP
};

enum
{
  IMAGE_BUMP,
  WAVES_BUMP
};

typedef struct
{
  gdouble     ambient_int;
  gdouble     diffuse_int;
  gdouble     diffuse_ref;
  gdouble     specular_ref;
  gdouble     highlight;
  gboolean    metallic;
  PicmanRGB     color;
} MaterialSettings;

typedef struct
{
  LightType    type;
  PicmanVector3  position;
  PicmanVector3  direction;
  PicmanRGB      color;
  gdouble      intensity;
  gboolean     active;
} LightSettings;

typedef struct
{
  gint32       drawable_id;
  gint32       bumpmap_id;
  gint32       envmap_id;

  /* Render variables */
  /* ================ */

  PicmanVector3      viewpoint;
  PicmanVector3      planenormal;
  LightSettings    lightsource[NUM_LIGHTS];
  MaterialSettings material;
  MaterialSettings ref_material;

  gdouble      pixel_treshold;
  gdouble      bumpmax,bumpmin;
  gint         max_depth;
  gint         bumpmaptype;

  /* Flags */
  gint         antialiasing;
  gint         create_new_image;
  gint         transparent_background;
  gint         bump_mapped;
  gint         env_mapped;
  gint         ref_mapped;
  gint         bumpstretch;
  gint         previewquality;
  gboolean     symbols;
  gboolean     interactive_preview;

  /* Misc */
  gboolean     update_enabled;
  gint         light_selected;
  gboolean     light_isolated;
  gdouble      preview_zoom_factor;
} LightingValues;

/* Externally visible variables */
/* ============================ */

extern LightingValues mapvals;

#endif  /* __LIGHTING_MAIN_H__ */
