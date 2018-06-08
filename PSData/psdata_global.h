#ifndef PSDATA_GLOBAL_H
#define PSDATA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PSDATA_LIBRARY)
#  define PSDATASHARED_EXPORT Q_DECL_EXPORT
#else
#  define PSDATASHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PSDATA_GLOBAL_H
