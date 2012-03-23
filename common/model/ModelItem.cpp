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
#include "common/extensions/ModelItemExtension.h"
#include "common/model/RelationshipGraph.h"

#include <QDebug>
#include <QCryptographicHash>
#include "Representation.h"

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
      insert(argumentId(name), value);
    }
    else
    {
      buffer.append(c);
    }
  }
}

//------------------------------------------------------------------------
QString ModelItem::Arguments::serialize(bool key) const
{
  QString args;
  foreach(ArgumentId id, keys())
  {
    if (false == key || id.isKey)
      args += argument(id, value(id));
  }
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

//   qDebug() << m_vertex<<"Model Relations" << res.size();
  return res;
}

//------------------------------------------------------------------------
QStringList ModelItem::availableInformations() const
{
  QStringList informations;
  foreach (ModelItemExtension *ext, m_insertionOrderedExtensions)
    informations << ext->availableInformations();

  return informations;
}

//------------------------------------------------------------------------
QStringList ModelItem::availableRepresentations() const
{
  QStringList representations;
  foreach (ModelItemExtension *ext, m_insertionOrderedExtensions)
    representations << ext->availableRepresentations();

  return representations;
}

//------------------------------------------------------------------------
QVariant ModelItem::information(QString name) const
{
  Q_ASSERT(m_informations.contains(name));
  return m_informations[name]->information(name);
}

//------------------------------------------------------------------------
Representation* ModelItem::representation(QString name) const
{
  Q_ASSERT(m_representations.contains(name));
  return m_representations[name];
}

//------------------------------------------------------------------------
ModelItemExtension* ModelItem::extension(QString name) const
{
  return m_extensions.value(name, NULL);
}


//------------------------------------------------------------------------
void ModelItem::addExtension(ModelItemExtension *ext)
{
  if (m_extensions.contains(ext->id()))
  {
     qWarning() << "Extension already registered";
     Q_ASSERT(false);
  }

  bool hasDependencies = true;
  foreach(QString reqExtId, ext->dependencies())
    hasDependencies = hasDependencies && m_extensions.contains(reqExtId);

  if (hasDependencies)
  {
    m_extensions.insert(ext->id(),ext);
    m_insertionOrderedExtensions << ext;
//     foreach(ISegmentationRepresentation::RepresentationId rep, ext->availableRepresentations())
//       m_representations.insert(rep, ext);
    foreach(QString info, ext->availableInformations())
    {
      m_informations.insert(info, ext);
//       EXTENSION_DEBUG("New Information: " << info);
    }
    // Try to satisfy pending extensions
    foreach(ModelItemExtension *pending, m_pendingExtensions)
      addExtension(pending);
  } 
  else
  {
    if (!m_pendingExtensions.contains(ext->id()))
      m_pendingExtensions.insert(ext->id(),ext);
  }
}

//------------------------------------------------------------------------
ModelItem* indexPtr(const QModelIndex& index)
{
  return static_cast<ModelItem *>(index.internalPointer());
}
