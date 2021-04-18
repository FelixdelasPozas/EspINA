/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include <Core/Types.h>
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
      using RepresentationManagerList  = QList<RepresentationManager *>;
      using RepresentationManagerSList = QList<RepresentationManagerSPtr>;

      class EspinaGUI_EXPORT RepresentationManager
      : public QObject
      {
        Q_OBJECT
      public:
        enum class Status: int8_t
        {
          IDLE,            /** no need to update the current representations.   */
          PENDING_DISPLAY  /** waiting to show new representations.             */
        };

        enum FlagValue
        {
          HAS_ACTORS   = 1 << 0,
          EXPORTS_3D   = 1 << 1,
          NEEDS_ACTORS = 1 << 2
        };

        Q_DECLARE_FLAGS(ManagerFlags, FlagValue)

      public:
        /** \brief Representation manager class virtual destructor.
         *
         */
        virtual ~RepresentationManager();

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

        /** \brief Returns true if the manager has actors on the view.
         *
         */
        bool hasActors() const;

        /** \brief Returns true if the manager needs actors from other managers to show it's own representations.
         *
         */
        bool needsActors() const;

        /** \brief Returns true if the representations of the manager can be exported in a 3D view.
         *
         */
        bool exports3D() const;

        /** \brief Returns the manager's properties flags values.
         *
         */
        ManagerFlags flags() const;

        /** \brief Returns the types of views supported by this manager (2D, 3D or both).
         *
         */
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
        void show(const GUI::Representations::FrameCSPtr frame);

        /** \brief Hides all representations
         *
         */
        void hide(const GUI::Representations::FrameCSPtr frame);

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
         * \param[in] time TimeStamp value.
         *
         */
        void display(TimeStamp time);

        /** \brief Returns the item picked
         *
         */
        virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const = 0;

        /** \brief Returns a new instance of the class.
         *
         */
        RepresentationManagerSPtr clone();

        /** \brief Returns the debug identification.
         *
         */
        QString debugName() const;

        /** \brief Returns the list of pools of the manager.
         *
         */
        virtual RepresentationPoolSList pools() const;

        /** \brief Resets the manager's state to the initial state.
         *
         * TODO: 17-06-2016 implement!!!
         */
        virtual void reset()
        {};

        /** \brief Disconnects and de-configures the RepresentationManager, also emits the terminated() signal to be
         * removed from the child list of its parent. A terminated representation manager should not be used.
         *
         */
        virtual void shutdown();

      public slots:
        /** \brief Managers the view's state frame change.
         * \param[in] frame new frame object.
         *
         */
        virtual void onFrameChanged(const GUI::Representations::FrameCSPtr frame);

      signals:
        void renderRequested();
        void terminated(RepresentationManager *);

      protected slots:
        /** \brief Emits a render request for the given frame if the conditions apply.
         * \param[in] frame const frame object.
         *
         */
        void emitRenderRequest(const GUI::Representations::FrameCSPtr frame);

        /** \brief Sets the status of the manager to "Waiting" for the given frame.
         * \param[in] frame const frame object.
         *
         */
        void waitForDisplay(const GUI::Representations::FrameCSPtr frame);

        /** \brief Sets the status of the manager to idle.
         *
         */
        void idle();

      protected:
        /** \brief RepresentationManager class constructor.
         * \param[in] supportedViews Supported view's flags for this manager.
         * \param[in] flags Manager properties flags.
         *
         */
        explicit RepresentationManager(ViewTypeFlags supportedViews, ManagerFlags flags);

        /** \brief Sets a manager flag.
         * \param[in] flag flag identifier.
         * \param[in] value boolean value to set.
         *
         */
        void setFlag(const FlagValue flag, const bool value);

        /** \brief Returns the frame corresponding to the given time in the frame range.
         * \param[in] t requested timestamp.
         */
        FrameCSPtr frame(TimeStamp t) const;

        /** \brief Returns the last frame in the frame range.
         *
         */
        FrameCSPtr lastFrame() const;

        /** \brief Returns if the manager should react to the requested crosshair change
         * \param[in] crosshair new crosshair.
         *
         */
        virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const = 0;

        /** \brief Returns if the manager should react to the requested resolution change
         * \param[in] resolution new resolution.
         *
         */
        virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const = 0;

        /** \brief Returns if the manager should react to the requested bounds change
         * \param[in] bounds new bounds.
         *
         */
        virtual bool acceptSceneBoundsChange(const Bounds &bounds) const = 0;

        /** \brief Returns true if the manager should react to the invalidation of one of its sources.
         * \param[in] frame const frame object.
         *
         */
        virtual bool acceptInvalidationFrame(const FrameCSPtr frame) const = 0;

        /** \brief Returns true if the frame modifies the current representations of the manager.
         * \param[in] frame const frame object.
         *
         */
        virtual bool acceptFrame(const FrameCSPtr frame);

        /** \brief Returns true if the manager needs to update the representations for the given frame.
         * \param[in] frame const frame object.
         *
         */
        virtual bool needsRepresentationUpdate(const FrameCSPtr frame);

        /** \brief Returns the current representation's crosshair.
         *
         */
        NmVector3 currentCrosshair() const;

        /** \brief Returns the current representation's resolution.
         *
         */
        NmVector3 currentSceneResolution() const;

        /** \brief Returns the current representation's bounds.
         *
         */
        Bounds currentSceneBounds() const;

        /** \brief Empties the frames range values.
         * \param[in] frame invalidation frame.
         *
         */
        void invalidateFrames(const FrameCSPtr frame);

      private slots:
        /** \brief Removes the given manager from the child list.
         * \param[in] manager previously cloned representation manager pointer.
         *
         */
        void onChildTerminated(RepresentationManager *manager);

      private:
        /** \brief Returns true if the manager has representations to show and false otherwise.
         *
         */
        virtual bool hasRepresentations() const = 0;

        /** \brief Updates the representations for the given frame.
         * \param[in] frame const frame object.
         *
         */
        void updateRepresentations(const FrameCSPtr frame);

        virtual void updateFrameRepresentations(const FrameCSPtr frame) = 0;

        /** \brief Returns true if the manager is waiting the representations for frames higher than the given time.
         * \param[in] t timestamp.
         *
         */
        bool waitingNewerFrames(TimeStamp t) const;

        /** \brief Displays the representations for the given frame.
         * \param[in] frame const frame object.
         *
         */
        virtual void displayRepresentations(const FrameCSPtr frame) = 0;

        /** \brief Hides the representations for the given frame.
         * \param[in] frame const frame object.
         *
         */
        virtual void hideRepresentations(const FrameCSPtr frame) = 0;

        /** \brief Updates the manager state on a show event.
         * \param[in] frame const frame object of the show event.
         *
         */
        virtual void onShow(const FrameCSPtr frame) = 0;

        /** \brief Updates the manager state on a hide event.
         * \param[in] frame const frame object of the hide event.
         *
         */
        virtual void onHide(const FrameCSPtr frame) = 0;

        /** \brief Clones the manager.
         *
         */
        virtual RepresentationManagerSPtr cloneImplementation() = 0;

        /** \brief Sets the view where representation are managed
         *
         */
        void setView(RenderView *view, const FrameCSPtr frame);

        /** \brief Adds the value to the ready times or stores it in the temporal buffer if the actors for the current position are not yet available.
         * \param[in] t time stamp to store.
         *
         */
        void reuseTimeValue(TimeStamp t);

        friend class ESPINA::RenderView; /* setView() */

      protected:
        RenderView               *m_view;   /** manager's view.        */
        RepresentationManagerList m_childs; /** list of cloned childs. */

      private:
        QString      m_name;        /** manager's name.                                               */
        QIcon        m_icon;        /** display icon.                                                 */
        QString      m_description; /** manager description.                                          */
        bool         m_isActive;    /** true if the manager is enabled and false otherwise.           */
        Status       m_status;      /** current manager's status.                                     */
        ManagerFlags m_flags;       /** manager's properties flags values                             */

        ViewTypeFlags m_supportedViews;   /** types of views supported by this manager. */
        RangedValue<FrameCSPtr> m_frames; /** frame range of available representations. */

        TimeStamp m_lastFrameChanged;            /** last frame the manager has attended.            */
        TimeStamp m_lastInvalidationFrame;       /** last received invalidation signal.              */
        QMap<TimeStamp, TimeStamp> m_lazyFrames; /** list of frames received during a waiting state. */
      };

      class EspinaGUI_EXPORT RepresentationManager2D
      {
      public:
        /** \brief Class RepresentationManager2D virtual destructor.
         *
         */
        virtual ~RepresentationManager2D()
        {};

        /** \brief Sets the plane of the managed representations for the 2D manager.
         * \param[in] plane plane enum value.
         *
         */
        virtual void setPlane(Plane plane) = 0;

        /** \brief Sets the depth in Nm where the representations of the manager should be shown.
         * \param[in] depth distance in Nm from the manager's current crosshair.
         *
         */
        virtual void setRepresentationDepth(Nm depth) = 0;
      };

      /** \brief Returns true if the frame invalidates the representations of the given item type.
       *
       */
      bool EspinaGUI_EXPORT invalidatesRepresentations(GUI::Representations::FrameCSPtr frame, ItemAdapter::Type type);

      Q_DECLARE_OPERATORS_FOR_FLAGS(RepresentationManager::ManagerFlags)

    } // namespace Representations
  } // namespace GUI
}// namespace ESPINA

#endif // ESPINA_REPRESENTATION_MANAGER_H
