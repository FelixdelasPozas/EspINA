/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_DIALOGS_STACKINSPECTOR_SLICREPRESENTATION2D_H_
#define APP_DIALOGS_STACKINSPECTOR_SLICREPRESENTATION2D_H_

// ESPINA
#include <GUI/Representations/Managers/TemporalManager.h>

// VTK
#include <vtkSmartPointer.h>

class vtkTextActor;
class vtkImageActor;
class vtkImageMapToColors;
class vtkImageData;
class vtkLookupTable;
class vtkPoints;
class vtkFollower;
class vtkPolyData;
class vtkGlyph3DMapper;

namespace ESPINA
{
  namespace Extensions
  {
    class StackSLIC;
  }
  /** \class SLICRepresentation2D
   * \brief Representation prototype for SLIC extension data.
   *
   */
  class SLICRepresentation2D
  : public ESPINA::GUI::Representations::Managers::TemporalRepresentation2D
  {
      Q_OBJECT
    public:
      /** \brief SLICRepresentation2D class constructor with extra parameters.
       * \param[in] extension SLIC extension object.
       * \param[in] opacity Opacity value of the representation in [0,1]
       * \param[in] useColors True to use random coloring and false to use supervoxel colors.
       *
       */
      explicit SLICRepresentation2D(std::shared_ptr<Extensions::StackSLIC> extension, float opacity = 0.3, bool useColors = true);

      /** \brief SLICRepresentation2D class virtual destructor.
       *
       */
      virtual ~SLICRepresentation2D();

      virtual void initialize(RenderView *view);

      virtual void uninitialize();

      virtual void show();

      virtual void hide();

      virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

      virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

      virtual bool acceptSceneBoundsChange(const Bounds &bounds) const;

      virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const;

      virtual void display(const GUI::Representations::FrameCSPtr &frame);

      virtual void setPlane(Plane plane) {};

      virtual void setRepresentationDepth(Nm depth) {};

    public slots:
      /** \brief Modifies the text onscreen when the SLIC is being computed.
       * \param[in] value Current task progress.
       *
       */
      void setSLICComputationProgress(int value);

      /** \brief Modifies the text onscreen when the SLIC computation is aborted.
       *
       */
      void setSLICComputationAborted();

      /** \brief Changes overlaid actor opacity.
       * \param[in] value Opacity value in [0,100].
       *
       */
      void opacityChanged(int value);

      /** \brief Changes how the supervoxels are colored in the preview.
       * \param[in] value UI checkbox value.
       *
       */
      void colorModeCheckChanged(int value);

    private:
      virtual GUI::Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation();

      /** \brief Updates the actor for the given frame.
       * \param[in] frame Frame object with the next frame information.
       *
       */
      void updateActor(const GUI::Representations::FrameCSPtr frame);

      /** \brief Builds the actor pipeline.
       *
       */
      void buildVTKPipeline();

      /** \brief Helper method that returns a grayscale LUT.
       *
       */
      vtkSmartPointer<vtkLookupTable> grayscaleLUT() const;

      /** \brief Helper method that returns a color LUT.
       *
       */
      vtkSmartPointer<vtkLookupTable> hueRangeLUT() const;

      /** \brief Helper method that returns a color LUT where consecutive values have not
       * similar colors.
       *
       */
      vtkSmartPointer<vtkLookupTable> hueNonConsecutiveLUT() const;

      /** \brief Helper method that returns a LUT of random colors. Can contain duplicated colors.
       *
       */
      vtkSmartPointer<vtkLookupTable> randomLUT() const;

    private:
      vtkSmartPointer<vtkTextActor>          m_textActor;    /** text representation actor.                    */
      vtkSmartPointer<vtkImageActor>         m_actor;        /** slice representation actor.                   */
      vtkSmartPointer<vtkImageMapToColors>   m_mapper;       /** mapper used for slic slices                   */
      vtkSmartPointer<vtkImageData>          m_data;         /** image data holding slice information          */
      vtkSmartPointer<vtkPoints>             m_points;       /** supervoxel centers for the centers actor.     */
      vtkSmartPointer<vtkPolyData>           m_pointsData;   /** points polydata.                              */
      vtkSmartPointer<vtkGlyph3DMapper>      m_pointsMapper; /** points mapper.                                */
      vtkSmartPointer<vtkFollower>           m_pointsActor;  /** points actor.                                 */
      RenderView                            *m_view;         /** view where the representations will be shown. */
      bool                                   m_active;       /** true if visible and false otherwise.          */
      Nm                                     m_lastSlice;    /** position of the last slice rendered.          */
      int                                    m_planeIndex;   /** index of the view's plane.                    */
      std::shared_ptr<Extensions::StackSLIC> m_extension;    /** slic extension instance                       */
      float                                  m_opacity;      /** preview opacity                               */
      bool                                   m_useColors;    /** whether to use colors or grayscale values     */
  };
} // ESPINA

#endif // APP_DIALOGS_STACKINSPECTOR_SLICREPRESENTATION2D_H_
