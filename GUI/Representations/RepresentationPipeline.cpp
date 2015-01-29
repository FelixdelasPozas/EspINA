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
bool RepresentationPipeline::Settings::hasPendingChanges() const
{
  for (auto pair : m_properties)
  {
    if (pair.second) return true;
  }

  return false;
}

//----------------------------------------------------------------------------
void RepresentationPipeline::Settings::apply(const Settings &settings)
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
void RepresentationPipeline::Settings::commit()
{
  for (auto pair : m_properties)
  {
    pair.second = false;
  }
}

//----------------------------------------------------------------------------
RepresentationPipeline::RepresentationPipeline(Type type)
: m_type(type)
{
  setState<TimeStamp>(TIME_STAMP, VTK_UNSIGNED_LONG_LONG_MAX);
}

//----------------------------------------------------------------------------
void RepresentationPipeline::updateState(const Settings &settings)
{
  m_state.apply(settings);
}

//----------------------------------------------------------------------------
bool RepresentationPipeline::isModified(const QString &tag)
{
  return m_state.isModified(tag);
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
void RepresentationPipeline::setCrosshairPoint(const NmVector3 &point)
{
  m_state.setValue<double>(CROSSHAIR_X, point[0]);
  m_state.setValue<double>(CROSSHAIR_Y, point[1]);
  m_state.setValue<double>(CROSSHAIR_Z, point[2]);
}

//----------------------------------------------------------------------------
NmVector3 RepresentationPipeline::crosshairPoint() const
{
  NmVector3   crosshair;

  crosshair[0] = m_state.getValue<double>(CROSSHAIR_X);
  crosshair[1] = m_state.getValue<double>(CROSSHAIR_Y);
  crosshair[2] = m_state.getValue<double>(CROSSHAIR_Z);

  return crosshair;
}

//----------------------------------------------------------------------------
Nm RepresentationPipeline::crosshairPosition(const Plane &plane) const
{
  switch (plane)
  {
    case Plane::XY:
      return m_state.getValue<double>(CROSSHAIR_Z);
    case Plane::XZ:
      return m_state.getValue<double>(CROSSHAIR_Y);
    case Plane::YZ:
      return m_state.getValue<double>(CROSSHAIR_X);
    default:
      qWarning() << "Unexpected crosshair plane";
  }

  return 0;
}

//----------------------------------------------------------------------------
bool RepresentationPipeline::isCrosshairPointModified() const
{
  return m_state.isModified(CROSSHAIR_X)
      || m_state.isModified(CROSSHAIR_Y)
      || m_state.isModified(CROSSHAIR_Z);
}

//----------------------------------------------------------------------------
bool RepresentationPipeline::isCrosshairPositionModified(const Plane &plane) const
{
  switch (plane)
  {
    case Plane::XY:
      return m_state.isModified(CROSSHAIR_Z);
    case Plane::XZ:
      return m_state.isModified(CROSSHAIR_Y);
    case Plane::YZ:
      return m_state.isModified(CROSSHAIR_X);
    default:
      qWarning() << "Unexpected crosshair plane";
  }

  return false;
}


//----------------------------------------------------------------------------
bool RepresentationPipeline::applySettings(const RepresentationPipeline::Settings &settings)
{
  QWriteLocker lock(&m_stateLock);
  applySettingsImplementation(settings);

  return m_state.hasPendingChanges();
}

//----------------------------------------------------------------------------
void RepresentationPipeline::update()
{
  QWriteLocker lock(&m_stateLock);
  if (updateImplementation())
  {
    m_state.commit();
  }
}
