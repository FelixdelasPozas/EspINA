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

// ESPINA
#include "ContourRepresentationSettings.h"
#include "ContourRepresentation.h"

using namespace ESPINA;

//----------------------------------------------------------------------------
int WidthToInt(ContourRepresentation::LineWidth width)
{
	switch(width)
	{
		case ContourRepresentation::LineWidth::huge:
			return 4;
			break;
		case ContourRepresentation::LineWidth::large:
			return 3;
			break;
		case ContourRepresentation::LineWidth::medium:
			return 2;
			break;
		case ContourRepresentation::LineWidth::small:
			return 1;
			break;
		case ContourRepresentation::LineWidth::tiny:
			return 0;
			break;
		default:
			break;
	}
	return 2;
}

//----------------------------------------------------------------------------
int PatternToInt(ContourRepresentation::LinePattern pattern)
{
	switch(pattern)
	{
		case ContourRepresentation::LinePattern::dashed:
			return 2;
			break;
		case ContourRepresentation::LinePattern::dotted:
			return 1;
			break;
		case ContourRepresentation::LinePattern::normal:
			return 0;
			break;
		default:
			break;
	}
	return 0;
}

//----------------------------------------------------------------------------
ContourRepresentation::LineWidth IntToWidth(int width)
{
	switch(width)
	{
		case 4:
			return ContourRepresentation::LineWidth::huge;
			break;
		case 3:
			return ContourRepresentation::LineWidth::large;
			break;
		case 2:
			return ContourRepresentation::LineWidth::medium;
			break;
		case 1:
			return ContourRepresentation::LineWidth::small;
			break;
		case 0:
			return ContourRepresentation::LineWidth::tiny;
			break;
		default:
			break;
	}
	return ContourRepresentation::LineWidth::medium;
}

//----------------------------------------------------------------------------
ContourRepresentation::LinePattern IntToPattern(int pattern)
{
	switch(pattern)
	{
		case 2:
			return ContourRepresentation::LinePattern::dashed;
			break;
		case 1:
			return ContourRepresentation::LinePattern::dotted;
			break;
		case 0:
			return ContourRepresentation::LinePattern::normal;
			break;
		default:
			break;
	}
	return ContourRepresentation::LinePattern::normal;
}

//----------------------------------------------------------------------------
ContourRepresentationSettings::ContourRepresentationSettings()
: m_init{false}
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
    m_widthCombo->setCurrentIndex(WidthToInt(contourRepresentation->lineWidth()));
    m_patternCombo->setCurrentIndex(PatternToInt(contourRepresentation->linePattern()));
    m_init = true;
  }

  if (m_widthCombo->currentIndex() != WidthToInt(contourRepresentation->lineWidth()))
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
    contourRepresentation->setLineWidth(IntToWidth(m_widthCombo->currentIndex()));
    contourRepresentation->setLinePattern(IntToPattern(m_patternCombo->currentIndex()));
  }
}
