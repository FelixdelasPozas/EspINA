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

#ifndef ESPINA_CF_ADAPTIVE_COUNTING_FRAME_H
#define ESPINA_CF_ADAPTIVE_COUNTING_FRAME_H

// PLugin
#include "CountingFramePlugin_Export.h"
#include "CountingFrames/CountingFrame.h"

// ESPINA
#include <Core/Utils/Bounds.h>

namespace ESPINA
{
  namespace CF
  {
    class vtkCountingFrameCommand;

    const QString ADAPTIVE_CF = QObject::tr("Adaptive");

    class CountingFramePlugin_EXPORT AdaptiveCountingFrame
    : public CountingFrame
    {
      public:
        /** \brief AdaptiveCountingFrame vtk-style static New constructor.
         * \param[in] extension extension of this CF.
         * \param[in] inclusion inclusion margins.
         * \param[in] exclusion exclusion margins.
         * \param[in] scheduler task scheduler.
         *
         */
        static AdaptiveCountingFrame *New(CountingFrameExtension *extension,
                                          Nm inclusion[3],
                                          Nm exclusion[3],
                                          SchedulerSPtr scheduler)
        { return new AdaptiveCountingFrame(extension, inclusion, exclusion, scheduler); }

        /** \brief AdaptiveCountingFrame class virtual destructor.
         *
         */
        virtual ~AdaptiveCountingFrame();

        virtual CFType cfType() const
        { return CF::ADAPTIVE; }

        virtual QString typeName() const { return ADAPTIVE_CF; }

        virtual void updateCountingFrameImplementation();

      protected:
        /** \brief AdaptiveCountingFrame class constructor.
         * \param[in] extension extension of this CF.
         * \param[in] inclusion inclusion margins.
         * \param[in] exclusion exclusion margins.
         * \param[in] scheduler task scheduler.
         *
         */
        explicit AdaptiveCountingFrame(CountingFrameExtension *extension,
                                       Nm inclusion[3],
                                       Nm exclusion[3],
                                       SchedulerSPtr scheduler);

      protected:
        /** \brief Returns the left inclusion margin.
         *
         */
        Nm leftOffset()   const {QReadLocker lock(&m_marginsMutex); return m_inclusion[0];}

        /** \brief Returns the top inclusion margin.
         *
         */
        Nm topOffset()    const {QReadLocker lock(&m_marginsMutex); return m_inclusion[1];}

        /** \brief Returns the front inclusion margin.
         *
         */
        Nm frontOffset()  const {QReadLocker lock(&m_marginsMutex); return m_inclusion[2];}

        /** \brief Returns the right exclusion margin.
         *
         */
        Nm rightOffset()  const {QReadLocker lock(&m_marginsMutex); return m_exclusion[0];}

        /** \brief Returns the bottom exclusion margin.
         *
         */
        Nm bottomOffset() const {QReadLocker lock(&m_marginsMutex); return m_exclusion[1];}

        /** \brief Returns the back exclusion margin.
         *
         */
        Nm backOffset()   const {QReadLocker lock(&m_marginsMutex); return m_exclusion[2];}

      private:
        Channel *m_channel; /** extension's extended item. */

        friend class vtkCountingFrameCommand;
    };
  } // namespace CF
} // namespace ESPINA

#endif // ESPINA_CF_ADAPTIVE_COUNTING_FRAME_H
