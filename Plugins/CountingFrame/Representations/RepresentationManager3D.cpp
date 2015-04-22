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

#include "RepresentationManager3D.h"

namespace ESPINA {

  namespace CF {
    //-----------------------------------------------------------------------------
    RepresentationManager3D::RepresentationManager3D(CountingFrameManager &manager, ViewTypeFlags supportedViews)
    : RepresentationManager(supportedViews, ManagerFlags())
    , m_manager(manager)
    {
      connect(&m_manager, SIGNAL(countingFrameCreated(CountingFrame*)),
              this,       SLOT(onCountingFrameCreated(CountingFrame*)));

      connect(&m_manager, SIGNAL(countingFrameDeleted(CountingFrame*)),
              this,       SLOT(onCountingFrameDeleted(CountingFrame*)));
    }

    //-----------------------------------------------------------------------------
    RepresentationManager3D::~RepresentationManager3D()
    {
      for(auto cf : m_widgets.keys())
      {
        deleteWidget(cf);
      }
    }

    //-----------------------------------------------------------------------------
    TimeRange RepresentationManager3D::readyRangeImplementation() const
    {
      return TimeRange();
    }

//     //-----------------------------------------------------------------------------
//     void RepresentationManager3D::displayImplementation(TimeStamp t)
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
//     }

    //-----------------------------------------------------------------------------
    ViewItemAdapterPtr RepresentationManager3D::pick(const NmVector3 &point, vtkProp *actor) const
    {
      return nullptr;
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager3D::onCountingFrameCreated(CountingFrame *cf)
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
    void RepresentationManager3D::onCountingFrameDeleted(CountingFrame *cf)
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
    void RepresentationManager3D::onShow(TimeStamp t)
    {
      for (auto cf : m_pendingCFs)
      {
        m_widgets[cf] = createWidget(cf);
      }

      m_pendingCFs.clear();
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager3D::onHide(TimeStamp t)
    {
    }

    //-----------------------------------------------------------------------------
    RepresentationManagerSPtr RepresentationManager3D::cloneImplementation()
    {
      return std::make_shared<RepresentationManager3D>(m_manager, supportedViews());
    }

    //-----------------------------------------------------------------------------
    vtkCountingFrameWidget *RepresentationManager3D::createWidget(CountingFrame *cf)
    {
      auto widget = cf->createWidget(m_view);
      Q_ASSERT(widget);

      return widget;
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager3D::showWidget(vtkCountingFrameWidget *widget)
    {
      widget->SetEnabled(true);
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager3D::hideWidget(vtkCountingFrameWidget *widget)
    {
      widget->SetEnabled(false);
    }

    //-----------------------------------------------------------------------------
    void RepresentationManager3D::deleteWidget(CountingFrame *cf)
    {
      auto widget = m_widgets[cf];

      hideWidget(widget);

      cf->deleteWidget(widget);

      m_widgets.remove(cf);
    }
  }
}

