/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "VisualizationState.h"

using namespace ESPINA;

const SegmentationExtension::Type VisualizationState::TYPE = "VisualizationState";

const std::string EXTENSION_VERSION = "1.0\n";

//------------------------------------------------------------------------
VisualizationState::VisualizationState()
: SegmentationExtension(InfoCache())
{
}

//------------------------------------------------------------------------
VisualizationState::~VisualizationState()
{
}

//------------------------------------------------------------------------
SegmentationExtension::InformationKeyList VisualizationState::availableInformation() const
{
  return InformationKeyList();
}

//------------------------------------------------------------------------
QVariant VisualizationState::information(const Key &tag) const
{
  qWarning() << TYPE << " Extension:"  << tag << " is not provided";
  return QVariant();
}

//------------------------------------------------------------------------
void VisualizationState::setState(const QString& representation, const QString& state)
{
  m_state[representation] = state;
}

//------------------------------------------------------------------------
QString VisualizationState::representationState(const QString& representation)
{
  return m_state.value(representation, QString());
}
