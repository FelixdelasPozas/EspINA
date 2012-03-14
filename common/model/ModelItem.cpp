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
#include "RelationshipGraph.h"

#include <QDebug>
#include <QCryptographicHash>

ModelItem::Arguments::Arguments()
{
}

ModelItem::Arguments::Arguments(const QMap<QString, QString>& args)
: QMap<QString, QString>(args)
{
}

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

QString ModelItem::Arguments::serialize() const
{
  QString args;
  foreach(QString key, keys())
  {
    args += argument(key, value(key));
  }
  return args;
}

QString ModelItem::Arguments::hash() const
{
  QCryptographicHash hasher(QCryptographicHash::Sha1);

  hasher.addData(serialize().toStdString().c_str(), serialize().size());

  return QString(hasher.result().toHex());
}



//------------------------------------------------------------------------
ModelItem::Vector ModelItem::relatedItems(ModelItem::RelationType rel, const QString filter)
{
  Vector res;

  Q_ASSERT(m_relations);
  m_vertex = m_relations->vertex(this);
  if (rel == IN || rel == INOUT)
    foreach(VertexProperty v, m_relations->ancestors(m_vertex, filter))
      res << v.item;

  if (rel == OUT || rel == INOUT)
    foreach(VertexProperty v, m_relations->succesors(m_vertex, filter))
      res << v.item;

  return res;
}

//------------------------------------------------------------------------
ModelItem::RelationList ModelItem::relations(const QString filter)
{
  RelationList res;

  Q_ASSERT(m_relations);
  m_vertex = m_relations->vertex(this);
  foreach(Edge edge, m_relations->edges(m_vertex, filter))
  {
    Relation rel;
    rel.ancestor = edge.source.item;
    rel.succesor = edge.target.item;
    rel.relation = edge.relationship.c_str();
    res << rel;
  }

  qDebug() << m_vertex<<"Model Relations" << res.size();
  return res;
}


//------------------------------------------------------------------------
ModelItem* indexPtr(const QModelIndex& index)
{
  return static_cast<ModelItem *>(index.internalPointer());
}
