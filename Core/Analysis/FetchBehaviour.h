/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef ESPINA_FETCH_BEHAVIOUR_H
#define ESPINA_FETCH_BEHAVIOUR_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Output.h>

// Qt
#include <QXmlStreamReader>

namespace ESPINA {

  class EspinaCore_EXPORT FetchBehaviour
  {
		public:
  		/* \brief Class FetchBehaviour class virtual destructor.
  		 *
  		 */
			virtual ~FetchBehaviour()
			{};

  		/* \brief Loads the data from disk and set the data into the given output.
  		 * \param[inout] output, output object smart pointer.
  		 * \param[in] storage, temporal storage where the files are.
  		 * \param[in] prefix, prefix of the data files.
  		 * \param[in] info, xml data that specifies the type of data to fetch.
  		 *
  		 */
			virtual void fetchOutputData(OutputSPtr output, TemporalStorageSPtr storage, QString prefix, QXmlStreamAttributes info) = 0;
  };

  using FetchBehaviourSPtr = std::shared_ptr<FetchBehaviour>;
}

#endif // ESPINA_NEURO_ITEM_H
