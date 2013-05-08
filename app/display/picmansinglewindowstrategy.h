/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansinglewindowstrategy.h
 * Copyright (C) 2011 Martin Nordholts <martinn@src.gnome.org>
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

#ifndef __PICMAN_SINGLE_WINDOW_STRATEGY_H__
#define __PICMAN_SINGLE_WINDOW_STRATEGY_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_SINGLE_WINDOW_STRATEGY            (picman_single_window_strategy_get_type ())
#define PICMAN_SINGLE_WINDOW_STRATEGY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SINGLE_WINDOW_STRATEGY, PicmanSingleWindowStrategy))
#define PICMAN_SINGLE_WINDOW_STRATEGY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SINGLE_WINDOW_STRATEGY, PicmanSingleWindowStrategyClass))
#define PICMAN_IS_SINGLE_WINDOW_STRATEGY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SINGLE_WINDOW_STRATEGY))
#define PICMAN_IS_SINGLE_WINDOW_STRATEGY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SINGLE_WINDOW_STRATEGY))
#define PICMAN_SINGLE_WINDOW_STRATEGY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SINGLE_WINDOW_STRATEGY, PicmanSingleWindowStrategyClass))


typedef struct _PicmanSingleWindowStrategyClass PicmanSingleWindowStrategyClass;

struct _PicmanSingleWindowStrategy
{
  PicmanObject  parent_instance;
};

struct _PicmanSingleWindowStrategyClass
{
  PicmanObjectClass  parent_class;
};


GType        picman_single_window_strategy_get_type          (void) G_GNUC_CONST;

PicmanObject * picman_single_window_strategy_get_singleton     (void);


#endif /* __PICMAN_SINGLE_WINDOW_STRATEGY_H__ */
