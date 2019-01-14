/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <App/ToolGroups/Analyze/Reports/SpineInformationReport.h>
#include <App/Dialogs/SpinesInformation/SpinesInformationDialog.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/Utils/ListUtils.hxx>
#include <Extensions/SkeletonInformation/DendriteInformation.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Dialogs/DefaultDialogs.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;

//--------------------------------------------------------------------
SpineInformationReport::SpineInformationReport(Support::Context& context)
: Support::WithContext(context)
{
}

//--------------------------------------------------------------------
QString SpineInformationReport::name() const
{
  return tr("Spines information");
}

//--------------------------------------------------------------------
QString SpineInformationReport::description() const
{
  return tr("Report of the spines of all the dendrites.");
}

//--------------------------------------------------------------------
SegmentationAdapterList SpineInformationReport::acceptedInput(SegmentationAdapterList segmentations) const
{
  SegmentationAdapterList valid;

  for(auto segmentation: segmentations)
  {
    if(segmentation->category()->classificationName().startsWith("Dendrite", Qt::CaseInsensitive))
    {
      valid << segmentation;
    }
  }

  return valid;
}

//--------------------------------------------------------------------
QString SpineInformationReport::requiredInputDescription() const
{
  return tr("Current report input does not contain any dendrites.");
}

//--------------------------------------------------------------------
void SpineInformationReport::show(SegmentationAdapterList input) const
{
  SegmentationAdapterList valid;

  {
    WaitingCursor cursor;

    if(input.isEmpty())
    {
      valid = acceptedInput(toRawList<SegmentationAdapter>(getModel()->segmentations()));
    }
    else
    {
      valid = acceptedInput(input);
    }

    if(valid.isEmpty())
    {
      QString message;
      if(input.isEmpty())
      {
        message = tr("There are no dendrites in the current session.");
      }
      else
      {
        message = tr("The current selection contains no dendrites.");
      }

      auto details = tr("The report will be empty if there are no spines.");
      DefaultDialogs::ErrorMessage(message, DefaultDialogs::DefaultTitle(), details);
      return;
    }
  }

  int hasSpines = false;
  for(auto segmentation: valid)
  {
    auto extension = retrieveOrCreateSegmentationExtension<DendriteSkeletonInformation>(segmentation, getFactory());
    auto spineInfo = extension->spinesInformation();

    hasSpines = !spineInfo.isEmpty();

    if(hasSpines) break;
  }

  if(!hasSpines)
  {
    QString message;
    if(input.isEmpty())
    {
      message = tr("The dendrites in the session have no spines.");
    }
    else
    {
      message = tr("The selected dendrites have no spines.");
    }

    auto details = tr("The report will be empty if there are no spines.");
    DefaultDialogs::ErrorMessage(message, DefaultDialogs::DefaultTitle(), details);
  }
  else
  {
    WaitingCursor cursor;

    auto dialog = new SpinesInformationDialog(input.isEmpty() ? input : valid, getContext());
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->show();
  }
}
