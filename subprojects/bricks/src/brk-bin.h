/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_BRICKS_INSIDE) && !defined(BRICKS_COMPILATION)
#error "Only <bricks.h> can be included directly."
#endif

#include "brk-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BRK_TYPE_BIN (brk_bin_get_type())

BRK_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (BrkBin, brk_bin, BRK, BIN, GtkWidget)

struct _BrkBinClass
{
  GtkWidgetClass parent_class;
};

BRK_AVAILABLE_IN_ALL
GtkWidget *brk_bin_new (void) G_GNUC_WARN_UNUSED_RESULT;

BRK_AVAILABLE_IN_ALL
GtkWidget *brk_bin_get_child (BrkBin    *self);
BRK_AVAILABLE_IN_ALL
void       brk_bin_set_child (BrkBin    *self,
                              GtkWidget *child);

G_END_DECLS
