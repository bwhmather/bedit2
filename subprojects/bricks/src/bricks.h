/*
 * Copyright (C) 2023 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#if !GTK_CHECK_VERSION(4, 14, 4)
# error "libbricks requires gtk4 >= 4.15.1"
#endif

#if !GLIB_CHECK_VERSION(2, 76, 0)
# error "libbricks requires glib-2.0 >= 2.76.0"
#endif

#define _BRICKS_INSIDE

#include "brk-version.h"
#include "brk-tab-bar.h"
#include "brk-tab-view.h"

#undef _BRICKS_INSIDE

G_END_DECLS
