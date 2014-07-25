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


#ifndef SEEDGROWSEGMENTATIONCOMMAND_H
#define SEEDGROWSEGMENTATIONCOMMAND_H

#include <QUndoStack>

#include <Core/Model/Filter.h>

namespace ESPINA
{
  class SeedGrowSegmentationCommand
  : public QUndoCommand
  {
  public:
    static const Filter::FilterType FILTER_TYPE;

  public:
    explicit SeedGrowSegmentationCommand(ChannelPtr               channel,
                                         itkVolumeType::IndexType seed,
                                         int                      voiExtent[6],
                                         int                      lowerThreshold,
                                         int                      upperThreshold,
                                         bool                     applyClosing,
                                         TaxonomyElementPtr       taxonomy,
                                         EspinaModel             *model,
                                         ViewManager             *viewManager,
                                         SegmentationSList       &createdSegmentations,
                                         QUndoCommand *           parent= NULL);

    virtual ~SeedGrowSegmentationCommand();

    virtual void redo();
    virtual void undo();

  private:
    EspinaModel *m_model;
    ViewManager *m_viewManager;

    SampleSPtr          m_sample;
    ChannelSPtr         m_channel;
    TaxonomyElementSPtr m_taxonomy;

    FilterSPtr       m_filter;
    SegmentationSPtr m_segmentation;
  };

} // namespace ESPINA

#endif // SEEDGROWSEGMENTATIONCOMMAND_H
