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
QString ModelItem::serialize() const
{
  return QString("none");
}


//------------------------------------------------------------------------
ModelItemList ModelItem::relatedItems(RelationType rel, const QString &filter)
{
  ModelItemList res;

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
ModelItem::RelationList ModelItem::relations(const QString &filter)
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
  foreach (ModelItemExtensionPtr ext, m_insertionOrderedExtensions)
    informations << ext->availableInformations();

  return informations;
}

//------------------------------------------------------------------------
QStringList ModelItem::availableRepresentations() const
{
  QStringList representations;
  foreach (ModelItemExtensionPtr ext, m_insertionOrderedExtensions)
    representations << ext->availableRepresentations();

  return representations;
}

//------------------------------------------------------------------------
QVariant ModelItem::information(const QString &name)
{
  Q_ASSERT(m_informations.contains(name));
  return m_informations[name]->information(name);
}

//------------------------------------------------------------------------
ModelItem::RepresentationPtr ModelItem::representation(const QString& name) const
{
  Q_ASSERT(m_representations.contains(name));
  return m_representations[name];
}

//------------------------------------------------------------------------
ModelItemExtensionPtr ModelItem::extension(const QString &name) const
{
  return m_extensions.value(name, ModelItemExtensionPtr());
}


//------------------------------------------------------------------------
void ModelItem::addExtension(ModelItemExtensionPtr ext)
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
    foreach(ModelItemExtensionPtr pending, m_pendingExtensions)
      addExtension(pending);
  } 
  else
  {
    if (!m_pendingExtensions.contains(ext->id()))
      m_pendingExtensions.insert(ext->id(),ext);
  }
}

//------------------------------------------------------------------------
void ModelItem::deleteExtension(ModelItemExtensionPtr ext)
{
  Q_ASSERT(false);
}

//------------------------------------------------------------------------
void ModelItem::deleteExtensions()
{
  Q_ASSERT(false);
}

//------------------------------------------------------------------------
ModelItemPtr EspINA::indexPtr(const QModelIndex& index)
{
  ModelItemPtr ptr = *(static_cast<ModelItemPtr *>(index.internalPointer()));
  Q_ASSERT(!ptr.isNull());

  return ptr;
}
