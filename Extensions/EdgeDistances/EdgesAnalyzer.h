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

#ifndef ESPINA_EDGES_ANALYZER_H
#define ESPINA_EDGES_ANALYZER_H

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/MultiTasking/Task.h>
#include <Core/Utils/Bounds.h>

namespace ESPINA
{
  namespace Extensions
  {
    class ChannelEdges;

    /** \class EdgesAnalyzer
     * \brief Analizes the values of the stack "surface" slices to determine if it's a electron
     *        microscopy stack or a optic one.
     */
    class EspinaExtensions_EXPORT EdgesAnalyzer
    : public Task
    {
      public:
        /** \brief EdgesAnalyzer class constructor.
         * \param[in] extension ChannelEdges raw pointer.
         * \param[in] scheduler scheduler smart pointer.
         *
         */
        explicit EdgesAnalyzer(ChannelEdges *extension,
                               SchedulerSPtr scheduler = SchedulerSPtr());

        /** \brief EdgesAnalyzer class destructor.
         *
         */
        virtual ~EdgesAnalyzer();

      protected:
        /** \brief Computes the edges of a channel.
         *
         */
        virtual void run();

      private:
        /** \brief Analizes the edge of a volume.
         * \param[in] volume volumetric volume smart pointer to analyze.
         * \param[in] edgeBounds bounds of the edge of the volume.
         *
         */
        void analyzeEdge(const Output::ReadLockData<DefaultVolumetricData> &volume, const Bounds &edgeBounds);

      private:
        int m_useDistanceToBounds; /** number of faces that habe been analyzed and can measure just with the distance to the stack bounds. */
        int m_bgIntensity;         /** computed background intensity value. */
        int m_bgThreshold;         /** computed background intensity values threshold. */

        ChannelEdges *m_extension; /** extension owner of the task. */
    };

    using EdgesAnalyzerPtr  = EdgesAnalyzer *;
    using EdgesAnalyzerSPtr = std::shared_ptr<EdgesAnalyzer>;
  } // namespace Extensions
} // namespace ESPINA

#endif // ESPINA_EDGES_ANALYZER_H
