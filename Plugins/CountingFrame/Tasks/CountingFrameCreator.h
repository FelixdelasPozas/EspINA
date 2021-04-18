/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef PLUGINS_COUNTINGFRAME_TASKS_COUNTINGFRAMECREATOR_H_
#define PLUGINS_COUNTINGFRAME_TASKS_COUNTINGFRAMECREATOR_H_

#include "CountingFramePlugin_Export.h"

// Plugin
#include <CountingFrames/CountingFrame.h>

// ESPINA
#include <Core/MultiTasking/Task.h>

// Qt
#include <QString>

namespace ESPINA
{
  namespace CF
  {
    class CountingFrame;
    class CountingFrameExtension;

    /** \class CountingFrameCreator
     * \brief Task to create a counting frame in a separate thread from the main one.
     *
     */
    class CountingFramePlugin_EXPORT CountingFrameCreator
    : public Task
    {
        Q_OBJECT
      public:
        struct Data
        {
          CountingFrameExtension *extension;    /** extension owner of the CF.            */
          CFType                  type;         /** type of the CF to create.             */
          NmVector3               inclusion;    /** CF inclusion margins.                 */
          NmVector3               exclusion;    /** CF exclusion margins.                 */
          CountingFrame::Id       id;           /** CF identificator.                     */
          QString                 constraint;   /** CF constraint.                        */
          bool                    editable;     /** true if editable and false otherwise. */
        };

      public:
        /** \brief CountingFrameCreator class constructor.
         * \param[in] data data of the counting frame to create.
         * \param[in] scheduler application task scheduler.
         * \param[in] factory stereological inclusion factory.
         *
         */
        explicit CountingFrameCreator(Data data, SchedulerSPtr scheduler, Core::SegmentationExtensionFactorySPtr factory);

        /** \brief CountingFrameCreate class virtual destructor.
         *
         */
        virtual ~CountingFrameCreator()
        {};

        /** \brief Method to obtain the created counting frame.
         *
         */
        CountingFrame *getCountingFrame() const;

        /** \brief Returns the parameters of the counting frame.
         *
         */
        Data getCountingFrameData() const;

      protected:
        virtual void run();

      private:
        Data                                   m_data;    /** Data of the counting frame to create. */
        CountingFrame                         *m_cf;      /** created counting frame.               */
        Core::SegmentationExtensionFactorySPtr m_factory; /** stereological inclusion factory.      */
    };
  
  } // namespace CF
} // namespace ESPINA

#endif // PLUGINS_COUNTINGFRAME_TASKS_COUNTINGFRAMECREATOR_H_
