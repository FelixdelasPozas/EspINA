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
#include "SpinBoxAction.h"
#include <Support/Settings/EspinaSettings.h>

#include <QAction>
#include <QSettings>

const QString BRUSHRADIUS("ManualEditionTools::BrushRadius");

namespace EspINA
{
  //------------------------------------------------------------------------
  ManualEditionTool::ManualEditionTool()
  : m_drawToolSelector(new ActionSelector())
  , m_radiusWidget(new SpinBoxAction())
  , m_enabled(false)
  {
    // draw with a disc
    QAction *discTool = new QAction(QIcon(":/espina/pencil2D.png"),
                                    tr("Modify segmentation drawing 2D discs"),
                                    m_drawToolSelector);

//    CircularBrushSPtr circularBrush(new CircularBrush(m_model,
//                                                      m_settings,
//                                                      m_undoStack,
//                                                      m_viewManager));
//    connect(circularBrush.get(), SIGNAL(stopDrawing()),
//            this, SLOT(cancelDrawOperation()));
//    connect(circularBrush.get(), SIGNAL(brushModeChanged(Brush::BrushMode)),
//            this, SLOT(changeCircularBrushMode(Brush::BrushMode)));

//    m_drawTools[discTool] =  circularBrush;
//    m_drawTools[discTool] = SelectorSPtr(this);
    m_drawToolSelector->addAction(discTool);

    // draw with a sphere
    QAction *sphereTool = new QAction(QIcon(":espina/pencil3D.png"),
                                      tr("Modify segmentation drawing 3D spheres"),
                                      m_drawToolSelector);

//    SphericalBrushSPtr sphericalBrush(new SphericalBrush(m_model,
//                                                         m_settings,
//                                                         m_undoStack,
//                                                         m_viewManager));
//    connect(sphericalBrush.get(), SIGNAL(stopDrawing()),
//            this, SLOT(cancelDrawOperation()));
//    connect(sphericalBrush.get(), SIGNAL(brushModeChanged(Brush::BrushMode)),
//            this, SLOT(changeSphericalBrushMode(Brush::BrushMode)));

//    m_drawTools[sphereTool] = sphericalBrush;
//    m_drawTools[sphereTool] = SelectorSPtr(this);
    m_drawToolSelector->addAction(sphereTool);

    // draw with contour
    QAction *contourTool = new QAction(QIcon(":espina/lasso.png"),
                                       tr("Modify segmentation drawing contour"),
                                       m_drawToolSelector);

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

//    m_drawTools[contourTool] = contour;
//    m_drawTools[contourTool] = SelectorSPtr(this);
    m_drawToolSelector->addAction(contourTool);

    // Add Draw Tool Selector to Editor Tool Bar
    m_drawToolSelector->setCheckable(true);
//    addAction(m_drawToolSelector);
//    connect(m_drawToolSelector, SIGNAL(triggered(QAction*)),
//            this, SLOT(changeDrawTool(QAction*)));
//    connect(m_drawToolSelector, SIGNAL(actionCanceled()),
//            this, SLOT(cancelDrawOperation()));

    m_drawToolSelector->setDefaultAction(discTool);

    m_radiusWidget->setLabelText(tr("Brush Radius"));

    QSettings settings(CESVIMA, ESPINA);
    m_radiusWidget->setRadius(settings.value(BRUSHRADIUS, 20).toInt());
  }
  
  //------------------------------------------------------------------------
  ManualEditionTool::~ManualEditionTool()
  {
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
