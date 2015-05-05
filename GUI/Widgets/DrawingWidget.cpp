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

// ESPINA
#include "DrawingWidget.h"
#include "ActionSelector.h"
#include "CategorySelector.h"
#include "NumericalInput.h"

#include <GUI/EventHandlers/CircularBrush.h>
#include <GUI/EventHandlers/SphericalBrush.h>
#include <GUI/EventHandlers/ContourPainter.h>
#include <GUI/View/Widgets/Contour/ContourWidget2D.h>
#include <GUI/View/Widgets/WidgetFactory.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/Widgets/Styles.h>
#include <Support/Widgets/Tool.h>
#include <QHBoxLayout>
#include <QPushButton>

const QString BRUSH_RADIUS("ManualEditionTools::BrushRadius");
const QString BRUSH_OPACITY("ManualEditionTools::BrushOpacity");
const QString CONTOUR_DISTANCE("ManualEditionTools::ContourDistance");

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::Contour;

//------------------------------------------------------------------------
DrawingWidget::DrawingWidget(Support::Context &context)
: m_context             (context)
, m_painterSelector     {new ActionSelector()}
, m_nestedWidgets       {new QWidgetAction(this)}
, m_categorySelector    {new CategorySelector(context.model())}
, m_radiusWidget        {new NumericalInput()}
, m_opacityWidget       {new NumericalInput()}
, m_eraserWidget        {Tool::createToolButton(":/espina/eraser.png", tr("Erase"))}
, m_rasterizeWidget     {Tool::createToolButton(":/espina/tick.png", tr("Rasterize the contour"))}
, m_showCategoryControls{true}
, m_showRadiusControls  {true}
, m_showOpacityControls {true}
, m_showEraserControls  {true}
, m_enabled             {true}
{
  initPainters();

  m_contourWidgetfactory = std::make_shared<WidgetFactory>(std::make_shared<ContourWidget2D>(m_contourPainter), EspinaWidget3DSPtr());

  initDrawingControls();

  setControlVisibility(false);
}

//------------------------------------------------------------------------
DrawingWidget::~DrawingWidget()
{
  ESPINA_SETTINGS(settings);
  settings.setValue(BRUSH_RADIUS,     m_brushRadius);
  settings.setValue(CONTOUR_DISTANCE, m_contourDistance);
  settings.setValue(BRUSH_OPACITY,    m_opacityWidget->value());
  settings.sync();

  if (m_currentPainter)
  {
    m_context.viewState().unsetEventHandler(m_currentPainter);
  }
}

//------------------------------------------------------------------------
void DrawingWidget::setDrawingColor(const QColor &color)
{
  auto alphaColor = color;
  alphaColor.setAlphaF(m_opacityWidget->value()/100.0);

  for (auto painter : m_painters)
  {
    painter->setColor(alphaColor);
  }
}

//------------------------------------------------------------------------
void DrawingWidget::setDrawingBorderColor(const QColor &color)
{
  for(auto painter: m_painters)
  {
    auto brush = std::dynamic_pointer_cast<BrushPainter>(painter);
    if(brush)
    {
      brush->setBorderColor(color);
    }
  }
}

//------------------------------------------------------------------------
void DrawingWidget::setBrushImage(const QImage &image)
{
  for(auto painter: m_painters)
  {
    auto brush = std::dynamic_pointer_cast<BrushPainter>(painter);
    if(brush)
    {
      brush->setImage(image);
    }
  }
}

//------------------------------------------------------------------------
void DrawingWidget::clearBrushImage()
{
  for(auto painter: m_painters)
  {
    auto brush = std::dynamic_pointer_cast<BrushPainter>(painter);
    if(brush)
    {
      brush->clearImage();
    }
  }
}

//------------------------------------------------------------------------
CategoryAdapterSPtr DrawingWidget::selectedCategory() const
{
  return m_categorySelector->selectedCategory();
}

//------------------------------------------------------------------------
void DrawingWidget::setMaskProperties(const NmVector3 &spacing, const NmVector3 &origin)
{
  for (auto painter : m_painters)
  {
    painter->setMaskProperties(spacing, origin);
  }
}

//------------------------------------------------------------------------
void DrawingWidget::setCategory(CategoryAdapterSPtr category)
{
  if (m_categorySelector->selectedCategory() != category)
  {
    m_categorySelector->selectCategory(category);
  }
}

//------------------------------------------------------------------------
QList<QAction *> DrawingWidget::actions() const
{
  QList<QAction *> actions;

  auto checked = m_currentPainter && m_context.viewState().eventHandler() == m_currentPainter;
  m_painterSelector->setChecked(checked);

  actions << m_painterSelector;
  actions << m_nestedWidgets;

  return actions;
}

//------------------------------------------------------------------------
void DrawingWidget::abortOperation()
{
  stopDrawing();
  unsetPainter();
}

//------------------------------------------------------------------------
void DrawingWidget::stopDrawing()
{
  if(m_currentPainter)
  {
    if(m_currentPainter.get() == m_contourPainter.get())
    {
      m_rasterizeWidget->setVisible(false);
      auto contourPainter = std::dynamic_pointer_cast<ContourPainter>(m_contourPainter);
      contourPainter->clearContours();
    }
  }
}

//------------------------------------------------------------------------
void DrawingWidget::setEnabled(bool value)
{
  if(m_enabled == value) return;

  for(auto action: actions())
  {
    action->setEnabled(value);
  }

  m_enabled = value;
}

//------------------------------------------------------------------------
void DrawingWidget::changePainter(QAction *action)
{
  Q_ASSERT(m_painters.keys().contains(action));

  setControlVisibility(true);

  m_currentPainter = m_painters[action];

  m_context.viewState().setEventHandler(m_currentPainter);
}


//------------------------------------------------------------------------
void DrawingWidget::unsetPainter()
{
  if (m_currentPainter != nullptr)
  {
    setControlVisibility(false);

    m_painterSelector->blockSignals(true);
    m_painterSelector->setChecked(false);
    m_painterSelector->blockSignals(false);

    setEraserMode(false);

    auto selector = m_currentPainter; //avoid re-entering this function on unset event
    m_currentPainter.reset();

    // This tool can be unset either by the tool itself or by other
    // event handler through the view manager
    m_context.viewState().unsetEventHandler(selector);
  }
}

//------------------------------------------------------------------------
void DrawingWidget::initPainters()
{
  auto circularBrush      = std::make_shared<CircularBrush>();
  m_circularPainter       = std::make_shared<BrushPainter>(circularBrush);
  m_circularPainterAction = registerBrush(QIcon(":/espina/pencil2D.png"),
                                           tr("Modify segmentation drawing 2D discs"),
                                           m_circularPainter);

  auto sphericalBrush      = std::make_shared<SphericalBrush>();
  m_sphericalPainter       = std::make_shared<BrushPainter>(sphericalBrush);
  m_sphericalPainterAction = registerBrush(QIcon(":/espina/pencil3D.png"),
                                           tr("Modify segmentation drawing 3D spheres"),
                                           m_sphericalPainter);

  auto contourPainter = std::make_shared<ContourPainter>();

  ESPINA_SETTINGS(settings);
  m_contourDistance = settings.value(CONTOUR_DISTANCE, 20).toInt();
  contourPainter->setMinimumPointDistance(m_contourDistance);
  m_contourPainterAction   = registerPainter(QIcon(":/espina/lasso.png"),
                                             tr("Modify segmentation drawing contours"),
                                             contourPainter);
  m_contourPainter = contourPainter;


  m_currentPainter = m_circularPainter;
  m_painterSelector->setDefaultAction(m_circularPainterAction);

  connect(m_painterSelector, SIGNAL(triggered(QAction*)),
          this,              SLOT(changePainter(QAction*)));
  connect(m_painterSelector, SIGNAL(actionCanceled()),
          this,              SLOT(unsetPainter()));
}

//------------------------------------------------------------------------
void DrawingWidget::initDrawingControls()
{
  ESPINA_SETTINGS(settings);

  m_contourDistance = settings.value(CONTOUR_DISTANCE, 20).toInt();

  auto widget = new QWidget();
  auto layout = new QHBoxLayout();

  initCategoryWidget (layout);
  initEraseWidget    (layout);
  initRasterizeWidget(layout);
  initRadiusWidget   (layout, settings);
  initOpacityWidget  (layout, settings);

  widget->setLayout(layout);
  Support::Widgets::Styles::setNestedStyle(widget);

  m_nestedWidgets->setDefaultWidget(widget);
}


//------------------------------------------------------------------------
void DrawingWidget::initCategoryWidget(QHBoxLayout *layout)
{
  layout->addWidget(m_categorySelector);

  connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
          this,               SLOT(onCategoryChange(CategoryAdapterSPtr)));
}

//------------------------------------------------------------------------
void DrawingWidget::initEraseWidget(QHBoxLayout *layout)
{
  m_eraserWidget->setCheckable(true);

  connect(m_eraserWidget, SIGNAL(toggled(bool)),
          this,           SLOT(setEraserMode(bool)));

  layout->addWidget(m_eraserWidget);
}

//------------------------------------------------------------------------
void DrawingWidget::initRasterizeWidget(QHBoxLayout *layout)
{
  m_rasterizeWidget->setVisible(false);

  connect(m_rasterizeWidget,      SIGNAL(clicked(bool)),
          m_contourPainter.get(), SIGNAL(rasterize()));

  layout->addWidget(m_rasterizeWidget);
}

//------------------------------------------------------------------------
void DrawingWidget::initRadiusWidget(QHBoxLayout *layout, const QSettings &settings)
{
  layout->addWidget(m_radiusWidget);

  m_brushRadius = settings.value(BRUSH_RADIUS,  20).toInt();

  m_radiusWidget->setValue(m_brushRadius);
  m_radiusWidget->setMinimum(5);
  m_radiusWidget->setMaximum(40);
  m_radiusWidget->setSpinBoxVisibility(false);
  m_radiusWidget->setLabelText(tr("Radius Size"));

  connect(m_radiusWidget, SIGNAL(valueChanged(int)),
          this,           SLOT(changeRadius(int)));
}

//------------------------------------------------------------------------
void DrawingWidget::initOpacityWidget(QHBoxLayout *layout, const QSettings &settings)
{
  layout->addWidget(m_opacityWidget);

  int opacity = settings.value(BRUSH_OPACITY, 50).toInt();

  m_opacityWidget->setMinimum(1);
  m_opacityWidget->setMaximum(100);
  m_opacityWidget->setValue(opacity);
  m_opacityWidget->setSpinBoxVisibility(false);
  m_opacityWidget->setLabelText(tr("Opacity"));

  connect(m_opacityWidget, SIGNAL(valueChanged(int)),
          this,            SLOT(changeOpacity(int)));
}


//------------------------------------------------------------------------
QAction *DrawingWidget::registerPainter(const QIcon    &icon,
                                        const QString  &description,
                                        MaskPainterSPtr painter)
{

  auto action = new QAction(icon, description, m_painterSelector);

  connect(painter.get(),     SIGNAL(eventHandlerInUse(bool)),
          m_painterSelector, SLOT(setChecked(bool)));
  connect(painter.get(),     SIGNAL(eventHandlerInUse(bool)),
          this,              SLOT(selectorInUse(bool)));
  connect(painter.get(),     SIGNAL(stopPainting(BinaryMaskSPtr<unsigned char>)),
          this,              SIGNAL(maskPainted(BinaryMaskSPtr<unsigned char>)));
  connect(painter.get(),     SIGNAL(drawingModeChanged(DrawingMode)),
          this,              SLOT(onDrawingModeChange(DrawingMode)));


  m_painters[action] = painter;
  m_painterSelector->addAction(action);

  return action;
}

//------------------------------------------------------------------------
QAction *DrawingWidget::registerBrush(const QIcon     &icon,
                                      const QString   &description,
                                      BrushPainterSPtr painter)
{
  auto action = registerPainter(icon, description, painter);

  auto brush = std::dynamic_pointer_cast<BrushPainter>(painter);
  if(brush)
  {
    brush->setRadius(m_brushRadius);
  }

  connect(painter.get(), SIGNAL(radiusChanged(int)),
          this,          SLOT(radiusChanged(int)));
  connect(painter.get(), SIGNAL(strokeStarted(BrushPainter *, RenderView*)),
          this,          SIGNAL(strokeStarted(BrushPainter *, RenderView *)));

  return action;
}

//------------------------------------------------------------------------
void DrawingWidget::setControlVisibility(bool visible)
{
  if(m_showCategoryControls) m_categorySelector->setVisible(visible);
  if(m_showRadiusControls)   m_radiusWidget    ->setVisible(visible);
  if(m_showOpacityControls)  m_opacityWidget   ->setVisible(visible);
  if(m_showEraserControls)   m_eraserWidget    ->setVisible(visible);

  m_nestedWidgets->setVisible(visible);
}

//-----------------------------------------------------------------------------
void DrawingWidget::changeRadius(int value)
{
  if(m_currentPainter.get() == m_contourPainter.get())
  {
    if(value == m_contourDistance) return;

    m_contourDistance = value;

    std::dynamic_pointer_cast<ContourPainter>(m_contourPainter)->setMinimumPointDistance(m_contourDistance);
  }
  else
  {
    if(value == m_brushRadius) return;

    m_brushRadius = value;

    m_circularPainter->blockSignals(true);
    m_circularPainter->setRadius(m_brushRadius);
    m_circularPainter->blockSignals(false);
    m_sphericalPainter->blockSignals(true);
    m_sphericalPainter->setRadius(m_brushRadius);
    m_sphericalPainter->blockSignals(false);
  }
}

//------------------------------------------------------------------------
void DrawingWidget::radiusChanged(int value)
{
  m_radiusWidget->blockSignals(true);
  m_radiusWidget->setValue(value);
  m_radiusWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void DrawingWidget::changeOpacity(int value)
{
  auto color = m_currentPainter->color();
  color.setAlphaF(value/100.0);

  for(auto painter: m_painters)
  {
    painter->setColor(color);
  }
}

//------------------------------------------------------------------------
void DrawingWidget::setCanErase(bool value)
{
  for (auto painter : m_painters)
  {
    painter->setCanErase(value);
  }

  if (!value)
  {
    setEraserMode(false);
  }
}

//------------------------------------------------------------------------
void DrawingWidget::setEraserMode(bool value)
{
  if (m_currentPainter)
  {
    m_currentPainter->setDrawingMode(value?DrawingMode::ERASING:DrawingMode::PAINTING);
  }

  drawingModeChanged(!value);

  m_eraserWidget->blockSignals(true);
  m_eraserWidget->setChecked(value);
  m_eraserWidget->blockSignals(false);
}


//------------------------------------------------------------------------
void DrawingWidget::drawingModeChanged(bool isDrawing)
{
  QAction *actualAction = m_painterSelector->getCurrentAction();
  QIcon icon;

  if (m_circularPainterAction == actualAction)
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
    if (m_sphericalPainterAction == actualAction)
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
      if(m_contourPainterAction == actualAction)
      {
        if(isDrawing)
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

  m_painterSelector->setIcon(icon);
}

//-----------------------------------------------------------------------------
void DrawingWidget::selectorInUse(bool value)
{
  auto selector = qobject_cast<MaskPainter *>(sender());

  if (value)
  {
    auto color = m_categorySelector->selectedCategory()->color();
    color.setAlphaF(m_opacityWidget->value()/100.0);

    for(auto painter: m_painters)
    {
      painter->setColor(color);
    }

    if(selector == m_contourPainter.get())
    {
      m_rasterizeWidget->setVisible(true);
      m_radiusWidget->setLabelText(tr("Minimum Point Distance"));
      m_radiusWidget->setValue(m_contourDistance);

      m_context.viewState().addWidgets(m_contourWidgetfactory);

      auto contourPainter = std::dynamic_pointer_cast<ContourPainter>(m_contourPainter);
      contourPainter->setMinimumPointDistance(m_contourDistance);
    }
    else
    {
      m_radiusWidget->setLabelText(tr("Radius Size"));
      m_radiusWidget->setValue(m_brushRadius);
    }
  }
  else
  {
    if(selector == m_contourPainter.get())
    {
      m_rasterizeWidget->setVisible(false);

      m_contourDistance = m_radiusWidget->value();

      m_context.viewState().removeWidgets(m_contourWidgetfactory);
    }
    else
    {
      m_brushRadius = m_radiusWidget->value();
    }

    unsetPainter();
  }
}

//-----------------------------------------------------------------------------
void DrawingWidget::onCategoryChange(CategoryAdapterSPtr category)
{
  auto color = category->color();
  color.setAlphaF(m_opacityWidget->value()/100.0);

  setDrawingColor(color);

  emit categoryChanged(category);
}

//-----------------------------------------------------------------------------
void DrawingWidget::onDrawingModeChange(DrawingMode mode)
{
  auto value = DrawingMode::PAINTING == mode;

  drawingModeChanged(value);

  m_eraserWidget->blockSignals(true);
  m_eraserWidget->setChecked(!value);
  m_eraserWidget->blockSignals(false);
}
