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
#include <Core/Analysis/Data/VolumetricData.hxx>
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
    /** \brief ChannelSliceRepresentation class constructor.
		 * \param[in] data, VolumetricData smart pointer of the data to represent.
 		 * \param[in] view, View2D pointer of the view this representation will be shown.
     *
     */
    explicit ChannelSliceRepresentation(DefaultVolumetricDataSPtr data, View2D *view);

    /** \brief ChannelSliceRepresentation class virtual destructor.
     *
     */
    virtual ~ChannelSliceRepresentation();

    /** \brief Implements Representation::settingsWidget().
     *
     */
    virtual RepresentationSettings *settingsWidget();

    /** \brief Overrides Representation::setColor().
     *
     */
    virtual void setColor(const QColor &color) override;

    /** \brief Overrides Representation::setBrightness().
     *
     */
    virtual void setBrightness(double value) override;

    /** \brief Overrides Representation::setContrast().
     *
     */
    virtual void setContrast(double value) override;

    /** \brief Overrides Representation::setOpacity().
     *
     */
    virtual void setOpacity(double value) override;

    /** \brief Implements Representation::isInside() const.
     *
     */
    virtual bool isInside(const NmVector3 &point) const;

    /** \brief Implements Representation::canRenderOnView() const.
     *
     */
    virtual RenderableView canRenderOnView() const
    { return Representation::RENDERABLEVIEW_SLICE; }

    /** \brief Implements Representation::hasActor() const.
     *
     */
    virtual bool hasActor(vtkProp *actor) const;

    /** \brief Implements Representation::updateRepresentation().
     *
     */
    virtual void updateRepresentation();

    /** \brief Implements Representation::getActors().
     *
     */
    virtual QList<vtkProp*> getActors();

    /** \brief Sets the plane of the slice representation.
     * \param[in] plane, plane of the representation.
     *
     */
    void setPlane(Plane plane)
    { m_planeIndex = normalCoordinateIndex(plane); }

    /** \brief Returns the plane of the representation.
     *
     */
    Plane plane()
    { return toPlane(m_planeIndex); }

    /** \brief Implements Representation::crosshairDependent() const.
     *
     */
    virtual bool crosshairDependent() const
    { return true; }

    /** \brief Implements Representation::needUpdate() const.
     *
     */
    virtual bool needUpdate() const
    { return m_lastUpdatedTime != m_data->lastModified(); }

  protected:
    /** \brief Implements Representation::cloneImplementation(View2D*).
     *
     */
    virtual RepresentationSPtr cloneImplementation(View2D *view);

    /** \brief Implements Representation::cloneImplementation(View3D*).
     *
     */
    virtual RepresentationSPtr cloneImplementation(View3D *view)
    { return RepresentationSPtr(); }

    /** \brief Implements Representation::updateVisibility().
     *
     */
    virtual void updateVisibility(bool visible);

  private:
    /** \brief Helper method to set the view of the representation.
     * \param[in] view, View2D raw pointer.
     *
     */
    void setView(View2D *view) { m_view = view; };

    /** \brief Helper method to initialize the vtk pipeline.
     *
     */
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
    /** \brief SegmentationSliceRepresentation class constructor.
		 * \param[in] data, VolumetricData smart pointer of the data to represent.
 		 * \param[in] view, View2D pointer of the view this representation will be shown.
     *
     */
    explicit SegmentationSliceRepresentation(DefaultVolumetricDataSPtr data,
                                             View2D *view);

    /** \brief SegmentationSliceRepresentation class destructor.
     *
     */
    virtual ~SegmentationSliceRepresentation();

    /** \brief Implements Representation::
     *
     */
    virtual RepresentationSettings *settingsWidget();

    /** \brief Overrides Representation::serializeSettings().
     *
     */
    virtual QString serializeSettings() override;

    /** \brief Overrides Representation::restoreSettings().
     *
     */
    virtual void restoreSettings(QString settings) override;

    /** \brief Overrides Representation::setColor().
     *
     */
    virtual void setColor(const QColor &color) override;

    /** \brief Overrides Representation::color().
     *
     */
    virtual QColor color() const override;

    /** \brief Overrides Representation::setHighlighted().
     *
     */
    virtual void setHighlighted(bool highlighted) override;

    /** \brief Implements Representation::isInside() const.
     *
     */
    virtual bool isInside(const NmVector3 &point) const;

    /** \brief Implements Representation::canRenderOnView().
     *
     */
    virtual RenderableView canRenderOnView() const
    { return Representation::RENDERABLEVIEW_SLICE; }

    /** \brief Implements Representation::hasActor() const.
     *
     */
    virtual bool hasActor(vtkProp *actor) const;

    /** \brief Implements Representation::updateRepresentation().
     *
     */
    virtual void updateRepresentation();

    /** \brief Implements Representation::getActors().
     *
     */
    virtual QList<vtkProp*> getActors();

    /** \brief Sets the plane of the slice representation.
     * \param[in] plane, plane of the representation.
     *
     */
    void setPlane(Plane plane)
    { m_planeIndex = normalCoordinateIndex(plane); }

    /** \brief Returns the plane of the representation().
     *
     */
    Plane plane()
    { return toPlane(m_planeIndex); }

    /** \brief Implements Representation::crosshairDependent() const.
     *
     */
    virtual bool crosshairDependent() const
    { return true; }

    /** \brief Implements Representation::needUpdate() const.
     *
     */
    virtual bool needUpdate() const
    { return m_lastUpdatedTime != m_data->lastModified(); }

  protected:
    /** \brief Implements Representation::cloneImplementeation(View2D*).
     *
     */
    virtual RepresentationSPtr cloneImplementation(View2D *view);

    /** \brief Implements Representation::cloneImplementeation(View3D*).
     *
     */
    virtual RepresentationSPtr cloneImplementation(View3D *view)
    { return RepresentationSPtr(); }

    /** \brief Implements Representation::updateVisibility().
     *
     */
    virtual void updateVisibility(bool visible);

  private:
    /** \brief Helper method to set the view of the representation.
     * \param[in] view, View2D raw pointer.
     *
     */
    void setView(View2D *view) { m_view = view; };

    /** \brief Helper method to initialize the vtk pipeline.
     *
     */
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
