#ifndef HASH_H
#define HASH_H

#include "cajalTypes.h"
#include <QString>
#include <QStringList>

QString generateSha1( QStringList& v );
QStringList reduceArgs( NodeParamList& nl);

#endif // HASHL_H