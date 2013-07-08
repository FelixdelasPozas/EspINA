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

#include "SliceRepresentationSettings.h"
#include "SliceRepresentation.h"

using namespace EspINA;

//----------------------------------------------------------------------------
SegmentationSliceRepresentationSettings::SegmentationSliceRepresentationSettings()
: m_init(false)
{
  setupUi(this);
}

//----------------------------------------------------------------------------
void SegmentationSliceRepresentationSettings::Get(GraphicalRepresentationSPtr representation)
{
  SegmentationSliceRepresentation *sliceRepresentation = dynamic_cast<SegmentationSliceRepresentation *>(representation.get());

  int opacity = int(sliceRepresentation->color().alphaF()*100);

  if (!m_init)
  {
    m_opacity->setValue(opacity);
    m_init = true;
  }

  if (m_opacity->value() != opacity)
  {
    m_opacity->setMinimum(-1);
    m_opacity->setValue(-1);
    m_opacity->setSpecialValueText(" ");
  }
}

//----------------------------------------------------------------------------
void SegmentationSliceRepresentationSettings::Set(GraphicalRepresentationSPtr representation)
{
  SegmentationSliceRepresentation *sliceRepresentation = dynamic_cast<SegmentationSliceRepresentation *>(representation.get());

  if (m_init)
  {
    if (m_opacity->value() > 0)
    {
      QColor color = sliceRepresentation->color();
      color.setAlphaF(m_opacity->value()/100.0);
      sliceRepresentation->setColor(color);
    }
  }
}