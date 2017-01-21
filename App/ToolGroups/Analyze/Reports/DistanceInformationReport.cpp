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

// ESPINA
#include "DistanceInformationReport.h"
#include <Dialogs/DistanceInformation/DistanceInformationDialog.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/QueryAdapter.h>

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
  return tr("Computes and reports the distance between segmentations in the stack.\n\n" \
            "The distance can be computed from centroid to centroid or from surface to surface. The report can have a single table or an individual table for each segmentation." \
            "If there aren't any selected segmentations the distances will be computed for all segmentations in the stack.");
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
void DistanceInformationReport::show(SegmentationAdapterList segmentations) const
{
  if(getModel()->segmentations().size() < 2)
  {
    DefaultDialogs::ErrorMessage(tr("There should be at least two segmentations to compute distances."));
  }
  else
  {
    DistanceInformationOptionsDialog optionsDialog(getContext());

    if (optionsDialog.exec() == QDialog::Accepted)
    {
      auto options = optionsDialog.getOptions();

      if(options.category != CategoryAdapterSPtr())
      {
        auto segmentations = QueryAdapter::segmentationsOfCategory(getModel(), options.category);
        if(segmentations.isEmpty())
        {
          DefaultDialogs::ErrorMessage(tr("There aren't any segmentations in the selected category '%1'.").arg(options.category->name()));
          return;
        }
      }

      DistanceInformationDialog dialog(segmentations, optionsDialog.getOptions(), getContext());
      dialog.exec();
    }
  }
}
