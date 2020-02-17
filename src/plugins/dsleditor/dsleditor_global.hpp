#pragma once

#include <qglobal.h>

#if defined(DSLEDITOR_LIBRARY)
#  define DSLEDITOR_EXPORT Q_DECL_EXPORT
#else
#  define DSLEDITOR_EXPORT Q_DECL_IMPORT
#endif
