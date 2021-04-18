/*

 Copyright (C) 2015 Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_GUI_VIEW_COORDINATE_SYSTEM_H
#define ESPINA_GUI_VIEW_COORDINATE_SYSTEM_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Utils/Bounds.h>
#include <Core/Utils/Vector3.hxx>

// C++
#include <memory>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      /** \class CoordinateSystem defines a 3D space defined for voxel of resolution
       *         size with its voxel (0,0,0) center is centered at origin position
       *
       */
      class EspinaGUI_EXPORT CoordinateSystem
      : public QObject
      {
          Q_OBJECT

        public:
          /** \brief CoordinateSystem class constructor.
           * \param[in] resolution resolution vector.
           *
           */
          explicit CoordinateSystem(const NmVector3 &resolution = NmVector3{1, 1, 1});

          /** \brief CoordinateSystem class constructor.
           * \param[in] resolution resolution vector.
           * \param[in] origin coordinates origin point.
           *
           */
          explicit CoordinateSystem(const NmVector3 &resolution,
                                    const NmVector3 &origin);


          /** \brief CoordinateSystem class constructor.
           * \param[in] resolution resolution vector.
           * \param[in] origin coordinates origin point.
           * \param[in] bounds total bounds of the coordinate system.
           *
           */
          explicit CoordinateSystem(const NmVector3 &resolution,
                                    const NmVector3 &origin,
                                    const Bounds    &bounds);

          /** \brief Sets the origin of the coordinate system
           *  \param[in] oring of the coordinate system
           */
          void setOrigin(const NmVector3 &origin);

          /** \brief Returns the origin of the coordinate system
           */
          NmVector3 origin() const;

          /** \brief Sets the reference voxel size of the coordinate system
           *  \param[in] resolution size in nm of each voxel in the three directions
           *
           *  This is mainly used to let widgets navigate the scene
           */
          void setResolution(const NmVector3 &resolution);

          /** \brief Returns the reference voxel size of the coordinate system
           *
           */
          NmVector3 resolution() const;

          /** \brief Updates the bounds of the coordinate system.
           * \param[in] bounds bounds object.
           *
           */
          void setBounds(const Bounds &bounds);

          /** \brief Returns the bounds of the coordinate system.
           *
           */
          Bounds bounds() const;

          /** \brief Returns the bottom value in Nm of the voxel in the given slice index and plane.
           * \param[in] sliceIndex integer slice index.
           * \param[in] plane orientation plane.
           *
           */
          Nm  voxelBottom(const int sliceIndex, const Plane plane) const;

          /** \brief Returns the bottom value in Nm of the voxel in the given Z position and plane.
           * \param[in] position Z position of the voxel.
           * \param[in] plane orientation plane.
           *
           */
          Nm  voxelBottom(const Nm position, const Plane plane) const;

          /** \brief Returns the center value in Nm of the voxel in the given slice index and plane.
           * \param[in] sliceIndex integer slice index.
           * \param[in] plane orientation plane.
           *
           */
          Nm  voxelCenter(const int sliceIndex, const Plane plane) const;

          /** \brief Returns the center value in Nm of the voxel in the given Z position and plane.
           * \param[in] position Z position of the voxel.
           * \param[in] plane orientation plane.
           *
           */
          Nm  voxelCenter(const Nm position, const Plane plane) const;

          /** \brief Returns the center value in Nm of the voxel with index x, y, z
           * \param[in] xIndex voxel's x coordinate
           * \param[in] yIndex voxel's y coordinate
           * \param[in] zIndex voxel's z coordinate
           *
           */
          NmVector3 voxelCenter(const int xIndex, const int yIndex, const int zIndex) const;

          /** \brief Returns the center value in Nm of the voxel at point
           * \param[in] point of a voxel
           *
           */
          NmVector3 voxelCenter(const NmVector3 &point) const;

          /** \brief Returns the top value in Nm of the voxel in the given slice index and plane.
           * \param[in] sliceIndex integer slice index.
           * \param[in] plane orientation plane.
           *
           */
          Nm  voxelTop(const int sliceIndex, const Plane plane) const;

          /** \brief Returns the top value in Nm of the voxel in the given Z position and plane.
           * \param[in] position Z position of the voxel.
           * \param[in] plane orientation plane.
           *
           */
          Nm  voxelTop(const Nm  position, const Plane plane) const;

          /** \brief Returns the numerical index of the slice given the slice position and plane.
           * \param[in] position slice position.
           * \param[in] plane orientation plane.
           *
           */
          int voxelSlice (const Nm position, const Plane plane) const;

        signals:
          void resolutionChanged(NmVector3 resolution);
          void boundsChanged(Bounds bounds);

        private:
          NmVector3 m_origin;     /** origin point.                   */
          NmVector3 m_resolution; /** resolution in each of the axis. */
          Bounds    m_bounds;     /** total bounds of the system.     */
      };

      using CoordinateSystemSPtr = std::shared_ptr<CoordinateSystem>;
    }
  }
}

#endif // ESPINA_GUI_VIEW_COORDINATESYSTEM_H
