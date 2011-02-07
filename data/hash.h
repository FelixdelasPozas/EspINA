#ifndef HASH_H
#define HASH_H

#include <vector>
#include <string>

#include "cajalTypes.h"
#include <QString>

QString generateSha1( std::vector<QString>& v );
std::vector<QString> reduceArgs( NodeParamList& nl);

#endif // HASHL_H