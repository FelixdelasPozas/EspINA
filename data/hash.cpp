#include "hash.h"
#include "cajalTypes.h"
#include <QCryptographicHash>

//! Generates a Sha1 hash code from a vector of strings 
QString generateSha1( QStringList& v )
{  
  QCryptographicHash* hasher = new QCryptographicHash( QCryptographicHash::Sha1 );
  
  foreach(QString str, v)
    hasher->addData(str.toStdString().c_str(), str.size());

  QString outputHash = QString(hasher->result().toHex());
  delete hasher;
  return outputHash;
}

//! It flatten the NodeParamList to returning a vector of strings
QStringList reduceArgs( NodeParamList& nl)
{
  QStringList v;
  NodeParamList::iterator it;
  for( it=nl.begin(); it < nl.end(); it++ )
  {
    v.push_back(it->first);
    v.push_back(it->second);
  }
  return v;
}
