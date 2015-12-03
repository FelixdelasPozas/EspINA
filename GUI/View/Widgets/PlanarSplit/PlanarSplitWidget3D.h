/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_PLANAR_SPLIT_WIDGET3D_H_
#define ESPINA_PLANAR_SPLIT_WIDGET3D_H_

// ESPINA
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPlane;
class vtkPoints;

using namespace ESPINA::GUI::Representations::Managers;

namespace ESPINA
{
  class RenderView;

  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        class PlanarSplitEventHandler;

        class PlanarSplitWidget3D
        : public PlanarSplitWidget
        , public EspinaWidget3D
        {
          public:
            /** \brief PlanarSplitWidget3D class constructor.
             * \param[in] handler PlanarSplitEventHandler class raw pointer.
             *
             */
            PlanarSplitWidget3D(PlanarSplitEventHandler *tool);

            /** \brief PlanarSplitWidget3D class virtual destructor.
             *
             */
            virtual ~PlanarSplitWidget3D();

            virtual TemporalRepresentation3DSPtr clone() override;

            virtual void disableWidget() override;

          protected:
            virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override;

            virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override;

            virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override;

            virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const;

            virtual void initializeImplementation(RenderView *view) override;

            virtual void uninitializeImplementation() override;

            virtual vtkAbstractWidget *vtkWidget() override;

            virtual void setPlanePoints(vtkSmartPointer<vtkPoints> points) override;

            virtual vtkSmartPointer<vtkPoints> getPlanePoints() const override;

            virtual vtkSmartPointer<vtkPlane> getImplicitPlane(const NmVector3 &spacing) const override;

            virtual void setSegmentationBounds(const Bounds &bounds) override;

            virtual bool planeIsValid() const override;

          private:
            vtkSmartPointer<vtkImplicitPlaneWidget2> m_widget;
            vtkSmartPointer<vtkSplitCommand>         m_command;
            RenderView                              *m_view;
        };

        using PlanarSplitWidget3DPtr  = PlanarSplitWidget3D *;
        using PlanarSplitWidget3DSPtr = std::shared_ptr<PlanarSplitWidget3D>;
      
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_PLANAR_SPLIT_WIDGET3D_H_
