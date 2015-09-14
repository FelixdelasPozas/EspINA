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
#include <Support/Utils/SelectionUtils.h>
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
  return tr("Current report input does not contain any synapses");
}

//----------------------------------------------------------------------------
void SASReport::show(SegmentationAdapterList input) const
{
    auto undoStack = getUndoStack();
    auto model     = getModel();
    auto factory   = getFactory();
    auto SAS       = QObject::tr("SAS");

//     if (model->classification()->category(SAS) == nullptr)
//     {
//       undoStack->beginMacro(tr("Apposition Surface"));
//       undoStack->push(new AddCategoryCommand(model->classification()->root(), SAS, model, QColor(255, 255, 0)));
//       undoStack->endMacro();
//
//       model->classification()->category(SAS)->addProperty(QString("Dim_X"), QVariant("500"));
//       model->classification()->category(SAS)->addProperty(QString("Dim_Y"), QVariant("500"));
//       model->classification()->category(SAS)->addProperty(QString("Dim_Z"), QVariant("500"));
//     }

    // check segmentations for SAS and create it if needed
//     for (auto segmentation : synapsis)
//     {
//       auto sasItems = model->relatedItems(segmentation, RelationType::RELATION_OUT, SAS);
//       if (sasItems.isEmpty())
//       {
//         if (!m_delayedAnalysis)
//         {
//           m_delayedAnalysis = true;
//           m_analysisSynapses = synapsis;
//           QApplication::setOverrideCursor(Qt::WaitCursor);
//         }

//         InputSList inputs;
//         inputs << segmentation->asInput();
//
//         auto filter = factory->createFilter<AppositionSurfaceFilter>(inputs, AS_FILTER);
//
//         struct Data data(filter, model->smartPointer(segmentation));
//         m_executingTasks.insert(filter.get(), data);
//
//         connect(filter.get(), SIGNAL(finished()),
//                 this,         SLOT(finishedTask()));
//
//         Task::submit(filter);
//       }
//       else
//       {
//         Q_ASSERT(sasItems.size() == 1);
//         auto sas        = std::dynamic_pointer_cast<SegmentationAdapter>(sasItems.first());
//         auto extensions = sas->extensions();
//         auto extension  = factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE);
//         std::dynamic_pointer_cast<AppositionSurfaceExtension>(extension)->setOriginSegmentation(model->smartPointer(segmentation));
//         extensions->add(extension);
//       }
//     }
//     if (!m_delayedAnalysis)
//     {
      SASReportDialog dialog(input, getContext());
      dialog.exec();
//
//       delete analysis;
//     };
}
