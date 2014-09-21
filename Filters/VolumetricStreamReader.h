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

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include <Core/Analysis/Data/Volumetric/StreamedVolume.hxx>
#include <Core/Analysis/Filter.h>

namespace ESPINA
{
  template class StreamedVolume<itkVolumeType>;

  /** \class VolumetricStreamReader
   * \brief Read a volume on demand from disk.
   *
   * If volume format is not MetaImage, then a MetaImage copy
   * will be stored in temporal storage and used to speed up
   * access to data.
   */
  class EspinaFilters_EXPORT VolumetricStreamReader
  : public Filter
  {
  public:
    struct File_Not_Found_Exception{};

  public:
    /** \brief VolumetricStreamReader class constructor.
		 * \param[in] inputs, list of input smart pointers.
		 * \param[in] type, VolumetricStreamReader type.
		 * \param[in] scheduler, scheduler smart pointer.
		 *
     */
    explicit VolumetricStreamReader(InputSList inputs, Type type, SchedulerSPtr scheduler);

    /** \brief Implements Persistent::restoreState().
     *
     */
    virtual void restoreState(const State& state);

    /** \brief Implements Persistent::state().
     *
     */
    virtual State state() const;

    /** \brief Sets the name of the image on disk to stream.
     * \param[in] filename, QFileInfo object.
     */
    void setFileName(const QFileInfo& fileName);

  protected:
    /** \brief Implements Filter::saveFilterSnapshot().
     *
     */
    virtual Snapshot saveFilterSnapshot() const
    { return Snapshot(); }

    /** \brief Implements Filter::needUpdate().
     *
     */
    virtual bool needUpdate() const;

    /** \brief Implements Filter::needUpdate(id).
     *
     */
    virtual bool needUpdate(Output::Id id) const;

    /** \brief Implements Filter::execute().
     *
     */
    virtual void execute();

    /** \brief Implements Filter::execute(id).
     *
     */
    virtual void execute(Output::Id id);

    /** \brief Implements Filter::ignoreStorageContent().
     *
     */
    virtual bool ignoreStorageContent() const
    {return false;}

    /** \brief Implements Filter::invalidateEditedRegions().
     *
     */
    virtual bool invalidateEditedRegions()
    { return false; }

  private:
    QFileInfo       m_fileName;
  };

}// namespace ESPINA

#endif // ESPINA_STREAM_READER_H
