/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "SliceViewSettingsPanel.h"

SliceViewSettingsPanel::SliceViewSettingsPanel(SliceView::SettingsPtr settings)
: m_settings(settings)
{
  setupUi(this);

  invertSliceOrder->setChecked(settings->invertSliceOrder());
  invertWheel->setChecked(settings->invertWheel());
  showAxis->setChecked(settings->showAxis());
  showAxis->setVisible(false);
}


//------------------------------------------------------------------------
const QString SliceViewSettingsPanel::shortDescription()
{
  switch (m_settings->plane())
  {
    case vtkSliceView::AXIAL:
      return QString("XY Slice View");
    case vtkSliceView::SAGITTAL:
      return QString("YZ Slice View");
    case vtkSliceView::CORONAL:
      return QString("XZ Slice View");
    default:
      Q_ASSERT(false);
  }
  return QString("Unknown");
}

//------------------------------------------------------------------------
void SliceViewSettingsPanel::acceptChanges()
{
  m_settings->setInvertSliceOrder(invertSliceOrder->isChecked());
  m_settings->setInvertWheel(invertWheel->isChecked());
  m_settings->setShowAxis(showAxis->isChecked());
}

//------------------------------------------------------------------------
bool SliceViewSettingsPanel::modified() const
{
  return invertSliceOrder->isChecked() != m_settings->invertSliceOrder()
  || invertWheel->isChecked() != m_settings->invertWheel()
  || showAxis->isChecked() != m_settings->showAxis();
}

//------------------------------------------------------------------------
ISettingsPanel* SliceViewSettingsPanel::clone()
{
  return new SliceViewSettingsPanel(m_settings);
}

