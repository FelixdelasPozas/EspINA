/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
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

#include "DistanceInformationReport.h"
#include <Dialogs/DistanceInformation/DistanceInformationDialog.h>

using namespace ESPINA;
using namespace ESPINA::GUI;

//----------------------------------------------------------------------------
DistanceInformationReport::DistanceInformationReport(Support::Context& context)
: WithContext(context)
{
}

//----------------------------------------------------------------------------
QString DistanceInformationReport::name() const
{
  return tr("Distance Information");
}

//----------------------------------------------------------------------------
QString DistanceInformationReport::description() const
{
  //TODO
  return tr("Create a table with the segmentation information.\n\n" \
              "Different types of information can be selected in the \"Select Information\" dialog in the menu and they will be shown in separated columns.");
}

//----------------------------------------------------------------------------
QPixmap DistanceInformationReport::preview() const
{
  //TODO
  return QPixmap(":/espina/preview_raw_information.png");
}

//----------------------------------------------------------------------------
SegmentationAdapterList DistanceInformationReport::acceptedInput(SegmentationAdapterList segmentations) const
{
  return segmentations;
}

//----------------------------------------------------------------------------
QString DistanceInformationReport::requiredInputDescription() const
{
  return QString();
}

//----------------------------------------------------------------------------
void DistanceInformationReport::show(SegmentationAdapterList input) const
{
  auto optionsDialog = std::make_shared<DistanceInformationOptionsDialog>();
  if (optionsDialog->exec() == QDialog::Rejected)
      return;

  auto options = optionsDialog->getOptions();

  auto dialog = std::make_shared<DistanceInformationDialog>(input, options, getContext());
  dialog->show();
}
