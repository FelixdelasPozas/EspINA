/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "ContourRepresentationSettings.h"
#include "ContourRepresentation.h"

using namespace EspINA;

//----------------------------------------------------------------------------
ContourRepresentationSettings::ContourRepresentationSettings()
: m_init(false)
{
  setupUi(this);
  m_borderWidth->setEnabled(false);
}

//----------------------------------------------------------------------------
void ContourRepresentationSettings::Get(GraphicalRepresentationSPtr representation)
{
  ContourRepresentation *contourRepresentation = dynamic_cast<ContourRepresentation *>(representation.get());

  if (!m_init)
  {
    m_borderWidth->setValue(contourRepresentation->lineWidth());
    m_init = true;
  }

  if (m_borderWidth->value() != contourRepresentation->lineWidth())
  {
    m_borderWidth->setMinimum(0);
    m_borderWidth->setValue(0);
    m_borderWidth->setSpecialValueText(" ");
  }
}

//----------------------------------------------------------------------------
void ContourRepresentationSettings::Set(GraphicalRepresentationSPtr representation)
{
  ContourRepresentation *contourRepresentation = dynamic_cast<ContourRepresentation *>(representation.get());

  if (m_init)
  {
    if (m_borderWidth->value() > 0)
      contourRepresentation->setLineWidth(m_borderWidth->value());
  }
}
