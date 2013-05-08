/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanmodule.h
 * (C) 1999 Austin Donnelly <austin@picman.org>
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

#ifndef __PICMAN_MODULE_H__
#define __PICMAN_MODULE_H__

#include <gmodule.h>

#define __PICMAN_MODULE_H_INSIDE__

#include <libpicmanmodule/picmanmoduletypes.h>

#include <libpicmanmodule/picmanmoduledb.h>

#undef __PICMAN_MODULE_H_INSIDE__

G_BEGIN_DECLS


/**
 * PICMAN_MODULE_ABI_VERSION:
 *
 * The version of the module system's ABI. Modules put this value into
 * #PicmanModuleInfo's @abi_version field so the code loading the modules
 * can check if it was compiled against the same module ABI the modules
 * are compiled against.
 *
 *  PICMAN_MODULE_ABI_VERSION is incremented each time one of the
 *  following changes:
 *
 *  - the libpicmanmodule implementation (if the change affects modules).
 *
 *  - one of the classes implemented by modules (currently #PicmanColorDisplay,
 *    #PicmanColorSelector and #PicmanController).
 **/
#define PICMAN_MODULE_ABI_VERSION 0x0004


/**
 * PicmanModuleState:
 * @PICMAN_MODULE_STATE_ERROR:       Missing picman_module_register() function
 *                                 or other error.
 * @PICMAN_MODULE_STATE_LOADED:      An instance of a type implemented by
 *                                 this module is allocated.
 * @PICMAN_MODULE_STATE_LOAD_FAILED: picman_module_register() returned %FALSE.
 * @PICMAN_MODULE_STATE_NOT_LOADED:  There are no instances allocated of
 *                                 types implemented by this module.
 *
 * The possible states a #PicmanModule can be in.
 **/
typedef enum
{
  PICMAN_MODULE_STATE_ERROR,
  PICMAN_MODULE_STATE_LOADED,
  PICMAN_MODULE_STATE_LOAD_FAILED,
  PICMAN_MODULE_STATE_NOT_LOADED
} PicmanModuleState;


#define PICMAN_MODULE_ERROR (picman_module_error_quark ())

GQuark  picman_module_error_quark (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_MODULE_FAILED             /* generic error condition */
} PicmanModuleError;


/**
 * PicmanModuleInfo:
 * @abi_version: The #PICMAN_MODULE_ABI_VERSION the module was compiled against.
 * @purpose:     The module's general purpose.
 * @author:      The module's author.
 * @version:     The module's version.
 * @copyright:   The module's copyright.
 * @date:        The module's release date.
 *
 * This structure contains information about a loadable module.
 **/
struct _PicmanModuleInfo
{
  guint32  abi_version;
  gchar   *purpose;
  gchar   *author;
  gchar   *version;
  gchar   *copyright;
  gchar   *date;
};


/**
 * PicmanModuleQueryFunc:
 * @module:  The #PicmanModule responsible for this loadable module.
 * @Returns: The #PicmanModuleInfo struct describing the module.
 *
 * The signature of the query function a loadable PICMAN module must
 * implement.  In the module, the function must be called
 * picman_module_query().
 *
 * #PicmanModule will copy the returned #PicmanModuleInfo struct, so the
 * module doesn't need to keep these values around (however in most
 * cases the module will just return a pointer to a constant
 * structure).
 **/
typedef const PicmanModuleInfo * (* PicmanModuleQueryFunc)    (GTypeModule *module);

/**
 * PicmanModuleRegisterFunc:
 * @module:  The #PicmanModule responsible for this loadable module.
 * @Returns: %TRUE on success, %FALSE otherwise.
 *
 * The signature of the register function a loadable PICMAN module must
 * implement.  In the module, the function must be called
 * picman_module_register().
 *
 * When this function is called, the module should register all the types
 * it implements with the passed @module.
 **/
typedef gboolean               (* PicmanModuleRegisterFunc) (GTypeModule *module);


/* PicmanModules have to implement these */
G_MODULE_EXPORT const PicmanModuleInfo * picman_module_query    (GTypeModule *module);
G_MODULE_EXPORT gboolean               picman_module_register (GTypeModule *module);


#define PICMAN_TYPE_MODULE            (picman_module_get_type ())
#define PICMAN_MODULE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MODULE, PicmanModule))
#define PICMAN_MODULE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MODULE, PicmanModuleClass))
#define PICMAN_IS_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MODULE))
#define PICMAN_IS_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MODULE))
#define PICMAN_MODULE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MODULE, PicmanModuleClass))


typedef struct _PicmanModuleClass PicmanModuleClass;

/**
 * PicmanModule:
 * @filename:
 * @verbose:
 * @state:
 * @on_disk:
 * @load_inhibit:
 * @info:
 * @last_module_error:
 *
 * #PicmanModule is a generic mechanism to dynamically load modules into
 * PICMAN.  It is a #GTypeModule subclass, implementing module loading
 * using #GModule.  #PicmanModule does not know which functionality is
 * implemented by the modules, it just provides a framework to get
 * arbitrary #GType implementations loaded from disk.
 **/
struct _PicmanModule
{
  GTypeModule      parent_instance;

  /*< public >*/
  gchar           *filename;     /* path to the module                       */
  gboolean         verbose;      /* verbose error reporting                  */
  PicmanModuleState  state;        /* what's happened to the module            */
  gboolean         on_disk;      /* TRUE if file still exists                */
  gboolean         load_inhibit; /* user requests not to load at boot time   */

  /* stuff from now on may be NULL depending on the state the module is in   */
  /*< private >*/
  GModule         *module;       /* handle on the module                     */

  /*< public >*/
  PicmanModuleInfo  *info;         /* returned values from module_query        */
  gchar           *last_module_error;

  /*< private >*/
  PicmanModuleQueryFunc     query_module;
  PicmanModuleRegisterFunc  register_module;
};

struct _PicmanModuleClass
{
  GTypeModuleClass  parent_class;

  void (* modified) (PicmanModule *module);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType         picman_module_get_type         (void) G_GNUC_CONST;

PicmanModule  * picman_module_new              (const gchar     *filename,
                                            gboolean         load_inhibit,
                                            gboolean         verbose);

gboolean      picman_module_query_module     (PicmanModule      *module);

void          picman_module_modified         (PicmanModule      *module);
void          picman_module_set_load_inhibit (PicmanModule      *module,
                                            gboolean         load_inhibit);

const gchar * picman_module_state_name       (PicmanModuleState  state);

PICMAN_DEPRECATED_FOR(g_type_module_register_enum)
GType         picman_module_register_enum    (GTypeModule      *module,
                                            const gchar      *name,
                                            const GEnumValue *const_static_values);


/*  PicmanModuleInfo functions  */

PicmanModuleInfo * picman_module_info_new  (guint32               abi_version,
                                        const gchar          *purpose,
                                        const gchar          *author,
                                        const gchar          *version,
                                        const gchar          *copyright,
                                        const gchar          *date);
PicmanModuleInfo * picman_module_info_copy (const PicmanModuleInfo *info);
void             picman_module_info_free (PicmanModuleInfo       *info);


G_END_DECLS

#endif  /* __PICMAN_MODULE_H__ */
