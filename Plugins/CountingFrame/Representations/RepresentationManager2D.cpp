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

#include "RepresentationManager2D.h"

namespace ESPINA {
  namespace CF {

    //-----------------------------------------------------------------------------
    RepresentationManager2D::RepresentationManager2D(CountingFrameManager &manager, ViewTypeFlags supportedViews)
    : RepresentationManager(supportedViews)
    , m_manager(manager)
    {

    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::setResolution(const NmVector3 &resolution)
    {

    }

    //-----------------------------------------------------------------------------
    RepresentationManager::PipelineStatus RepresentationManager2D::pipelineStatus() const
    {

    }

    //-----------------------------------------------------------------------------
    TimeRange RepresentationManager2D::readyRange() const
    {

    }

    //-----------------------------------------------------------------------------
    ViewItemAdapterPtr RepresentationManager2D::pick(const NmVector3 &point, vtkProp *actor) const
    {

    }

    //-----------------------------------------------------------------------------
    RepresentationManagerSPtr RepresentationManager2D::cloneImplementation()
    {
      return std::make_shared<RepresentationManager2D>(m_manager, supportedViews());
    }

  }
}

