/*
 
 Copyright (C) <year>  <name of author>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include "Segmentation.h"
#include "Category.h"

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>

#include <QDebug>
#include <QPainter>
#include <QTextStream>
#include <QUuid>
#include <QXmlStreamWriter>
#include <string>

using namespace std;
using namespace EspINA;

const QString STATE_NUMBER   = "Number";
const QString STATE_ALIAS    = "Alias";
const QString STATE_USERS    = "Users";
const QString STATE_CATEGORY = "Category";

Segmentation::Segmentation(InputSPtr input)
: ViewItem(input)
, m_number{0}
, m_users{QSet<QString>()}
, m_category{nullptr}
{
}

//------------------------------------------------------------------------
Segmentation::~Segmentation()
{
  m_category = nullptr;

//   this->output()->markToSave(false);

  for (auto extension: m_extensions)
    extension = nullptr;

  m_extensions.clear();
}

//------------------------------------------------------------------------
void Segmentation::addExtension(SegmentationExtensionSPtr extension) throw (SegmentationExtension::Existing_Extension)
{
  if (m_extensions.contains(extension->type()))
    throw SegmentationExtension::Existing_Extension();

  extension->setExtendedItem(this);

  m_extensions.insert(extension->type(), extension);
}

//------------------------------------------------------------------------
void Segmentation::deleteExtension(SegmentationExtensionSPtr extension) throw (SegmentationExtension::Extension_Not_Found)
{
  if (!m_extensions.contains(extension->type()))
    throw SegmentationExtension::Extension_Not_Found();

  m_extensions.remove(extension->type());
}

//------------------------------------------------------------------------
SegmentationExtensionSPtr Segmentation::extension(const SegmentationExtension::Type& type) const throw(SegmentationExtension::Extension_Not_Found)
{
  if (!m_extensions.contains(type))
  {
    throw SegmentationExtension::Extension_Not_Found();
  }

  return m_extensions.value(type, SegmentationExtensionSPtr());
}

//------------------------------------------------------------------------
bool Segmentation::hasExtension(const SegmentationExtension::Type& type) const
{
  return m_extensions.keys().contains(type);
}

//------------------------------------------------------------------------
QVariant Segmentation::information(const SegmentationExtension::InfoTag& tag) const
{
  for(auto extension: m_extensions.values())
  {
    if (extension->availableInformations().contains(tag))
    {
      return extension->information(tag);
    }
  }

  return QVariant();
}

//------------------------------------------------------------------------
bool Segmentation::isInformationReady(const QString& tag) const
{
  for(auto extension: m_extensions.values())
  {
    if (extension->availableInformations().contains(tag))
    {
      return extension->readyInformation().contains(tag);
    }
  }

  return false;
}

//------------------------------------------------------------------------
SegmentationExtension::InfoTagList Segmentation::informationTags() const
{
  SegmentationExtension::InfoTagList list;

  for (auto extension: m_extensions.values())
    list << extension->availableInformations();

  return list;
}

//------------------------------------------------------------------------
Snapshot Segmentation::snapshot() const
{
  Snapshot snapshot;

  if (!m_extensions.isEmpty())
  {
    QByteArray xml;

    QXmlStreamWriter stream(&xml);

    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("Segmentation");
    stream.writeAttribute("Name", name());
    for(auto extension : m_extensions)
    {
      stream.writeStartElement("Extension");
      stream.writeAttribute("Type", extension->type());
      stream.writeAttribute("InvalidateOnChange", QString("%1").arg(extension->invalidateOnChange()));
      for(auto tag : extension->readyInformation())
      {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out << extension->information(tag);

        stream.writeStartElement("Info");
        stream.writeAttribute("Name", tag);
        stream.writeCharacters(data.toBase64());
        stream.writeEndElement();
      }
      auto state = extension->state();
      if (!state.isEmpty())
      {
        stream.writeStartElement("State");
        stream.writeCharacters(state);
        stream.writeEndElement();
      }
      stream.writeEndElement();

      for(auto data: extension->snapshot())
      {
        QString file = extensionDataPath(extension, data.first);
        snapshot << SnapshotData(file, data.second);
      }
    }
    stream.writeEndElement();
    stream.writeEndDocument();

    QString file = extensionsPath() + QString("%1.xml").arg(uuid());
    snapshot << SnapshotData(file, xml);
  }

  return snapshot;
}

//------------------------------------------------------------------------
State Segmentation::state() const
{
  State state = QString("%1=%2;").arg(STATE_NUMBER).arg(m_number);
  QStringList usersList = m_users.toList();

  QStringList::iterator it = usersList.begin();
  state += QString("%1=").arg(STATE_USERS);
  while (it != usersList.end())
  {
    state += (*it);
    ++it;
    if (it != usersList.end())
      state += QString("/");
  }
  state += QString(";");

//   if (output())
//     state += QString("OUTPUT=%1;").arg(output()->id());
  state += QString("%1=%2;").arg(STATE_CATEGORY).arg(m_category?m_category->classificationName():"");

  if (!m_alias.isEmpty())
  {
    state += QString("%1=%2;").arg(STATE_ALIAS).arg(m_alias);
  }

  return state;
}

//------------------------------------------------------------------------
void Segmentation::restoreState(const State& state)
{
  for (auto token : state.split(';'))
  {
    QStringList tokens = token.split('=');
    if (tokens.size() != 2)
      continue;

    if (STATE_NUMBER == tokens[0])
    {
      m_number = tokens[1].toUInt();
    } else if (STATE_USERS == tokens[0])
    {
      m_users = tokens[1].split(',').toSet();
    } else if (STATE_ALIAS == tokens[0])
    {
      m_alias = tokens[1];
    }
  }
}

//------------------------------------------------------------------------
void Segmentation::unload()
{
  // TODO
}

//------------------------------------------------------------------------
void Segmentation::setCategory(CategorySPtr category)
{
  m_category = category;
}
