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

#include "EspinaGUI_Export.h"

// EspINA
#include "GUI/Representations/GraphicalRepresentation.h"
#include <Core/OutputRepresentations/VolumeRepresentation.h>
#include <Core/Model/HierarchyItem.h>
#include <GUI/QtWidget/SliceView.h>

// VTK
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

  class EspinaGUI_EXPORT ChannelSliceRepresentation
  : public ChannelGraphicalRepresentation
  {
    Q_OBJECT
  public:
    explicit ChannelSliceRepresentation(ChannelVolumeSPtr data,
                                        SliceView        *view);
    virtual ~ChannelSliceRepresentation() {};

    virtual void setBrightness(double value);

    virtual void setContrast(double value);

    virtual GraphicalRepresentationSettings *settingsWidget();

    virtual void setColor(const QColor &color);

    virtual void setOpacity(double value);

    virtual bool isInside(Nm point[3]);

    virtual RenderableView canRenderOnView() const
    { return GraphicalRepresentation::RENDERABLEVIEW_SLICE; }

    virtual bool hasActor(vtkProp *actor) const;

    virtual void updateRepresentation();

    virtual QList<vtkProp*> getActors();

  protected:
    virtual GraphicalRepresentationSPtr cloneImplementation(SliceView *view);

    virtual GraphicalRepresentationSPtr cloneImplementation(VolumeView *view)
    { return GraphicalRepresentationSPtr(); }

    virtual void updateVisibility(bool visible);

  private slots:
    void updatePipelineConnections();

  private:
    void setView(SliceView *view) { m_view = view; };
    void initializePipeline();

  private:
    ChannelVolumeSPtr m_data;

    vtkSmartPointer<vtkImageReslice>     m_reslice;
    vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
    vtkSmartPointer<vtkImageShiftScale>  m_shiftScaleFilter;
    vtkSmartPointer<vtkImageActor>       m_actor;
    vtkSmartPointer<vtkLookupTable>      m_lut;
  };

  class EspinaGUI_EXPORT SegmentationSliceRepresentation
  : public SegmentationGraphicalRepresentation
  {
    Q_OBJECT
  public:
    explicit SegmentationSliceRepresentation(SegmentationVolumeSPtr data,
                                             SliceView             *view);
    virtual ~SegmentationSliceRepresentation() {};

    virtual GraphicalRepresentationSettings *settingsWidget();

    virtual void setColor(const QColor &color);

    virtual void setHighlighted(bool highlighted);

    virtual bool isInside(Nm point[3]);

    virtual RenderableView canRenderOnView() const
    { return GraphicalRepresentation::RENDERABLEVIEW_SLICE; }

    virtual bool hasActor(vtkProp *actor) const;

    virtual void updateRepresentation();

    virtual QList<vtkProp*> getActors();

  protected:
    virtual GraphicalRepresentationSPtr cloneImplementation(SliceView *view);

    virtual GraphicalRepresentationSPtr cloneImplementation(VolumeView *view)
    { return GraphicalRepresentationSPtr(); }

    virtual void updateVisibility(bool visible);

  private slots:
    void updatePipelineConnections();

  private:
    void setView(SliceView *view) { m_view = view; };
    void initializePipeline();

  private:
    SegmentationVolumeSPtr m_data;

    vtkSmartPointer<vtkImageReslice>     m_reslice;
    vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
    vtkSmartPointer<vtkImageActor>       m_actor;

    static TransparencySelectionHighlighter *s_highlighter;
  };

  typedef boost::shared_ptr<ChannelSliceRepresentation> ChannelSliceRepresentationSPtr;
  typedef QList<ChannelSliceRepresentationSPtr> ChannelSliceRepresentationSList;

  typedef boost::shared_ptr<SegmentationSliceRepresentation> SegmentationSliceRepresentationSPtr;
  typedef QList<SegmentationSliceRepresentationSPtr> SegmentationSliceRepresentationSList;

} // namespace EspINA

#endif // SLICEREPRESENTATION_H
