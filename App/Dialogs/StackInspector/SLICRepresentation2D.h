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

namespace ESPINA
{
  namespace Extensions {
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
      /** \brief SLICRepresentation2D class constructor.
       *
       */
      explicit SLICRepresentation2D(std::shared_ptr<Extensions::StackSLIC> extension);

      /** \brief SLICRepresentation2D class virtual destructor.
       *
       */
      virtual ~SLICRepresentation2D();

      /** \brief Sets the SLIC data to use.
       *
       * TODO
       *
       */
      void setSLICExtension(std::shared_ptr<Extensions::StackSLIC> extension);

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

    private:
      vtkSmartPointer<vtkTextActor>         m_textActor;  /** text representation actor.                    */
      vtkSmartPointer<vtkImageActor>        m_actor;      /** slice representation actor.                   */
      vtkSmartPointer<vtkImageMapToColors>  m_mapper;     /** mapper used for slic slices                   */
      vtkSmartPointer<vtkImageData>         m_data;       /** image data holding slice information          */
      RenderView                           *m_view;       /** view where the representations will be shown. */
      bool                                  m_active;     /** true if visible and false otherwise.          */
      Nm                                    m_lastSlice;  /** position of the last slice rendered.          */
      int                                   m_planeIndex; /** index of the view's plane.                    */
      std::shared_ptr<Extensions::StackSLIC>m_extension;  /** slic extension instance                       */
  };
} // ESPINA

#endif // APP_DIALOGS_STACKINSPECTOR_SLICREPRESENTATION2D_H_
