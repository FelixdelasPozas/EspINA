/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Utils/ListUtils.hxx>
#include <Dialogs/AdjacencyMatrix/AdjacencyMatrixDialog.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Widgets/Styles.h>
#include <ToolGroups/Analyze/Reports/AdjacencyMatrixReport.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::Support;
using namespace ESPINA::GUI::Widgets::Styles;

//--------------------------------------------------------------------
AdjacencyMatrixReport::AdjacencyMatrixReport(Support::Context& context)
: WithContext(context)
{
}

//--------------------------------------------------------------------
QString AdjacencyMatrixReport::name() const
{
  return tr("Adjacency matrix");
}

//--------------------------------------------------------------------
QString AdjacencyMatrixReport::description() const
{
  return tr("Reports the adjacency matrix (connections) between segmentations.");
}

//--------------------------------------------------------------------
SegmentationAdapterList AdjacencyMatrixReport::acceptedInput(SegmentationAdapterList segmentations) const
{
  return segmentations;
}

//--------------------------------------------------------------------
QString AdjacencyMatrixReport::requiredInputDescription() const
{
  return QString();
}

//--------------------------------------------------------------------
void AdjacencyMatrixReport::show(SegmentationAdapterList input) const
{
  auto model = getModel();
  SegmentationAdapterSList inputSList;
  int count = 0;

  {
    WaitingCursor cursor;

    if(input.isEmpty())
    {
      inputSList = model->segmentations();
    }
    else
    {
      for(auto seg: input)
      {
        inputSList << model->smartPointer(seg);
      }
    }

    for(auto seg: inputSList)
    {
      count += model->connections(seg).size();
    }
  }

  if(count == 0)
  {
    QString message;
    if(input.isEmpty())
    {
      message = tr("There are no connections defined in the current session.");
    }
    else
    {
      message = tr("The selected segmentations doesn't have any connections associated to them.");
    }

    auto details = tr("The adjacency matrix is empty if there are no connections to show.");

    DefaultDialogs::ErrorMessage(message, DefaultDialogs::DefaultTitle(), details);
  }
  else
  {
    WaitingCursor cursor;

    auto dialog = new AdjacencyMatrixDialog(input, getContext());
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->show();
  }
}
