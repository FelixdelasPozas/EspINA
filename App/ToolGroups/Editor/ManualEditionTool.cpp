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

#include "ManualEditionTool.h"

#include <App/Tools/Brushes/CircularBrushSelector.h>
#include <App/Tools/Brushes/SphericalBrushSelector.h>
#include <Core/Analysis/Filter.h>
#include <GUI/Widgets/SliderAction.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Filters/FreeFormSource.h>
#include <Filters/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.h>
#include <Support/Settings/EspinaSettings.h>
#include <Undo/AddSegmentations.h>
#include <App/Undo/BrushUndoCommand.h>

#include <QAction>
#include <QSettings>

using EspINA::Filter;

const QString BRUSH_RADIUS("ManualEditionTools::BrushRadius");
const QString BRUSH_OPACITY("ManualEditionTools::BrushOpacity");

const Filter::Type FREEFORM_FILTER    = "FreeFormSource";
const Filter::Type FREEFORM_FILTER_V4 = "EditorToolBar::FreeFormSource";

namespace EspINA
{
  //-----------------------------------------------------------------------------
  FilterTypeList ManualEditionTool::ManualFilterFactory::providedFilters() const
  {
    FilterTypeList filters;

    filters << FREEFORM_FILTER << FREEFORM_FILTER_V4;

    return filters;
  }

  //-----------------------------------------------------------------------------
  FilterSPtr ManualEditionTool::ManualFilterFactory::createFilter(InputSList         inputs,
                                                                  const Filter::Type& filter,
                                                                  SchedulerSPtr       scheduler) const throw(Unknown_Filter_Exception)
  {
    if (FREEFORM_FILTER != filter && FREEFORM_FILTER_V4 != filter) throw Unknown_Filter_Exception();

    auto ffsFilter = FilterSPtr{new FreeFormSource(inputs, FREEFORM_FILTER, scheduler)};
    if (!m_fetchBehaviour)
    {
      m_fetchBehaviour = FetchBehaviourSPtr{new MarchingCubesFromFetchedVolumetricData()};
    }
    ffsFilter->setFetchBehaviour(m_fetchBehaviour);

    return ffsFilter;
  }

  //------------------------------------------------------------------------
  ManualEditionTool::ManualEditionTool(ModelAdapterSPtr model,
                                       ModelFactorySPtr factory,
                                       ViewManagerSPtr  viewManager,
                                       QUndoStack      *undoStack)
  : m_model(model)
  , m_factory(factory)
  , m_viewManager(viewManager)
  , m_undoStack(undoStack)
  , m_filterFactory(new ManualFilterFactory())
  , m_drawToolSelector(new ActionSelector())
  , m_categorySelector(new CategorySelector(model))
  , m_radiusWidget(new SliderAction())
  , m_opacityWidget(new SliderAction())
  , m_enabled(false)
  {
    m_factory->registerFilterFactory(m_filterFactory);

    connect(m_radiusWidget, SIGNAL(valueChanged(int)),
            this, SLOT(changeRadius(int)));

    connect(m_opacityWidget, SIGNAL(valueChanged(int)),
            this, SLOT(changeOpacity(int)));

    // draw with a disc
    m_discTool = new QAction(QIcon(":/espina/pencil2D.png"),
                             tr("Modify segmentation drawing 2D discs"),
                             m_drawToolSelector);

    m_circularBrushSelector = CircularBrushSelectorSPtr(new CircularBrushSelector(m_viewManager, m_categorySelector));
    connect(m_circularBrushSelector.get(), SIGNAL(stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,  SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
    connect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
            m_drawToolSelector, SLOT(setChecked(bool)));
    connect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
            this, SLOT(selectorInUse(bool)));
    connect(m_circularBrushSelector.get(), SIGNAL(radiusChanged(int)), this, SLOT(radiusChanged(int)));
    connect(m_circularBrushSelector.get(), SIGNAL(drawingModeChanged(bool)), this, SLOT(drawingModeChanged(bool)));

    m_drawTools[m_discTool] = m_circularBrushSelector;
    m_drawToolSelector->addAction(m_discTool);


    // draw with a sphere
    m_sphereTool = new QAction(QIcon(":espina/pencil3D.png"),
                               tr("Modify segmentation drawing 3D spheres"),
                               m_drawToolSelector);

    m_sphericalBrushSelector = SphericalBrushSelectorSPtr(new SphericalBrushSelector(m_viewManager, m_categorySelector));
    connect(m_sphericalBrushSelector.get(), SIGNAL(stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,  SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
    connect(m_sphericalBrushSelector.get(), SIGNAL(stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,  SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
    connect(m_sphericalBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
            m_drawToolSelector, SLOT(setChecked(bool)));
    connect(m_sphericalBrushSelector.get(), SIGNAL(radiusChanged(int)), this, SLOT(radiusChanged(int)));
    connect(m_sphericalBrushSelector.get(), SIGNAL(drawingModeChanged(bool)), this, SLOT(drawingModeChanged(bool)));

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
    connect(m_drawToolSelector, SIGNAL(triggered(QAction*)),
            this, SLOT(changeSelector(QAction*)));
    connect(m_drawToolSelector, SIGNAL(actionCanceled()),
            this, SLOT(unsetSelector()));

    QSettings settings(CESVIMA, ESPINA);
    int radius = settings.value(BRUSH_RADIUS, 20).toInt();
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

    if (m_actualSelector)
      m_viewManager->unsetEventHandler(m_actualSelector);
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::changeSelector(QAction* action)
  {
    Q_ASSERT(m_drawTools.keys().contains(action));

    m_actualSelector = m_drawTools[action];
    m_actualSelector->initBrush();
    m_actualSelector->setRadius(m_radiusWidget->value());

    m_viewManager->setEventHandler(m_actualSelector);
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::unsetSelector()
  {
    m_viewManager->unsetEventHandler(m_actualSelector);
    m_actualSelector.reset();
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::changeRadius(int value)
  {
    if (m_actualSelector != nullptr)
      m_actualSelector->setRadius(m_radiusWidget->value());
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::changeOpacity(int value)
  {
    if (m_actualSelector != nullptr)
      m_actualSelector->setBrushOpacity(m_opacityWidget->value());
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::selectorInUse(bool value)
  {
    if (!value)
    {
      m_actualSelector = nullptr;
      emit stopDrawing();
    }
    else
    {
      if (value && m_viewManager->activeCategory() && m_viewManager->activeChannel())
        m_actualSelector->initBrush();
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

    actions << m_categorySelector;
    actions << m_drawToolSelector;
    actions << m_radiusWidget;
    actions << m_opacityWidget;

    return actions;
  }

  //------------------------------------------------------------------------
  void ManualEditionTool::drawStroke(ViewItemAdapterPtr item, Selector::WorldRegion region, Nm radius, Plane plane)
  {
    auto mask = m_actualSelector->voxelSelectionMask();
    SegmentationAdapterSPtr segmentation;

    switch(item->type())
    {
      case ViewItemAdapter::Type::CHANNEL:
      {
        auto adapter = m_factory->createFilter<FreeFormSource>(InputSList(), FREEFORM_FILTER);
        auto filter = adapter->get();
        filter->setMask(mask);

        segmentation = m_factory->createSegmentation(adapter, 0);

        auto category = m_categorySelector->selectedCategory();
        segmentation->setCategory(category);

        auto channelItem = static_cast<ChannelAdapterPtr>(item);

        SampleAdapterSList samples;
        samples << QueryAdapter::sample(channelItem);
        Q_ASSERT(channelItem && (samples.size() == 1));

        m_undoStack->beginMacro(tr("Add Segmentation"));
        m_undoStack->push(new AddSegmentations(segmentation, samples, m_model));
        m_undoStack->endMacro();

        SegmentationAdapterList list;
        list << segmentation.get();
        m_viewManager->selection()->set(list);
      }
        break;
      case ViewItemAdapter::Type::SEGMENTATION:
      {
        segmentation = m_model->smartPointer(reinterpret_cast<SegmentationAdapterPtr>(item));
        m_undoStack->beginMacro(tr("Modify Segmentation"));
        m_undoStack->push(new DrawUndoCommand(segmentation, mask));
        m_undoStack->endMacro();
      }
        break;
      default:
        Q_ASSERT(false);
        break;
    }

    m_actualSelector->initBrush();

    m_viewManager->updateSegmentationRepresentations(segmentation.get());
    m_viewManager->updateViews();
  }

  //------------------------------------------------------------------------
  void ManualEditionTool::abortOperation()
  {
    if (m_actualSelector)
      m_actualSelector->abortOperation();
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
