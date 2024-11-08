#pragma once

#include <glib.h>

#include "bedit-application.h"

G_BEGIN_DECLS

void
bedit_application_actions_init_class(BeditApplicationClass *class);

void
bedit_application_actions_init_instance(BeditApplication *self);

G_END_DECLS
