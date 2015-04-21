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
      connect(&m_manager, SIGNAL(countingFrameCreated(CountingFrame*)),
              this,       SLOT(onCountingFrameCreated(CountingFrame*)));
      connect(&m_manager, SIGNAL(countingFrameDeleted(CountingFrame*)),
              this,       SLOT(onCountingFrameDeleted(CountingFrame*)));
    }

    //-----------------------------------------------------------------------------
    RepresentationManager2D::~RepresentationManager2D()
    {
      for(auto cf : m_widgets.keys())
      {
        deleteWidget(cf);
      }
    }

//     //-----------------------------------------------------------------------------
//     void RepresentationManager2D::setResolution(const GUI::View::CoordinateSystemSPtr system)
//     {
//       m_resolution = system->resolution();
//     }

    //-----------------------------------------------------------------------------
    TimeRange RepresentationManager2D::readyRangeImplementation() const
    {
      return m_crosshairs.timeRange();
    }

//     //-----------------------------------------------------------------------------
//     void RepresentationManager2D::displayImplementation(TimeStamp t)
//     {
//       if (representationsShown())
//       {
//         for (auto widget : m_widgets)
//         {
//           showWidget(widget);
//         }
//       }
//       else
//       {
//         for (auto widget : m_widgets)
//         {
//           hideWidget(widget);
//         }
//       }
//
//       m_crosshairs.invalidatePreviousValues(t);
//     }
//
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
      if (isActive())
      {
        auto widget = createWidget(cf);

        m_widgets.insert(cf, widget);

        emit renderRequested();
      }
      else
      {
        m_pendingCFs << cf;
      }
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::onCountingFrameDeleted(CountingFrame *cf)
    {
      if (m_pendingCFs.contains(cf))
      {
        m_pendingCFs.removeOne(cf);
      }
      else
      {
        Q_ASSERT(m_widgets.keys().contains(cf));

        deleteWidget(cf);

        emit renderRequested();
      }
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::onShow(TimeStamp t)
    {
      for (auto cf : m_pendingCFs)
      {
        m_widgets[cf] = createWidget(cf);
      }

      m_pendingCFs.clear();
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::onHide(TimeStamp t)
    {
    }

//     //-----------------------------------------------------------------------------
//     void RepresentationManager2D::setCrosshair(const NmVector3 &crosshair, TimeStamp t)
//     {
//       if (m_crosshairs.isEmpty() || isNormalDifferent(m_crosshairs.last(), crosshair))
//       {
//         m_crosshairs.addValue(crosshair, t);
//
//         emit renderRequested();
//       }
//       else
//       {
//         m_crosshairs.reusePreviousValue(t);
//       }
//     }

    //-----------------------------------------------------------------------------
    RepresentationManagerSPtr RepresentationManager2D::cloneImplementation()
    {
      return std::make_shared<RepresentationManager2D>(m_manager, supportedViews());
    }

    //-----------------------------------------------------------------------------
    Nm RepresentationManager2D::slicingPosition(TimeStamp t) const
    {
      auto crosshair = m_crosshairs.value(t, NmVector3());

      Q_ASSERT(m_plane != Plane::UNDEFINED);
      return crosshair[normalCoordinateIndex(m_plane)];
    }

    //-----------------------------------------------------------------------------
    vtkCountingFrameSliceWidget *RepresentationManager2D::createWidget(CountingFrame *cf)
    {
      auto widget = cf->createSliceWidget(m_view);
      Q_ASSERT(widget);

      return widget;
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::showWidget(vtkCountingFrameSliceWidget *widget)
    {
      widget->SetEnabled(true);
      widget->SetSlice(slicingPosition(m_crosshairs.lastTime()));
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::hideWidget(vtkCountingFrameSliceWidget *widget)
    {
      widget->SetEnabled(false);
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager2D::deleteWidget(CountingFrame *cf)
    {
      auto widget = m_widgets[cf];

      hideWidget(widget);

      cf->deleteSliceWidget(widget);

      m_widgets.remove(cf);
    }

    //-----------------------------------------------------------------------------
    bool RepresentationManager2D::isNormalDifferent(const NmVector3 &p1, const NmVector3 &p2) const
    {
      auto normal = normalCoordinateIndex(m_plane);

      return p1[normal] != p2[normal];
    }
  }
}

