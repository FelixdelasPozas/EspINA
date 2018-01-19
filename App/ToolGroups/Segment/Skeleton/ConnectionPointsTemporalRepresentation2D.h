/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_SEGMENT_SKELETON_CONNECTIONPOINTSTEMPORALREPRESENTATION2D_H_
#define APP_TOOLGROUPS_SEGMENT_SKELETON_CONNECTIONPOINTSTEMPORALREPRESENTATION2D_H_

// ESPINA
#include <GUI/Representations/Managers/TemporalManager.h>

class vtkGlyph3DMapper;
class vtkFollower;
class vtkPoints;
class vtkPolyData;
class vtkGlyphSource2D;

namespace ESPINA
{
  class RenderView;

  /** \class ConnectionPointsTemporalRepresentation2D
   * \brief Temporal representation for connection and enter/exit points.
   *
   */
  class ConnectionPointsTemporalRepresentation2D
  : public GUI::Representations::Managers::TemporalRepresentation2D
  {
      Q_OBJECT
    public:
      /** \brief ConnectionPointsTemporalRepresentation2D class constructor.
       *
       */
      explicit ConnectionPointsTemporalRepresentation2D();

      /** \brief ConnectionPointsTemporalRepresentation2D class virtual destructor.
       *
       */
      virtual ~ConnectionPointsTemporalRepresentation2D();

      /** \brief Sets the size of the point's representation.
       * \param[in] size Integer value [3-15].
       *
       */
      void setRepresentationSize(const int size);

      virtual void initialize(RenderView *view);

      virtual void uninitialize();

      virtual void show();

      virtual void hide();

      virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

      virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

      virtual bool acceptSceneBoundsChange(const Bounds &bounds) const;

      virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const;

      virtual void display(const GUI::Representations::FrameCSPtr &frame) override;

      virtual void setPlane(Plane plane) {};

      virtual void setRepresentationDepth(Nm depth) {};

    public slots:
      /** \brief Adds the point to the connection points list to represent.
       * \param[in] point 3D point coordinates.
       *
       */
      void onConnectionPointAdded(const NmVector3 point);

      /** \brief Adds the point to the connection points list to represent.
       * \param[in] point 3D point coordinates.
       *
       */
      void onConnectionPointRemoved(const NmVector3 point);

      /** \brief Empties the point lists.
       *
       */
      void clearPoints();

    private:
      virtual GUI::Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation();

      /** \brief Helper method to create the representation pipeline.
       *
       */
      void buildPipeline();

      /** \brief Updates the actor for the given slice if different from the current one.
       * \param[in] slice Slice position in Nm.
       *
       */
      void updateActor(const Nm slice);

      vtkSmartPointer<vtkPoints>        m_points;      /** connection points.                                             */
      vtkSmartPointer<vtkPolyData>      m_polyData;    /** connections polydata.                                          */
      vtkSmartPointer<vtkGlyph3DMapper> m_glyphMapper; /** connections glyph mapper.                                      */
      vtkSmartPointer<vtkGlyphSource2D> m_glyph2D;     /** connection glyph source for 2D views, a circle.                */
      vtkSmartPointer<vtkFollower>      m_actor;       /** connections representation actor.                              */
      int                               m_scale;       /** representation's scale value.                                  */
      RenderView                       *m_view;        /** view of the representation.                                    */
      int                               m_planeIndex;  /** views plane index.                                             */
      Nm                                m_lastSlice;   /** last represented slice value.                                  */
      bool                              m_active;      /** true if representation is active and visible, false otherwise. */
      QList<NmVector3>                  m_connections; /** connection points list.                                        */
  };

  using ConnectionPointsTemporalRepresentation2DSPtr = std::shared_ptr<ConnectionPointsTemporalRepresentation2D>;

} // namespace ESPINA

#endif // APP_TOOLGROUPS_SEGMENT_SKELETON_CONNECTIONPOINTSTEMPORALREPRESENTATION2D_H_
