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
  return tr("Create a table with the segmentation information.\n\n" \
            "Different types of information can be selected in the \"Select Information\" dialog in the menu and they will be shown in separated columns.");
}

//----------------------------------------------------------------------------
QPixmap RawInformationReport::preview() const
{
  return QPixmap(":/espina/preview_raw_information.png");
}

//----------------------------------------------------------------------------
SegmentationAdapterList RawInformationReport::acceptedInput(SegmentationAdapterList segmentations) const
{
  return segmentations;
}

//----------------------------------------------------------------------------
QString RawInformationReport::requiredInputDescription() const
{
  return tr("Current report input doesn't contain segmentations.");
}

//----------------------------------------------------------------------------
void RawInformationReport::show(SegmentationAdapterList input) const
{
  auto dialog = new RawInformationDialog(input, getContext());

  connect(dialog, SIGNAL(finished(int)),
          dialog, SLOT(deleteLater()));

  dialog->show();
}
