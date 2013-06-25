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
#include <GUI/vtkWidgets/ContourWidget.h>

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
                         ViewManager *vm,
                         FilledContour *tool);

      virtual ~ContourUndoCommand();

      virtual void redo();
      virtual void undo();

      void rasterizeContour(ContourWidget::ContourData) const;

    private:
      // helper method
      bool intersect(const EspinaRegion &region1, const EspinaRegion &region2) const;

      typedef SegmentationVolume::EditedVolumeRegionSList EditedRegionSList;

      SegmentationSPtr                   m_segmentation;
      ViewManager                       *m_viewManager;

      mutable itkVolumeType::Pointer     m_prevVolume;
      mutable itkVolumeType::Pointer     m_newVolume;
      mutable bool                       m_needReduction;
      mutable EditedRegionSList          m_prevRegions;
      FilledContour                     *m_tool;
      mutable Nm                         m_bounds[6];
      mutable bool                       m_rasterized;

      mutable ContourWidget::ContourData m_contour;
  };

  class ContourAddSegmentation
  : public QUndoCommand
  {
  public:
    explicit ContourAddSegmentation(ChannelSPtr                channel,
                                    TaxonomyElementSPtr        taxonomy,
                                    EspinaModel               *model,
                                    ViewManager               *vm,
                                    FilledContour             *tool);
    virtual ~ContourAddSegmentation();

    virtual void redo();
    virtual void undo();

    void rasterizeContour(ContourWidget::ContourData) const;

    SegmentationSPtr getCreatedSegmentation() const;

  private:
    EspinaModel *m_model;

    SampleSPtr                         m_sample;
    ChannelSPtr                        m_channel;
    mutable FilterSPtr                 m_filter;
    mutable SegmentationSPtr           m_segmentation;
    TaxonomyElementSPtr                m_taxonomy;
    ViewManager                       *m_viewManager;
    FilledContour                     *m_tool;
    mutable bool                       m_rasterized;
    mutable ContourWidget::ContourData m_contour;
  };

} /* namespace EspINA */
#endif /* CONTOURUNDOCOMMAND_H_ */
