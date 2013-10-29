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


#ifndef CHANNELREADER_H
#define CHANNELREADER_H

#include "EspinaCore_Export.h"

#include "Core/Model/Filter.h"
#include <Core/IO/SegFileReader.h>

#include <itkImageIOBase.h>
#include <itkImageFileReader.h>

namespace EspINA
{
  class EspinaCore_EXPORT ChannelReader
  : public ChannelFilter
  {
  public:
    static const QString TYPE;

    static const ArgumentId FILE;
    static const ArgumentId SPACING;

  public:
    explicit ChannelReader(NamedInputs     inputs,
                           Arguments       args,
                           FilterType      type,
                           IOErrorHandler *handler);

    /// Implements Model Item Interface
    virtual QString id() const {return m_args[ID];}
    virtual QString serialize() const;

    void setSpacing(itkVolumeType::SpacingType spacing);

  protected:
    virtual ChannelRepresentationSPtr createRepresentationProxy(FilterOutputId id, const FilterOutput::OutputRepresentationName &type);

    virtual bool ignoreCurrentOutputs() const
    { return false; }

    virtual bool needUpdate(FilterOutputId oId) const;

    virtual void run();

    virtual void run(FilterOutputId oId);

    itkVolumeType::SpacingType spacing();

  private:
    IOErrorHandler *m_handler;
  };

}// namespace EspINA

#endif // CHANNELREADER_H