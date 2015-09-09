/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "ZoomRegionTool.h"

#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/View/Widgets/Zoom/ZoomWidget2D.h>
#include <GUI/View/Widgets/Zoom/ZoomWidget3D.h>
#include <Support/Context.h>

using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View;
using namespace ESPINA::GUI::View::Widgets;

using namespace ESPINA;

//----------------------------------------------------------------------------
ZoomRegionTool::ZoomRegionTool(Support::Context &context)
: ProgressTool("ZoomRegion", ":/espina/zoom_region.svg", tr("Zoom Region"), context)
, m_viewState(context.viewState())
, m_handler  {new ZoomEventHandler()}
, m_factory  {new TemporalPrototypes{std::make_shared<ZoomWidget2D>(m_handler.get()), std::make_shared<ZoomWidget3D>(m_handler.get())}}
{
  setCheckable(true);

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(onToolActivated(bool)));

  setEventHandler(m_handler);
}

//----------------------------------------------------------------------------
ZoomRegionTool::~ZoomRegionTool()
{
}

//----------------------------------------------------------------------------
void ZoomRegionTool::abortOperation()
{
  setChecked(false);
}

//----------------------------------------------------------------------------
void ZoomRegionTool::onToolActivated(bool value)
{
  if (value)
  {
    m_viewState.addTemporalRepresentations(m_factory);
  }
  else
  {
    m_viewState.removeTemporalRepresentations(m_factory);
  }
}
