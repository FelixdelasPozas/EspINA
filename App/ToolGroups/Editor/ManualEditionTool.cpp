/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#include "ManualEditionTool.h"

#include <App/Tools/Brushes/CircularBrushSelector.h>
#include <App/Tools/Brushes/SphericalBrushSelector.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Widgets/SliderAction.h>
#include <Support/Settings/EspinaSettings.h>

#include <QAction>
#include <QSettings>

const QString BRUSH_RADIUS("ManualEditionTools::BrushRadius");
const QString BRUSH_OPACITY("ManualEditionTools::BrushOpacity");

namespace EspINA
{
  //------------------------------------------------------------------------
  ManualEditionTool::ManualEditionTool(ModelAdapterSPtr model,
                                       ViewManagerSPtr  viewManager)
  : m_model(model)
  , m_viewManager(viewManager)
  , m_drawToolSelector(new ActionSelector())
  , m_categorySelector(new CategorySelector(model))
  , m_radiusWidget(new SliderAction())
  , m_opacityWidget(new SliderAction())
  , m_showOpacityControls{true}
  , m_showRadiusControls{true}
  , m_showCategoryControls{true}
  , m_enabled(false)
  {
    qRegisterMetaType<ViewItemAdapterPtr>("ViewItemAdapterPtr");
    qRegisterMetaType<CategoryAdapterSPtr>("CategoryAdapterSPtr");
    qRegisterMetaType<BinaryMaskSPtr<unsigned char>>("BinaryMaskSPtr<unsigned char>");

    connect(m_radiusWidget, SIGNAL(valueChanged(int)),
            this, SLOT(changeRadius(int)));

    connect(m_opacityWidget, SIGNAL(valueChanged(int)),
            this, SLOT(changeOpacity(int)));

    // draw with a disc
    m_discTool = new QAction(QIcon(":/espina/pencil2D.png"),
                             tr("Modify segmentation drawing 2D discs"),
                             m_drawToolSelector);

    m_circularBrushSelector = CircularBrushSelectorSPtr(new CircularBrushSelector(m_viewManager));
    connect(m_circularBrushSelector.get(), SIGNAL(  stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,                          SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
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

    m_sphericalBrushSelector = SphericalBrushSelectorSPtr(new SphericalBrushSelector(m_viewManager));
    connect(m_sphericalBrushSelector.get(), SIGNAL(  stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,                           SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
    connect(m_sphericalBrushSelector.get(), SIGNAL(  stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,                           SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
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
    connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
            this,               SLOT(  categoryChanged(CategoryAdapterSPtr)));

    QSettings settings(CESVIMA, ESPINA);
    int radius  = settings.value(BRUSH_RADIUS,  20).toInt();
    int opacity = settings.value(BRUSH_OPACITY, 50).toInt();

    m_radiusWidget->setValue(radius);
    m_radiusWidget->setLabelText(tr("Radius Size"));

    m_opacityWidget->setSliderMinimum(1);
    m_opacityWidget->setSliderMaximum(100);
    m_opacityWidget->setValue(opacity);
    m_opacityWidget->setLabelText(tr("Opacity"));
  }
  
  //------------------------------------------------------------------------
  ManualEditionTool::~ManualEditionTool()
  {
    QSettings settings(CESVIMA, ESPINA);
    settings.setValue(BRUSH_RADIUS, m_radiusWidget->value());
    settings.setValue(BRUSH_OPACITY, m_opacityWidget->value());
    settings.sync();

    if (m_currentSelector)
      m_viewManager->unsetEventHandler(m_currentSelector);
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::changeSelector(QAction* action)
  {
    Q_ASSERT(m_drawTools.keys().contains(action));

    SelectionSPtr selection = m_viewManager->selection();
    SegmentationAdapterList segs = selection->segmentations();
    QColor color = m_categorySelector->selectedCategory()->color();
    if (segs.size() == 1)
    {
      color = segs.first()->category()->color();
    }

    m_currentSelector = m_drawTools[action];
    m_currentSelector->setBrushColor(color);
    m_currentSelector->initBrush();
    m_currentSelector->setRadius(m_radiusWidget->value());

    m_viewManager->setEventHandler(m_currentSelector);
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::unsetSelector()
  {
    m_viewManager->unsetEventHandler(m_currentSelector);
    m_currentSelector.reset();
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::categoryChanged(CategoryAdapterSPtr category)
  {
    if (m_currentSelector)
    {
      m_currentSelector->setBrushColor(category->color());
    }
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::changeRadius(int value)
  {
    if (m_currentSelector != nullptr)
      m_currentSelector->setRadius(m_radiusWidget->value());
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
    if (!value)
    {
      m_currentSelector = nullptr;
      emit stopDrawing();
    }
    else
    {
      if (m_viewManager->activeCategory() && m_viewManager->activeChannel())
        m_currentSelector->initBrush();
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

    if (m_currentSelector)
    {
      m_drawToolSelector->setChecked(m_viewManager->eventHandler() == m_currentSelector);
    }

    if (m_showCategoryControls)
      actions << m_categorySelector;

    actions << m_drawToolSelector;

    if (m_showRadiusControls)
      actions << m_radiusWidget;

    if (m_showOpacityControls)
      actions << m_opacityWidget;

    return actions;
  }

  //------------------------------------------------------------------------
  void ManualEditionTool::drawStroke(ViewItemAdapterPtr item, Selector::WorldRegion region, Nm radius, Plane plane)
  {
    auto mask = m_currentSelector->voxelSelectionMask();
    auto category = m_categorySelector->selectedCategory();
    emit stroke(item, category, mask);

    m_currentSelector->initBrush();
  }

  //------------------------------------------------------------------------
  void ManualEditionTool::abortOperation()
  {
    if (m_currentSelector)
      m_currentSelector->abortOperation();
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

} // namespace EspINA
