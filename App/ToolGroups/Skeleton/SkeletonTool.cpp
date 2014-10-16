/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include <ToolGroups/Skeleton/SkeletonTool.h>
#include <GUI/View/Widgets/Skeleton/SkeletonWidget.h>
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Widgets/SpinBoxAction.h>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  SkeletonTool::SkeletonTool(ModelAdapterSPtr model, ViewManagerSPtr viewManager)
  : m_vm              {viewManager}
  , m_enabled         {false}
  , m_categorySelector{new CategorySelector(model)}
  , m_toleranceBox    {new SpinBoxAction()}
  , m_action          {new QAction(QIcon(":/espina/pencil.png"), tr("Manual creation of skeletons."), this)}
  {
    m_action->setCheckable(true);

    m_toleranceBox->setLabelText("Tolerance");
    m_toleranceBox->setSuffix(" nm");
    m_toleranceBox->setToolTip("Minimum distance between points.");
    m_toleranceBox->setSpinBoxMinimum(1);

    connect(m_action, SIGNAL(triggered(bool)),
            this,     SLOT(initTool(bool)), Qt::QueuedConnection);

    connect(m_vm->selection().get(), SIGNAL(selectionChanged()),
            this,                    SLOT(updateState()), Qt::QueuedConnection);

    connect(m_toleranceBox, SIGNAL(valueChanged(int)),
            this,           SLOT(toleranceChanged(int)), Qt::QueuedConnection);

    connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
            this,               SLOT(categoryChanged(CategoryAdapterSPtr)), Qt::QueuedConnection);

    setControlsVisibility(false);
  }
  
  //-----------------------------------------------------------------------------
  SkeletonTool::~SkeletonTool()
  {
    if(m_widget)
    {
      m_widget->setEnabled(false);
      m_widget = nullptr;
    }

    disconnect(m_action, SIGNAL(triggered(bool)),
               this,     SLOT(initTool(bool)));

    disconnect(m_vm->selection().get(), SIGNAL(selectionChanged()),
               this,                    SLOT(updateState()));

    disconnect(m_toleranceBox, SIGNAL(valueChanged(int)),
               this,           SLOT(toleranceChanged(int)));

    disconnect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
               this,               SLOT(categoryChanged(CategoryAdapterSPtr)));
  }
  
  //-----------------------------------------------------------------------------
  void SkeletonTool::setEnabled(bool value)
  {
    m_enabled = value;

    if (m_widget)
      m_widget->setEnabled(value);

    m_action->setEnabled(value);
    m_categorySelector->setEnabled(value);
    m_toleranceBox->setEnabled(value);
  }
  
  //-----------------------------------------------------------------------------
  void SkeletonTool::updateState()
  {
    auto selectedSegs = m_vm->selection()->segmentations();
    auto value = (selectedSegs.size() <= 1);

    m_action->setEnabled(value);
    m_categorySelector->setEnabled(value);
    m_toleranceBox->setEnabled(value);

    if(value)
    {
      if(selectedSegs.empty())
      {
        m_item = nullptr;
        m_itemCategory = m_categorySelector->selectedCategory();
      }
      else
      {
        m_item = selectedSegs.first();
        m_itemCategory = m_item->category();
      }
      m_categorySelector->selectCategory(m_itemCategory);
    }
    else
    {
      if(m_widget != nullptr)
        initTool(false);
    }
  }

  //-----------------------------------------------------------------------------
  QList<QAction*> SkeletonTool::actions() const
  {
    QList<QAction *> actions;
    actions << m_action;
    actions << m_categorySelector;
    actions << m_toleranceBox;

    return actions;
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::initTool(bool value)
  {
    if (value)
    {
      if(nullptr == m_vm->activeChannel())
        return;

      auto spacing = volumetricData(m_vm->activeChannel()->output())->spacing();
      auto minimumValue = std::ceil(std::max(spacing[0], std::max(spacing[1], spacing[2]))) + 1;
      if(m_toleranceBox->getSpinBoxMinimumValue() < minimumValue)
        m_toleranceBox->setSpinBoxMinimum(minimumValue);
      m_toleranceBox->setSpinBoxMaximum(100*minimumValue);

      QColor color;
      auto selection = m_vm->selection()->segmentations();
      if(selection.size() != 1)
        color = m_categorySelector->selectedCategory()->color();
      else
        color = m_vm->colorEngine()->color(selection.first());

      auto widget = new SkeletonWidget();
      widget->setTolerance(m_toleranceBox->value());
      widget->setRepresentationColor(color);

      m_widget = EspinaWidgetSPtr{widget};
      m_handler = std::dynamic_pointer_cast<EventHandler>(m_widget);
      connect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
              this,            SLOT(eventHandlerToogled(bool)), Qt::QueuedConnection);

      m_vm->setEventHandler(m_handler);
      m_vm->addWidget(m_widget);
      m_vm->setSelectionEnabled(false);
      m_widget->setEnabled(true);
    }
    else
    {
      m_action->blockSignals(true);
      m_action->setChecked(false);
      m_action->blockSignals(false);

      m_skeleton = dynamic_cast<SkeletonWidget *>(m_widget.get())->getSkeleton();

      disconnect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
                 this,            SLOT(eventHandlerToogled(bool)));

      m_widget->setEnabled(false);
      m_vm->removeWidget(m_widget);

      m_vm->unsetEventHandler(m_handler);
      m_handler = nullptr;
      m_vm->setSelectionEnabled(true);
      m_widget = nullptr;

      if(m_skeleton->GetNumberOfPoints() != 0)
        emit stoppedOperation();
      else
        m_skeleton = nullptr;
    }

    setControlsVisibility(value);
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::setControlsVisibility(bool value)
  {
    m_categorySelector->setVisible(value);
    m_toleranceBox->setVisible(value);
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::toleranceChanged(int value)
  {
    if(nullptr == m_widget)
      return;

    auto widget = dynamic_cast<SkeletonWidget *>(m_widget.get());
    widget->setTolerance(value);
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::categoryChanged(CategoryAdapterSPtr category)
  {
    if(m_widget != nullptr)
      dynamic_cast<SkeletonWidget *>(m_widget.get())->setRepresentationColor(category->color());
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::eventHandlerToogled(bool value)
  {
    if(value)
      return;

    if(m_widget != nullptr)
    {
      m_action->blockSignals(true);
      m_action->setChecked(false);
      m_action->blockSignals(false);

      m_skeleton = dynamic_cast<SkeletonWidget *>(m_widget.get())->getSkeleton();

      disconnect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
                 this,            SLOT(eventHandlerToogled(bool)));

      m_widget->setEnabled(false);
      m_vm->removeWidget(m_widget);
      m_handler = nullptr;
      m_vm->setSelectionEnabled(true);
      m_widget = nullptr;

      if(m_skeleton != nullptr && m_skeleton->GetNumberOfPoints() > 0)
        emit stoppedOperation();
      else
        m_skeleton = nullptr;

      setControlsVisibility(value);
    }
  }
} // namespace EspINA

