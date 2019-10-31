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


#ifndef ESPINA_STREAM_READER_H
#define ESPINA_STREAM_READER_H

#include <Core/EspinaCore_Export.h>

// ESPINA
#include <Core/Analysis/Data/Volumetric/StreamedVolume.hxx>
#include <Core/Analysis/Filter.h>

namespace ESPINA
{
  namespace Core
  {
    /** \class VolumetricStreamReader
     * \brief Read a stack from disk (totally or on demand).
     *
     */
    class EspinaCore_EXPORT VolumetricStreamReader
    : public Filter
    {
      public:
        static const QString STREAMING_OPTION; /** load options key. */

        /** \brief VolumetricStreamReader class constructor.
         * \param[in] inputs list of input smart pointers.
         * \param[in] type VolumetricStreamReader type.
         * \param[in] scheduler scheduler smart pointer.
         *
         */
        explicit VolumetricStreamReader(InputSList inputs, Type type, SchedulerSPtr scheduler);

        /** \brief VolumetricStreamReader class virtual destructor.
         *
         */
        virtual ~VolumetricStreamReader()
        {}

        virtual void restoreState(const State& state) override;

        virtual State state() const override;

        /** \brief Sets the name of the image on disk to stream.
         * \param[in] filename QFileInfo object.
         */
        void setFileName(const QFileInfo& fileName);

        /** \brief Sets stack streaming.
         * \param[in] value true to set stack streaming from disk and false to read all stack to memory.
         *
         */
        void setStreaming(bool value);

        /** \brief Returns the state of the streaming parameter.
         *
         */
        bool streamingEnabled() const
        { return m_streaming; }

      protected:
        virtual Snapshot saveFilterSnapshot() const override
        { return Snapshot(); }

        virtual bool needUpdate() const override;

        virtual void execute() override;

        virtual bool ignoreStorageContent() const override
        { return false; }

      private:
        QFileInfo           m_fileName;         /** image file filename.                                                         */
        bool                m_streaming;        /** true if the stack is streamed from file, false for read all stack to memory. */
        TemporalStorageSPtr m_streamingStorage; /** storage for streaming data.                                                  */
        QFileInfo           m_streamingFile;    /** streaming image file info. Need one different for the one used by analysis.  */
        bool                m_changedStreaming; /** true if the streaming mode has changed and needs update, false otherwise.    */
    };
  } // namespace Core
}// namespace ESPINA

#endif // ESPINA_STREAM_READER_H
