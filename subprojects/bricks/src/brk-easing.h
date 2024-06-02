/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_BRICKS_INSIDE) && !defined(BRICKS_COMPILATION)
#error "Only <bricks.h> can be included directly."
#endif

#include "brk-version.h"

#include <glib.h>

#include "brk-enums.h"

G_BEGIN_DECLS

typedef enum {
  BRK_LINEAR,
  BRK_EASE_IN_QUAD,
  BRK_EASE_OUT_QUAD,
  BRK_EASE_IN_OUT_QUAD,
  BRK_EASE_IN_CUBIC,
  BRK_EASE_OUT_CUBIC,
  BRK_EASE_IN_OUT_CUBIC,
  BRK_EASE_IN_QUART,
  BRK_EASE_OUT_QUART,
  BRK_EASE_IN_OUT_QUART,
  BRK_EASE_IN_QUINT,
  BRK_EASE_OUT_QUINT,
  BRK_EASE_IN_OUT_QUINT,
  BRK_EASE_IN_SINE,
  BRK_EASE_OUT_SINE,
  BRK_EASE_IN_OUT_SINE,
  BRK_EASE_IN_EXPO,
  BRK_EASE_OUT_EXPO,
  BRK_EASE_IN_OUT_EXPO,
  BRK_EASE_IN_CIRC,
  BRK_EASE_OUT_CIRC,
  BRK_EASE_IN_OUT_CIRC,
  BRK_EASE_IN_ELASTIC,
  BRK_EASE_OUT_ELASTIC,
  BRK_EASE_IN_OUT_ELASTIC,
  BRK_EASE_IN_BACK,
  BRK_EASE_OUT_BACK,
  BRK_EASE_IN_OUT_BACK,
  BRK_EASE_IN_BOUNCE,
  BRK_EASE_OUT_BOUNCE,
  BRK_EASE_IN_OUT_BOUNCE,
} BrkEasing;

BRK_AVAILABLE_IN_ALL
double brk_easing_ease (BrkEasing self,
                        double    value);

G_END_DECLS
