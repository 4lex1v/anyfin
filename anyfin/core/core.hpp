
#pragma once

#include "anyfin/base.hpp"

#ifdef FIN_EMBED_STATE
#define FIN_EXTERN_STATE
#else
#define FIN_EXTERN_STATE extern
#endif
