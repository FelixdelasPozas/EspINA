/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SLICE_CACHED_REPRESENTATION_H_
#define ESPINA_SLICE_CACHED_REPRESENTATION_H_

// EspINA
#include "Representation.h"
#include "RepresentationEmptySettings.h"
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/EspinaTypes.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkSmartPointer.h>

class vtkImageActor;

namespace EspINA
{
  class TransparencySelectionHighlighter;

  class EspinaGUI_EXPORT CachedRepresentation
  : public Representation
  {
    public:
      explicit CachedRepresentation(DefaultVolumetricDataSPtr data,
                                    View2D *view);
      virtual ~CachedRepresentation()
      {};

      /* \brief Returns true if a change in crosshair point affects the representation.
       *
       */
      virtual bool crosshairDependent() const
      { return false; }

      /* \brief Returns the actor of the channel in the specified slice position.
       *
       */
      virtual vtkSmartPointer<vtkImageActor> getActor(const Nm pos) const = 0;

      /* \brief Implements Representation::settingsWidget.
       *
       */
      virtual RepresentationSettings* settingsWidget()
      { return new RepresentationEmptySettings(); }

      /* \brief Implements Representation::getActors.
       *
       */
      virtual QList<vtkProp*> getActors()
      { return QList<vtkProp *>(); }

      /* \brief Implements Representation::hasActor.
       *
       */
      virtual bool hasActor(vtkProp*) const
      { return false; }

      /* \brief Returns the plane this representation is on.
       *
       * Returns the plane this representation is on. To get the correct value the
       * method SetView() must have been called first.
       */
      Plane plane()
      { return toPlane(m_planeIndex); }

      /* \brief Returns the type of view this representation can be rendered on.
       *
       */
      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_SLICE; }

      /* \brief Returns true if the representation can generate an actor in that position.
       *
       */
      bool existsIn(const Nm position) const;

      /* \brief Returns the value of the last modification time of the m_data
       * when the last actor was created.
       */
      TimeStamp getModificationTime()
      { return m_lastUpdatedTime; }

      /* \brief Implements Representation::needUpdate();
       *
       */
      virtual bool needUpdate() const
      {
        return (m_data->lastModified() != m_lastUpdatedTime);
      }

    protected:
      /* \brief Compute the values of m_min && m_max for the representation.
       *
       */
      void computeLimits();

      DefaultVolumetricDataSPtr m_data;       // data that will be represented.
      int                       m_planeIndex; // plane index for the view.

      Nm                        m_min;        // m_min && m_max are values stored from the bounds in this plane
      Nm                        m_max;        // to avoid recomputing them every time in existsIn().
  };

  using CachedRepresentationPtr  = CachedRepresentation *;
  using CachedRepresentationSPtr = std::shared_ptr<CachedRepresentation>;
  using CachedRepresentationSList = QList<CachedRepresentationSPtr>;

  //-----------------------------------------------------------------------------
  class EspinaGUI_EXPORT ChannelSliceCachedRepresentation
  : public CachedRepresentation
  {
    Q_OBJECT
    public:
      static const Representation::Type TYPE;

    public:
      /* \brief ChannelSliceCachedRepresentation class constructor.
       *
       */
      ChannelSliceCachedRepresentation(DefaultVolumetricDataSPtr data,
                                       View2D *view);

      /* \brief ChannelSliceCachedRepresentation virtual destructor.
       *
       */
      virtual ~ChannelSliceCachedRepresentation()
      {};

      /* \brief Returns true if the specified point is inside the channel.
       *
       * Returns true if the specified point is inside the channel. Right now only checks if it's
       * inside the bounds, there is no detection of non aligned border channels.
       */
      virtual bool isInside(const NmVector3 &point) const;

      /* \brief Method that triggers the update of the actors of the representation.
       * Empty as this is not needed for this representation.
       *
       */
      virtual void updateRepresentation();

      /* \brief Returns the actor of the channel in the specified slice position.
       *
       */
      vtkSmartPointer<vtkImageActor> getActor(const Nm pos) const;

      /* \brief Implements Representation::updateVisibility.
       * \param[in] unused This value is ignored.
       *
       */
      virtual void updateVisibility(bool unused);

    signals:
      void update();
      void changeVisibility();

    protected:
      /* \brief Clone this representation for the specified 2D view.
       *
       */
      virtual RepresentationSPtr cloneImplementation(View2D *view);

      /* \brief Clone this representation for the specified 3D view.
       *
       */
      virtual RepresentationSPtr cloneImplementation(View3D *view)
      { return RepresentationSPtr(); }

      /* \brief Sets the view this representation will be renderer on.
       *
       * Sets the view this representation will be renderer on. The temporary actor (or symbolic as it
       * extends the bounds of the data in that view) is computed here.
       */
      void setView(View2D *view);
    };

  using ChannelSliceCachedRepresentationPtr  = ChannelSliceCachedRepresentation *;
  using ChannelSliceCachedRepresentationSPtr = std::shared_ptr<ChannelSliceCachedRepresentation>;

  //----------------------------------------------------------------------------- TODO
  class EspinaGUI_EXPORT SegmentationSliceCachedRepresentation
  : public CachedRepresentation
  {
    Q_OBJECT
    public:
      static const Representation::Type TYPE;
      static TransparencySelectionHighlighter *s_highlighter;

    public:
      /* \brief SegmentationSliceCachedRepresentation class constructor.
       *
       */
      explicit SegmentationSliceCachedRepresentation(DefaultVolumetricDataSPtr data,
                                                     View2D *view);

      /* \brief SegmentationSliceCachedRepresentation virtual destructor.
       *
       */
      virtual ~SegmentationSliceCachedRepresentation()
      {};

      /* \brief Returns a serialization of this representation's settings.
       *
       */
      virtual QString serializeSettings();

      /* \brief Restores the settings of the representation with the specified one.
       *
       */
      virtual void restoreSettings(QString settings);

      /* \brief Empty, no use in segmentations
       *
       */
      virtual void setBrightness(double value)
      {};

      /* \brief Empty, no use in segmentations
       *
       */
      virtual void setContrast(double value)
      {};

      /* \brief Sets the color of the segmentation.
       *
       * Sets the color of the segmentation, specified as a QColor.
       */
      virtual void setColor(const QColor &color);

      /* \brief Returns the color of the segmentation representation.
       *
       */
      virtual QColor color() const;

      /* \brief Sets the highlighter value for this representation.
       *
       */
      virtual void setHighlighted(bool highlighted);

      /* \brief Returns true if the point given is inside the segmentation.
       *
       */
      virtual bool isInside(const NmVector3 &point) const;

      /* \brief Method that triggers the update of the actors of the representation.
       * Empty as this is not needed in this kind of representation.
       *
       */
      virtual void updateRepresentation();

      /* \brief Implements Representation::updateVisibility.
       * \param[in] unused This value is ignored.
       *
       */
      virtual void updateVisibility(bool unused);

      /* \brief Returns the actor of the channel in the specified slice position.
       *
       */
      vtkSmartPointer<vtkImageActor> getActor(const Nm pos) const;

      /* \brief Sets the view this representation will be renderer on.
       *
       */
      void setView(View2D *view);

    signals:
      void update();
      void changeColor();
      void changeVisibility();

    protected slots:
      void dataChanged();

    protected:
      /* \brief Clone this representation for the specified 2D view.
       *
       */
      virtual RepresentationSPtr cloneImplementation(View2D *view);

      /* \brief Clone this representation for the specified 3D view.
       *
       */
      virtual RepresentationSPtr cloneImplementation(View3D *view)
      { return RepresentationSPtr(); }

    private:
      NmVector3 m_depth; // depth of the actor for this view
  };

  using SegmentationSliceCachedRepresentationPtr  = SegmentationSliceCachedRepresentation *;
  using SegmentationSliceCachedRepresentationSPtr = std::shared_ptr<SegmentationSliceCachedRepresentation>;


} /* namespace EspINA */

#endif /* SLICECACHEDREPRESENTATION_H_ */
