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
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Widgets/SliderAction.h>
#include <Support/Settings/EspinaSettings.h>

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

  m_circularBrushSelector = CircularBrushSelectorSPtr(new CircularBrushSelector());
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

  m_sphericalBrushSelector = SphericalBrushSelectorSPtr(new SphericalBrushSelector());
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

  // TODO: contour filter, tool y selector.

  // draw with contour
  //    QAction *contourTool = new QAction(QIcon(":espina/lasso.png"),
  //                                       tr("Modify segmentation drawing contour"),
  //                                       m_drawToolSelector);
  //    FilledContourSPtr contour(new FilledContour(m_model,
  //                                                m_undoStack,
  //                                                m_viewManager));
  //
  //    connect(contour.get(), SIGNAL(changeMode(Brush::BrushMode)),
  //            this, SLOT(changeContourMode(Brush::BrushMode)));
  //    connect(contour.get(), SIGNAL(stopDrawing()),
  //            this, SLOT(cancelDrawOperation()));
  //    connect(contour.get(), SIGNAL(startDrawing()),
  //            this, SLOT(startContourOperation()));
  //
  //    m_drawTools[contourTool] = contour;
  //    m_drawTools[contourTool] = SelectorSPtr(this);
  //    m_drawToolSelector->addAction(contourTool);

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

  //     SelectionSPtr selection      = m_viewManager->selection();
  //     SegmentationAdapterList segs = selection->segmentations();
  //
  //     QColor color = m_categorySelector->selectedCategory()->color();
  //     if (segs.size() == 1)
  //     {
  //       color = segs.first()->category()->color();
  //     }

  m_currentSelector = m_drawTools[action];
  //m_currentSelector->setBrushColor(color);
  m_currentSelector->setRadius(m_radiusWidget->value());

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
    m_circularBrushSelector->setReferenceItem(nullptr);

    emit stopDrawing(referenceItem, m_hasEnteredEraserMode);

    setEraserMode(false);

    auto selector = m_currentSelector; //avoid re-entering this function on unset event
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

    auto selection = m_viewManager->selection();
    selection->clear();

    ChannelAdapterList channels;
    channels << m_viewManager->activeChannel();
    selection->set(channels);
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
    m_currentSelector->setBrushOpacity(m_opacityWidget->value());
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
  } else {
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
      icon = QIcon(":/espina/pencil2D.png");
    else
      icon = QIcon(":/espina/eraser2D.png");
  }
  else
  {
    if (m_sphereTool == actualAction)
    {
      if (isDrawing)
        icon = QIcon(":/espina/pencil3D.png");
      else
        icon = QIcon(":/espina/eraser3D.png");
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

  if (segmentations.size() == 1)
  {
    item = segmentations.first();
  }
  else
  {
    item = m_viewManager->activeChannel();
    image = QImage(":/espina/brush_new.svg");
    enableEraser = false;

    if (m_currentSelector)
    {
      setEraserMode(false);
    }
  }

  if(selection->items().empty())
  {
    item = m_viewManager->activeChannel();
  }
  else
  {
    if(selection->segmentations().empty())
    {
      item = selection->channels().first();
    }
    else
    {
      auto segmentation = selection->segmentations().first();
      auto category     = segmentation->category();

      m_categorySelector->blockSignals(true);
      if (m_categorySelector->selectedCategory() != category)
      {
        m_categorySelector->selectCategory(category);
      }
      m_categorySelector->blockSignals(false);

      item = segmentation;
    }
  }

  auto category = m_categorySelector->selectedCategory();

  m_circularBrushSelector ->setBrushColor(category->color());
  m_sphericalBrushSelector->setBrushColor(category->color());


  m_circularBrushSelector ->setReferenceItem(item);
  m_sphericalBrushSelector->setReferenceItem(item);

  if (currentItem && currentItem != item)
  {
    emit stopDrawing(currentItem, m_hasEnteredEraserMode);
  }

  m_circularBrushSelector ->setBrushImage(image);
  m_sphericalBrushSelector->setBrushImage(image);

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
  } else {
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
