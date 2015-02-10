/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "RepresentationPipeline.h"
#include "App/ToolGroups/View/Representations/RepresentationSettings.h"
#include <Core/EspinaTypes.h>

#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Representations;

//----------------------------------------------------------------------------
bool RepresentationPipeline::State::hasPendingChanges() const
{
  for (auto pair : m_properties)
  {
    if (pair.second) return true;
  }

  return false;
}

//----------------------------------------------------------------------------
void RepresentationPipeline::State::apply(const State &settings)
{
  for (auto key : settings.m_properties.keys())
  {
    auto pair    = settings.m_properties[key];
    auto oldPair = m_properties.value(key, Pair(QVariant(), false));

    oldPair.second |= pair.first!= oldPair.first;

    if (oldPair.second)
    {
       oldPair.first = pair.first;
       m_properties[key] = oldPair;
    }
  }
}

//----------------------------------------------------------------------------
void RepresentationPipeline::State::commit()
{
  for (auto &pair : m_properties)
  {
    pair.second = false;
  }
}

//----------------------------------------------------------------------------
RepresentationPipeline::RepresentationPipeline(Type type)
: m_type(type)
{
}

//----------------------------------------------------------------------------
QString RepresentationPipeline::serializeSettings()
{
  return QString();
}

//----------------------------------------------------------------------------
void RepresentationPipeline::restoreSettings(QString settings)
{

}

//----------------------------------------------------------------------------
void ESPINA::setCrosshairPoint(const NmVector3 &point,
                               RepresentationPipeline::State &state)
{
  state.setValue<double>(CROSSHAIR_X, point[0]);
  state.setValue<double>(CROSSHAIR_Y, point[1]);
  state.setValue<double>(CROSSHAIR_Z, point[2]);
}

//----------------------------------------------------------------------------
NmVector3 ESPINA::crosshairPoint(const RepresentationPipeline::State &settings)
{
  NmVector3   crosshair;

  crosshair[0] = settings.getValue<double>(CROSSHAIR_X);
  crosshair[1] = settings.getValue<double>(CROSSHAIR_Y);
  crosshair[2] = settings.getValue<double>(CROSSHAIR_Z);

  return crosshair;
}

//----------------------------------------------------------------------------
Nm ESPINA::crosshairPosition(const Plane &plane, const RepresentationPipeline::State &settings)
{
  switch (plane)
  {
    case Plane::XY:
      return settings.getValue<double>(CROSSHAIR_Z);
    case Plane::XZ:
      return settings.getValue<double>(CROSSHAIR_Y);
    case Plane::YZ:
      return settings.getValue<double>(CROSSHAIR_X);
    default:
      qWarning() << "Unexpected crosshair plane";
  }

  return 0;
}

//----------------------------------------------------------------------------
bool ESPINA::isCrosshairPointModified(const RepresentationPipeline::State &settings)
{
  return settings.isModified(CROSSHAIR_X)
      || settings.isModified(CROSSHAIR_Y)
      || settings.isModified(CROSSHAIR_Z);
}

//----------------------------------------------------------------------------
bool ESPINA::isCrosshairPositionModified(const Plane &plane,
                                         const RepresentationPipeline::State &settings)
{
  switch (plane)
  {
    case Plane::XY:
      return settings.isModified(CROSSHAIR_Z);
    case Plane::XZ:
      return settings.isModified(CROSSHAIR_Y);
    case Plane::YZ:
      return settings.isModified(CROSSHAIR_X);
    default:
      qWarning() << "Unexpected crosshair plane";
  }

  return false;
}