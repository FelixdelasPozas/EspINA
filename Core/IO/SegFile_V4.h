/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_SEGFILE_V4_H
#define ESPINA_SEGFILE_V4_H

#include "SegFileInterface.h"
#include <Core/Analysis/Output.h>
#include <Core/Analysis/FetchBehaviour.h>

namespace EspINA {

  namespace IO {

    namespace SegFile {

      class SegFile_V4
      : public SegFileInterface
      {
        using UuidMap = QMap<int, QUuid>;

        class Loader
        {
        public:
          Loader(QuaZip&          zip,
                 CoreFactorySPtr  factory = CoreFactorySPtr(),
                 ErrorHandlerSPtr handler = ErrorHandlerSPtr());

          AnalysisSPtr load();

      private:
        DirectedGraph::Vertex findInflatedVertexByIdV4(int id) const;

        QPair<FilterSPtr, Output::Id> findOutput(DirectedGraph::Vertex roVertex,
                                                 const QString        &linkName);

        SampleSPtr createSample(DirectedGraph::Vertex roVertex);

        FilterSPtr createFilter(DirectedGraph::Vertex roVertex);

        ChannelSPtr createChannel(DirectedGraph::Vertex roVertex);

        QString parseCategoryName(const State& state);

        int parseOutputId(const State& state);

        SegmentationSPtr createSegmentation(DirectedGraph::Vertex roVertex);

        void loadTrace();

        DirectedGraph::Vertex inflateVertexV4(DirectedGraph::Vertex roVertex);

        void createSegmentations();

        void restoreRelations();

        void createFilterOutputsFile(FilterSPtr filter, int filterVertex);

        private:
          QuaZip                 &m_zip;
          CoreFactorySPtr         m_factory;
          ErrorHandlerSPtr        m_handler;
          FetchBehaviourSPtr      m_fetchBehaviour;

          AnalysisSPtr            m_analysis;
          TemporalStorageSPtr     m_storage;

          QMap<int, QUuid>        m_vertexUuids;
          QMap<int, QUuid>        m_filerUuids;
          DirectedGraph::Vertices m_loadedVertices;
          DirectedGraphSPtr       m_trace;
          DirectedGraph::Vertices m_pendingSegmenationVertices;
        };

      public:
        static const QString FORMAT_INFO_FILE;

      public:
        SegFile_V4();

        virtual AnalysisSPtr load(QuaZip&          zip,
                                  CoreFactorySPtr  factory = CoreFactorySPtr(),
                                  ErrorHandlerSPtr handler = ErrorHandlerSPtr());

        virtual void save(AnalysisPtr      analysis,
                          QuaZip&          zip,
                          ErrorHandlerSPtr handler = ErrorHandlerSPtr());

      };
    }
  }
}

#endif // ESPINA_SEGFILE_V4_H
