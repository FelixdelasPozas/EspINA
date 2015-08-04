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

#include "RawInformationReport.h"
#include <Dialogs/RawInformation/RawInformationDialog.h>
#include <QPixmap>

using namespace ESPINA;
using namespace ESPINA::GUI;

//----------------------------------------------------------------------------
RawInformationReport::RawInformationReport(Support::Context &context)
: WithContext(context)
{

}

//----------------------------------------------------------------------------
QString RawInformationReport::name() const
{
  return tr("Raw Information");
}

//----------------------------------------------------------------------------
QString RawInformationReport::description() const
{
  return tr("Tabular report of the information available for each structure.");
}

//----------------------------------------------------------------------------
QPixmap RawInformationReport::preview() const
{
  return QPixmap();
}

//----------------------------------------------------------------------------
void RawInformationReport::show() const
{
  if (getModel()->segmentations().isEmpty())
  {
    auto title   = tr("Raw Information");
    auto message = tr("Current analysis does not contain any segmentations");

    DefaultDialogs::InformationMessage(title, message);
  }
  else
  {
    auto dialog = new RawInformationDialog(context());

    connect(dialog, SIGNAL(finished(int)),
            dialog, SLOT(deleteLater()));

    dialog->show();
  }
}