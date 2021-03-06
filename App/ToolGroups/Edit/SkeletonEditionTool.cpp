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
#include <App/ToolGroups/Edit/SkeletonEditionTool.h>
#include <App/ToolGroups/Segment/Skeleton/SkeletonToolsUtils.h>
#include <App/ToolGroups/Segment/Skeleton/SkeletonCreationTool.h>
#include <App/Dialogs/SkeletonStrokeDefinition/StrokeDefinitionDialog.h>
#include <App/ToolGroups/Segment/Skeleton/ConnectionPointsTemporalRepresentation2D.h>
#include <App/ToolGroups/Segment/Skeleton/SkeletonToolWidget2D.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/View/Widgets/Skeleton/SkeletonWidget2D.h>
#include <GUI/Widgets/DoubleSpinBoxAction.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/View/Widgets/Skeleton/vtkSkeletonWidgetRepresentation.h>
#include <Support/Representations/RepresentationUtils.h>
#include <Undo/ModifySkeletonCommand.h>
#include <Undo/RemoveSegmentations.h>

// Qt
#include <QApplication>
#include <QComboBox>
#include <QBitmap>

// VTK
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkDoubleArray.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::Representations::Settings;
using namespace ESPINA::GUI::View::Widgets::Skeleton;
using namespace ESPINA::Support::Representations::Utils;
using namespace ESPINA::SkeletonToolsUtils;

const QString MODIFY_HUE_SETTINGS_KEY = QString{"Modify same-hue strokes to avoid coincidences"};

//--------------------------------------------------------------------
SkeletonEditionTool::SkeletonEditionTool(Support::Context& context)
: EditTool    {"SkeletonEditionTool", ":/espina/tubular.svg", tr("Manual modification of skeletons."), context}
, m_init      {false}
, m_item      {nullptr}
, m_allowSwich{false}
{
  initEventHandler();
  initRepresentationFactories();

  setCheckable(true);
  setExclusive(true);

  connect(m_eventHandler.get(), SIGNAL(eventHandlerInUse(bool)),
          this                , SLOT(initTool(bool)));

  connect(m_eventHandler.get(), SIGNAL(modifier(bool)),
          this                , SLOT(onModifierPressed(bool)));

  initParametersWidgets();

  connect(m_eventHandler.get(), SIGNAL(selectedStroke(int)),
          m_strokeCombo,        SLOT(setCurrentIndex(int)), Qt::DirectConnection);
}

//--------------------------------------------------------------------
SkeletonEditionTool::~SkeletonEditionTool()
{
  if(m_item != nullptr) initTool(false);
}

//--------------------------------------------------------------------
void SkeletonEditionTool::initTool(bool value)
{
  if(value)
  {
    connect(getSelection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
            this,                 SLOT(onSelectionChanged(SegmentationAdapterList)));

    if(!m_init)
    {
      onResolutionChanged();

      connect(getContext().viewState().coordinateSystem().get(), SIGNAL(resolutionChanged(NmVector3)),
              this,                                              SLOT(onResolutionChanged()));
      connect(getModel().get(), SIGNAL(aboutToBeReset()),
              this,             SLOT(onModelReset()));
    }

    m_item = segmentationPtr(getSelection()->segmentations().first());

    if(m_item->isBeingModified())
    {
      auto message = tr("The segmentation %1 can't be edited right now because it's currently being modified by another tool.").arg(m_item->data().toString());
      auto title   = tr("Edit skeleton");
      DefaultDialogs::ErrorMessage(message, title);

      m_item = nullptr;
      initTool(false);
      return;
    }

    m_item->setTemporalRepresentation(std::make_shared<NullRepresentationPipeline>());
    m_item->setBeingModified(true);

    auto skeleton   = readLockSkeleton(m_item->output())->skeleton();
    auto definition = Core::toSkeletonDefinition(skeleton);

    m_strokes.clear();

    populateStrokes(definition.strokes);

    definition.clear();

    updateStrokes();

    if(!getViewState().hasTemporalRepresentation(m_factory)) getViewState().addTemporalRepresentations(m_factory);
    if(!getViewState().hasTemporalRepresentation(m_pointsFactory)) getViewState().addTemporalRepresentations(m_pointsFactory);

    connect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
            this,             SLOT(onSegmentationsRemoved(ViewItemAdapterSList)));


    SkeletonWidget2D::initializeData(readLockSkeleton(m_item->output())->skeleton());

    onStrokeTypeChanged(m_strokeCombo->currentIndex());

    const auto hue = segmentationPtr(getSelection()->segmentations().first())->category()->color().hue();
    for(auto widget: m_widgets)
    {
      widget->setDefaultHue(hue);
      widget->updateRepresentation();
    }

    updateWidgetsMode();
    m_item->invalidateRepresentations();
  }
  else
  {
    disconnect(getSelection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
               this,                 SLOT(onSelectionChanged(SegmentationAdapterList)));

    if(m_init)
    {
      for(auto widget: m_widgets)
      {
        disconnect(widget.get(), SIGNAL(modified(vtkSmartPointer<vtkPolyData>)),
                   this,         SLOT(onSkeletonModified(vtkSmartPointer<vtkPolyData>)));
      }

      m_widgets.clear();

      if(getViewState().hasTemporalRepresentation(m_factory)) getViewState().removeTemporalRepresentations(m_factory);
      if(getViewState().hasTemporalRepresentation(m_pointsFactory)) getViewState().removeTemporalRepresentations(m_pointsFactory);

      if(m_item)
      {
        m_item->clearTemporalRepresentation();
        m_item->invalidateRepresentations();
        m_item->setBeingModified(false);
      }
      m_item = nullptr;
      m_strokes.clear();

      disconnect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
                 this,             SLOT(onSegmentationsRemoved(ViewItemAdapterSList)));

      vtkSkeletonWidgetRepresentation::ClearRepresentation();
    }
  }

  getViewState().refresh();
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onSegmentationsRemoved(ViewItemAdapterSList segmentations)
{
  if(!m_init || m_item) return;

  for(auto seg: segmentations)
  {
    if(seg.get() == m_item)
    {
      initTool(false);
      break;
    }
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onResolutionChanged()
{
  auto channel = getActiveChannel();

  if(channel)
  {
    auto minimumValue = m_minWidget->value();
    auto maximumValue = m_maxWidget->value();

    auto spacing = channel->output()->spacing();
    auto minValue = std::min(spacing[0], std::min(spacing[1], spacing[2]));
    if(minValue == 0) minValue = 1;

    if(minimumValue == 0)
    {
      minimumValue = minValue * 5;
      maximumValue = minimumValue * 5;
    }
    else
    {
      // assume maximumValue != 0
      if(minimumValue < minValue)
      {
        minimumValue = minValue;
      }
    }

    m_minWidget->setValue(minimumValue);
    m_minWidget->setSpinBoxMinimum(minValue);
    m_minWidget->setSpinBoxMaximum(maximumValue);
    m_minWidget->setStepping(minValue);

    m_maxWidget->setValue(maximumValue);
    m_maxWidget->setSpinBoxMinimum(minimumValue);
    m_maxWidget->setSpinBoxMaximum(maximumValue*10);
    m_maxWidget->setStepping(minValue);

    m_eventHandler->setMinimumPointDistance(m_minWidget->value());
    m_eventHandler->setMaximumPointDistance(m_maxWidget->value());

    if(m_widgets.empty())
    {
      m_init = true;
    }
    else
    {
      for(auto widget: m_widgets)
      {
        widget->setSpacing(spacing);
      }
    }
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onModelReset()
{
  if(isChecked())
  {
    setChecked(false);
  }

  disconnect(getContext().viewState().coordinateSystem().get(), SIGNAL(resolutionChanged(NmVector3)),
             this,                                              SLOT(onResolutionChanged()));
  disconnect(getModel().get(), SIGNAL(aboutToBeReset()),
             this,             SLOT(onModelReset()));

  m_init = false;
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onSkeletonWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone)
{
  auto skeletonWidget = std::dynamic_pointer_cast<SkeletonWidget2D>(clone);

  if(skeletonWidget && m_item)
  {
    auto segmentation = dynamic_cast<SegmentationAdapterPtr>(m_item);
    if(segmentation)
    {
      auto index    = std::min(std::max(0, m_strokeCombo->currentIndex()), m_strokes.size()-1);
      auto stroke   = m_strokes.at(index);
      skeletonWidget->setStroke(stroke);
      skeletonWidget->setSpacing(getActiveChannel()->output()->spacing());
      skeletonWidget->setRepresentationTextColor(segmentation->category()->color());
      skeletonWidget->setStrokeHueModification(m_changeHueButton->isChecked());
      skeletonWidget->updateRepresentation();

      connect(skeletonWidget.get(), SIGNAL(modified(vtkSmartPointer<vtkPolyData>)),
              this,                 SLOT(onSkeletonModified(vtkSmartPointer<vtkPolyData>)), Qt::DirectConnection);

      connect(skeletonWidget.get(), SIGNAL(truncationSuccess()),
              this,                 SLOT(onTruncationSuccess()), Qt::DirectConnection);

      connect(skeletonWidget.get(), SIGNAL(strokeChanged(const Core::SkeletonStroke)),
              this,                 SLOT(onStrokeChanged(const Core::SkeletonStroke)), Qt::DirectConnection);

      m_widgets << skeletonWidget;

      updateWidgetsMode();
    }
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onPointWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone)
{
  auto pointWidget = std::dynamic_pointer_cast<ConnectionPointsTemporalRepresentation2D>(clone);

  if(pointWidget)
  {
    connect(m_eventHandler.get(), SIGNAL(addConnectionPoint(const NmVector3)),
            pointWidget.get(),    SLOT(onConnectionPointAdded(const NmVector3)));

    connect(m_eventHandler.get(), SIGNAL(removeConnectionPoint(const NmVector3)),
            pointWidget.get(),    SLOT(onConnectionPointRemoved(const NmVector3)));

    connect(m_eventHandler.get(), SIGNAL(clearConnections()),
            pointWidget.get(),    SLOT(clearPoints()));

    m_pointWidgets << pointWidget;
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onSkeletonModified(vtkSmartPointer<vtkPolyData> polydata)
{
  auto widget = dynamic_cast<SkeletonWidget2D *>(sender());

  if(widget)
  {
    Q_ASSERT(m_item);
    auto undoStack    = getUndoStack();
    auto model        = getModel();
    auto segmentation = segmentationPtr(m_item);

    if(widget->getSkeleton()->GetNumberOfPoints() != 0)
    {
      ConnectionList connections;
      auto segmentationSPtr   = model->smartPointer(segmentation);
      auto classificationName = segmentation->category()->classificationName();

      // insert connections if it's a dendrite or axon
      if(classificationName.startsWith("Dendrite") || classificationName.startsWith("Axon"))
      {
        connections = GUI::Model::Utils::connections(polydata, model);
        for(auto &connection: connections)
        {
          connection.item1 = segmentationSPtr;
          Q_ASSERT(connection.item1 && connection.item1.get());
        }
      }

      // modification
      QString mode{"adding"};
      if(m_eraseButton->isChecked())         mode = "erasing";
      else if(m_moveButton->isChecked())     mode = "moving";
      else if(m_truncateButton->isChecked()) mode = "marking";

      WaitingCursor cursor;

      undoStack->beginMacro(tr("Modify skeleton of '%1' by %2 points.").arg(segmentationSPtr->data().toString()).arg(mode));
      undoStack->push(new ModifySkeletonCommand(segmentationSPtr, widget->getSkeleton(), connections));
      undoStack->endMacro();
    }
    else
    {
      // removal
      {
        WaitingCursor cursor;

        undoStack->beginMacro(tr("Remove segmentation '%1'.").arg(segmentation->data().toString()));
        undoStack->push(new RemoveSegmentations(segmentation, getModel()));
        undoStack->endMacro();
      }

      deactivateEventHandler();
    }
  }
  else
  {
    qWarning() << "onSkeletonModified received signal but couldn't identify the sender." << __FILE__ << __LINE__;
  }

  getViewState().refresh();
}

//--------------------------------------------------------------------
bool SkeletonEditionTool::acceptsNInputs(int n) const
{
  return (n == 1);
}

//--------------------------------------------------------------------
bool SkeletonEditionTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  bool hasRequiredData = true;

  for(auto segmentation : segmentations)
  {
    hasRequiredData &= hasSkeletonData(segmentation->output());
  }

  return hasRequiredData;
}

//--------------------------------------------------------------------
void SkeletonEditionTool::initRepresentationFactories()
{
  auto settingsList     = getPoolSettings<SegmentationSkeletonPoolSettings>(getContext());
  auto skeletonSettings = std::dynamic_pointer_cast<SegmentationSkeletonPoolSettings>(settingsList.first());
  auto representation2D = std::make_shared<SkeletonToolWidget2D>(m_eventHandler, skeletonSettings);

  connect(representation2D.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)),
          this,                   SLOT(onSkeletonWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)));

  m_factory = std::make_shared<TemporalPrototypes>(representation2D, TemporalRepresentation3DSPtr(), tr("%1 - Skeleton Widget 2D").arg(id()));

  settingsList              = getPoolSettings<ConnectionPoolSettings>(getContext());
  auto connectionSettings   = std::dynamic_pointer_cast<ConnectionPoolSettings>(settingsList.first());
  auto pointsRepresentation = std::make_shared<ConnectionPointsTemporalRepresentation2D>(connectionSettings);

  connect(pointsRepresentation.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)),
          this,                       SLOT(onPointWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)));

  m_pointsFactory = std::make_shared<TemporalPrototypes>(pointsRepresentation, TemporalRepresentation3DSPtr(), tr("%1 - Points representations").arg(id()));
}

//--------------------------------------------------------------------
void SkeletonEditionTool::initParametersWidgets()
{
  m_eraseButton = createToolButton(":/espina/eraser.png", tr("Erase skeleton nodes."));
  m_eraseButton->setCheckable(true);
  m_eraseButton->setChecked(false);

  connect(m_eraseButton, SIGNAL(clicked(bool)), this, SLOT(onModeChanged(bool)));
  connect(this, SIGNAL(toggled(bool)), m_eraseButton, SLOT(setVisible(bool)));

  addSettingsWidget(m_eraseButton);

  auto label = new QLabel(tr("Points distance:"));
  label->setToolTip(tr("Manage distance between points"));

  connect(this, SIGNAL(toggled(bool)), label, SLOT(setVisible(bool)));

  addSettingsWidget(label);

  m_minWidget = new DoubleSpinBoxAction();
  m_minWidget->setToolTip(tr("Minimum distance between points."));

  m_minWidget->setLabelText(tr("Minimum"));
  m_minWidget->setSuffix(tr(" nm"));
  m_minWidget->setValue(0.0);

  connect(m_minWidget, SIGNAL(valueChanged(double)),
          this,        SLOT(onMinimumDistanceChanged(double)));

  connect(this, SIGNAL(toggled(bool)), m_minWidget, SLOT(setVisible(bool)));

  addSettingsWidget(m_minWidget->createWidget(nullptr));

  m_maxWidget = new DoubleSpinBoxAction();
  m_maxWidget->setToolTip(tr("Maximum distance between points."));

  m_maxWidget->setLabelText(tr("Maximum"));
  m_maxWidget->setSuffix(tr(" nm"));
  m_maxWidget->setValue(0.0);

  connect(m_maxWidget, SIGNAL(valueChanged(double)),
          this,        SLOT(onMaximumDistanceChanged(double)));

  connect(this, SIGNAL(toggled(bool)), m_maxWidget, SLOT(setVisible(bool)));

  addSettingsWidget(m_maxWidget->createWidget(nullptr));

  auto strokeLabel = new QLabel{tr("Stroke type:")};
  strokeLabel->setToolTip(tr("Select stroke type."));

  addSettingsWidget(strokeLabel);

  m_strokeCombo = new QComboBox();
  m_strokeCombo->setEditable(false);
  m_strokeCombo->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
  m_strokeCombo->setToolTip(tr("Select stroke type."));

  connect(m_strokeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onStrokeTypeChanged(int)));

  addSettingsWidget(m_strokeCombo);

  m_strokeButton = createToolButton(":/espina/tag.svg", tr("Define the stroke types."));
  connect(m_strokeButton, SIGNAL(pressed()), this, SLOT(onStrokeConfigurationPressed()));

  addSettingsWidget(m_strokeButton);

  m_moveButton = createToolButton(":/espina/tubular_move.svg", tr("Translate skeleton nodes."));
  m_moveButton->setCheckable(true);
  m_moveButton->setChecked(false);

  connect(m_moveButton, SIGNAL(clicked(bool)), this, SLOT(onModeChanged(bool)));
  connect(this, SIGNAL(toggled(bool)), m_moveButton, SLOT(setVisible(bool)));

  addSettingsWidget(m_moveButton);

  m_truncateButton = createToolButton(":/espina/truncate.svg", tr("Mark branches as truncated."));
  m_truncateButton->setCheckable(true);
  m_truncateButton->setChecked(false);

  connect(m_truncateButton, SIGNAL(clicked(bool)), this, SLOT(onModeChanged(bool)));
  connect(this, SIGNAL(toggled(bool)), m_truncateButton, SLOT(setVisible(bool)));

  addSettingsWidget(m_truncateButton);

  m_changeHueButton = createToolButton(":/espina/skeletonColors.svg", tr("Modify coincident color strokes to facilitate visualization during edition."));
  m_changeHueButton->setCheckable(true);
  m_changeHueButton->setChecked(false);
  connect(m_changeHueButton, SIGNAL(clicked(bool)), this, SLOT(onHueModificationsButtonClicked(bool)));

  addSettingsWidget(m_changeHueButton);
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onModeChanged(bool value)
{
  auto button = dynamic_cast<GUI::Widgets::ToolButton *>(sender());
  auto tools  = QList<GUI::Widgets::ToolButton *>{ m_moveButton, m_eraseButton, m_truncateButton };

  if(value)
  {
    for(auto otherButton: tools)
    {
      if(otherButton == button) continue;

      if(otherButton->isChecked())
      {
        otherButton->blockSignals(true);
        otherButton->setChecked(false);
        otherButton->blockSignals(false);
      }
    }
  }

  auto enableOtherWidgets = !m_eraseButton->isChecked() && !m_moveButton->isChecked() && !m_truncateButton->isChecked();
  m_minWidget->setEnabled(enableOtherWidgets);
  m_maxWidget->setEnabled(enableOtherWidgets);
  m_strokeButton->setEnabled(enableOtherWidgets);
  m_strokeCombo->setEnabled(enableOtherWidgets);

  updateWidgetsMode();
}

//-----------------------------------------------------------------------------
void SkeletonEditionTool::onMinimumDistanceChanged(double value)
{
  m_eventHandler->setMinimumPointDistance(value);
  m_maxWidget->setSpinBoxMinimum(value);
}

//-----------------------------------------------------------------------------
void SkeletonEditionTool::onMaximumDistanceChanged(double value)
{
  m_eventHandler->setMaximumPointDistance(value);
  m_minWidget->setSpinBoxMaximum(value);
}

//--------------------------------------------------------------------
void SkeletonEditionTool::initEventHandler()
{
  m_eventHandler = std::make_shared<SkeletonToolsEventHandler>(getContext());
  m_eventHandler->setInterpolation(true);
  m_eventHandler->setCursor(Qt::CrossCursor);

  connect(m_eventHandler.get(), SIGNAL(checkStartNode(const NmVector3 &)),
          this,                 SLOT(onPointCheckRequested(const NmVector3 &)), Qt::DirectConnection);

  setEventHandler(m_eventHandler);
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onModifierPressed(bool value)
{
  if(!m_widgets.isEmpty() && m_widgets.first()->mode() != SkeletonWidget2D::Mode::MODIFY)
  {
    m_eraseButton->setChecked(!m_eraseButton->isChecked());
    updateWidgetsMode();
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::updateWidgetsMode()
{
  QList<NmVector3> points;
  auto mode = SkeletonWidget2D::Mode::CREATE;

  if(m_eraseButton->isChecked())         mode = SkeletonWidget2D::Mode::ERASE;
  else if(m_moveButton->isChecked())     mode = SkeletonWidget2D::Mode::MODIFY;
  else if(m_truncateButton->isChecked()) mode = SkeletonWidget2D::Mode::MARK;

  switch(mode)
  {
    case SkeletonWidget2D::Mode::CREATE:
      m_eventHandler->setMode(SkeletonEventHandler::Mode::CREATE);
      break;
    case SkeletonWidget2D::Mode::MARK:
      {
        auto model   = getModel();
        auto segPtr  = segmentationPtr(m_item);
        Q_ASSERT(segPtr);
        auto segSPtr = model->smartPointer(segPtr);
        Q_ASSERT(segSPtr);

        for (auto connection : model->connections(segSPtr))
        {
          if (!points.contains(connection.point))
          {
            points << connection.point;
          }
        }
      }
      m_eventHandler->setMode(SkeletonEventHandler::Mode::OTHER);
      break;
    default:
      m_eventHandler->setMode(SkeletonEventHandler::Mode::OTHER);
      break;
  }

  for(auto widget: m_widgets)
  {
    widget->setMode(mode);
    widget->setConnectionPoints(points);
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onStrokeTypeChanged(int index)
{
  if(m_item)
  {
    auto segmentation = segmentationPtr(m_item);
    auto name         = segmentation->category()->classificationName();

    index = std::min(std::max(0,index), m_strokes.size() - 1);
    const auto stroke = m_strokes.at(index);
    Q_ASSERT(index >= 0);

    for(auto widget: m_widgets)
    {
      widget->setStroke(stroke);
    }

    m_eventHandler->setStroke(stroke);
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onStrokeConfigurationPressed()
{
  if(m_item)
  {
    auto segmentation   = segmentationPtr(m_item);
    auto name           = segmentation->category()->classificationName();
    auto index          = m_strokeCombo->currentIndex();

    StrokeDefinitionDialog dialog(m_strokes, segmentation->category());

    connect(&dialog, SIGNAL(strokeModified(const Core::SkeletonStroke &)),    this, SLOT(onStrokeModified(const Core::SkeletonStroke &)), Qt::DirectConnection);
    connect(&dialog, SIGNAL(strokeRenamed(const QString &, const QString &)), this, SLOT(onStrokeRenamed(const QString &, const QString &)), Qt::DirectConnection);
    connect(&dialog, SIGNAL(strokeAdded(const Core::SkeletonStroke &)),       this, SLOT(onStrokeModified(const Core::SkeletonStroke &)), Qt::DirectConnection);
    connect(&dialog, SIGNAL(strokeRemoved(const Core::SkeletonStroke &)),     this, SLOT(onStrokeRemoved(const Core::SkeletonStroke &)), Qt::DirectConnection);

    dialog.exec();

    if(dialog.hasModifiedStrokes() && !m_widgets.isEmpty())
    {
      const auto skeleton = m_widgets.first()->getSkeleton();

      auto undoStack    = getUndoStack();
      auto model        = getModel();

      if(skeleton->GetNumberOfPoints() != 0)
      {
        ConnectionList connections;
        auto segmentationSPtr   = model->smartPointer(segmentation);
        auto classificationName = segmentation->category()->classificationName();

        // insert connections if it's a dendrite or axon
        if(classificationName.startsWith("Dendrite") || classificationName.startsWith("Axon"))
        {
          connections = GUI::Model::Utils::connections(skeleton, model);
          for(auto &connection: connections)
          {
            connection.item1 = segmentationSPtr;
            Q_ASSERT(connection.item1 && connection.item1.get());
          }
        }

        WaitingCursor cursor;

        undoStack->beginMacro(tr("Modify skeleton strokes of '%1'.").arg(segmentationSPtr->data().toString()));
        undoStack->push(new ModifySkeletonCommand(segmentationSPtr, skeleton, connections));
        undoStack->endMacro();
      }
    }

    index = std::min(std::max(0, index), m_strokes.size() -1);

    updateStrokes();

    onStrokeTypeChanged(index);
  }
}

//--------------------------------------------------------------------
bool SkeletonEditionTool::selectionIsNotBeingModified(SegmentationAdapterList segmentations)
{
  for(auto segmentation: segmentations)
  {
    if(segmentation == segmentationPtr(m_item)) continue;

    if(segmentation->isBeingModified()) return false;
  }

  return true;
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onStrokeChanged(const Core::SkeletonStroke stroke)
{
  if(m_item)
  {
    m_strokeCombo->blockSignals(true);
    m_strokeCombo->setCurrentIndex(m_strokes.indexOf(stroke));
    m_strokeCombo->blockSignals(false);

    m_eventHandler->setStroke(stroke);
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onSelectionChanged(SegmentationAdapterList segmentations)
{
  if(!isChecked()) return;

  if(segmentations.size() != 1)
  {
    abortOperation();
  }
  else
  {
    auto seg = segmentationPtr(m_item);
    auto selectedSeg = segmentations.first();

    if(selectedSeg == seg) return;

    if(selectedSeg->isBeingModified() || !hasSkeletonData(selectedSeg->output()))
    {
      abortOperation();
    }
    else
    {
      m_item->clearTemporalRepresentation();
      m_item->invalidateRepresentations();
      m_item->setBeingModified(false);

      m_item = selectedSeg;

      m_item->setTemporalRepresentation(std::make_shared<NullRepresentationPipeline>());
      m_item->setBeingModified(true);

      auto skeleton = readLockSkeleton(selectedSeg->output())->skeleton();
      auto definition = Core::toSkeletonDefinition(skeleton);

      m_strokes.clear();

      populateStrokes(definition.strokes);

      definition.clear();

      updateStrokes();

      SkeletonWidget2D::ClearRepresentation();
      SkeletonWidget2D::initializeData(readLockSkeleton(m_item->output())->skeleton());

      for(auto widget: m_widgets)
      {
        widget->updateRepresentation();
      }

      onStrokeTypeChanged(m_strokeCombo->currentIndex());

      updateWidgetsMode();
      m_item->invalidateRepresentations();

      getViewState().refresh();
    }
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::updateStatus()
{
  auto selection = getSelectedSegmentations();
  auto enabled = acceptsNInputs(selection.size())
              && (selectionIsNotBeingModified(selection) || m_allowSwich)
              && acceptsSelection(selection);

  setEnabled(enabled);
}

//--------------------------------------------------------------------
void SkeletonEditionTool::updateStrokes()
{
  if(m_item)
  {
    auto segmentation = segmentationPtr(m_item);
    auto category     = segmentation->category();
    const auto name   = category->classificationName();
	const auto hue    = category->color().hue();

    m_strokeCombo->blockSignals(true);
    m_strokeCombo->clear();

    const auto keys = STROKES.keys();
    if(keys.contains(name))
    {
      populateStrokes(STROKES[name]);
    }

    for(int i = 0; i < m_strokes.size(); ++i)
    {
      auto stroke = m_strokes.at(i);

      QPixmap original(ICONS.at(stroke.type));
      QPixmap copy(original.size());
	    const auto color = stroke.colorHue == -1 ? hue : stroke.colorHue;
      copy.fill(QColor::fromHsv(color,255,255));
      copy.setMask(original.createMaskFromColor(Qt::transparent));

      m_strokeCombo->insertItem(i, QIcon(copy), stroke.name);
    }

    m_strokeCombo->blockSignals(false);

    m_eventHandler->setStrokes(m_strokes, category);

    if(m_strokeCombo->currentIndex() < 0 || m_strokeCombo->currentIndex() >= m_strokes.size())
    {
      m_strokeCombo->setCurrentIndex(0);
    }

    m_eventHandler->setStroke(m_strokes.at(m_strokeCombo->currentIndex()));
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onPointCheckRequested(const NmVector3 &point)
{
  if(!m_widgets.isEmpty())
  {
    auto skeletonWidget = std::dynamic_pointer_cast<SkeletonToolWidget2D>(m_widgets.first());

    if(skeletonWidget)
    {
      m_eventHandler->setIsStartNode(skeletonWidget->isStartNode(point));
    }
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onHueModificationsButtonClicked(bool value)
{
  for(auto widget: m_widgets)
  {
    widget->setStrokeHueModification(value);
  }

  if(m_item) m_item->invalidateRepresentations();
}

//--------------------------------------------------------------------
void SkeletonEditionTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  settings->setValue(MODIFY_HUE_SETTINGS_KEY, m_changeHueButton->isChecked());
}

//--------------------------------------------------------------------
void SkeletonEditionTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  m_changeHueButton->setChecked(settings->value(MODIFY_HUE_SETTINGS_KEY, false).toBool());
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onExclusiveToolInUse(ProgressTool* tool)
{
  ProgressTool::onExclusiveToolInUse(tool);

  // clients want to switch from "creation" tool to "edition" tool without having to
  // deactivate the previous one. Sigh...
  m_allowSwich = (dynamic_cast<SkeletonCreationTool *>(tool) != nullptr);
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onTruncationSuccess()
{
  if(m_truncateButton->isChecked())
  {
    m_truncateButton->setChecked(false);
    onModeChanged(true);
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onStrokeModified(const Core::SkeletonStroke &stroke)
{
  if(!m_widgets.isEmpty())
  {
    auto widget = m_widgets.first();
    widget->setStroke(stroke);

    getViewState().refresh();
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onStrokeRenamed(const QString &oldName, const QString &newName)
{
  if(!m_widgets.isEmpty())
  {
    auto widget = m_widgets.first();
    widget->renameStroke(oldName, newName);

    getViewState().refresh();
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onStrokeRemoved(const Core::SkeletonStroke &stroke)
{
  if(!m_widgets.isEmpty())
  {
    auto widget = m_widgets.first();
    widget->removeStroke(stroke);

    getViewState().refresh();
  }
}

//--------------------------------------------------------------------
void SkeletonEditionTool::populateStrokes(const Core::SkeletonStrokes& strokes)
{
  auto segmentation = segmentationPtr(m_item);
  if(segmentation)
  {
    const auto defaultStrokes = SkeletonToolsUtils::defaultStrokes(segmentation->category());

    for(const auto &stroke: strokes)
    {
      // Some categories can have an unused stroke "Stroke" present in every category originated in early
      // skeleton tools and data development. So if the default strokes size is not 1 then we can hide it.
      if(stroke.name == "Stroke" && defaultStrokes.size() > 1) continue;

      auto equalOp = [&stroke](const Core::SkeletonStroke &other) { return stroke.name == other.name; };
      auto exists = std::any_of(m_strokes.constBegin(), m_strokes.constEnd(), equalOp);
      if(!exists) { m_strokes << stroke; }
    };
  }

  qSort(m_strokes);
}
