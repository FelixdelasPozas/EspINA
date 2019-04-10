/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_CROSSHAIR_MANAGER_H_
#define ESPINA_CROSSHAIR_MANAGER_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationManager.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPoints;
class vtkPolyData;
class vtkCellArray;
class vtkPolyDataMapper;
class vtkActor;

namespace ESPINA
{
  namespace GUI
  {
    namespace Representations
    {
      namespace Managers
      {
        /** \class CrosshairManager
         * \brief Representation manager for the crosshair representations.
         *
         */
        class EspinaGUI_EXPORT CrosshairManager
        : public RepresentationManager
        {
        public:
          /** \brief Crosshair class protected constructor.
           *
           */
          explicit CrosshairManager(ViewTypeFlags supportedViews);

          /** \brief CrosshairManager class virtual destructor.
           *
           */
          virtual ~CrosshairManager()
          {};

          /** \brief Enables/Disables the plane intersection lines in 3D views.
           * \param[in] value True to show intersections, false otherwise.
           *
           */
          void setShowIntersections(const bool value);

          virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

        private:
          virtual bool hasRepresentations() const override;

          virtual void updateFrameRepresentations(const FrameCSPtr frame) override;

          virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override;

          virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override;

          virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override;

          virtual bool acceptInvalidationFrame(const FrameCSPtr frame) const override;

          virtual void displayRepresentations(const FrameCSPtr frame) override;

          virtual void hideRepresentations(const FrameCSPtr frame) override;

          virtual void onHide(const FrameCSPtr frame) override;

          virtual void onShow(const FrameCSPtr frame) override;

          virtual RepresentationManagerSPtr cloneImplementation() override;

          /** \brief Configure the 2D actors for the given crosshair.
           * \param[in] crosshair crosshair point.
           *
           */
          void configure2DActors(const NmVector3 &crosshair);

          /** \brief Configure the 3D actors for the given crosshair.
           * \param[in] crosshair crosshair point.
           *
           */
          void configure3DActors(const NmVector3 &crosshair);

          /** \brief Configure the actors for 2D views.
           * \param[in] index normal coordinate index of the actors.
           * \param[in] point1 inital point of the crosshair line.
           * \param[in] point2 final point of the crosshair line.
           *
           */
          void setPointInternal(int index, double *point1, double *point2);

          /** \brief Updates the actors with the crosshair associated to t.
           *
           */
          void updateCrosshairs(const FrameCSPtr frame);

        private:
          vtkSmartPointer<vtkPoints>         m_points[3];          /** actor points.                                            */
          vtkSmartPointer<vtkCellArray>      m_cells[3];           /** actor cells.                                             */
          vtkSmartPointer<vtkPolyData>       m_datas[3];           /** actor polydatas.                                         */
          vtkSmartPointer<vtkPolyDataMapper> m_mappers[3];         /** actor mappers.                                           */
          vtkSmartPointer<vtkActor>          m_actors[3];          /** actors.                                                  */
          vtkSmartPointer<vtkPolyData>       m_intersectionData;   /** intersection actor polydatas.                            */
          vtkSmartPointer<vtkPolyDataMapper> m_intersectionMapper; /** intersection actor mappers.                              */
          vtkSmartPointer<vtkActor>          m_intersectionActor;  /** intersection actors.                                     */
          bool                               m_showIntersections;  /** true to show plane intersections in 3d, false otherwise. */
        };
      }
    }
  }
} // namespace ESPINA

#endif // ESPINA_CROSSHAIR_MANAGER_H_
