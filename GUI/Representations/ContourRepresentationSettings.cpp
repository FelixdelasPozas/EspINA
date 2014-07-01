/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
  m_widthCombo->addItem("Tiny");
  m_widthCombo->addItem("Small");
  m_widthCombo->addItem("Medium");
  m_widthCombo->addItem("Large");
  m_widthCombo->addItem("Huge");

  m_patternCombo->addItem("Solid");
  m_patternCombo->addItem("Dotted");
  m_patternCombo->addItem("Dashed");
}

//----------------------------------------------------------------------------
void ContourRepresentationSettings::get(RepresentationSPtr representation)
{
  ContourRepresentation *contourRepresentation = dynamic_cast<ContourRepresentation *>(representation.get());

  if (!m_init)
  {
    m_widthCombo->setCurrentIndex(contourRepresentation->lineWidth());
    m_patternCombo->setCurrentIndex(contourRepresentation->linePattern());
    m_init = true;
  }

  if (m_widthCombo->currentIndex() != contourRepresentation->lineWidth())
  {
    m_widthCombo->setCurrentIndex(2);
    m_patternCombo->setCurrentIndex(0);
  }
}

//----------------------------------------------------------------------------
void ContourRepresentationSettings::set(RepresentationSPtr representation)
{
  ContourRepresentation *contourRepresentation = dynamic_cast<ContourRepresentation *>(representation.get());

  if (m_init)
  {
    contourRepresentation->setLineWidth((ContourRepresentation::LineWidth)m_widthCombo->currentIndex());
    contourRepresentation->setLinePattern((ContourRepresentation::LinePattern)m_patternCombo->currentIndex());
  }
}
