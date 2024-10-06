#pragma once

#include <glib.h>

#include "bedit-window.h"

G_BEGIN_DECLS

void
bedit_window_actions_init_class(BeditWindowClass *class);

void
bedit_window_actions_init_instance(BeditWindow *self);

G_END_DECLS
