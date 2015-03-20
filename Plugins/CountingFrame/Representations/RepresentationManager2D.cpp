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
    : RepresentationManager{supportedViews}
    , m_plane{Plane::UNDEFINED}
    , m_manager(manager)
    {
      m_requiresRender = false;
      connect(&m_manager, SIGNAL(countingFrameCreated(CountingFrame*)),
              this,       SLOT(onCountingFrameCreated(CountingFrame*)));
      connect(&m_manager, SIGNAL(countingFrameDeleted(CountingFrame*)),
              this,       SLOT(onCountingFrameDeleted(CountingFrame*)));
    }

    //-----------------------------------------------------------------------------
    RepresentationManager2D::~RepresentationManager2D()
    {
      for(auto cf : m_insertedCFs.keys())
      {
        onCountingFrameDeleted(cf);
      }
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::setResolution(const NmVector3 &resolution)
    {
      m_resolution = resolution;
    }

    //-----------------------------------------------------------------------------
    RepresentationManager::PipelineStatus RepresentationManager2D::pipelineStatus() const
    {
      return PipelineStatus::RANGE_DEPENDENT;
    }

    //-----------------------------------------------------------------------------
    TimeRange RepresentationManager2D::readyRange() const
    {
      return m_crosshairs.timeRange();
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::display(TimeStamp t)
    {
      for (auto widget : m_insertedCFs)
      {
        widget->SetSlice(slicingPosition(t));
      }

      m_crosshairs.invalidatePreviouesRepresentations(t);
    }

    //-----------------------------------------------------------------------------
    ViewItemAdapterPtr RepresentationManager2D::pick(const NmVector3 &point, vtkProp *actor) const
    {
      return nullptr;
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::setPlane(Plane plane)
    {
      m_plane = plane;
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::onCountingFrameCreated(CountingFrame *cf)
    {
      if (m_view)
      {
        auto widget = cf->createSliceWidget(m_view);
        Q_ASSERT(widget);

        widget->SetSlice(slicingPosition(m_crosshairs.lastTime()));
        widget->SetEnabled(isActive());

        m_insertedCFs.insert(cf, widget);

        m_requiresRender = true;

        if (isActive())
        {
          emit renderRequested();
        }
      }
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::onCountingFrameDeleted(CountingFrame *cf)
    {
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::onShow()
    {
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::onHide()
    {
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::setCrosshair(const NmVector3 &crosshair, TimeStamp t)
    {
      if (m_crosshairs.isEmpty() || m_crosshairs.last() != crosshair)
      {
        m_crosshairs.addRepresentation(crosshair, t);
        emit renderRequested();
      }
      else
      {
        m_crosshairs.usePreviousRepresentation(t);
      }
    }

    //-----------------------------------------------------------------------------
    RepresentationManagerSPtr RepresentationManager2D::cloneImplementation()
    {
      return std::make_shared<RepresentationManager2D>(m_manager, supportedViews());
    }

    //-----------------------------------------------------------------------------
    Nm RepresentationManager2D::slicingPosition(TimeStamp t) const
    {
      auto crosshair = m_crosshairs.representation(t, NmVector3());

      Q_ASSERT(m_plane != Plane::UNDEFINED);
      return crosshair[normalCoordinateIndex(m_plane)];
    }
  }
}

