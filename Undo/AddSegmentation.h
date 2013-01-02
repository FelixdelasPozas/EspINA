/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ADDSEGMENTATION_H
#define ADDSEGMENTATION_H

#include <QUndoCommand>

#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>

namespace EspINA
{
  class AddSegmentation
  : public QUndoCommand
  {
  public:
    explicit AddSegmentation(ChannelSPtr         channel,
                             FilterSPtr          filter,
                             SegmentationSPtr    seg,
                             TaxonomyElementSPtr taxonomy,
                             EspinaModelSPtr     model
    );
    virtual void redo();
    virtual void undo();

  private:
    EspinaModelSPtr     m_model;

    SampleSPtr          m_sample;
    ChannelSPtr         m_channel;
    FilterSPtr          m_filter;
    SegmentationSPtr    m_seg;
    TaxonomyElementSPtr m_taxonomy;
  };

} // namespace EspINA

#endif // ADDSEGMENTATION_H
