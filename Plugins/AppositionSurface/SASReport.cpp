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

#include "SASReport.h"
#include "AppositionSurfacePlugin.h"
#include "GUI/Analysis/SASReportDialog.h"

#include <Core/Analysis/Segmentation.h>
#include <Undo/AddCategoryCommand.h>

using namespace ESPINA;
using namespace ESPINA::GUI;

//----------------------------------------------------------------------------
SASReport::SASReport(Support::Context &context)
: WithContext(context)
{
}

//----------------------------------------------------------------------------
QString SASReport::name() const
{
  return tr("Synaptic Apposition Surfaces");
}

//----------------------------------------------------------------------------
QString SASReport::description() const
{
  return tr("Display the information of every synapsis and its synaptic apposition surface in the same row.\n\n"  \
            "Different types of information can be selected in the \"Select Information\" dialog in the menu and " \
            "they will be shown in separated columns.");
}

//----------------------------------------------------------------------------
SegmentationAdapterList SASReport::acceptedInput(SegmentationAdapterList segmentations) const
{
  SegmentationAdapterList result;

  for (auto segmentation : segmentations)
  {
    if (AppositionSurfacePlugin::isValidSynapse(segmentation))
    {
      result << segmentation;
    }
  }

  return result;
}

//----------------------------------------------------------------------------
QString SASReport::requiredInputDescription() const
{
  return tr("Current report input does not contain any synapses.");
}

//----------------------------------------------------------------------------
void SASReport::show(SegmentationAdapterList input) const
{
  auto dialog = new SASReportDialog(input, getContext());
  dialog->setAttribute(Qt::WA_DeleteOnClose, true);
  dialog->show();
}
