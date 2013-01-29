/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "ModelItem.h"

#include <QStringList>
#include "Core/Extensions/ModelItemExtension.h"
#include "Core/Model/RelationshipGraph.h"
#include "EspinaModel.h"

#include <QDebug>
#include <QCryptographicHash>

using namespace EspINA;

template<class T>
QString arg3(const T val[3])
{
  return QString("%1,%2,%3").arg(val[0]).arg(val[1]).arg(val[2]);
}

template<class T>
QString arg6(const T val[6])
{
  return QString("%1,%2,%3,%4,%5,%6").arg(val[0]).arg(val[1]).arg(val[2]).arg(val[3]).arg(val[4]).arg(val[5]);
}


const ModelItem::ArgumentId ModelItem::EXTENSIONS = "Extensions";

//------------------------------------------------------------------------
ModelItem::Arguments::Arguments()
{
}

//------------------------------------------------------------------------
ModelItem::Arguments::Arguments(const QMap<ArgumentId, Argument>& args)
: QMap<ArgumentId, Argument>(args)
{
}

//------------------------------------------------------------------------
ModelItem::Arguments::Arguments(const QString args)
{
  QString name, value, buffer;
  int balanceo = 0;

  foreach(QChar c, args)
  {
    if( c == '=' && balanceo == 0)
    {
      name = buffer;
      buffer = "";
    }
    else if( c == '[')
    {
      if(balanceo > 0)
        buffer.append(c);
      balanceo++;
    }
    else if( c== ']')
    {
      balanceo--;
      if(balanceo > 0)
        buffer.append(c);
    }
    else if( c == ';' && balanceo == 0)
    {
      value = buffer;
      buffer = "";
      insert(name, value);
    }
    else
    {
      buffer.append(c);
    }
  }
}

//------------------------------------------------------------------------
QString ModelItem::Arguments::serialize() const
{
  QString args;
  foreach(ArgumentId id, keys())
      args += argument(id, value(id));

  return args;
}

//------------------------------------------------------------------------
QString ModelItem::Arguments::hash() const
{
  QCryptographicHash hasher(QCryptographicHash::Sha1);

  hasher.addData(serialize().toStdString().c_str(), serialize().size());

  return QString(hasher.result().toHex());
}

//------------------------------------------------------------------------
ModelItem::~ModelItem()
{
  qDebug() << "Destruyendo ModelItem";
}

//------------------------------------------------------------------------
QString ModelItem::serialize() const
{
  return QString("none");
}

//------------------------------------------------------------------------
ModelItemSList ModelItem::relatedItems(RelationType relType,
                                       const QString& relName)
{
  ModelItemSList res;
  if (m_model)
    res = m_model->relatedItems(this, relType, relName);

  return res;
}

//------------------------------------------------------------------------
RelationList ModelItem::relations(const QString &relName)
{
  RelationList res;
  if (m_model)
    res = m_model->relations(this, relName);

  return res;
}

//------------------------------------------------------------------------
ModelItemPtr EspINA::indexPtr(const QModelIndex& index)
{
  ModelItemPtr ptr = static_cast<ModelItemPtr>(index.internalPointer());
  Q_ASSERT(ptr);

  return ptr;
}
