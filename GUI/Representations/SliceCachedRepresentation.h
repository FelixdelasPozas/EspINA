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

#include "CachedRepresentation.h"
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/EspinaTypes.h>
#include <GUI/View/View2D.h>

namespace EspINA
{
  class CachedRepresentationTask;
  class TransparencySelectionHighlighter;

  //-----------------------------------------------------------------------------
  class EspinaGUI_EXPORT ChannelSliceCachedRepresentation
  : public CachedRepresentation
  {
    public:
      static const Representation::Type TYPE;

    public:
      /* \brief Default class constructor.
       *
       */
      ChannelSliceCachedRepresentation(DefaultVolumetricDataSPtr data,
                                       View2D *view,
                                       SchedulerSPtr scheduler);

      /* \brief Class virtual destructor.
       *
       */
      virtual ~ChannelSliceCachedRepresentation() {};

      /* \brief Returns this representation settings widget.
       *
       */
      virtual RepresentationSettings *settingsWidget();

      /* \brief Sets the color of the channel.
       *
       * Sets the color of the channel, specified as a QColor.
       */
      virtual void setColor(const QColor &color);

      /* \brief Sets the brightness of the channel representation.
       *
       * Sets the brightness of the channel representation. Brightness value belongs to [-1,1].
       */
      virtual void setBrightness(double value);

      /* \brief Sets the channel representation constrat value.
       *
       * Sets the channel representation constrat value. Contrast value belongs to [0,2].
       */
      virtual void setContrast(double value);

      /* brief Sets the channel representation opacity.
       *
       * Sets the channel representation opacity. Opacity value belongs to [0,1].
       */
      virtual void setOpacity(double value);

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

      /* \brief Returns true if the representation contains the actor.
       *
       */
      virtual bool hasActor(vtkProp *actor) const;

      /* \brief Method that triggers the update of the actors of the representation.
       *
       */
      virtual void updateRepresentation();

      /* \brief Returns the actors that comprise this representation.
       *
       */
      virtual QList<vtkProp*> getActors();

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
      { return true; }

      /* \brief Returns true if the representations needs to update at the moment of calling
       * this method.
       *
       */
      virtual bool needUpdate();

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

      /* \brief Updates the representation visibility.
       *
       */
      virtual void updateVisibility(bool visible);

      /* \brief Returns a CachedRepresentationTask for the position and with the specified priority.
       *
       * Returns a CachedRepresentationTask for the position and with the specified priority. Is the
       * responsibility of this method to see if the position is valid for the data that it holds.
       * If not, it must return a nullptr.
       */
      CachedRepresentationTaskSPtr createTask(int position, Priority priority = Priority::NORMAL);

      /* \brief Sets the view this representation will be renderer on.
       *
       * Sets the view this representation will be renderer on. The temporary actor (or symbolic as it
       * extends the bounds of the data in that view) is computed here.
       */
      void setView(View2D *view);

    private:
      DefaultVolumetricDataSPtr m_data;         // data that will be represented.
      SchedulerSPtr             m_scheduler;    // scheduler for the CachedRepresentationTasks
      int                       m_planeIndex;   // plane index for the view.
      Nm                        m_planeSpacing; // spacing value in the movement of the view.
      Nm                        m_min;          // minimal value in Nm of the bounds of movement for the data for this view.
      Nm                        m_max;          // maximal value in Nm of the bounds of movement for the data for this view.
    };

  using ChannelSliceCachedRepresentationPtr  = ChannelSliceCachedRepresentation *;
  using ChannelSliceCachedRepresentationSPtr = std::shared_ptr<ChannelSliceCachedRepresentation>;

  //----------------------------------------------------------------------------- TODO
  class EspinaGUI_EXPORT SegmentationSliceCachedRepresentation
  : public CachedRepresentation
  {
    public:
      static const Representation::Type TYPE;
      static TransparencySelectionHighlighter *s_highlighter;

    public:
      /* \brief Default class constructor.
       *
       */
      explicit SegmentationSliceCachedRepresentation(DefaultVolumetricDataSPtr data,
                                                     View2D *view,
                                                     SchedulerSPtr scheduler);

      /* \brief Class virtual destructor.
       *
       */
      virtual ~SegmentationSliceCachedRepresentation() {};

      /* \brief Returns this representation settings widget.
       *
       */
      virtual RepresentationSettings *settingsWidget();

      /* \brief Returns a serialization of this representation's settings.
       *
       */
      virtual QString serializeSettings();

      /* \brief Restores the settings of the representation with the specified one.
       *
       */
      virtual void restoreSettings(QString settings);

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

      /* \brief Returns true if the representation contains the actor.
       *
       */
      virtual bool hasActor(vtkProp *actor) const;

      /* \brief Method that triggers the update of the actors of the representation.
       *
       */
      virtual void updateRepresentation();

      /* \brief Returns the actors that comprise this representation.
       *
       */
      virtual QList<vtkProp*> getActors();

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
      { return true; }

      /* \brief Returns true if the representations needs to update at the moment of calling
       * this method.
       *
       */
      virtual bool needUpdate();

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

      /* \brief Updates the representation visibility.
       *
       */
      virtual void updateVisibility(bool visible);

      /* \brief Returns a CachedRepresentationTask for the position and with the specified priority.
       *
       * Returns a CachedRepresentationTask for the position and with the specified priority. Is the
       * responsibility of this method to see if the position is valid for the data that it holds.
       * If not, it must return a nullptr.
       */
      CachedRepresentationTaskSPtr createTask(int position, Priority priority = Priority::NORMAL);

      /* \brief Sets the view this representation will be renderer on.
       *
       * Sets the view this representation will be renderer on. The temporary actor (or symbolic as it
       * extends the bounds of the data in that view) is computed here.
       */
      void setView(View2D *view);

    private:
      DefaultVolumetricDataSPtr m_data;         // data that will be represented.
      SchedulerSPtr             m_scheduler;    // scheduler for the CachedRepresentationTasks
      int                       m_planeIndex;   // plane index for the view.
      Nm                        m_planeSpacing; // spacing value in the movement of the view.
      Nm                        m_min;          // minimal value in Nm of the bounds of movement for the data for this view.
      Nm                        m_max;          // maximal value in Nm of the bounds of movement for the data for this view.
  };

  using SegmentationSliceCachedRepresentationPtr  = SegmentationSliceCachedRepresentation *;
  using SegmentationSliceCachedRepresentationSPtr = std::shared_ptr<SegmentationSliceCachedRepresentation>;


} /* namespace EspINA */

#endif /* SLICECACHEDREPRESENTATION_H_ */
