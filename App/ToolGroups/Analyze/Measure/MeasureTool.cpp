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
#include "MeasureTool.h"

#include <GUI/View/Widgets/Measures/MeasureEventHandler.h>
#include <GUI/View/Widgets/Measures/MeasureWidget.h>
#include <Support/Context.h>

// Qt
#include <QAction>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::Measures;

//----------------------------------------------------------------------------
MeasureTool::MeasureTool(Support::Context &context)
: ProgressTool("MeasureTool", ":/espina/measure.png", tr("Measure Segmentations"), context)
, m_viewState(context.viewState())
, m_handler   {new MeasureEventHandler()}
, m_prototypes{new TemporalPrototypes(std::make_shared<MeasureWidget>(m_handler.get()), TemporalRepresentation3DSPtr())}
{
  setCheckable(true);

  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(onToolActivated(bool)), Qt::QueuedConnection);

  setEventHandler(m_handler);
}

//----------------------------------------------------------------------------
MeasureTool::~MeasureTool()
{
}

//----------------------------------------------------------------------------
void MeasureTool::abortOperation()
{
}

//----------------------------------------------------------------------------
void MeasureTool::onToolActivated(bool value)
{
  if (value)
  {
    m_viewState.addTemporalRepresentations(m_prototypes);
  }
  else
  {
    m_viewState.removeTemporalRepresentations(m_prototypes);

    emit stopMeasuring();
  }
}
