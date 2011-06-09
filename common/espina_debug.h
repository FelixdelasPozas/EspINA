#ifndef ESPINA_DEBUG_H
#define ESPINA_DEBUG_H

#include <QDebug>
#include <assert.h>

#define DEBUG_CACHE 0
#define DEBUG_FILTERS 0
#define DEBUG_MODEL 0

#define CACHE_DEBUG(exp) if (DEBUG_CACHE)      \
			  qDebug() << "Cache:" << exp;


#endif// ESPINA_DEBUG_H