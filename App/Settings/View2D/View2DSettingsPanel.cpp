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
#include "View2DSettingsPanel.h"

using namespace ESPINA;
using namespace ESPINA::Support::Settings;

//------------------------------------------------------------------------
View2DSettingsPanel::View2DSettingsPanel(View2D *view)
: m_view{view}
{
  setupUi(this);

  invertSliceOrder->setVisible(false);

  invertSliceOrder->setChecked(view->invertSliceOrder());
  invertWheel->setChecked(view->invertWheel());
  //showAxis->setChecked(view->showAxis());
  showAxis->setVisible(false);
}

//------------------------------------------------------------------------
const QString View2DSettingsPanel::shortDescription()
{
  switch (m_view->plane())
  {
    case Plane::XY:
      return QString("XY Slice View");
    case Plane::XZ:
      return QString("XZ Slice View");
    case Plane::YZ:
      return QString("YZ Slice View");
    default:
      Q_ASSERT(false);
      break;
  }
  return QString("Unknown");
}

//------------------------------------------------------------------------
void View2DSettingsPanel::acceptChanges()
{
  m_view->setInvertSliceOrder(invertSliceOrder->isChecked());
  m_view->setInvertWheel(invertWheel->isChecked());
  //m_view->setShowAxis(showAxis->isChecked());
}

//------------------------------------------------------------------------
void View2DSettingsPanel::rejectChanges()
{
}

//------------------------------------------------------------------------
bool View2DSettingsPanel::modified() const
{
  return invertSliceOrder->isChecked() != m_view->invertSliceOrder()
      || invertWheel->isChecked()      != m_view->invertWheel();
      //|| showAxis->isChecked()         != m_view->showAxis();
}

//------------------------------------------------------------------------
SettingsPanelPtr View2DSettingsPanel::clone()
{
  return new View2DSettingsPanel(m_view);
}
