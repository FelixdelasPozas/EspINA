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
#include "Styles.h"

#include <GUI/EventHandlers/CircularBrush.h>
#include <GUI/EventHandlers/SphericalBrush.h>
#include <GUI/EventHandlers/ContourPainter.h>
#include <GUI/View/Widgets/Contour/ContourWidget2D.h>
#include <GUI/View/Widgets/Contour/ContourWidget2D.h>
#include <QHBoxLayout>
#include <QPushButton>

const QString BRUSH_RADIUS     = "Brush radius";
const QString BRUSH_OPACITY    = "Brush opacity";
const QString CONTOUR_DISTANCE = "Contour distance";
const QString MODE             = "Drawing mode";

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::Contour;
using namespace ESPINA::GUI::Widgets;

//------------------------------------------------------------------------
DrawingWidget::DrawingWidget(View::ViewState &viewState, ModelAdapterSPtr model, QWidget *parent)
: QWidget(parent)
, m_viewState           (viewState)
, m_categorySelector    {new CategorySelector(model)}
, m_radiusWidget        {new NumericalInput()}
, m_opacityWidget       {new NumericalInput()}
, m_eraserWidget        {Styles::createToolButton(":/espina/eraser.png", tr("Erase"))}
, m_rasterizeWidget     {Styles::createToolButton(":/espina/tick.png", tr("Rasterize the contour"))}
, m_showCategoryControls{true}
, m_showRadiusControls  {true}
, m_showOpacityControls {true}
, m_showEraserControls  {true}
, m_enabled             {true}
{
  // default values.
  m_opacity         = 50;
  m_brushRadius     = 20;
  m_contourDistance = 20;

  setLayout(new QHBoxLayout(this));

  initCategoryWidget();
  initPainters();
  initEraseWidget();
  initRadiusWidget();
  initOpacityWidget();
  initRasterizeWidget();

  m_painters.key(m_currentPainter)->setChecked(true);

  updateVisibleControls();
}

//------------------------------------------------------------------------
DrawingWidget::~DrawingWidget()
{
}

//------------------------------------------------------------------------
MaskPainterSPtr DrawingWidget::painter() const
{
  return m_currentPainter;
}

//------------------------------------------------------------------------
void DrawingWidget::setDrawingColor(const QColor &color)
{
  auto alphaColor = color;
  alphaColor.setAlphaF(m_opacity/100.0);

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
void DrawingWidget::showCategoryControls(bool value)
{
  m_showCategoryControls = value;

  updateVisibleControls();
}

//------------------------------------------------------------------------
void DrawingWidget::showRadiusControls(bool value)
{
  m_showRadiusControls = value;

  updateVisibleControls();
}

//------------------------------------------------------------------------
void DrawingWidget::showEraserControls(bool value)
{
  m_showOpacityControls = value;

  updateVisibleControls();
}

//------------------------------------------------------------------------
void DrawingWidget::showOpacityControls(bool value)
{
  m_showEraserControls = value;

  updateVisibleControls();
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
void DrawingWidget::abortOperation()
{
  stopDrawing();
  //unsetPainter();
}

//------------------------------------------------------------------------
void DrawingWidget::stopDrawing()
{
  if(m_currentPainter)
  {
    if(m_currentPainter.get() == m_contourPainter.get())
    {
      m_contourPainter->clearContours();
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
void DrawingWidget::changePainter(bool  checked)
{
  auto eventButton = static_cast<QPushButton *>(sender());

  Q_ASSERT(m_painters.keys().contains(eventButton));

  for (auto button : m_painters.keys())
  {
    if (button != eventButton)
    {
      button->blockSignals(true);
      button->setChecked(false);
      button->blockSignals(false);
    }
  }

  if (displayContourControls())
  {
    m_viewState.removeTemporalRepresentations(m_contourWidgetfactory);
  }

  m_currentPainter = checked?m_painters[eventButton]:nullptr;

  if (displayContourControls() && checked)
  {
    m_viewState.addTemporalRepresentations(m_contourWidgetfactory);
  }

  updateVisibleControls();

  emit painterChanged(m_currentPainter);
}

//------------------------------------------------------------------------
void DrawingWidget::initPainters()
{
  auto circularBrush      = std::make_shared<CircularBrush>();
  m_circularPainter       = std::make_shared<BrushPainter>(circularBrush);
  m_circularPainterAction = registerBrush(":/espina/brush_2D.svg",
                                          tr("Modify segmentation drawing 2D discs"),
                                          m_circularPainter);

  auto sphericalBrush      = std::make_shared<SphericalBrush>();
  m_sphericalPainter       = std::make_shared<BrushPainter>(sphericalBrush);
  m_sphericalPainterAction = registerBrush(":/espina/brush_3D.svg",
                                           tr("Modify segmentation drawing 3D spheres"),
                                           m_sphericalPainter);

  m_contourPainter       = std::make_shared<ContourPainter>();
  m_contourWidgetfactory = std::make_shared<TemporalPrototypes>(std::make_shared<ContourWidget2D>(m_contourPainter), TemporalRepresentation3DSPtr());

  m_contourPainter->setMinimumPointDistance(m_contourDistance);
  m_contourPainterAction   = registerPainter(":/espina/drawing_contour.svg",
                                             tr("Modify segmentation drawing contours"),
                                             m_contourPainter);

  m_currentPainter = m_circularPainter;
}

//------------------------------------------------------------------------
void DrawingWidget::initCategoryWidget()
{
  addWidget(m_categorySelector);

  connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
          this,               SLOT(onCategoryChange(CategoryAdapterSPtr)));
}

//------------------------------------------------------------------------
void DrawingWidget::initEraseWidget()
{
  addWidget(m_eraserWidget);

  m_eraserWidget->setCheckable(true);

  connect(m_eraserWidget, SIGNAL(toggled(bool)),
          this,           SLOT(setEraserMode(bool)));
}

//------------------------------------------------------------------------
void DrawingWidget::initRasterizeWidget()
{
  addWidget(m_rasterizeWidget);

  connect(m_rasterizeWidget,      SIGNAL(clicked(bool)),
          m_contourPainter.get(), SIGNAL(rasterize()));
}

//------------------------------------------------------------------------
void DrawingWidget::initRadiusWidget()
{
  addWidget(m_radiusWidget);

  m_radiusWidget->setValue(m_brushRadius);
  m_radiusWidget->setMinimum(5);
  m_radiusWidget->setMaximum(40);
  m_radiusWidget->setSpinBoxVisibility(false);
  m_radiusWidget->setLabelText(tr("Radius Size"));

  connect(m_radiusWidget, SIGNAL(valueChanged(int)),
          this,           SLOT(changeRadius(int)));
}

//------------------------------------------------------------------------
void DrawingWidget::initOpacityWidget()
{
  addWidget(m_opacityWidget);

  m_opacityWidget->setMinimum(1);
  m_opacityWidget->setMaximum(100);
  m_opacityWidget->setValue(m_opacity);
  m_opacityWidget->setSpinBoxVisibility(false);
  m_opacityWidget->setLabelText(tr("Opacity"));

  connect(m_opacityWidget, SIGNAL(valueChanged(int)),
          this,            SLOT(changeOpacity(int)));
}


//------------------------------------------------------------------------
void DrawingWidget::addWidget(QWidget* widget)
{
  layout()->addWidget(widget);
}

//------------------------------------------------------------------------
QPushButton *DrawingWidget::registerPainter(const QString  &icon,
                                            const QString  &description,
                                            MaskPainterSPtr painter)
{
  auto button = Styles::createToolButton(icon, description);

  button->setCheckable(true);

  connect(button, SIGNAL(clicked(bool)),
          this,   SLOT(changePainter(bool)));

  connect(painter.get(),     SIGNAL(stopPainting(BinaryMaskSPtr<unsigned char>)),
          this,              SIGNAL(maskPainted(BinaryMaskSPtr<unsigned char>)));

  connect(painter.get(),     SIGNAL(drawingModeChanged(DrawingMode)),
          this,              SLOT(onDrawingModeChange(DrawingMode)));


  m_painters[button] = painter;

  addWidget(button);

  return button;
}

//------------------------------------------------------------------------
QPushButton *DrawingWidget::registerBrush(const QString   &icon,
                                          const QString   &description,
                                          BrushPainterSPtr painter)
{
  auto button = registerPainter(icon, description, painter);

  painter->setRadius(m_brushRadius);

  connect(painter.get(), SIGNAL(radiusChanged(int)),
          this,          SLOT(radiusChanged(int)));

  connect(painter.get(), SIGNAL(strokeStarted(BrushPainter *, RenderView*)),
          this,          SIGNAL(strokeStarted(BrushPainter *, RenderView *)));

  return button;
}

//------------------------------------------------------------------------
bool DrawingWidget::displayBrushControls() const
{
  return !displayContourControls();
}

//------------------------------------------------------------------------
void DrawingWidget::restoreSettings(std::shared_ptr<QSettings> settings)
{
  m_opacity         = settings->value(BRUSH_OPACITY,    50).toInt();
  m_brushRadius     = settings->value(BRUSH_RADIUS,     20).toInt();
  m_contourDistance = settings->value(CONTOUR_DISTANCE, 20).toInt();

  auto eraserEnabled = settings->value(MODE, false).toBool();
  m_eraserWidget->setChecked(eraserEnabled);
}

//------------------------------------------------------------------------
void DrawingWidget::saveSettings(std::shared_ptr<QSettings> settings)
{
  settings->setValue(BRUSH_RADIUS,     m_brushRadius);
  settings->setValue(CONTOUR_DISTANCE, m_contourDistance);
  settings->setValue(BRUSH_OPACITY,    m_opacity);
  settings->setValue(MODE,             m_eraserWidget->isChecked());
}

//------------------------------------------------------------------------
bool DrawingWidget::displayContourControls() const
{
  return m_currentPainter == m_contourPainter;
}

//-----------------------------------------------------------------------------
void DrawingWidget::updateVisibleControls()
{
  m_categorySelector->setVisible(m_showCategoryControls);
  m_radiusWidget    ->setVisible(m_showRadiusControls  && displayBrushControls());
  m_opacityWidget   ->setVisible(m_showOpacityControls && displayBrushControls());
  m_eraserWidget    ->setVisible(m_showEraserControls);
  m_rasterizeWidget ->setVisible(displayContourControls());

  if (displayContourControls())
  {
    m_radiusWidget->setLabelText(tr("Minimum Point Distance"));
    m_radiusWidget->setValue(m_contourDistance);

    m_contourPainter->setMinimumPointDistance(m_contourDistance);
  }
  else
  {
    m_radiusWidget->setLabelText(tr("Radius Size"));
    m_radiusWidget->setValue(m_brushRadius);
  }
}

//-----------------------------------------------------------------------------
void DrawingWidget::changeRadius(int value)
{
  if(m_currentPainter.get() == m_contourPainter.get())
  {
    if(value == m_contourDistance) return;

    m_contourDistance = value;

    m_contourPainter->setMinimumPointDistance(m_contourDistance);
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
  m_eraserWidget->setEnabled(value);

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
    m_currentPainter->setDrawingMode(value ? DrawingMode::ERASING : DrawingMode::PAINTING);
  }

  m_eraserWidget->blockSignals(true);
  m_eraserWidget->setChecked(value);
  m_eraserWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void DrawingWidget::onCategoryChange(CategoryAdapterSPtr category)
{
  auto color = category->color();
  color.setAlphaF(m_opacityWidget->value()/100.0);

  if (m_showCategoryControls)
  {
    setDrawingColor(color);
  }

  emit categoryChanged(category);
}

//-----------------------------------------------------------------------------
void DrawingWidget::onDrawingModeChange(DrawingMode mode)
{
  if(sender() != m_currentPainter.get()) return;

  auto actualButtonMode = m_eraserWidget->isChecked() ? DrawingMode::ERASING : DrawingMode::PAINTING;

  if(actualButtonMode != mode)
  {
    m_eraserWidget->blockSignals(true);
    m_eraserWidget->setChecked(!m_eraserWidget->isChecked());
    m_eraserWidget->blockSignals(false);
  }
}
