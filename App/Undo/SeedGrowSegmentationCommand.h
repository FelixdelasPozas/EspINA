/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef SEEDGROWSEGMENTATIONCOMMAND_H
#define SEEDGROWSEGMENTATIONCOMMAND_H

#include <QUndoStack>

#include <Core/Model/EspinaModel.h>

namespace EspINA
{
  class SeedGrowSegmentationCommand
  : public QUndoCommand
  {
  public:
    explicit SeedGrowSegmentationCommand(ChannelPtr               channel,
                                         itkVolumeType::IndexType seed,
                                         int                      voiExtent[6],
                                         int                      lowerThreshold,
                                         int                      upperThreshold,
                                         bool                     applyClosing,
                                         TaxonomyElementPtr       taxonomy,
                                         EspinaModel          *model,
                                         QUndoCommand *           parent= NULL);

    virtual ~SeedGrowSegmentationCommand();

    virtual void redo();
    virtual void undo();

  private:
    EspinaModel *m_model;

    SampleSPtr               m_sample;
    ChannelSPtr         m_channel;
    TaxonomyElementSPtr m_taxonomy;

    FilterSPtr       m_filter;
    SegmentationSPtr m_segmentation;
  };

} // namespace EspINA

#endif // SEEDGROWSEGMENTATIONCOMMAND_H
