/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "ManualEditionTool.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Widgets/SliderAction.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/Utils/SelectionUtils.h>

// Qt
#include <QAction>
#include <QSettings>

const QString BRUSH_RADIUS("ManualEditionTools::BrushRadius");
const QString BRUSH_OPACITY("ManualEditionTools::BrushOpacity");

using namespace ESPINA;


//------------------------------------------------------------------------
ManualEditionTool::ManualEditionTool(ModelAdapterSPtr model,
                                     ViewManagerSPtr  viewManager)
: m_model               {model}
, m_viewManager         {viewManager}
, m_drawToolSelector    {new ActionSelector()}
, m_categorySelector    {new CategorySelector(model)}
, m_radiusWidget        {new SliderAction()}
, m_opacityWidget       {new SliderAction()}
, m_eraserWidget        {new QAction(QIcon(":/espina/eraser.png"), tr("Erase"), this)}
, m_showCategoryControls{true}
, m_showRadiusControls  {true}
, m_showOpacityControls {true}
, m_showEraserControls  {true}
, m_enabled             {false}
, m_hasEnteredEraserMode{false}
{
  qRegisterMetaType<ViewItemAdapterPtr>("ViewItemAdapterPtr");
  qRegisterMetaType<CategoryAdapterSPtr>("CategoryAdapterSPtr");
  qRegisterMetaType<BinaryMaskSPtr<unsigned char>>("BinaryMaskSPtr<unsigned char>");

  // draw with a disc
  m_discTool = new QAction(QIcon(":/espina/pencil2D.png"),
                           tr("Modify segmentation drawing 2D discs"),
                           m_drawToolSelector);

  m_circularBrushSelector = std::make_shared<CircularBrushSelector>();
  connect(m_circularBrushSelector.get(), SIGNAL(itemsSelected(Selector::Selection)),
          this,                          SLOT(  drawStroke(Selector::Selection)));
  connect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
          m_drawToolSelector,            SLOT(         setChecked(bool)));
  connect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
          this,                          SLOT(      selectorInUse(bool)));
  connect(m_circularBrushSelector.get(), SIGNAL(radiusChanged(int)),
          this,                          SLOT(  radiusChanged(int)));
  connect(m_circularBrushSelector.get(), SIGNAL(drawingModeChanged(bool)),
          this,                          SLOT(  drawingModeChanged(bool)));

  m_drawTools[m_discTool] = m_circularBrushSelector;
  m_drawToolSelector->addAction(m_discTool);


  // draw with a sphere
  m_sphereTool = new QAction(QIcon(":espina/pencil3D.png"),
                             tr("Modify segmentation drawing 3D spheres"),
                             m_drawToolSelector);

  m_sphericalBrushSelector = std::make_shared<SphericalBrushSelector>();
  connect(m_sphericalBrushSelector.get(), SIGNAL(itemsSelected(Selector::Selection)),
          this,                           SLOT(  drawStroke(Selector::Selection)));
  connect(m_sphericalBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
          m_drawToolSelector,             SLOT(         setChecked(bool)));
  connect(m_sphericalBrushSelector.get(), SIGNAL(radiusChanged(int)),
          this,                           SLOT(  radiusChanged(int)));
  connect(m_sphericalBrushSelector.get(), SIGNAL(drawingModeChanged(bool)),
          this,                           SLOT(  drawingModeChanged(bool)));

  m_drawTools[m_sphereTool] = m_sphericalBrushSelector;
  m_drawToolSelector->addAction(m_sphereTool);

  m_contourTool = new QAction(QIcon(":espina/lasso.png"),
                              tr("Modify segmentation drawing contours"),
                              m_drawToolSelector);

  m_contourSelector = std::make_shared<ContourSelector>();

  connect(m_contourSelector.get(), SIGNAL(itemsSelected(Selector::Selection)),
          this,                    SLOT(  drawStroke(Selector::Selection)));
  connect(m_contourSelector.get(), SIGNAL(eventHandlerInUse(bool)),
          m_drawToolSelector,      SLOT(  setChecked(bool)));
  connect(m_contourSelector.get(), SIGNAL(drawingModeChanged(bool)),
          this,                    SLOT(  drawingModeChanged(bool)));

  m_drawTools[m_contourTool] = m_contourSelector;
  m_drawToolSelector->addAction(m_contourTool);

  m_drawToolSelector->setDefaultAction(m_discTool);
  connect(m_drawToolSelector, SIGNAL(   triggered(QAction*)),
          this,               SLOT(changeSelector(QAction*)));
  connect(m_drawToolSelector, SIGNAL(actionCanceled()),
          this,               SLOT(   unsetSelector()));

  ESPINA_SETTINGS(settings);
  int radius  = settings.value(BRUSH_RADIUS,  20).toInt();
  int opacity = settings.value(BRUSH_OPACITY, 50).toInt();

  m_radiusWidget->setValue(radius);
  m_radiusWidget->setLabelText(tr("Radius Size"));

  connect(m_radiusWidget, SIGNAL(valueChanged(int)),
          this, SLOT(changeRadius(int)));

  m_opacityWidget->setSliderMinimum(1);
  m_opacityWidget->setSliderMaximum(100);
  m_opacityWidget->setValue(opacity);
  m_opacityWidget->setLabelText(tr("Opacity"));

  connect(m_opacityWidget, SIGNAL(valueChanged(int)),
          this, SLOT(changeOpacity(int)));

  m_eraserWidget->setCheckable(true);

  connect(m_eraserWidget, SIGNAL(toggled(bool)),
          this, SLOT(setEraserMode(bool)));

  setControlVisibility(false);

  connect(m_viewManager->selection().get(), SIGNAL(selectionChanged()),
          this, SLOT(updateReferenceItem()));
}

//------------------------------------------------------------------------
ManualEditionTool::~ManualEditionTool()
{
  ESPINA_SETTINGS(settings);
  settings.setValue(BRUSH_RADIUS, m_radiusWidget->value());
  settings.setValue(BRUSH_OPACITY, m_opacityWidget->value());
  settings.sync();

  if (m_currentSelector)
  {
    m_viewManager->unsetEventHandler(m_currentSelector);
  }
}

//-----------------------------------------------------------------------------
void ManualEditionTool::changeSelector(QAction* action)
{
  Q_ASSERT(m_drawTools.keys().contains(action));

  setControlVisibility(true);

  updateReferenceItem();

  auto selector = m_drawTools[action];
  if(action == m_discTool || action == m_sphereTool)
  {
    if(m_contourWidget)
    {
      initializeContourWidget(false);
    }

    m_radiusWidget->setVisible(true);
    selector->setRadius(m_radiusWidget->value());
  }
  else // contour tool selected
  {
    m_radiusWidget->setVisible(false);

    if(!m_contourWidget)
    {
      initializeContourWidget(true);
    }
  }

  selector->setBrushOpacity(m_opacityWidget->value());

  auto selection = selectSegmentations(m_viewManager);
  QColor color = m_categorySelector->selectedCategory()->color();
  if(selection.size() == 1)
  {
    color = m_viewManager->colorEngine()->color(selection.first());
  }
  selector->setBrushColor(color);

  m_currentSelector = m_drawTools[action];
  m_viewManager->setEventHandler(m_currentSelector);
}

//-----------------------------------------------------------------------------
void ManualEditionTool::unsetSelector()
{
  if (m_currentSelector != nullptr)
  {
    setControlVisibility(false);

    m_drawToolSelector->blockSignals(true);
    m_drawToolSelector->setChecked(false);
    m_drawToolSelector->blockSignals(false);

    auto referenceItem = m_currentSelector->referenceItem();
    m_circularBrushSelector->setReferenceItem(nullptr);
    m_sphericalBrushSelector->setReferenceItem(nullptr);
    m_contourSelector->setReferenceItem(nullptr);

    emit stopDrawing(referenceItem, m_hasEnteredEraserMode);

    setEraserMode(false);

    auto selector = m_currentSelector; //avoid re-entering this function on unset event
    if(m_currentSelector == m_contourSelector)
    {
      initializeContourWidget(false);
    }
    m_currentSelector.reset();

    // This tool can be unset either by the tool itself or by other
    // event handler through the view manager
    m_viewManager->unsetEventHandler(selector);
  }
}

//-----------------------------------------------------------------------------
void ManualEditionTool::categoryChanged(CategoryAdapterSPtr unused)
{
  if (m_categorySelector)
  {
    m_eraserWidget->setChecked(false);

    if(m_viewManager->activeChannel() == nullptr)
    {
      return;
    }

    ChannelAdapterList channels;
    channels << m_viewManager->activeChannel();

    if(!channels.empty())
    {
      auto selection = m_viewManager->selection();
      selection->clear();
      selection->set(channels);
    }
  }

  updateReferenceItem();
}

//-----------------------------------------------------------------------------
void ManualEditionTool::changeRadius(int value)
{
  if (m_currentSelector != nullptr)
  {
    m_currentSelector->setRadius(m_radiusWidget->value());
  }
}

//-----------------------------------------------------------------------------
void ManualEditionTool::changeOpacity(int value)
{
  if (m_currentSelector != nullptr)
  {
    m_currentSelector->setBrushOpacity(m_opacityWidget->value());
  }
}

//-----------------------------------------------------------------------------
void ManualEditionTool::selectorInUse(bool value)
{
  if (value)
  {
    updateReferenceItem();
    m_hasEnteredEraserMode = false;
  }
  else
  {
    unsetSelector();
  }
}

//------------------------------------------------------------------------
void ManualEditionTool::setEnabled(bool value)
{
  m_enabled = value;
  m_categorySelector->setEnabled(value);
  m_drawToolSelector->setEnabled(value);
  m_radiusWidget->setEnabled(value);
}

//------------------------------------------------------------------------
bool ManualEditionTool::enabled() const
{
  return m_enabled;
}

//------------------------------------------------------------------------
QList<QAction *> ManualEditionTool::actions() const
{
  QList<QAction *> actions;

  if (m_currentSelector != nullptr)
  {
    m_drawToolSelector->setChecked(m_viewManager->eventHandler() == m_currentSelector);
  }
  else
  {
    m_drawToolSelector->setChecked(false);
  }

  actions << m_drawToolSelector;
  actions << m_categorySelector;
  actions << m_eraserWidget;
  actions << m_radiusWidget;
  actions << m_opacityWidget;

  return actions;
}

//------------------------------------------------------------------------
void ManualEditionTool::drawStroke(Selector::Selection selection)
{
  auto mask = selection.first().first;
  auto category = m_categorySelector->selectedCategory();
  emit stroke(category, mask);
}

//------------------------------------------------------------------------
void ManualEditionTool::abortOperation()
{
  unsetSelector();
}

//------------------------------------------------------------------------
void ManualEditionTool::radiusChanged(int value)
{
  m_radiusWidget->blockSignals(true);
  m_radiusWidget->setValue(value);
  m_radiusWidget->blockSignals(false);
}

//------------------------------------------------------------------------
void ManualEditionTool::drawingModeChanged(bool isDrawing)
{
  QAction *actualAction = m_drawToolSelector->getCurrentAction();
  QIcon icon;

  m_hasEnteredEraserMode |= !isDrawing; //sticky

  if (m_discTool == actualAction)
  {
    if (isDrawing)
    {
      icon = QIcon(":/espina/pencil2D.png");
    }
    else
    {
      icon = QIcon(":/espina/eraser2D.png");
    }
  }
  else
  {
    if (m_sphereTool == actualAction)
    {
      if (isDrawing)
      {
        icon = QIcon(":/espina/pencil3D.png");
      }
      else
      {
        icon = QIcon(":/espina/eraser3D.png");
      }
    }
    else
    {
      if(m_contourTool == actualAction)
      {
        if (isDrawing)
        {
          icon = QIcon(":/espina/lasso.png");
        }
        else
        {
          icon = QIcon(":/espina/lassoErase.png");
        }
      }
    }
  }

  m_drawToolSelector->setIcon(icon);
}

//------------------------------------------------------------------------
void ManualEditionTool::updateReferenceItem()
{
  QImage image;
  QColor borderColor {Qt::blue};
  QColor fillColor   {Qt::gray};
  bool   enableEraser{true};

  ViewItemAdapterPtr currentItem = nullptr;
  ViewItemAdapterPtr item = nullptr;

  if (m_currentSelector)
  {
    currentItem = m_currentSelector->referenceItem();
  }

  auto selection     = m_viewManager->selection();
  auto segmentations = selection->segmentations();

  if (segmentations.empty() || !hasVolumetricData(segmentations.first()->output()))
  {
    image = QImage(":/espina/brush_new.svg");
    enableEraser = false;

    if (m_currentSelector)
    {
      setEraserMode(false);
    }
  }

  if(selection->items().empty() || selection->segmentations().empty())
  {
    item = m_viewManager->activeChannel();
  }
  else
  {
    auto segmentation = segmentations.first();
    auto category     = segmentation->category();

    m_categorySelector->blockSignals(true);
    if (m_categorySelector->selectedCategory() != category)
    {
      m_categorySelector->selectCategory(category);
    }
    m_categorySelector->blockSignals(false);

    item = segmentation;
  }

  auto category = m_categorySelector->selectedCategory();
  QColor color;
  auto seg = dynamic_cast<SegmentationAdapterPtr>(item);
  if(seg)
  {
    color = m_viewManager->colorEngine()->color(seg);
  }
  else
  {
    color = category->color();
  }

  m_circularBrushSelector ->setBrushColor(color);
  m_sphericalBrushSelector->setBrushColor(color);
  m_contourSelector       ->setBrushColor(color);

  m_circularBrushSelector ->setReferenceItem(item);
  m_sphericalBrushSelector->setReferenceItem(item);
  m_contourSelector       ->setReferenceItem(item);

  if (currentItem && currentItem != item)
  {
    emit stopDrawing(currentItem, m_hasEnteredEraserMode);
  }

  m_circularBrushSelector ->setBrushImage(image);
  m_sphericalBrushSelector->setBrushImage(image);
  m_contourSelector       ->setBrushImage(image);

  m_eraserWidget->setEnabled(enableEraser);
}

//------------------------------------------------------------------------
void ManualEditionTool::setControlVisibility(bool visible)
{
  if(m_showCategoryControls) m_categorySelector->setVisible(visible);
  if(m_showRadiusControls)   m_radiusWidget    ->setVisible(visible);
  if(m_showOpacityControls)  m_opacityWidget   ->setVisible(visible);
  if(m_showEraserControls)   m_eraserWidget    ->setVisible(visible);

  if (visible)
  {
    auto currentCategory = currentReferenceCategory();

    if (currentCategory)
    {
      m_categorySelector->blockSignals(true);
      m_categorySelector->selectCategory(currentCategory);
      m_categorySelector->blockSignals(false);
    }

    connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
            this,               SLOT(  categoryChanged(CategoryAdapterSPtr)));
  }
  else
  {
    disconnect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
               this,               SLOT(  categoryChanged(CategoryAdapterSPtr)));
  }
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ManualEditionTool::currentReferenceCategory()
{
  CategoryAdapterSPtr currentCategory{nullptr};

  if (m_currentSelector)
  {
    auto item = m_currentSelector->referenceItem();

    if (item && isSegmentation(item))
    {
      auto segmentation = segmentationPtr(item);

      currentCategory = segmentation->category();
    }
  }

  return currentCategory;
}

//------------------------------------------------------------------------
void ManualEditionTool::setEraserMode(bool value)
{
  m_currentSelector->setEraseMode(value);
  drawingModeChanged(!value);
  m_eraserWidget->blockSignals(true);
  m_eraserWidget->setChecked(value);
  m_eraserWidget->blockSignals(false);
}

//------------------------------------------------------------------------
void ManualEditionTool::initializeContourWidget(bool value)
{
  if(value)
  {
    if(!m_contourWidget)
    {
      m_contourWidget = std::make_shared<ContourWidget>();

      connect(m_contourWidget.get(), SIGNAL(endContour()),
              this,                   SLOT(contourEnded()));
      connect(m_contourWidget.get(), SIGNAL(rasterizeContours(ContourWidget::ContourList)),
              this,                   SLOT(rasterizeContour(ContourWidget::ContourList)));
    }
    std::dynamic_pointer_cast<ContourSelector>(m_contourSelector)->setContourWidget(m_contourWidget.get());
    m_viewManager->addWidget(m_contourWidget);
    m_contourWidget->setEnabled(true);
    resetContourTool();
  }
  else
  {
    auto contours = m_contourWidget->getContours();

    if(!contours.empty())
    {
      emit drawContours(contours);
    }

    disconnect(m_contourWidget.get(), SIGNAL(endContour()),
               this,                   SLOT(contourEnded()));
    disconnect(m_contourWidget.get(), SIGNAL(rasterizeContours(ContourWidget::ContourList)),
               this,                   SLOT(rasterizeContour(ContourWidget::ContourList)));

    std::dynamic_pointer_cast<ContourSelector>(m_contourSelector)->setContourWidget(nullptr);
    m_viewManager->removeWidget(m_contourWidget);
    m_viewManager->updateViews();
    m_contourWidget->setEnabled(false);
    m_contourWidget = nullptr;
  }
}

//------------------------------------------------------------------------
void ManualEditionTool::resetContourTool()
{
  if(!m_contourWidget) return;

  m_contourWidget->setMode(BrushSelector::BrushMode::BRUSH);

  auto category = m_categorySelector->selectedCategory();
  auto selectedSegs = selectSegmentations(m_viewManager);
  QColor color;
  if(selectedSegs.empty())
  {
    color = category->color();
  }
  else
  {
    color = m_viewManager->colorEngine()->color(selectedSegs.first());
  }

  m_contourWidget->setPolygonColor(color);
}

//------------------------------------------------------------------------
ContourWidget::ContourData ManualEditionTool::getContour()
{
  auto result = ContourWidget::ContourData();

  if(m_contourWidget)
  {
    auto contours = m_contourWidget->getContours();

    if(!contours.empty())
    {
      result = contours.first();
    }
  }

  return result;
}

//------------------------------------------------------------------------
void ManualEditionTool::setContour(ContourWidget::ContourData contour)
{
  if(m_contourWidget)
  {
    m_contourWidget->initialize(contour);
  }
}

//------------------------------------------------------------------------
void ManualEditionTool::contourEnded()
{
  qDebug() << "ManualEditionTools::contourEnded";
  auto item = m_currentSelector->referenceItem();
  auto seg = dynamic_cast<SegmentationAdapterPtr>(item);

  emit contourEnded(seg, m_categorySelector->selectedCategory());

}
