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

#ifndef ESPINA_SEGFILE_V5_H
#define ESPINA_SEGFILE_V5_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/IO/SegFileInterface.h"
#include <Core/Analysis/Extension.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Analysis/Output.h>
#include <Core/Analysis/FetchBehaviour.h>

namespace ESPINA {

  namespace IO {

    namespace SegFile {

      class EspinaCore_EXPORT SegFile_V5
      : public SegFileInterface
      {
      	/** \class SegFile_V5::Loader
      	 * \brief Helper class to load ESPINA SEG version 5 files.
      	 *
      	 */
        class Loader
        {
        	public:
        		/* SegFile_V5::Loader class constructor.
        		 * \param[in] zip, QuaZip handler.
        		 * \param[in] factory, core factory smart pointer.
        		 * \param[in] handler, error handler smart pointer.
        		 */
        		Loader(QuaZip &zip,
        				   CoreFactorySPtr factory = CoreFactorySPtr(),
        				   ErrorHandlerSPtr handler = ErrorHandlerSPtr());

        		/* \brief Process the data and returns an analysis.
        		 *
        		 */
        		AnalysisSPtr load();
        	private:
        		/* \brief Finds and returns the vertex that match the uuid.
        		 * \param[in] vertices, directed graph vertices group.
        		 * \param[in] uuid, unique id.
        		 *
        		 */
        		static DirectedGraph::Vertex findVertex(DirectedGraph::Vertices vertices, Persistent::Uuid uuid);

        		/* \brief Creates and returns a sample smart pointer from the information in the given vertes.
        		 * \param[in] roVertex, read only vertex.
        		 *
        		 */
        		SampleSPtr createSample(DirectedGraph::Vertex roVertex);

        		/* \brief Creates and returns a filter smart pointer from the information in the given vertex.
        		 * \param[in] roVertex, read only vertex.
        		 *
        		 */
        		FilterSPtr createFilter(DirectedGraph::Vertex roVertex);

        		/* \brief Finds an returns the output that serves as input of the given vertex.
        		 * \param[in] roVertex, read only vertex.
        		 *
        		 */
        		QPair<FilterSPtr, Output::Id> findOutput(DirectedGraph::Vertex roVertex);

        		/* \brief Creates and returns a channel smart pointer from the information in the given vertex.
        		 * \param[in] roVertex, read only vertex.
        		 *
        		 */
        		ChannelSPtr createChannel(DirectedGraph::Vertex roVertex);

        		/* \brief Returns the category name from a given state of a vertex.
        		 * \param[in] state, state of a vertex.
        		 *
        		 */
        		static QString parseCategoryName(const State& state);

        		/* \brief Creates and returns a segmentation smart pointer from the information in the given vertex.
        		 * \param[in] roVertex, read only vertex.
        		 *
        		 */
        		SegmentationSPtr createSegmentation(DirectedGraph::Vertex roVertex);

        		/* \brief Creates the analysis object specified in the given vertex an resturns it.
        		 * \param[in] roVertex, read only vertex.
        		 *
        		 */
        		DirectedGraph::Vertex inflateVertex(DirectedGraph::Vertex roVertex);

        		/* \brief Creates the content graph of an analysis.
        		 *
        		 */
        		void loadContent();

        		/* \brief Creates the relationship graph of an anlysis.
        		 *
        		 */
        		void loadRelations();

        		/* \brief Creates a channel extension.
        		 * \param[in] channel, smart pointer of the channel that has the extension.
        		 * \param[in] type, type of the channel extension.
        		 * \param[in] cache, cache object.
        		 * \param[in] state, state of the extension.
        		 *
        		 */
        		void createChannelExtension(ChannelSPtr channel,
        	                              const ChannelExtension::Type &type,
        	                              const ChannelExtension::InfoCache &cache,
        	                              const State &state);

        		/* \brief Loads and creates the extensions of a given channel.
        		 * \param[in] channel, channel smart pointer.
        		 *
        		 */
        	  void loadExtensions(ChannelSPtr channel);

        		/* \brief Creates a segmentation extension.
        		 * \param[in] segmentation, smart pointer of the segmentation that has the extension.
        		 * \param[in] type, type of the segmentation extension.
        		 * \param[in] cache, cache object.
        		 * \param[in] state, state of the extension.
        		 *
        		 */
        	  void createSegmentationExtension(SegmentationSPtr segmentation,
        	                              const SegmentationExtension::Type &type,
        	                              const SegmentationExtension::InfoCache &cache,
        	                              const State &state);

        		/* \brief Loads and creates the extensions of a given segmentation.
        		 * \param[in] segmentation, segmentation smart pointer.
        		 *
        		 */
        	  void loadExtensions(SegmentationSPtr segmentation);

        	  QuaZip                 &m_zip;
        	  CoreFactorySPtr         m_factory;
        	  ErrorHandlerSPtr        m_handler;
        	  AnalysisSPtr            m_analysis;
        	  TemporalStorageSPtr     m_storage;
        	  FetchBehaviourSPtr      m_fetchBehaviour;
        	  DirectedGraphSPtr       m_content;
        	  DirectedGraph::Vertices m_loadedVertices;
        };

      public:
        static const QString FORMAT_INFO_FILE;

      public:
        /* \brief SegFile_V5 class constructor.
         *
         */
        SegFile_V5();

        /* \brief Implements SegFileInterface::load().
         *
         */
        virtual AnalysisSPtr load(QuaZip&          zip,
                                  CoreFactorySPtr  factory = CoreFactorySPtr(),
                                  ErrorHandlerSPtr handler = ErrorHandlerSPtr());

        /* \brief Implements SegFileInterface::save().
         *
         */
        virtual void save(AnalysisPtr      analysis,
                          QuaZip&          zip,
                          ErrorHandlerSPtr handler = ErrorHandlerSPtr());
      };
    }
  }
}

#endif // ESPINA_SEGFILE_V5_H
