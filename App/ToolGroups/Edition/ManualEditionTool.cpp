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
  : m_model               {model}
  , m_viewManager         {viewManager}
  , m_drawToolSelector    {new ActionSelector()}
  , m_categorySelector    {new CategorySelector(model)}
  , m_radiusWidget        {new SliderAction()}
  , m_opacityWidget       {new SliderAction()}
  , m_showOpacityControls {true}
  , m_showRadiusControls  {true}
  , m_showCategoryControls{true}
  , m_enabled             {false}
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

    m_categorySelector->setVisible(false);
    m_radiusWidget->setVisible(false);
    m_opacityWidget->setVisible(false);

    connect(m_viewManager.get(), SIGNAL(selectionChanged(SelectionSPtr)),
            this, SLOT(updateReferenceItem(SelectionSPtr)));

    auto selection = m_viewManager->selection();

    updateReferenceItem(selection);
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
    if(m_showCategoryControls)
      m_categorySelector->setVisible(true);

    if(m_showRadiusControls)
      m_radiusWidget->setVisible(true);

    if(m_showOpacityControls)
      m_opacityWidget->setVisible(true);

    SelectionSPtr selection = m_viewManager->selection();
    SegmentationAdapterList segs = selection->segmentations();
    QColor color = m_categorySelector->selectedCategory()->color();
    if (segs.size() == 1)
    {
      color = segs.first()->category()->color();
    }

    m_currentSelector = m_drawTools[action];
    m_currentSelector->setBrushColor(color);
    m_currentSelector->setRadius(m_radiusWidget->value());

    m_viewManager->setEventHandler(m_currentSelector);
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::unsetSelector()
  {
    if(m_showCategoryControls)
      this->m_categorySelector->setVisible(false);

    if(m_showRadiusControls)
      this->m_radiusWidget->setVisible(false);

    if(m_showOpacityControls)
      this->m_opacityWidget->setVisible(false);

    m_viewManager->setEventHandler(nullptr);
    m_currentSelector.reset();
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::categoryChanged(CategoryAdapterSPtr category)
  {
    if (m_currentSelector != nullptr)
    {
      auto selection = m_viewManager->selection();
      selection->clear();
      ChannelAdapterList channels;
      channels << m_viewManager->activeChannel();
      selection->set(channels);
      m_currentSelector->setBrushColor(category->color());
      updateReferenceItem(selection);
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
      updateReferenceItem(m_viewManager->selection());
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

    actions << m_drawToolSelector;
    actions << m_categorySelector;
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
    if (m_currentSelector != nullptr)
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

  //------------------------------------------------------------------------
  void ManualEditionTool::updateReferenceItem(SelectionSPtr selection)
  {
    QImage image;
    QColor borderColor{Qt::blue};

    ViewItemAdapterPtr item = nullptr;

    auto segs = selection->segmentations();
    if (segs.size() == 1)
    {
      item = segs.first();
    }
    else
    {
      item = m_viewManager->activeChannel();
      image = QImage(":/espina/add.svg");
    }

    if(selection->items().empty())
      item = m_viewManager->activeChannel();
    else
    {
      if(!selection->segmentations().empty())
        item = selection->segmentations().first();
      else
        item = selection->channels().first();
    }

    m_circularBrushSelector->setReferenceItem(item);
    m_circularBrushSelector->setBrushImage(image);

    m_sphericalBrushSelector->setReferenceItem(item);
    m_sphericalBrushSelector->setBrushImage(image);
  }

} // namespace EspINA
