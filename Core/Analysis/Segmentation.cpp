/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) <year>  <name of author>

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
#include "Segmentation.h"
#include "Category.h"

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>

#include <QDebug>
#include <QPainter>
#include <QTextStream>
#include <QUuid>
#include <QXmlStreamReader>
#include <string>

using namespace std;
using namespace EspINA;

Segmentation::Segmentation(FilterSPtr filter, const Output::Id output)
: ViewItem(filter, output)
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
      stream.writeStartElement(extension->type());
      stream.writeAttribute("InvalidateOnChange", QString("%1").arg(extension->invalidateOnChange()));
      for(auto tag : extension->availableInformations())
      {
        stream.writeStartElement(tag);
        stream.writeCharacters(extension->information(tag).toString());
        stream.writeEndElement();
      }
      stream.writeCharacters(extension->state());
      //stream.writeEndElement();

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
  State state = QString("NUMBER=") + QString::number(m_number) + QString(";");
  QStringList usersList = m_users.toList();

  QStringList::iterator it = usersList.begin();
  state += QString("USERS=");
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
  state += QString("CATEGORY=%1;").arg(m_category?m_category->classificationName():"");

  if (!m_alias.isEmpty())
  {
    state += QString("ALIAS=%1;").arg(m_alias);
  }

  return state;
}

//------------------------------------------------------------------------
void Segmentation::restoreState(const State& state)
{
  QStringList strings = state.split(';');
  QStringList::iterator it = strings.begin();
  for (auto it = strings.begin(); it != strings.end(); ++it)
  {
    QStringList tokens = (*it).split('=');
    if (tokens.size() != 2)
      continue;

    if ("NUMBER" == tokens[0])
    {
      m_number = tokens[1].toUInt();
    } else if ("USERS" == tokens[0])
    {
      m_users = tokens[1].split(',').toSet();
    } else if ("ALIAS" == tokens[0])
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