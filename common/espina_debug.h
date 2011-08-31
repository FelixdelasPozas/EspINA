#ifndef ESPINA_DEBUG_H
#define ESPINA_DEBUG_H

#include <QDebug>
#include <assert.h>

#define DEBUG_CACHE 0
#define DEBUG_FILTERS 0
#define DEBUG_MODEL 0
#define DEBUG_GUI 1
#define DEBUG_EXTENSIONS 0
#define DEBUG_PICKING 0

#define CACHE_DEBUG(exp) if (DEBUG_CACHE)      \
			  qDebug() << "Cache:" << exp;

#define EXTENSION_DEBUG(exp) if (DEBUG_EXTENSIONS)      \
			  qDebug() << "Extensions: " << exp;

#define PICKING_DEBUG(exp) if (DEBUG_PICKING)      \
			  qDebug() << "Slice View:" Picked: << exp;
			  
			  
#define COUT_6VECT(vect) std::cout << #vect << ": " << vect[0] << " " << vect[1] << " " << vect[2] << " " << vect[3] << " " << vect[4] << " "<< vect[5] << std::endl;

#endif// ESPINA_DEBUG_H
