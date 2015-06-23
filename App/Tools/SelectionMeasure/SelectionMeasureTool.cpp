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
#include "SelectionMeasureTool.h"

#include <Core/EspinaTypes.h>
#include <Core/Utils/Bounds.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/View/Widgets/SelectionMeasure/Widget2D.h>
#include <GUI/View/Widgets/SelectionMeasure/Widget3D.h>
#include <Support/Context.h>

// VTK
#include <vtkMath.h>

// Qt
#include <QAction>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::SelectionMeasure;

//----------------------------------------------------------------------------
SelectionMeasureTool::SelectionMeasureTool(Support::Context &context)
: ProgressTool(tr("SelectionMeasureTool"), ":/espina/measure3D.png", tr("Measure Selection"), context)
, m_viewState(context.viewState())
, m_factory  {new TemporalPrototypes(std::make_shared<Widget2D>(m_viewState.selection()), std::make_shared<Widget3D>(m_viewState.selection()))}
{
  setCheckable(true);

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(onToolActivated(bool)));
}

//----------------------------------------------------------------------------
SelectionMeasureTool::~SelectionMeasureTool()
{
}

//----------------------------------------------------------------------------
void SelectionMeasureTool::abortOperation()
{
}

//----------------------------------------------------------------------------
void SelectionMeasureTool::onToolActivated(bool value)
{
  if (value)
  {
    if(m_viewState.selection()->items().isEmpty())
    {
      m_viewState.selection()->set(toViewItemList(m_viewState.selection()->activeChannel()));
    }
    m_viewState.addTemporalRepresentations(m_factory);
  }
  else
  {
    m_viewState.removeTemporalRepresentations(m_factory);
  }
}
