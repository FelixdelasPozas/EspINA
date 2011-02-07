#include "hash.h"
#include <vector>
#include <string>

#include "cajalTypes.h"
//std::string generateSha1( std::vector<std::string> v );
#include <QCryptographicHash>
#include <QString>


//! Generates a Sha1 hash code from a vector of strings 
QString generateSha1( std::vector<QString>& v )
{  
  QCryptographicHash* hasher = new QCryptographicHash( QCryptographicHash::Sha1 );
  
  std::vector<QString>::iterator it;
  for(it=v.begin(); it < v.end(); it++)
  {
    hasher->addData(it->toStdString().c_str(), it->size());
  }
  QString outputHash = QString::QString(hasher->result().toHex());
  delete hasher;
  return outputHash;
}

//! It flatten the NodeParamList to returning a vector of strings
std::vector<QString> reduceArgs( NodeParamList& nl)
{
  std::vector<QString> v;
  NodeParamList::iterator it;
  for( it=nl.begin(); it < nl.end(); it++ )
  {
    v.push_back(it->first);
    v.push_back(it->second);
  }
  return v;
}
