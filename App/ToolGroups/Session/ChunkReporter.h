/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_CHUNK_REPORTER_H
#define ESPINA_CHUNK_REPORTER_H

// ESPINA
#include <Core/IO/ProgressReporter.h>
#include <Support/Types.h>

namespace ESPINA
{
  /** \class ChunkReporter
   * \brief Class to report the progress of a process divided in chunks to a tool.
   *
   */
  class ChunkReporter
  : public IO::ProgressReporter
  {
    public:
      /** \brief ChunkReporter class constructor.
       * \param[in] chunks number of chunks.
       * \param[in] tool pointer to the tool to be reported.
       *
       */
      ChunkReporter(unsigned int chunks, Support::Widgets::ProgressTool *tool);

      /** \brief ChunkReporter class virtual destructor.
       *
       */
      virtual ~ChunkReporter()
      {};

      /** \brief Resets the counters on a new chunk.
       *
       */
      void nextChunk();

      /** \brief Sets the progress of the associated tool and the internal counters.
       *
       */
      virtual void setProgress(unsigned int progress);

    private:
      unsigned int                    m_completedChunks; /** number of completed chunks.                 */
      float                           m_chunkProgress;   /** percentage of progress per completed chunk. */
      Support::Widgets::ProgressTool *m_tool;            /** pointer of the tool to report to.           */
  };
}

#endif // ESPINA_CHUNK_REPORTER_H
