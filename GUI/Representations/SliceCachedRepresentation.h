/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

  //-----------------------------------------------------------------------------
  class EspinaGUI_EXPORT ChannelSliceCachedRepresentation
  : public Representation
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

      /* \brief Returns the type of view this representation can be rendered on.
       *
       */
      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_SLICE; }

      /* \brief Method that triggers the update of the actors of the representation.
       * Empty as this is not needed for this representation.
       *
       */
      virtual void updateRepresentation();

      /* \brief Returns the plane this representation is on.
       *
       * Returns the plane this representation is on. To get the correct value the
       * method SetView() must have been called first.
       */
      Plane plane()
      { return toPlane(m_planeIndex); }

      /* \brief Returns true if a change in crosshair point affects the representation.
       *
       */
      virtual bool crosshairDependent() const
      { return false; }

      /* \brief Returns the actor of the channel in the specified slice position.
       *
       */
      vtkSmartPointer<vtkImageActor> getActor(Nm pos);

      /* \brief Implements Representation::settingsWidget.
       *
       */
      virtual RepresentationSettings* settingsWidget()
      { return new RepresentationEmptySettings(); }

      /* \brief Implements Representation::hasActor.
       *
       */
      virtual bool hasActor(vtkProp*) const
      { return false; }

      /* \brief Implements Representation::getActors.
       *
       */
      virtual QList<vtkProp*> getActors()
      { return QList<vtkProp *>(); }

      /* \brief Implements Representation::updateVisibility.
       *
       */
      virtual void updateVisibility(bool value);

      /* \brief Returns the value of the last modification time of the m_data
       * when the last actor was created.
       */
      TimeStamp getModificationTime()
      { return m_timeStamp; }

    signals:
      void update();
      void changeVisibility(bool);

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

    private:
      DefaultVolumetricDataSPtr m_data;       // data that will be represented.
      int                       m_planeIndex; // plane index for the view.
      TimeStamp                 m_timeStamp;  // Modification time of the m_data when the actor was created.
    };

  using ChannelSliceCachedRepresentationPtr  = ChannelSliceCachedRepresentation *;
  using ChannelSliceCachedRepresentationSPtr = std::shared_ptr<ChannelSliceCachedRepresentation>;

  //----------------------------------------------------------------------------- TODO
  class EspinaGUI_EXPORT SegmentationSliceCachedRepresentation
  : public Representation
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

      /* \brief Returns the type of view this representation can be rendered on.
       *
       */
      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_SLICE; }

      /* \brief Method that triggers the update of the actors of the representation.
       * Empty as this is not needed in this kind of representation.
       *
       */
      virtual void updateRepresentation();

      /* \brief Returns the plane this representation is on.
       *
       * Returns the plane this representation is on. To get the correct value the
       * method SetView() must have been called first.
       */
      Plane plane()
      { return toPlane(m_planeIndex); }

      /* \brief Returns true if a change in crosshair point affects the representation.
       *
       */
      virtual bool crosshairDependent() const
      { return false; }

      /* \brief Returns the actor of the channel in the specified slice position.
       *
       */
      vtkSmartPointer<vtkImageActor> getActor(Nm pos);

      /* \brief Implements Representation::settingsWidget.
       *
       */
      virtual RepresentationSettings* settingsWidget()
      { return new RepresentationEmptySettings(); }

      /* \brief Implements Representation::hasActor.
       *
       */
      virtual bool hasActor(vtkProp*) const
      { return false; }

      /* \brief Implements Representation::getActors.
       *
       */
      virtual QList<vtkProp*> getActors()
      { return QList<vtkProp *>(); }

      /* \brief Implements Representation::updateVisibility.
       *
       */
      virtual void updateVisibility(bool value);

      /* \brief Sets the view this representation will be renderer on.
       *
       */
      void setView(View2D *view);

      /* \brief Returns the value of the last modification time of the m_data
       * when the last actor was created.
       */
      TimeStamp getModificationTime()
      { return m_timeStamp; }

    signals:
      void update();
      void changeVisibility(bool);

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
      DefaultVolumetricDataSPtr m_data;       // data that will be represented.
      int                       m_planeIndex; // plane index for the view.
      NmVector3                 m_depth;      // depth of the actor for this view
      TimeStamp                 m_timeStamp;  // Modification time of the m_data when the actor was created.
  };

  using SegmentationSliceCachedRepresentationPtr  = SegmentationSliceCachedRepresentation *;
  using SegmentationSliceCachedRepresentationSPtr = std::shared_ptr<SegmentationSliceCachedRepresentation>;


} /* namespace EspINA */

#endif /* SLICECACHEDREPRESENTATION_H_ */
