/*
 * AppositionSurfaceAction.cpp
 *
 *  Created on: Jan 16, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#include "AppositionSurfaceAction.h"

// EspINA
#include <Core/Model/PickableItem.h>
#include <Core/Model/Segmentation.h>
#include <GUI/ViewManager.h>
#include <Undo/AppositionSurfaceCommand.h>

// Qt
#include <QDebug>

namespace EspINA
{
  //------------------------------------------------------------------------
  AppositionSurfaceAction::AppositionSurfaceAction(ViewManager *vm,
                                                   QUndoStack *undo,
                                                   EspinaModel *model,
                                                   QObject *parent)
  : QAction(parent)
  , m_viewManager(vm)
  , m_undoStack(undo)
  , m_model(model)
  {
    setIcon(QIcon(":/AppSurface.svg"));
    setEnabled(false);

    connect(this, SIGNAL(triggered()), this, SLOT(computeASurfaces()));
  }
  
  //------------------------------------------------------------------------
  AppositionSurfaceAction::~AppositionSurfaceAction()
  {
  }

  //------------------------------------------------------------------------
  void AppositionSurfaceAction::computeASurfaces()
  {
    SegmentationList selectedSegs = m_viewManager->selectedSegmentations();
    SegmentationList validSegs = SegmentationList();

    foreach(SegmentationPtr seg, selectedSegs)
      if (seg->taxonomy()->qualifiedName().contains("Synapse"))
        validSegs << seg;

    m_undoStack->beginMacro("Compute apposition surfaces");
    m_undoStack->push(new AppositionSurfaceCommand(validSegs, m_model, m_viewManager));
    m_undoStack->endMacro();
  }

} /* namespace EspINA */
