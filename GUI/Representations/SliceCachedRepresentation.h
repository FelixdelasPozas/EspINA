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

// ESPINA
#include "Representation.h"
#include "RepresentationEmptySettings.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/EspinaTypes.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkSmartPointer.h>

class vtkImageActor;

namespace ESPINA
{
  class TransparencySelectionHighlighter;

  class EspinaGUI_EXPORT CachedRepresentation
  : public Representation
  {
    public:
  		/** \brief CachedRepresentation class constructor.
  		 * \param[in] data, VolumetricData smart pointer of the data to represent.
  		 * \param[in] view, View2D pointer of the view this representation will be shown.
  		 *
  		 */
      explicit CachedRepresentation(DefaultVolumetricDataSPtr data,
                                    View2D *view);

      /** \brief CachedRepresentation class virtual destructor.
       *
       */
      virtual ~CachedRepresentation()
      {};

      /** \brief Implements Representation::crosshairDependent() const.
       *
       */
      virtual bool crosshairDependent() const
      { return false; }

      /** \brief Returns the actor of the channel in the specified slice position.
       * \param[in] pos, channel position.
       *
       */
      virtual vtkSmartPointer<vtkImageActor> getActor(const Nm pos) const = 0;

      /** \brief Implements Representation::settingsWidget.
       *
       */
      virtual RepresentationSettings* settingsWidget()
      { return new RepresentationEmptySettings(); }

      /** \brief Implements Representation::getActors.
       *
       */
      virtual QList<vtkProp*> getActors()
      { return QList<vtkProp *>(); }

      /** \brief Implements Representation::hasActor.
       *
       */
      virtual bool hasActor(vtkProp*) const
      { return false; }

      /** \brief Returns the plane this representation is on.
       *
       * Returns the plane this representation is on. To get the correct value the
       * method SetView() must have been called first.
       */
      Plane plane()
      { return toPlane(m_planeIndex); }

      /** \brief Implements Representation::canRenderOnView() const.
       *
       */
      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_SLICE; }

      /** \brief Returns true if the representation can generate an actor in that position.
       * \param[in] pos, actor position.
       *
       */
      bool existsIn(const Nm position) const;

      /** \brief Returns the value of the last modification time of the m_data
       * when the last actor was created.
       */
      TimeStamp getModificationTime()
      { return m_lastUpdatedTime; }

      /** \brief Implements Representation::needUpdate();
       *
       */
      virtual bool needUpdate() const
      { return (m_data->lastModified() != m_lastUpdatedTime); }

    protected:
      /** \brief Compute the values of m_min && m_max for the representation.
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
      /** \brief ChannelSliceCachedRepresentation class constructor.
  		 * \param[in] data, VolumetricData smart pointer of the data to represent.
  		 * \param[in] view, View2D pointer of the view this representation will be shown.
       *
       */
      ChannelSliceCachedRepresentation(DefaultVolumetricDataSPtr data,
                                       View2D *view);

      /** \brief ChannelSliceCachedRepresentation class virtual destructor.
       *
       */
      virtual ~ChannelSliceCachedRepresentation()
      {};

      /** \brief Implements Representation::isInside().
       *
       * Returns true if the specified point is inside the channel. Right now only checks if it's
       * inside the bounds, there is no detection of non aligned border channels.
       */
      virtual bool isInside(const NmVector3 &point) const;

      /** \brief Implements Representation::updateRepresentation().
       *
       * Empty as this is not needed for this representation.
       *
       */
      virtual void updateRepresentation();

      /** \brief Implements CachedRepresentation::getActor().
       *
       */
      vtkSmartPointer<vtkImageActor> getActor(const Nm pos) const;

      /** \brief Implements Representation::updateVisibility().
       *
       */
      virtual void updateVisibility(bool unused);

      /** \brief Overrides Representation::setContrast().
       *
       */
      virtual void setContrast(double contrast) override;

      /** \brief Overrides Representation::setBrightness().
       *
       */
      virtual void setBrightness(double brightness) override;


      /** \brief Overrides Representation::setOpacity().
       *
       */
      virtual void setOpacity(double opacity) override;

      /** \brief Overrides Representation::setColor().
       *
       */
      virtual void setColor(const QColor &color) override;

    signals:
      void update();
      void changeVisibility();
      void changeOpacity();
      void changeContrastAndBrightness();
      void changeColor();
      void changeSaturation();

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

      /** \brief Sets the view this representation will be renderer on.
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
      /** \brief SegmentationSliceCachedRepresentation class constructor.
  		 * \param[in] data, VolumetricData smart pointer of the data to represent.
  		 * \param[in] view, View2D pointer of the view this representation will be shown.
       *
       */
      explicit SegmentationSliceCachedRepresentation(DefaultVolumetricDataSPtr data,
                                                     View2D *view);

      /** \brief SegmentationSliceCachedRepresentation virtual destructor.
       *
       */
      virtual ~SegmentationSliceCachedRepresentation()
      {};

      /** \brief Overrides Representation::serializeSettings().
       *
       */
      virtual QString serializeSettings() override;

      /** \brief Overrides Representation::serializeSettings().
       *
       */
      virtual void restoreSettings(QString settings) override;

      /** \brief Overrides Representation::setBrightness().
       *
       * Emtpy, no use in segmentations.
       *
       */
      virtual void setBrightness(double value) override
      {};

      /** \brief Overrides Representation::setContrast().
       *
       * Emtpy, no use in segmentations.
       *
       */
      virtual void setContrast(double value) override
      {};

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

      /** \brief Overrides Representation::updateRepresentation().
       *
       * Empty as this is not needed in this kind of representation.
       *
       */
      virtual void updateRepresentation();

      /** \brief Implements Representation::updateVisibility().
       *
       * \param[in] unused This value is ignored.
       *
       */
      virtual void updateVisibility(bool unused);

      /** \brief Implements CachedRepresentation::getActor() const.
       *
       */
      vtkSmartPointer<vtkImageActor> getActor(const Nm pos) const;

      /** \brief Sets the view this representation will be renderer on.
       * \param[in] view, View2D raw pointer.
       *
       */
      void setView(View2D *view);

    signals:
      void update();
      void changeColor();
      void changeVisibility();

    protected slots:
    	/** \brief Updates limits values when the data changes.
    	 *
    	 */
      void dataChanged();

    protected:
			/** \brief Implements Representation::cloneImplementation(View2D*).
			 *
			 */
      virtual RepresentationSPtr cloneImplementation(View2D *view);

			/** \brief Implements Representation::cloneImplementation(View2D*).
			 *
			 */
      virtual RepresentationSPtr cloneImplementation(View3D *view)
      { return RepresentationSPtr(); }

    private:
      Nm m_shift; // depth of the actor for this view
  };

  using SegmentationSliceCachedRepresentationPtr  = SegmentationSliceCachedRepresentation *;
  using SegmentationSliceCachedRepresentationSPtr = std::shared_ptr<SegmentationSliceCachedRepresentation>;


} /* namespace ESPINA */

#endif /* SLICECACHEDREPRESENTATION_H_ */
