#pragma once

#include <QtGlobal>

#if defined(CR_LIBRARY)
#  define CRSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CRSHARED_EXPORT Q_DECL_IMPORT
#endif
