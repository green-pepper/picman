/* parasitelist.h: Copyright 1998 Jay Cox <jaycox@picman.org>
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

#ifndef __PICMAN_PARASITE_LIST_H__
#define __PICMAN_PARASITE_LIST_H__


#include "picmanobject.h"


#define PICMAN_TYPE_PARASITE_LIST            (picman_parasite_list_get_type ())
#define PICMAN_PARASITE_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PARASITE_LIST, PicmanParasiteList))
#define PICMAN_PARASITE_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PARASITE_LIST, PicmanParasiteListClass))
#define PICMAN_IS_PARASITE_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PARASITE_LIST))
#define PICMAN_IS_PARASITE_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PARASITE_LIST))
#define PICMAN_PARASITE_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PARASITE_LIST, PicmanParasiteListClass))


typedef struct _PicmanParasiteListClass PicmanParasiteListClass;

struct _PicmanParasiteList
{
  PicmanObject  object;

  GHashTable *table;
};

struct _PicmanParasiteListClass
{
  PicmanObjectClass parent_class;

  void (* add)    (PicmanParasiteList *list,
                   PicmanParasite     *parasite);
  void (* remove) (PicmanParasiteList *list,
                   PicmanParasite     *parasite);
};


GType                picman_parasite_list_get_type (void) G_GNUC_CONST;

PicmanParasiteList   * picman_parasite_list_new      (void);
PicmanParasiteList   * picman_parasite_list_copy     (const PicmanParasiteList *list);
void                 picman_parasite_list_add      (PicmanParasiteList       *list,
                                                  const PicmanParasite     *parasite);
void                 picman_parasite_list_remove   (PicmanParasiteList       *list,
                                                  const gchar            *name);
gint                 picman_parasite_list_length   (PicmanParasiteList       *list);
gint                 picman_parasite_list_persistent_length (PicmanParasiteList *list);
void                 picman_parasite_list_foreach  (PicmanParasiteList       *list,
                                                  GHFunc                  function,
                                                  gpointer                user_data);
const PicmanParasite * picman_parasite_list_find     (PicmanParasiteList       *list,
                                                  const gchar            *name);


#endif  /*  __PICMAN_PARASITE_LIST_H__  */
