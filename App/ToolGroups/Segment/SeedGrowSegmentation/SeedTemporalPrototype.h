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

#ifndef APP_TOOLGROUPS_SEGMENT_SEEDGROWSEGMENTATION_SEEDTEMPORALPROTOTYPES_H_
#define APP_TOOLGROUPS_SEGMENT_SEEDGROWSEGMENTATION_SEEDTEMPORALPROTOTYPES_H_

// ESPINA
#include <Filters/SeedGrowSegmentationFilter.h>
#include <GUI/Representations/Managers/TemporalManager.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPoints;
class vtkPolyData;
class vtkGlyphSource2D;
class vtkFollower;
class vtkGlyph3DMapper;

namespace ESPINA
{
  class SeedGrowSegmentationFilter;
  class RenderView;

  /** \class SeedTemporalPrototype
   * \brief Manages the representation of the SGS seed on the 2D views.
   *
   */
  class SeedTemporalRepresentation
  : public GUI::Representations::Managers::TemporalRepresentation2D
  {
      Q_OBJECT
    public:
      /** \brief SeedTemporalRepresentation class constructor.
       *
       */
      explicit SeedTemporalRepresentation(SeedGrowSegmentationFilter *filter);

      /** \brief SeedTemporalRepresentation class virtual destructor.
       *
       */
      virtual ~SeedTemporalRepresentation();

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

    private:
      virtual GUI::Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation();

      /** \brief Updates the actor for the given frame.
       * \param[in] frame Frame object with the next frame information.
       *
       */
      void updateActor(const GUI::Representations::FrameCSPtr frame);

      /** \brief Builds the actor pipeline using the information from the given frame.
       *
       */
      void buildVTKPipeline();

    private:
      vtkSmartPointer<vtkPoints>        m_points;      /** connection points.                            */
      vtkSmartPointer<vtkPolyData>      m_polyData;    /** polydata.                                     */
      vtkSmartPointer<vtkGlyphSource2D> m_glyph2D;     /** glyph source for 2D views, a circle.          */
      vtkSmartPointer<vtkGlyph3DMapper> m_glyphMapper; /** glyph mapper.                                 */
      vtkSmartPointer<vtkFollower>      m_actor;       /** representation actor.                         */
      NmVector3                         m_seed;        /** filter's seed point.                          */
      RenderView                       *m_view;        /** view where the representations will be shown. */
      SeedGrowSegmentationFilter       *m_filter;      /** filter object.                                */
      int                               m_planeIndex;  /** view's plane index.                           */
      Nm                                m_lastSlice;   /** position of the last slice rendered.          */
      bool                              m_active;      /** true if visible and false otherwise.          */
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_SEGMENT_SEEDGROWSEGMENTATION_SEEDTEMPORALPROTOTYPES_H_
