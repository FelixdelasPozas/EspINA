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
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Utils/Spatial.h>
#include <GUI/Representations/GraphicalRepresentation.h>
#include <GUI/View/View2D.h>

// ITK
#include <itkImageToVTKImageFilter.h>

// VTK
#include <vtkSmartPointer.h>

//Forward declaration
class vtkImageReslice;
class vtkImageMapToColors;
class vtkImageShiftScale;
class vtkImageActor;
class vtkLookupTable;
class vtkImageImport;

namespace EspINA
{
  class TransparencySelectionHighlighter;
  class View2D;

  using SegmentationVolumeSPtr = std::shared_ptr<VolumetricData<itkVolumeType>>;
  using ChannelVolumeSPtr = std::shared_ptr<VolumetricData<itkVolumeType>>;

  class EspinaGUI_EXPORT ChannelSliceRepresentation
  : public Representation
  {
  public:
    explicit ChannelSliceRepresentation(ChannelVolumeSPtr data,
                                        View2D        *view);
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
    { m_plane = plane; }

    Plane plane()
    { return m_plane; }

  protected:
    virtual RepresentationSPtr cloneImplementation(View2D *view);

    virtual RepresentationSPtr cloneImplementation(View3D *view)
    { return RepresentationSPtr(); }

    virtual void updateVisibility(bool visible);

    virtual void onCrosshairChanged(const NmVector3& point)
    { updateRepresentation(); }

  private:
    void setView(View2D *view) { m_view = view; };
    void initializePipeline();

  private:
    ChannelVolumeSPtr m_data;
    Plane             m_plane;

    using ExporterType = itk::ImageToVTKImageFilter<itkVolumeType>;


    ExporterType::Pointer                m_exporter;
    vtkSmartPointer<vtkImageImport>      m_importer;
    vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
    vtkSmartPointer<vtkImageShiftScale>  m_shiftScaleFilter;
    vtkSmartPointer<vtkImageActor>       m_actor;
    vtkSmartPointer<vtkLookupTable>      m_lut;
  };

  class EspinaGUI_EXPORT SegmentationSliceRepresentation
  : public Representation
  {
  public:
    explicit SegmentationSliceRepresentation(SegmentationVolumeSPtr data,
                                             View2D                *view);
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
    { m_plane = plane; }

    Plane plane()
    { return m_plane; }

  protected:
    virtual RepresentationSPtr cloneImplementation(View2D *view);

    virtual RepresentationSPtr cloneImplementation(View3D *view)
    { return RepresentationSPtr(); }

    virtual void updateVisibility(bool visible);

    virtual void onCrosshairChanged(const NmVector3& point);

  private:
    void setView(View2D *view) { m_view = view; };
    void initializePipeline();

  private:
    SegmentationVolumeSPtr m_data;
    Plane                  m_plane;

    using ExporterType = itk::ImageToVTKImageFilter<itkVolumeType>;

    ExporterType::Pointer                m_exporter;
    vtkSmartPointer<vtkImageImport>      m_importer;
    vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
    vtkSmartPointer<vtkImageActor>       m_actor;

    static TransparencySelectionHighlighter *s_highlighter;
  };

  using ChannelSliceRepresentationPtr  = ChannelSliceRepresentation *;
  using ChannelSliceRepresentationSPtr = std::shared_ptr<ChannelSliceRepresentation>;

  using SegmentationSliceRepresentationPtr  = SegmentationSliceRepresentation *;
  using SegmentationSliceRepresentationSPtr = std::shared_ptr<SegmentationSliceRepresentation>;

} // namespace EspINA

#endif // SLICEREPRESENTATION_H
