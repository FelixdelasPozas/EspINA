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

#ifndef ESPINA_SLICE_REPRESENTATION_H
#define ESPINA_SLICE_REPRESENTATION_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Utils/Spatial.h>
#include <GUI/Representations/Representation.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkSmartPointer.h>

//Forward declaration
class vtkImageReslice;
class vtkImageMapToColors;
class vtkImageShiftScale;
class vtkImageActor;
class vtkLookupTable;

namespace ESPINA
{
  class TransparencySelectionHighlighter;
  class View2D;

  class EspinaGUI_EXPORT ChannelSliceRepresentation
  : public Representation
  {
  public:
    static const Representation::Type TYPE;

  public:
    explicit ChannelSliceRepresentation(DefaultVolumetricDataSPtr data, View2D *view);
    virtual ~ChannelSliceRepresentation();

    virtual RepresentationSettings *settingsWidget();

    virtual void setColor(const QColor &color);

    virtual void setBrightness(double value);

    virtual void setContrast(double value);

    virtual void setOpacity(double value);

    virtual bool isInside(const NmVector3 &point) const;

    virtual RenderableView canRenderOnView() const
    { return Representation::RENDERABLEVIEW_SLICE; }

    virtual bool hasActor(vtkProp *actor) const;

    virtual void updateRepresentation();

    virtual QList<vtkProp*> getActors();

    void setPlane(Plane plane)
    { m_planeIndex = normalCoordinateIndex(plane); }

    Plane plane()
    { return toPlane(m_planeIndex); }

    virtual bool crosshairDependent() const
    { return true; }

    virtual bool needUpdate()
    { return m_lastUpdatedTime != m_data->lastModified(); }

  protected:
    virtual RepresentationSPtr cloneImplementation(View2D *view);

    virtual RepresentationSPtr cloneImplementation(View3D *view)
    { return RepresentationSPtr(); }

    virtual void updateVisibility(bool visible);

  private:
    void setView(View2D *view) { m_view = view; };
    void initializePipeline();

  private:
    DefaultVolumetricDataSPtr m_data;
    int m_planeIndex;
    Nm m_reslicePoint;

    vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
    vtkSmartPointer<vtkImageShiftScale>  m_shiftScaleFilter;
    vtkSmartPointer<vtkImageActor>       m_actor;
    vtkSmartPointer<vtkLookupTable>      m_lut;
  };

  class EspinaGUI_EXPORT SegmentationSliceRepresentation
  : public Representation
  {
  public:
    static const Representation::Type TYPE;

  public:
    explicit SegmentationSliceRepresentation(DefaultVolumetricDataSPtr data,
                                             View2D *view);
    virtual ~SegmentationSliceRepresentation();

    virtual RepresentationSettings *settingsWidget();

    virtual QString serializeSettings();

    virtual void restoreSettings(QString settings);

    virtual void setColor(const QColor &color);

    virtual QColor color() const;

    virtual void setHighlighted(bool highlighted);

    virtual bool isInside(const NmVector3 &point) const;

    virtual RenderableView canRenderOnView() const
    { return Representation::RENDERABLEVIEW_SLICE; }

    virtual bool hasActor(vtkProp *actor) const;

    virtual void updateRepresentation();

    virtual QList<vtkProp*> getActors();

    void setPlane(Plane plane)
    { m_planeIndex = normalCoordinateIndex(plane); }

    Plane plane()
    { return toPlane(m_planeIndex); }

    virtual bool crosshairDependent() const
    { return true; }

    virtual bool needUpdate()
    { return m_lastUpdatedTime != m_data->lastModified(); }

  protected:
    virtual RepresentationSPtr cloneImplementation(View2D *view);

    virtual RepresentationSPtr cloneImplementation(View3D *view)
    { return RepresentationSPtr(); }

    virtual void updateVisibility(bool visible);

  private:
    void setView(View2D *view) { m_view = view; };
    void initializePipeline();

  private:
    DefaultVolumetricDataSPtr m_data;
    int m_planeIndex;
    Nm m_reslicePoint;
    vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
    vtkSmartPointer<vtkImageActor>       m_actor;

    static TransparencySelectionHighlighter *s_highlighter;
  };

  using ChannelSliceRepresentationPtr  = ChannelSliceRepresentation *;
  using ChannelSliceRepresentationSPtr = std::shared_ptr<ChannelSliceRepresentation>;

  using SegmentationSliceRepresentationPtr  = SegmentationSliceRepresentation *;
  using SegmentationSliceRepresentationSPtr = std::shared_ptr<SegmentationSliceRepresentation>;

} // namespace ESPINA

#endif // ESPINA_SLICE_REPRESENTATION_H
