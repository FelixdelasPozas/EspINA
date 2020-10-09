/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include <App/ToolGroups/Visualize/DayNightTool.h>
#include <App/EspinaMainWindow.h>

// Qt
#include <QApplication>

using namespace ESPINA;

const QString StateOnTooltip = QString("Set the visual theme to light.");
const QString StateOffTooltip = QString("Set the visual theme to dark.");

//--------------------------------------------------------------------
DayNightTool::DayNightTool(Support::Context &context)
: GenericTogglableTool{tr("Visual theme switcher"), ":/espina/day-and-night.svg", StateOffTooltip, context}
{
  setShortcut(Qt::Key_F12);

  connect(this, SIGNAL(toggled(bool)), this, SLOT(onToggled(bool)));
}

//--------------------------------------------------------------------
void DayNightTool::onToggled(bool value)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QString sheet;
  if(!value)
  {
    setToolTip(StateOffTooltip);
  }
  else
  {
    setToolTip(StateOnTooltip);
    QFile file(":qdarkstyle/style.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&file);
    sheet = ts.readAll();
  }

  qApp->setStyleSheet(sheet);

  QApplication::restoreOverrideCursor();
}
