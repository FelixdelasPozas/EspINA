/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_COUNTING_FRAME_EXTENSION_H
#define ESPINA_COUNTING_FRAME_EXTENSION_H

#include "CountingFramePlugin_Export.h"

// ESPINA
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Core/Analysis/Extensions.h>
#include <Plugins/CountingFrame/CountingFrames/CountingFrame.h>
#include <Plugins/CountingFrame/Extensions/StereologicalInclusion.h>

namespace ESPINA
{
  namespace CF
  {
    class CountingFrameManager;
    class StereologicalInclusion;

    /** \class CountingFrameExtension
     * \brief Adds CF information to a channel.
     *
     */
    class CountingFramePlugin_EXPORT CountingFrameExtension
    : public ChannelExtension
    {
        Q_OBJECT
        static const QString FILE;

      public:
        static Type TYPE;

      public:
        /** \brief CountingFrameExtension class constructor.
         * \param[in] manager counting frame manager object pointer.
         * \param[in] scheduler task scheduler.
         * \param[in] state extension initial state.
         *
         */
        explicit CountingFrameExtension(CountingFrameManager *manager,
                                        SchedulerSPtr         scheduler,
                                        const State          &state = State());

        /** \brief CountingFrameExtension class virtual destructor.
         *
         */
        virtual ~CountingFrameExtension();

        virtual Type type() const
        { return TYPE; }

        virtual bool invalidateOnChange() const
        { return true; }

        virtual State state() const;

        virtual Snapshot snapshot() const;

        virtual TypeList dependencies() const
        {
          Extension::TypeList deps;
          deps << ChannelEdges::TYPE;
          return deps;
        }

        virtual QString toolTipText() const
        { return tr("Counting Frames: %1").arg(countingFrames().size()); }

        virtual InformationKeyList availableInformation() const
        { return InformationKeyList(); }

        /** \brief Returns true if the given filename corresponds to a cache file for this extension.
         * \param[in] file filename.
         *
         */
        virtual bool isCacheFile(const QString &file) const
        { return FILE == file; }

        /** \brief Creates a counting frame with the given parameters.
         * \param[in] type counting frame type.
         * \param[in] inclusion inclusion margins.
         * \param[in] exclusion exclusion margins.
         * \param[in] contraint name of the segmentations' category the counting frame will apply.
         *
         */
        void createCountingFrame(CFType type,
                                 Nm inclusion[3],
                                 Nm exclusion[3],
                                 const QString &constraint = QString(),
                                 const CountingFrame::Id &id = CountingFrame::Id());

        /** \brief Removes a counting frame from the counting frame manager.
         *
         */
        void deleteCountingFrame(CountingFrame *countingFrame);

        /** \brief Returns the list of created counting frames.
         *
         */
        CountingFrameList countingFrames() const
        { return m_countingFrames; }

      protected:
        virtual QVariant cacheFail(const InformationKey& tag) const
        { return QVariant(); }

        virtual void onExtendedItemSet(Channel *channel);

      protected slots:
        /** \brief Sets the id and constraint and adds it to the manager.
         *
         */
        void onCountingFrameCreated();

      private:
        CountingFrameManager *m_manager;         /** counting frame manager.             */
        SchedulerSPtr         m_scheduler;       /** task scheduler.                     */

        State m_prevState;                       /** previous state of the extension.    */

        CountingFrameList      m_countingFrames; /** list of created counting frames.    */
        mutable QReadWriteLock m_CFmutex;        /** protects CF list.                   */
    };

    using CountingFrameExtensionPtr  = CountingFrameExtension *;
    using CountingFrameExtensionSPtr = std::shared_ptr<CountingFrameExtension>;

    CountingFrameExtensionSPtr countingFrameExtensionPtr(ChannelExtensionSPtr extension);

  } // namespace CF
} // namespace ESPINA

#endif // ESPINA_COUNTING_FRAME_EXTENSION_H
