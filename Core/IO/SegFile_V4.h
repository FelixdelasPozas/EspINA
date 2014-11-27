/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "SegFileInterface.h"
#include <Core/Analysis/Output.h>
#include <Core/Analysis/DataFactory.h>

namespace ESPINA {

  namespace IO {

    namespace SegFile {

      class EspinaCore_EXPORT SegFile_V4
      : public SegFileInterface
      {
        using UuidMap = QMap<int, QUuid>;

        /** \class Loader
         * \brief Helper class to load ESPINA SEG version 4 files.
         *
         */
        class Loader
        {
        public:
          /* SegFile_V4::Loader class constructor.
           * \param[in] zip, QuaZip handler.
           * \param[in] factory, core factory smart pointer.
           * \param[in] handler, error handler smart pointer.
           */
          Loader(QuaZip&          zip,
                 CoreFactorySPtr  factory = CoreFactorySPtr(),
                 ErrorHandlerSPtr handler = ErrorHandlerSPtr());

          /** \brief Process the data and returns an analysis.
           *
           */
          AnalysisSPtr load();

        private:
          /** \brief Finds and returns the already built vertex that has the given id.
           * \param[in] id id of the vertex.
           */
          DirectedGraph::Vertex findInflatedVertexByIdV4(int id) const;

          /** \brief Returns the filter and output id that serves as input for the given vertex.
           * \param[in] roVertex read only vertex.
           * \param[in] linkName edge relation specifier.
           *
           */
          QPair<FilterSPtr, Output::Id> findOutput(DirectedGraph::Vertex roVertex,
                                                   const QString        &linkName);

          /** \brief Creates and returns a sample smart pointer from the information in the given vertes.
           * \param[in] roVertex read only vertex.
           *
           */
          SampleSPtr createSample(DirectedGraph::Vertex roVertex);

          /** \brief Creates and returns a filter smart pointer from the information in the given vertex.
           * \param[in] roVertex read only vertex.
           *
           */
          FilterSPtr createFilter(DirectedGraph::Vertex roVertex);

          /** \brief Creates and returns a channel smart pointer from the information in the given vertex.
           * \param[in] roVertex read only vertex.
           *
           */
          ChannelSPtr createChannel(DirectedGraph::Vertex roVertex);

          /** \brief Returns the category name from a given state of a vertex.
           * \param[in] state, state of a vertex.
           *
           */
          QString parseCategoryName(const State& state);

          /** \brief Returns the output id from a given state of a vertex.
           * \param[in] state, state of a vertex.
           *
           */
          int parseOutputId(const State& state);

          /** \brief Creates and returns a segmentation smart pointer from the information in the given vertex.
           * \param[in] roVertex, read only vertex.
           *
           */
          SegmentationSPtr createSegmentation(DirectedGraph::Vertex roVertex);

          /** \brief Loads and creates the vertices of the v4 analysis graph.
           *
           */
          void loadTrace();

          /** \brief Creates the analysis object specified in the given vertex an resturns it.
           * \param[in] roVertex, read only vertex.
           *
           */
          DirectedGraph::Vertex inflateVertexV4(DirectedGraph::Vertex roVertex);

          /** \brief Creates graph segmentations.
           *
           */
          void createSegmentations();

          /** \brief Restores graph relations.
           *
           */
          void restoreRelations();

          /** \brief Create v5 outputs files for the given filter.
           * \param[in] filter, filter smart pointer.
           * \param[in] filterVertex, filter vertex id.
           *
           */
          void createFilterOutputsFile(FilterSPtr filter, int filterVertex);

        private:
          QuaZip                 &m_zip;
          CoreFactorySPtr         m_factory;
          ErrorHandlerSPtr        m_handler;
          DataFactorySPtr         m_dataFactory;

          AnalysisSPtr            m_analysis;
          TemporalStorageSPtr     m_storage;

          QMap<int, QUuid>        m_vertexUuids;
          QMap<int, QUuid>        m_filerUuids;
          DirectedGraph::Vertices m_loadedVertices;
          DirectedGraphSPtr       m_trace;
          DirectedGraph::Vertices m_pendingSegmentationVertices;
        };

      public:
        static const QString FORMAT_INFO_FILE;

      public:
        /** \brief SegFile_V4 class constructor.
         *
         */
        SegFile_V4();

        /** \brief Implements SegFileInterface::load().
         *
         */
        virtual AnalysisSPtr load(QuaZip&          zip,
                                  CoreFactorySPtr  factory = CoreFactorySPtr(),
                                  ErrorHandlerSPtr handler = ErrorHandlerSPtr());

        /** \brief Implements SegFileInterface::save().
         *
         */
        virtual void save(AnalysisPtr      analysis,
                          QuaZip&          zip,
                          ErrorHandlerSPtr handler = ErrorHandlerSPtr());

      };
    }
  }
}

#endif // ESPINA_SEGFILE_V4_H
