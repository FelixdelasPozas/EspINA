/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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


#include "CompositionToolBar.h"
#include <Undo/CompositionCommand.h>

#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <GUI/ViewManager.h>

#include <QAction>
#include <QUndoStack>

using namespace EspINA;

const QString COMPOSE_SEG_TOOLTIP = QObject::tr("Compose Selected Segmentations");

//----------------------------------------------------------------------------
CompositionToolBar::CompositionToolBar(EspinaModelSPtr model,
                                       QUndoStack    *undoStack,
                                       ViewManager   *viewManager,
                                       QWidget       *parent)
: IToolBar     ( parent      )
, m_model      ( model       )
, m_undoStack  ( undoStack   )
, m_viewManager( viewManager )
, m_compose    ( NULL        )
{
  setObjectName("CompositionToolBar");
  setWindowTitle("Composition Tool Bar");

  initFactoryExtension(m_model->factory());

  initComposeTool();

  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
          this, SLOT(updateAvailableOperations()));
  updateAvailableOperations();
}

//----------------------------------------------------------------------------
CompositionToolBar::~CompositionToolBar()
{
  qDebug() << "********************************************************";
  qDebug() << "          Destroying Composition ToolbBar";
  qDebug() << "********************************************************";
}

//----------------------------------------------------------------------------
void CompositionToolBar::initToolBar(EspinaModelSPtr model,
                                     QUndoStack     *undoStack,
                                     ViewManager    *viewManager)
{

}

//----------------------------------------------------------------------------
void CompositionToolBar::initFactoryExtension(EspinaFactoryPtr factory)
{
  // TODO 2012-12-21 El fin del mundo ha llegado a EspINA!!!
  //                 Esta toolbar registra el mismo filtro que el editor tool bar!
  //factory->registerFilter(this, CompositionFilter);
  //Q_ASSERT(false);
}

//----------------------------------------------------------------------------
FilterSPtr CompositionToolBar::createFilter(const QString              &filter,
                                                 const Filter::NamedInputs  &inputs,
                                                 const ModelItem::Arguments &args)
{
  Q_ASSERT(false);//TODO 2012-12-17
  return FilterSPtr();
}

//----------------------------------------------------------------------------
void CompositionToolBar::reset()
{
  updateAvailableOperations();
}

//----------------------------------------------------------------------------
void CompositionToolBar::createSegmentationFromComponents()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = m_viewManager->selectedSegmentations();
  if (input.size() > 1)
  {
    m_viewManager->clearSelection(true);
    m_undoStack->push(new CompositionCommand(input,
                                             m_model->findTaxonomyElement(m_viewManager->activeTaxonomy()),
                                             m_model));
  }
}

//----------------------------------------------------------------------------
void CompositionToolBar::updateAvailableOperations()
{
  SegmentationList segs = m_viewManager->selectedSegmentations();

//   bool one = segs.size() == 1;
//   QString oneToolTip;
//   if (!one)
//     oneToolTip = tr(" (This tool requires just one segmentation to be selected)");

  bool atLeastTwo = segs.size()  > 1;
  QString atLeastTwoToolTip;
  if (!atLeastTwo)
    atLeastTwoToolTip = tr(" (This tool requires at least two segmentation to be selected)");

//   bool several    = segs.size()  > 0;
//   QString severalToolTip;
//   if (!several)
//     severalToolTip = tr(" (This tool requires at least one segmentation to be selected)");

  m_compose->setEnabled(atLeastTwo);
  m_compose->setToolTip(tr("Create a segmentations from selected segmentations") + atLeastTwo);
}

//----------------------------------------------------------------------------
void CompositionToolBar::initComposeTool()
{
  m_compose = addAction(COMPOSE_SEG_TOOLTIP);

  m_compose->setIcon(QIcon(":/espina/create_seg_from_parts.svg"));

  connect(m_compose, SIGNAL(triggered(bool)),
          this, SLOT(createSegmentationFromComponents()));
}
