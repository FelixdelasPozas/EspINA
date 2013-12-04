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

#include <Core/Analysis/Filter.h>
#include "ManualEditionTool.h"
#include "SpinBoxAction.h"
#include <Support/Settings/EspinaSettings.h>
#include <Filters/FreeFormSource.h>
#include <App/Tools/Brushes/CircularBrushSelector.h>
#include <App/Tools/Brushes/SphericalBrushSelector.h>

#include <QAction>
#include <QSettings>

using EspINA::Filter;

const QString BRUSHRADIUS("ManualEditionTools::BrushRadius");

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
  , m_radiusWidget(new SpinBoxAction())
  , m_enabled(false)
  {
    m_factory->registerFilterFactory(this);

    connect(m_radiusWidget, SIGNAL(radiusChanged(int)),
            this, SLOT(changeRadius(int)));

    // draw with a disc
    QAction *discTool = new QAction(QIcon(":/espina/pencil2D.png"),
                                    tr("Modify segmentation drawing 2D discs"),
                                    m_drawToolSelector);

    m_circularBrushSelector = CircularBrushSelectorSPtr(new CircularBrushSelector(m_viewManager));
    connect(m_circularBrushSelector.get(), SIGNAL(stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,  SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
    connect(m_circularBrushSelector.get(), SIGNAL(selectorInUse(bool)),
            m_drawToolSelector, SLOT(setChecked(bool)));
    connect(m_circularBrushSelector.get(), SIGNAL(selectorInUse(bool)),
            this, SLOT(selectorInUse(bool)));

    m_drawTools[discTool] = m_circularBrushSelector;
    m_drawToolSelector->addAction(discTool);


    // draw with a sphere
    QAction *sphereTool = new QAction(QIcon(":espina/pencil3D.png"),
                                      tr("Modify segmentation drawing 3D spheres"),
                                      m_drawToolSelector);

    m_sphericalBrushSelector = SphericalBrushSelectorSPtr(new SphericalBrushSelector(m_viewManager));
    connect(m_sphericalBrushSelector.get(), SIGNAL(stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,  SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
    connect(m_sphericalBrushSelector.get(), SIGNAL(stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
            this,  SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
    connect(m_sphericalBrushSelector.get(), SIGNAL(selectorInUse(bool)),
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
    int radius = settings.value(BRUSHRADIUS, 20).toInt();

    m_radiusWidget->setRadius(radius);
    m_radiusWidget->setLabelText(tr("Brush Radius"));
  }
  
  //------------------------------------------------------------------------
  ManualEditionTool::~ManualEditionTool()
  {
    if (m_actualSelector)
      m_viewManager->unsetSelector(m_actualSelector);
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::changeSelector(QAction* action)
  {
    Q_ASSERT(m_drawTools.keys().contains(action));

    m_actualSelector = m_drawTools[action];
    m_circularBrushSelector->initBrush();
    m_circularBrushSelector->setRadius(m_radiusWidget->radius());
    m_sphericalBrushSelector->initBrush();
    m_sphericalBrushSelector->setRadius(m_radiusWidget->radius());

    m_viewManager->setSelector(m_actualSelector);
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::unsetSelector()
  {
    qDebug() << "unsed";
    m_viewManager->unsetSelector(m_actualSelector);
    m_actualSelector.reset();
  }

  //-----------------------------------------------------------------------------
  void ManualEditionTool::changeRadius(int value)
  {
    if (m_actualSelector != nullptr)
    {
      m_circularBrushSelector->setRadius(m_radiusWidget->radius());
      m_sphericalBrushSelector->setRadius(m_radiusWidget->radius());
    }
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
      {
        m_circularBrushSelector->initBrush();
        m_sphericalBrushSelector->initBrush();
      }
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

    actions << m_drawToolSelector;
    actions << m_radiusWidget;

    return actions;
  }

} // namespace EspINA
