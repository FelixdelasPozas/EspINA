/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include <Core/Analysis/Filter.h>

namespace EspINA
{
  /** \brief Read a volume on demand from disk
   * 
   * If volume format is not MetaImage, then a MetaImage copy
   * will be stored in temporal storage and used to speed up
   * access to data
   */
  class EspinaFilters_EXPORT VolumetricStreamReader
  : public Filter
  {
  public:
    struct File_Not_Found_Exception{};

  public:
    explicit VolumetricStreamReader(InputSList inputs, Type type, SchedulerSPtr scheduler);

    virtual void restoreState(const State& state);

    virtual State state() const;

    void setFileName(const QFileInfo& fileName);

  protected:
    virtual Snapshot saveFilterSnapshot() const 
    { return Snapshot(); }

    virtual bool needUpdate() const;

    virtual bool needUpdate(Output::Id id) const;

    virtual void execute();

    virtual void execute(Output::Id id);

    virtual bool ignoreStorageContent() const
    {return false;}

    virtual bool invalidateEditedRegions()
    { return false; }

  private:
    QFileInfo       m_fileName;
  };

}// namespace EspINA

#endif // ESPINA_STREAM_READER_H
