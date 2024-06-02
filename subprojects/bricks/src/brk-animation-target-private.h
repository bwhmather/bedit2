/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_BRICKS_INSIDE) && !defined(BRICKS_COMPILATION)
#error "Only <bricks.h> can be included directly."
#endif

#include "brk-animation-target.h"

G_BEGIN_DECLS

void brk_animation_target_set_value (BrkAnimationTarget *self,
                                     double              value);

G_END_DECLS
