/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#include "MorphologicalEditionTool.h"
#include "SpinBoxAction.h"
#include <Support/Settings/EspinaSettings.h>
#include <Filters/MorphologicalEditionFilter.h>

#include <QAction>
#include <QSettings>
#include <QUndoStack>

const QString DILATE_RADIUS("MorphologicalEditionTools::DilateRadius");
const QString ERODE_RADIUS ("MorphologicalEditionTools::ErodeRadius");
const QString OPEN_RADIUS  ("MorphologicalEditionTools::OpenRadius");
const QString CLOSE_RADIUS ("MorphologicalEditionTools::CloseRadius");

namespace EspINA
{
  //------------------------------------------------------------------------
  MorphologicalEditionTool::MorphologicalEditionTool(ModelAdapterSPtr model,
                                                     ModelFactorySPtr factory,
                                                     ViewManagerSPtr  viewManager,
                                                     QUndoStack      *undoStack)
  : m_model(model)
  , m_factory(factory)
  , m_viewManager(viewManager)
  , m_undoStack(undoStack)
  , m_enabled(false)
  {
    m_addition = new QAction(QIcon(":/espina/add.svg"), tr("Merge selected segmentations"), nullptr);
    connect(m_addition, SIGNAL(triggered(bool)),
            this, SLOT(mergeSegmentations()));

    m_subtract = new QAction(QIcon(":/espina/remove.svg"), tr("Subtract selected segmentations"), nullptr);
    connect(m_subtract, SIGNAL(triggered(bool)),
            this, SLOT(subtractSegmentations()));

    m_erode = new QAction(QIcon(":/espina/erode.png"), tr("Erode selected segmentations"), nullptr);
    connect(m_erode, SIGNAL(triggered(bool)),
            this, SLOT(erodeSegmentations()));

    m_dilate = new QAction(QIcon(":/espina/dilate.png"), tr("Dilate selected segmentations"), nullptr);
    connect(m_dilate, SIGNAL(triggered(bool)),
            this, SLOT(dilateSegmentations()));

    m_open = new QAction(QIcon(":/espina/open.png"), tr("Open selected segmentations"), nullptr);
    connect(m_open, SIGNAL(triggered(bool)),
            this, SLOT(openSegmentations()));

    m_close = new QAction(QIcon(":/espina/close.png"), tr("Close seleceted segmentations"), nullptr);
    connect(m_close, SIGNAL(triggered(bool)),
            this, SLOT(closeSegmentations()));

    m_fill = new QAction(QIcon(":/espina/fillHoles.svg"), tr("Fill internal holes in selected segmentations"), nullptr);
    connect(m_fill, SIGNAL(triggered(bool)),
            this, SLOT(fillHoles()));

    m_dilateRadiusWidget = new SpinBoxAction();
    m_erodeRadiusWidget = new SpinBoxAction();
    m_openRadiusWidget = new SpinBoxAction();
    m_closeRadiusWidget = new SpinBoxAction();

    QSettings settings(CESVIMA, ESPINA);
    m_dilateRadiusWidget->setLabelText(tr("Dilate Radius"));
    m_dilateRadiusWidget->setRadius(settings.value(DILATE_RADIUS, 3).toInt());
    m_erodeRadiusWidget->setLabelText(tr("Erode Radius"));
    m_erodeRadiusWidget->setRadius(settings.value(ERODE_RADIUS, 3).toInt());
    m_openRadiusWidget->setLabelText(tr("Open Radius"));
    m_openRadiusWidget->setRadius(settings.value(OPEN_RADIUS, 3).toInt());
    m_closeRadiusWidget->setLabelText(tr("Close Radius"));
    m_closeRadiusWidget->setRadius(settings.value(CLOSE_RADIUS, 3).toInt());
  }

  //------------------------------------------------------------------------
  MorphologicalEditionTool::~MorphologicalEditionTool()
  {
    delete m_addition;
    delete m_subtract;
    delete m_erode;
    delete m_dilate;
    delete m_open;
    delete m_close;
    delete m_fill;
    delete m_dilateRadiusWidget;
    delete m_erodeRadiusWidget;
    delete m_openRadiusWidget;
    delete m_closeRadiusWidget;
  }
  
  //------------------------------------------------------------------------
  void MorphologicalEditionTool::setEnabled(bool value)
  {
    m_enabled = value;

    m_addition->setEnabled(value);
    m_subtract->setEnabled(value);
    m_erode->setEnabled(value);
    m_dilate->setEnabled(value);
    m_open->setEnabled(value);
    m_close->setEnabled(value);
    m_fill->setEnabled(value);
    m_dilateRadiusWidget->setEnabled(value);
    m_erodeRadiusWidget->setEnabled(value);
    m_openRadiusWidget->setEnabled(value);
    m_closeRadiusWidget->setEnabled(value);
  }

  //------------------------------------------------------------------------
  bool MorphologicalEditionTool::enabled() const
  {
    return m_enabled;
  }

  //------------------------------------------------------------------------
  QList<QAction *> MorphologicalEditionTool::actions() const
  {
    QList<QAction *> actions;

    actions << m_addition;
    actions << m_subtract;
    actions << m_erode;
    actions << m_erodeRadiusWidget;
    actions << m_dilate;
    actions << m_dilateRadiusWidget;
    actions << m_open;
    actions << m_openRadiusWidget;
    actions << m_close;
    actions << m_closeRadiusWidget;
    actions << m_fill;

    return actions;
  }

  //------------------------------------------------------------------------
  void MorphologicalEditionTool::mergeSegmentations()
  {
    m_viewManager->setSelector(nullptr);

    SegmentationAdapterList inputs = m_viewManager->selectedSegmentations();

    if (inputs.size() > 1)
    {
      SegmentationAdapterList createdSegmentations;
      m_viewManager->clearSelection();

      m_undoStack->beginMacro(tr("Merge Segmentations"));
//      m_undoStack->push(new ImageLogicCommand(input,
//                                              ImageLogicFilter::ADDITION,
//                                              m_viewManager->activeTaxonomy(),
//                                              m_model,
//                                              createdSegmentations));
//      m_model->emitSegmentationAdded(createdSegmentations);
      m_undoStack->endMacro();

    }
  }

  //------------------------------------------------------------------------
  void MorphologicalEditionTool::subtractSegmentations()
  {
//    m_viewManager->unsetActiveTool();
//
//    SegmentationList input = m_viewManager->selectedSegmentations();
//    if (input.size() > 1)
//    {
//      SegmentationSList createdSegmentations;
//      m_viewManager->clearSelection(true);
//      m_undoStack->beginMacro("Subtract Segmentations");
//      m_undoStack->push(new ImageLogicCommand(input,
//                                              ImageLogicFilter::SUBTRACTION,
//                                              m_viewManager->activeTaxonomy(),
//                                              m_model,
//                                              createdSegmentations));
//      m_model->emitSegmentationAdded(createdSegmentations);
//      m_undoStack->endMacro();
//    }
  }

  //------------------------------------------------------------------------
  void MorphologicalEditionTool::erodeSegmentations()
  {
//    m_viewManager->unsetActiveTool();
//
//    SegmentationList input = m_viewManager->selectedSegmentations();
//    if (input.size() > 0)
//    {
//      CODECommand *erodeCommand = new CODECommand(input, CODECommand::ERODE, m_settings->erodeRadius(), m_model, m_viewManager);
//      if(erodeCommand->getRemovedSegmentations().size() > 0)
//      {
//        QMessageBox info;
//        info.setWindowTitle("Erode Segmentations");
//        info.setIcon(QMessageBox::Warning);
//        QString message(tr("The following segmentations will be deleted by the ERODE operation:\n"));
//        foreach(SegmentationPtr seg, erodeCommand->getRemovedSegmentations())
//          message += QString("  - ") + seg->data().toString() + QString("\n");
//        message += tr("\nDo you want to continue with the operation?");
//        info.setText(message);
//        info.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
//        if (info.exec() == QMessageBox::No)
//        {
//          delete erodeCommand;
//          return;
//        }
//      }
//
//      m_undoStack->beginMacro(tr("Erode Segmentation"));
//      m_undoStack->push(erodeCommand);
//      m_undoStack->endMacro();
//    }
  }

  //------------------------------------------------------------------------
  void MorphologicalEditionTool::dilateSegmentations()
  {
//    m_viewManager->unsetActiveTool();
//
//    SegmentationList input = m_viewManager->selectedSegmentations();
//    if (input.size() > 0)
//    {
//      int r = m_settings->dilateRadius();
//      m_undoStack->beginMacro(tr("Dilate Segmentation"));
//      m_undoStack->push(new CODECommand(input, CODECommand::DILATE, r, m_model, m_viewManager));
//      m_undoStack->endMacro();
//    }
  }

  //------------------------------------------------------------------------
  void MorphologicalEditionTool::openSegmentations()
  {
//    m_viewManager->unsetActiveTool();
//
//    SegmentationList input = m_viewManager->selectedSegmentations();
//    if (input.size() > 0)
//    {
//      CODECommand *openCommand = new CODECommand(input, CODECommand::OPEN, m_settings->openRadius(), m_model, m_viewManager);
//      if (openCommand->getRemovedSegmentations().size() > 0)
//      {
//        QMessageBox info;
//        info.setWindowTitle("Open Segmentations");
//        info.setIcon(QMessageBox::Warning);
//        QString message(tr("The following segmentations will be deleted by the OPEN operation:\n"));
//        foreach(SegmentationPtr seg, openCommand->getRemovedSegmentations())
//          message += QString("  - ") + seg->data().toString() + QString("\n");
//        message += tr("\nDo you want to continue with the operation?");
//        info.setText(message);
//        info.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
//        if (info.exec() == QMessageBox::No)
//        {
//          delete openCommand;
//          return;
//        }
//      }
//
//      m_undoStack->beginMacro(tr("Open Segmentation"));
//      m_undoStack->push(openCommand);
//      m_undoStack->endMacro();
//    }
  }

  //------------------------------------------------------------------------
  void MorphologicalEditionTool::closeSegmentations()
  {
//    m_viewManager->unsetActiveTool();
//
//    SegmentationList input = m_viewManager->selectedSegmentations();
//    if (input.size() > 0)
//    {
//      CODECommand *closeCommand = new CODECommand(input, CODECommand::CLOSE, m_settings->closeRadius(), m_model,
//          m_viewManager);
//      if (closeCommand->getRemovedSegmentations().size() > 0)
//      {
//        QMessageBox info;
//        info.setWindowTitle(CLOSING_TOOLTIP);
//        info.setIcon(QMessageBox::Warning);
//        QString message(tr("The following segmentations will be deleted by the CLOSE operation:\n"));
//        foreach(SegmentationPtr seg, closeCommand->getRemovedSegmentations())message += QString("  - ") + seg->data().toString() + QString("\n");
//        message += tr("\nDo you want to continue with the operation?");
//        info.setText(message);
//        info.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//        if (info.exec() == QMessageBox::No)
//        {
//          delete closeCommand;
//          return;
//        }
//      }
//
//      m_undoStack->beginMacro(tr("Close Segmentation"));
//      m_undoStack->push(closeCommand);
//      m_undoStack->endMacro();
//    }
  }

  //------------------------------------------------------------------------
  void MorphologicalEditionTool::fillHoles()
  {
//    m_viewManager->unsetActiveTool();
//
//    SegmentationList input = m_viewManager->selectedSegmentations();
//    if (input.size() > 0)
//    {
//      m_undoStack->push(new FillHolesCommand(input, m_model, m_viewManager));
//    }
  }


} /* namespace EspINA */
