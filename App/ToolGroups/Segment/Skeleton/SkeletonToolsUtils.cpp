/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "SkeletonToolsUtils.h"

// Qt
#include <QString>
#include <QObject>

using namespace ESPINA;
using namespace ESPINA::Core;

QMap<const QString, Core::SkeletonStrokes> SkeletonToolsUtils::STROKES;

//--------------------------------------------------------------------
Core::SkeletonStrokes ESPINA::SkeletonToolsUtils::defaultStrokes(const CategoryAdapterSPtr category)
{
  SkeletonStrokes result;

  if(category)
  {
    auto hue = category->color().hue();

    if(category->classificationName().startsWith("Axon"))
    {
      result << SkeletonStroke(QObject::tr("Shaft"), hue, 0, true);
      result << SkeletonStroke(QObject::tr("Synapse en passant"), hue, 1, false);

      return result;
    }

    if(category->classificationName().startsWith("Dendrite"))
    {
      result << SkeletonStroke(QObject::tr("Shaft"), hue, 0, true);
      result << SkeletonStroke(QObject::tr("Spine"), hue, 0, true);
      result << SkeletonStroke(QObject::tr("Synapse on shaft"), hue, 1, false);
      result << SkeletonStroke(QObject::tr("Synapse on spine head"), hue, 1, false);

      return result;
    }

    result << SkeletonStroke(QObject::tr("Stroke"), hue, 0, true);
  }

  return result;
}

//--------------------------------------------------------------------
void ESPINA::SkeletonToolsUtils::saveStrokes(std::shared_ptr<QSettings> settings)
{
  settings->beginGroup("SkeletonStrokes");

  for(auto category: STROKES.keys())
  {
    settings->beginGroup(category);

    for(auto stroke: STROKES[category])
    {
      settings->setValue(stroke.name, QVariant::fromValue(stroke));
    }

    settings->endGroup();
  }

  settings->endGroup();
}

//--------------------------------------------------------------------
void ESPINA::SkeletonToolsUtils::loadStrokes(std::shared_ptr<QSettings> settings)
{
  STROKES.clear();

  settings->beginGroup("SkeletonStrokes");

  for(auto group: settings->childGroups())
  {
    settings->beginGroup(group);

    for(auto key: settings->childKeys())
    {
      QVariant value = settings->value(key);

      STROKES[group] << value.value<ESPINA::Core::SkeletonStroke>();
    }

    settings->endGroup();
  }

  settings->endGroup();
}