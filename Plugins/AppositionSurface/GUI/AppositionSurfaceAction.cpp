/*
 * AppositionSurfaceAction.cpp
 *
 *  Created on: Jan 16, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#include "AppositionSurfaceAction.h"

#include "Undo/AppositionSurfaceCommand.h"
#include <Core/Extensions/AppositionSurfaceExtension.h>
#include "AppositionSurfacePlugin.h"

// EspINA
#include <Core/Model/PickableItem.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaModel.h>
#include <GUI/ViewManager.h>
#include <Undo/TaxonomiesCommand.h>

// Qt
#include <QDebug>
#include <QApplication>


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

    QApplication::setOverrideCursor(Qt::WaitCursor);
    {
      SegmentationSList createdSegmentations;
      m_undoStack->beginMacro("Compute Synaptic Apposition Surface");
      TaxonomySPtr taxonomy = m_model->taxonomy();
      if (taxonomy->element(SAS).isNull())
      {
        m_undoStack->push(new AddTaxonomyElement(taxonomy->root().data(), SAS, m_model, QColor(255, 255, 0)));
        m_model->taxonomy()->element(SAS)->addProperty(QString("Dim_X"), QVariant("500"));
        m_model->taxonomy()->element(SAS)->addProperty(QString("Dim_Y"), QVariant("500"));
        m_model->taxonomy()->element(SAS)->addProperty(QString("Dim_Z"), QVariant("500"));
      }
      m_undoStack->push(new AppositionSurfaceCommand(validSegs, m_model, m_viewManager, createdSegmentations));
      m_model->emitSegmentationAdded(createdSegmentations);
      m_undoStack->endMacro();
    }
    QApplication::restoreOverrideCursor();
  }

} /* namespace EspINA */
