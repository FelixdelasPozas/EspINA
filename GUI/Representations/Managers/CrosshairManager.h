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
  class CrosshairManager
  : public RepresentationManager
  {
    public:
      /** \brief Crosshair class protected constructor.
       *
       */
      explicit CrosshairManager();

      /** \brief CrosshairManager class virtual destructor.
       *
       */
      virtual ~CrosshairManager()
      {};

      virtual TimeRange readyRange() const;

      virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

    private:
      virtual void setCrosshair(const NmVector3 &crosshair, TimeStamp t);

      virtual void onSceneResolutionChanged(const NmVector3 &resolution, TimeStamp t);

      virtual void onSceneBoundsChanged(const Bounds &bounds, TimeStamp t);

      virtual void displayImplementation(TimeStamp t);

      virtual void onHide(TimeStamp t);

      virtual void onShow(TimeStamp t);

      virtual RepresentationManagerSPtr cloneImplementation();

      /** \brief Helper method to configure the 3D actors for the given crosshair.
       * \param[in] crosshair crosshair point.
       *
       */
      void configure3DActors(const NmVector3 &crosshair);

      /** \brief Helper method to configure the 2D actors for the given crosshair.
       * \param[in] crosshair crosshair point.
       *
       */
      void configure2DActors(const NmVector3 &crosshair);

      /** \brief Helper method to configure the actors for 2D views.
       * \param[in] index normal coordinate index of the actors.
       * \param[in] point1 inital point of the crosshair line.
       * \param[in] point2 final point of the crosshair line.
       *
       */
      void setPointInternal(int index, double *point1, double *point2);

    private:
      bool                               m_init;
      bool                               m_actorsInUse;
      NmVector3                          m_crosshair;
      vtkSmartPointer<vtkPoints>         m_points[3];
      vtkSmartPointer<vtkCellArray>      m_cells[3];
      vtkSmartPointer<vtkPolyData>       m_datas[3];
      vtkSmartPointer<vtkPolyDataMapper> m_mappers[3];
      vtkSmartPointer<vtkActor>          m_actors[3];
  };

} // namespace ESPINA

#endif // ESPINA_CROSSHAIR_MANAGER_H_
