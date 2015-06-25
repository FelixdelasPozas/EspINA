/*

    Copyright (C) 2014  Jorge Pe�a Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_REPRESENTATION_MANAGER_H
#define ESPINA_REPRESENTATION_MANAGER_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <GUI/View/SelectableView.h>
#include <GUI/View/ViewTypeFlags.h>
#include <GUI/View/CoordinateSystem.h>
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/Representations/RepresentationPool.h>

// Qt
#include <QString>
#include <QIcon>

namespace ESPINA
{
  class RenderView;

  namespace GUI
  {
    namespace Representations
    {
      class RepresentationManager;
      using RepresentationManagerSPtr  = std::shared_ptr<RepresentationManager>;
      using RepresentationManagerSList = QList<RepresentationManagerSPtr>;

      class EspinaGUI_EXPORT RepresentationManager
      : public QObject
      {
        Q_OBJECT
      public:
        enum class Status: int8_t {
          IDLE,
          PENDING_DISPLAY
        };

        enum FlagValue
        {
          HAS_ACTORS   = 0x1,
          EXPORTS_3D   = 0x2,
          NEEDS_ACTORS = 0x4
        };

        Q_DECLARE_FLAGS(ManagerFlags, FlagValue)

      public:
        virtual ~RepresentationManager()
        {}

        /** \brief Sets the name of the representation manager
         *
         */
        void setName(const QString &name);

        /** \brief Returns the name of the representation manager
         *
         */
        QString name() const;

        /** \brief Sets the description of the representation manager
         *
         */
        void setDescription(const QString &description);

        /** \brief Returns the description of the representation manager
         *
         */
        QString description() const;

        /** \brief Sets the icon of the representation manager
         *
         */
        void setIcon(const QIcon &icon);

        /** \brief Returns the icon of the representation manager
         *
         */
        QIcon icon() const;

        bool hasActors() const;

        bool needsActors() const;

        bool exports3D() const;

        ManagerFlags flags() const;

        ViewTypeFlags supportedViews() const
        { return m_supportedViews; }

        /** \brief Returns if managed representations are visible or not
         *
         */
        bool representationsVisibility() const
        { return m_isActive; }

        /** \brief Shows all representations
         *
         */
        void show(TimeStamp t);

        /** \brief Hides all representations
         *
         */
        void hide(TimeStamp t);

        /** \brief Returns if the manager has been requested to display its actors
         *
         */
        bool isActive() const;

        /** \brief Returns true if the manager is idle, false otherwise
         *
         */
        bool isIdle() const;

        /** \brief Returns the range of ready pipelines.
         *
         */
        TimeRange readyRange() const;

        /** \brief Updates view's actors with those available at the given time.
         *
         */
        void display(TimeStamp t);

        /** \brief Returns the item picked
         *
         */
        virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const = 0;

        /** \brief Returns a new instance of the class.
         *
         */
        RepresentationManagerSPtr clone();

        QString debugName() const;

      public slots:
        void onCrosshairChanged(NmVector3 crosshair, TimeStamp t);

        void onSceneResolutionChanged(const NmVector3 &resolution, TimeStamp t);

        void onSceneBoundsChanged(const Bounds &bounds, TimeStamp t);

      signals:
        void renderRequested();

      protected slots:
        void invalidateRepresentations();

        void emitRenderRequest(TimeStamp t);

        void waitForDisplay();

        void idle();

      protected:
        explicit RepresentationManager(ViewTypeFlags supportedViews, ManagerFlags flags);

        void setFlag(const FlagValue flag, const bool value);

        /** \brief Returns if the manager should react to the requested crosshair change
         *
         */
        virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const = 0;

        virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const = 0;

        virtual bool acceptSceneBoundsChange(const Bounds &bounds) const = 0;

        NmVector3 currentCrosshair() const;

        NmVector3 currentSceneResolution() const;

        Bounds currentSceneBounds() const;

      private:
        virtual TimeRange readyRangeImplementation() const = 0;

        virtual bool hasRepresentations() const = 0;

        void updateRepresentations(TimeStamp t);

        virtual void updateRepresentations(const NmVector3 &crosshair, const NmVector3 &resolution, const Bounds &bounds, TimeStamp t) = 0;

        /** \brief Performs the actual crosshair change for the underlying representations
         *
         */
        virtual void changeCrosshair(const NmVector3 &crosshair, TimeStamp t) {}

        virtual void changeSceneResolution(const NmVector3 &resolution, TimeStamp t) {}

        virtual void changeSceneBounds(const Bounds &bounds, TimeStamp t) {}

        bool hasNewerFrames(TimeStamp t) const;

        virtual void displayRepresentations(TimeStamp t) = 0;

        virtual void hideRepresentations(TimeStamp t) = 0;

        virtual void onShow(TimeStamp t) = 0;

        virtual void onHide(TimeStamp t) = 0;

        virtual RepresentationManagerSPtr cloneImplementation() = 0;

        /** \brief Sets the view where representation are managed
         *
         */
        void setView(RenderView *view);

        friend class ESPINA::RenderView;

      protected:
        RenderView *m_view;

      private:
        QString      m_name;
        QIcon        m_icon;
        QString      m_description;
        bool         m_isActive;
        Status       m_status;
        ManagerFlags m_flags;

        ViewTypeFlags m_supportedViews;
        TimeStamp     m_lastRenderRequestTime;

        NmVector3 m_crosshair;
        NmVector3 m_resolution; // CoordinateSystem?
        Bounds    m_bounds;

        RepresentationManagerSList m_childs;
      };

      class RepresentationManager2D
      {
      public:
        /** \brief Class RepresentationManager2D virtual destructor.
         *
         */
        virtual ~RepresentationManager2D()
        {};

        virtual void setPlane(Plane plane) = 0;

        virtual void setRepresentationDepth(Nm depth) = 0;
      };

      Q_DECLARE_OPERATORS_FOR_FLAGS(RepresentationManager::ManagerFlags)

    } // namespace Representations
  } // namespace GUI
}// namespace ESPINA

#endif // ESPINA_REPRESENTATION_MANAGER_H
