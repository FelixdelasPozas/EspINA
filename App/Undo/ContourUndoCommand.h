/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef CONTOURUNDOCOMMAND_H_
#define CONTOURUNDOCOMMAND_H_

// EspINA
#include <Core/EspinaTypes.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaModel.h>

// Qt
#include <QUndoStack>

class vtkPolyData;

namespace EspINA
{
  class ViewManager;
  class FilledContour;
  
  class ContourUndoCommand
  : public QUndoCommand
  {
    public:
      ContourUndoCommand(SegmentationSPtr seg,
                         vtkPolyData* contour,
                         Nm pos,
                         PlaneType plane,
                         itkVolumeType::PixelType value,
                         ViewManager *vm,
                         FilledContour *tool);

      virtual ~ContourUndoCommand();

      virtual void redo();
      virtual void undo();

    private:
      SegmentationSPtr         m_segmentation;
      Nm                       m_contourBounds[6];
      vtkPolyData             *m_contour;
      PlaneType                m_plane;
      Nm                       m_pos;
      itkVolumeType::PixelType m_value;
      ViewManager             *m_viewManager;

      itkVolumeType::Pointer   m_prevVolume;
      itkVolumeType::Pointer   m_newVolume;
      bool                     m_needReduction;
      QList<EspinaRegion>      m_prevRegions;
      FilledContour           *m_tool;
      bool                     m_abortOperation;
  };

  class ContourAddSegmentation
  : public QUndoCommand
  {
  public:
    explicit ContourAddSegmentation(ChannelSPtr         channel,
                                    FilterSPtr          filter,
                                    SegmentationSPtr    seg,
                                    TaxonomyElementSPtr taxonomy,
                                    EspinaModel        *model,
                                    FilledContour      *tool);
    virtual void redo();
    virtual void undo();

  private:
    EspinaModel *m_model;

    SampleSPtr          m_sample;
    ChannelSPtr         m_channel;
    FilterSPtr          m_filter;
    SegmentationSPtr    m_seg;
    TaxonomyElementSPtr m_taxonomy;
    FilledContour      *m_tool;
    bool                m_abortOperation;
  };

} /* namespace EspINA */
#endif /* CONTOURUNDOCOMMAND_H_ */
