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
#include <Dialogs/AdjacencyMatrix/AdjacencyMatrixDialog.h>
#include <ToolGroups/Analyze/Reports/AdjacencyMatrixReport.h>

using namespace ESPINA;
using namespace ESPINA::Support;

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
  auto dialog = new AdjacencyMatrixDialog(input, getContext());
  dialog->setAttribute(Qt::WA_DeleteOnClose, true);
  dialog->show();
}
