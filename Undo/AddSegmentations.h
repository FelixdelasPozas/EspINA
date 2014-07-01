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


#ifndef ESPIN_ADD_SEGMENTATION_H
#define ESPIN_ADD_SEGMENTATION_H

#include "Undo/EspinaUndo_Export.h"

// EspINA
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoCommand>

namespace EspINA
{
  class EspinaUndo_EXPORT AddSegmentations
  : public QUndoCommand
  {
  public:
    explicit AddSegmentations(SegmentationAdapterSPtr segmentation,
                              SampleAdapterSList      samples,
                              ModelAdapterSPtr        model,
                              QUndoCommand           *parent = nullptr);

    explicit AddSegmentations(SegmentationAdapterSList segmentations,
                              SampleAdapterSList       samples,
                              ModelAdapterSPtr         model,
                              QUndoCommand            *parent = nullptr);
    virtual void redo();
    virtual void undo();

  private:
    SampleAdapterSList findSamplesUsingInputChannels(SegmentationAdapterSPtr segmentation);

  private:
    SampleAdapterSList m_samples;
    ModelAdapterSPtr m_model;

    SegmentationAdapterSList m_segmentations;
  };
} // namespace EspINA

#endif // ESPIN_ADD_SEGMENTATION_H
