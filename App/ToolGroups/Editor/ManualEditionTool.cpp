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
#include <GUI/Widgets/SpinBoxAction.h>
#include <Filters/FreeFormSource.h>
#include <Support/Settings/EspinaSettings.h>
#include <Undo/AddSegmentations.h>
#include <App/Undo/BrushUndoCommand.h>

#include <QAction>
#include <QSettings>

using EspINA::Filter;

const QString BRUSH_RADIUS("ManualEditionTools::BrushRadius");
const QString BRUSH_OPACITY("ManualEditionTools::BrushOpacity");

const Filter::Type FREEFORM_FILTER = "FreeFormSource";

namespace EspINA
{
  //------------------------------------------------------------------------
  ManualEditionTool::ManualEditionTool(ModelAdapterSPtr model,
                                       ModelFactorySPtr factory,
                                       ViewManagerSPtr  viewManager,
                                       QUndoStack      *undoStack)
  : m_model(model)
  , m_factory(factory)
  , m_viewManager(viewManager)
  , m_undoStack(undoStack)
  , m_drawToolSelector(new ActionSelector())
  , m_categorySelector(new CategorySelector(model))
  , m_radiusWidget(new SpinBoxAction())
  , m_opacityWidget(new SpinBoxAction())
  , m_enabled(false)
  {
    m_factory->registerFilterFactory(this);

    connect(m_radiusWidget, SIGNAL(valueChanged(int)),
            this, SLOT(changeRadius(int)));

    connect(m_opacityWidget, SIGNAL(valueChanged(int)),
            this, SLOT(changeOpacity(int)));

    // draw with a disc
    QAction *discTool = new QAction(QIcon(":/espina/pencil2D.png"),
                                    tr("Modify segmentation drawing 2D discs"),
                                    m_drawToolSelector);

    m_circularBrushSelector = CircularBrushSelectorSPtr(new CircularBrushSelector(m_viewManager, m_categorySelector));
    connect(m_circularBrushSelector.get(), SIGNAL(stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,  SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
    connect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
            m_drawToolSelector, SLOT(setChecked(bool)));
    connect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
            this, SLOT(selectorInUse(bool)));

    m_drawTools[discTool] = m_circularBrushSelector;
    m_drawToolSelector->addAction(discTool);


    // draw with a sphere
    QAction *sphereTool = new QAction(QIcon(":espina/pencil3D.png"),
                                      tr("Modify segmentation drawing 3D spheres"),
                                      m_drawToolSelector);

    m_sphericalBrushSelector = SphericalBrushSelectorSPtr(new SphericalBrushSelector(m_viewManager, m_categorySelector));
    connect(m_sphericalBrushSelector.get(), SIGNAL(stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,  SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
    connect(m_sphericalBrushSelector.get(), SIGNAL(stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,  SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
    connect(m_sphericalBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
            m_drawToolSelector, SLOT(setChecked(bool)));

    m_drawTools[sphereTool] = m_sphericalBrushSelector;
    m_drawToolSelector->addAction(sphereTool);

    // draw with contour
    QAction *contourTool = new QAction(QIcon(":espina/lasso.png"),
                                       tr("Modify segmentation drawing contour"),
                                       m_drawToolSelector);

    // TODO: contour filter, tool y selector.
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
    m_drawToolSelector->addAction(contourTool);

    m_drawToolSelector->setDefaultAction(discTool);
    connect(m_drawToolSelector, SIGNAL(triggered(QAction*)),
            this, SLOT(changeSelector(QAction*)));
    connect(m_drawToolSelector, SIGNAL(actionCanceled()),
            this, SLOT(unsetSelector()));

    QSettings settings(CESVIMA, ESPINA);
    int radius = settings.value(BRUSH_RADIUS, 20).toInt();
    int opacity = settings.value(BRUSH_OPACITY, 50).toInt();

    m_radiusWidget->setValue(radius);
    m_radiusWidget->setLabelText(tr("Brush Radius"));

    m_opacityWidget->setSpinBoxMinimum(1);
    m_opacityWidget->setSpinBoxMaximum(100);
    m_opacityWidget->setValue(opacity);
    m_opacityWidget->setSuffix("%");
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
      SelectionSPtr selection = m_viewManager->selection();

      if (value && m_viewManager->activeCategory() && m_viewManager->activeChannel())
        m_actualSelector->initBrush();
    }
  }

  //-----------------------------------------------------------------------------
  FilterTypeList ManualEditionTool::providedFilters() const
  {
    FilterTypeList filters;

    filters << FREEFORM_FILTER;

    return filters;
  }

  //-----------------------------------------------------------------------------
  FilterSPtr ManualEditionTool::createFilter(OutputSList         inputs,
                                             const Filter::Type& filter,
                                             SchedulerSPtr       scheduler) const throw(Unknown_Filter_Exception)
  {
    if (filter != FREEFORM_FILTER) throw Unknown_Filter_Exception();

    return FilterSPtr{new FreeFormSource(inputs, filter, scheduler)};
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
        auto adapter = m_factory->createFilter<FreeFormSource>(OutputSList(), FREEFORM_FILTER);
        auto filter = adapter->get();
        filter->setMask(mask);

        segmentation = m_factory->createSegmentation(adapter, 0);

        auto category = m_categorySelector->selectedCategory();
        segmentation->setCategory(category);

        m_undoStack->beginMacro(tr("Add Segmentation"));
        m_undoStack->push(new AddSegmentations(segmentation, m_model));
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
    m_actualSelector->abortOperation();
  }

} // namespace EspINA
