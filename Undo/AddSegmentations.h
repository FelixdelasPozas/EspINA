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
                              ModelAdapterSPtr        model,
                              QUndoCommand           *parent = nullptr);

    explicit AddSegmentations(SegmentationAdapterSList segmentations,
                              ModelAdapterSPtr         model,
                              QUndoCommand            *parent = nullptr);
    virtual void redo();
    virtual void undo();

  private:
    SampleAdapterSList findSamplesUsingInputChannels(SegmentationAdapterSPtr segmentation);

  private:
    ModelAdapterSPtr m_model;

    QMap<SegmentationAdapterSPtr, SampleAdapterSList> m_samples;
    SegmentationAdapterSList m_segmentations;
  };
} // namespace EspINA

#endif // ESPIN_ADD_SEGMENTATION_H
