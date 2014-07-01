/*
 
 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// Qt
#include <QFileDialog>
#include <QTextStream>

// EspINA
#include "TubularFilterInspector.h"
#include <App/Tools/TubularSegmentation/TubularTool.h>
#include <GUI/QtWidget/EspinaRenderView.h>
#include <GUI/ViewManager.h>
#include <GUI/vtkWidgets/TubularWidget.h>

namespace EspINA
{
  //----------------------------------------------------------------------------
  TubularFilterInspector::TubularFilterInspector(TubularSegmentationFilter::Pointer source,
                                                         QUndoStack* undo,
                                                         ViewManager *vm,
                                                         IToolSPtr tool)
  : m_source(source)
  , m_undoStack(undo)
  , m_viewManager(vm)
  , m_tool(tool)
  {

  }

  //----------------------------------------------------------------------------
  TubularFilterInspector::~TubularFilterInspector()
  {
  }

  //----------------------------------------------------------------------------
  QWidget *TubularFilterInspector::createWidget(QUndoStack *stack, ViewManager *vm)
  {
    return new Widget(m_source, stack, vm, m_tool);
  }

  //----------------------------------------------------------------------------
  TubularFilterInspector::Widget::Widget(TubularSegmentationFilter::Pointer source,
                                                         QUndoStack* undo,
                                                         ViewManager *vm,
                                                         IToolSPtr tool)
  : m_source(source)
  , m_undoStack(undo)
  , m_viewManager(vm)
  , m_tool(tool)
  {
    setupUi(this);
    QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
    writeDataToFile->setIcon(iconSave);

    extremesCheckBox->setChecked(m_source->getRoundedExtremes());
    updateNodeList();

    connect(writeDataToFile, SIGNAL(clicked(bool)), this, SLOT(exportNodeList()));
    connect(m_source.get(), SIGNAL(modified(ModelItemPtr)), this, SLOT(updateNodeList()));
    connect(modifySource, SIGNAL(toggled(bool)), this, SLOT(editSpine(bool)));
    connect(extremesCheckBox, SIGNAL(toggled(bool)), this, SLOT(modifyRoundedExtremes(bool)));

    if (!m_tool)
      modifySource->setEnabled(false);
  }

  //----------------------------------------------------------------------------
  TubularFilterInspector::Widget::~Widget()
  {
  }

  //----------------------------------------------------------------------------
  void TubularFilterInspector::Widget::exportNodeList()
  {
    TubularSegmentationFilter::NodeList nodes = m_source->nodes();
    if (nodes.isEmpty())
      return;

    QFileDialog fileDialog;
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);

    if (fileDialog.exec() == QFileDialog::AcceptSave)
    {
      QFile spineFile(fileDialog.selectedFiles().first());
      spineFile.open(QIODevice::WriteOnly | QIODevice::Text);

      QString separator = " ";
      QTextStream out(&spineFile);
      foreach(QVector4D node, nodes){
      out << node.x() << separator << node.y() << separator
      << node.z() << separator << node.z() << "\n";
    }
      spineFile.close();
    }
  }

  //----------------------------------------------------------------------------
  void TubularFilterInspector::Widget::updateNodeList()
  {
    listView->clear();
    TubularSegmentationFilter::NodeList nodes = m_source->nodes();
    foreach(QVector4D node, nodes)
    {
      QString label = tr("C: %1, %2, %3 R: %4")
      .arg(node.x())
      .arg(node.y())
      .arg(node.z())
      .arg(node.w());
      listView->addItem(label);
    }
  }

  //----------------------------------------------------------------------------
  void TubularFilterInspector::Widget::updateSpine(TubularSegmentationFilter::NodeList nodes)
  {
    Q_ASSERT(m_source);

    if (nodes == m_source->nodes())
      return;

    m_undoStack->beginMacro("Modify Tubular Segmentation Nodes");
    m_undoStack->push(new TubularTool::UpdateSegmentationNodes(m_source, nodes));
    m_undoStack->endMacro();

    m_viewManager->updateViews();
  }

  //----------------------------------------------------------------------------
  void TubularFilterInspector::Widget::editSpine(bool editing)
  {
    if (m_tool)
    {
      if (editing)
      {
        TubularTool *tool = reinterpret_cast<TubularTool *>(m_tool.get());
        tool->setFilter(m_source);
        m_viewManager->setActiveTool(m_tool);
        tool->setNodes(m_source->nodes());
      }
      else
        m_viewManager->unsetActiveTool(m_tool);

      m_viewManager->updateViews();
    }
  }

  //----------------------------------------------------------------------------
  void TubularFilterInspector::Widget::modifyRoundedExtremes(bool value)
  {
    if (m_tool)
    {
      TubularTool *tool = reinterpret_cast<TubularTool *>(m_tool.get());
      tool->setRoundedExtremes(value);
    }

    m_source->setRoundedExtremes(value);
    m_viewManager->updateViews();
  }
}
