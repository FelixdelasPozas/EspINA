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

// ESPINA
#include "Segmentation.h"
#include "Category.h"
#include "Data/SkeletonData.h"

// VTK
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>

// Qt
#include <QPainter>
#include <QTextStream>
#include <QUuid>
#include <QXmlStreamWriter>

// C++
#include <string>

using namespace std;
using namespace ESPINA;

const QString STATE_NUMBER   = "Number";
const QString STATE_ALIAS    = "Alias";
const QString STATE_USERS    = "Users";
const QString STATE_CATEGORY = "Category";

//------------------------------------------------------------------------
Segmentation::Segmentation(InputSPtr input)
: ViewItem  (input)
, Extensible(this)
, m_number  {0}
, m_users   {QSet<QString>()}
, m_category{nullptr}
{
}

//------------------------------------------------------------------------
Segmentation::~Segmentation()
{
  m_category = nullptr;
}

//------------------------------------------------------------------------
Snapshot Segmentation::snapshot() const
{
  Snapshot snapshot;

  auto extensions = readOnlyExtensions();

  if (!extensions->isEmpty())
  {
    QByteArray xml;

    QXmlStreamWriter stream(&xml);

    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("Segmentation");
    stream.writeAttribute("Name", name());

    for(auto type: extensions->available())
    {
      auto extension = extensions[type];

      if(!extension)
      {
        qWarning() << "Segmentation::snapshot() -> Couldn't save " << type << "extension: null pointer. Name:" << name();
        continue;
      }

      stream.writeStartElement("Extension");
      stream.writeAttribute("Type", type);
      stream.writeAttribute("InvalidateOnChange", QString("%1").arg(extension->invalidateOnChange()));
      for(auto key : extension->readyInformation())
      {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out << extension->information(key);

        stream.writeStartElement("Info");
        stream.writeAttribute("Name", key.value());
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
        auto file = extensionDataPath(extension, data.first);
        snapshot << SnapshotData(file, data.second);
      }
    }
    stream.writeEndElement();
    stream.writeEndDocument();

    auto file = extensionsPath() + QString("%1.xml").arg(uuid());
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
    if (tokens.size() != 2) continue;

    if (STATE_NUMBER == tokens[0])
    {
      m_number = tokens[1].toUInt();
    }
    else
    {
      if (STATE_USERS == tokens[0])
      {
        m_users = tokens[1].split(',').toSet();
      }
      else
      {
        if (STATE_ALIAS == tokens[0])
        {
          m_alias = tokens[1];
        }
      }
    }
  }
}

//------------------------------------------------------------------------
void Segmentation::unload()
{
  // NOTE: Future versions?
}

//------------------------------------------------------------------------
void Segmentation::setCategory(CategorySPtr category)
{
  auto oldCategory = m_category;
  m_category = category;

  if(oldCategory && m_category && hasSkeletonData(output()))
  {
    auto oldHue = oldCategory->color();
    auto newHue = m_category->color();

    auto data     = writeLockSkeleton(output());
    auto skeleton = data->skeleton();

    auto strokeColors = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeColor"));
    for(int i = 0; i < strokeColors->GetNumberOfTuples(); ++i)
    {
      if(strokeColors->GetValue(i) == oldHue)
      {
        strokeColors->SetValue(i, newHue);
      }
    }

    data->setSkeleton(skeleton);
  }
}
