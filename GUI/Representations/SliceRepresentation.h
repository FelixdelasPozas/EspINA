/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SLICEREPRESENTATION_H
#define SLICEREPRESENTATION_H

#include "GUI/Representations/GraphicalRepresentation.h"

#include <Core/Outputs/VolumeRepresentation.h>
#include <Core/Model/HierarchyItem.h>

#include <vtkSmartPointer.h>

//Forward declaration
class vtkImageReslice;
class vtkImageMapToColors;
class vtkImageShiftScale;
class vtkImageActor;
class vtkLookupTable;

namespace EspINA
{
  class TransparencySelectionHighlighter;
  class SliceView;

  class ChannelSliceRepresentation
  : public ChannelGraphicalRepresentation
  {
    Q_OBJECT
  public:
    explicit ChannelSliceRepresentation(ChannelVolumeSPtr data,
                                        SliceView        *view);
    virtual ~ChannelSliceRepresentation();

    virtual void setBrightness(double value);

    virtual void setContrast(double value);

    virtual void setColor(const QColor &color);

    virtual void setHighlighted(bool highlighted);

    virtual void setOpacity(double value);

    virtual void setVisible(bool visible);

    virtual bool isInside(Nm point[3]);

    virtual RenderableView canRenderOnView() const
    { return GraphicalRepresentation::SLICE_VIEW; }

    virtual GraphicalRepresentationSPtr clone(SliceView *view);

    virtual GraphicalRepresentationSPtr clone(VolumeView *view)
    { return GraphicalRepresentationSPtr(); }

    virtual bool hasActor(vtkProp *actor) const;

    virtual void updateRepresentation();

  private slots:
    void updatePipelineConnections();

  private:
    void initializePipeline(EspINA::SliceView *view);

  private:
    ChannelVolumeSPtr m_data;

    vtkSmartPointer<vtkImageReslice>     reslice;
    vtkSmartPointer<vtkImageMapToColors> mapToColors;
    vtkSmartPointer<vtkImageShiftScale>  shiftScaleFilter;
    vtkSmartPointer<vtkImageActor>       slice;
    vtkSmartPointer<vtkLookupTable>      lut;
  };

  class SegmentationSliceRepresentation
  : public SegmentationGraphicalRepresentation
  {
    Q_OBJECT
  public:
    explicit SegmentationSliceRepresentation(SegmentationVolumeSPtr data,
                                             SliceView             *view);
    virtual ~SegmentationSliceRepresentation();

    virtual void setColor(const QColor &color);

    virtual void setHighlighted(bool highlighted);

    virtual void setVisible(bool visible);

    virtual bool isInside(Nm point[3]);

    virtual RenderableView canRenderOnView() const
    { return GraphicalRepresentation::SLICE_VIEW; }

    virtual GraphicalRepresentationSPtr clone(SliceView *view);

    virtual GraphicalRepresentationSPtr clone(VolumeView *view)
    { return GraphicalRepresentationSPtr(); }

    virtual bool hasActor(vtkProp *actor) const;

    virtual void updateRepresentation();

  private slots:
    void updatePipelineConnections();

  private:
    void initializePipeline(EspINA::SliceView *view);

  private:
    SegmentationVolumeSPtr m_data;
    SliceView             *m_view;

    vtkSmartPointer<vtkImageReslice>     reslice;
    vtkSmartPointer<vtkImageMapToColors> mapToColors;
    vtkSmartPointer<vtkImageActor>       slice;

    static TransparencySelectionHighlighter *s_highlighter;
  };


} // namespace EspINA

#endif // SLICEREPRESENTATION_H
