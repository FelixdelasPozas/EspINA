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

#ifndef ESPINA_VTKPOLYDATAUTILS_H_
#define ESPINA_VTKPOLYDATAUTILS_H_

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Types.h>
#include <Core/Utils/BinaryMask.hxx>
#include <Core/Utils/Vector3.hxx>
#include <vtkSmartPointer.h>

class vtkPolyData;
class QByteArray;
class QString;

namespace ESPINA
{
  namespace PolyDataUtils
  {
    /** \brief Converts the vtkPolyData object to a byte array and returns it.
     * \param[in] polyData smart pointer of the vtkPolyData object to convert.
     *
     */
    QByteArray EspinaCore_EXPORT savePolyDataToBuffer(const vtkSmartPointer<vtkPolyData> polydata);

    /** \brief Converts a byte array to a vtkPolyData smart pointer and returns it.
     * \param[in] filename file name.
     *
     */
    vtkSmartPointer<vtkPolyData> EspinaCore_EXPORT readPolyDataFromFile(QString fileName);

    /** \brief Scales the polydata given the ration in each coordinate.
     * \param[inout] polydata polydata smart pointer.
     * \param[in] ratio NmVector3 of scaling ratio in each coordinate.
     *
     */
    void EspinaCore_EXPORT scalePolyData(vtkSmartPointer<vtkPolyData> polyData, const NmVector3 &ratio);

    /** \brief Rasterizes the polyData to a vtkImageData.
     * \param[in] polyData vtkPolyData raw pointer to rasterize to a volume.
     * \param[in] plane orientation of the contour.
     * \param[in] slice position of the contour.
     * \param[in] spacing spacing of the volume.
     *
     */
    vtkSmartPointer<vtkImageData> EspinaCore_EXPORT rasterizeContourToVTKImage(vtkPolyData *contour, const Plane plane, const Nm slice, const NmVector3 &spacing);

    /** \brief Rasterizes the polyData to a BinaryMask object.
     * \param[in] polyData vtkPolyData raw pointer to rasterize to a volume.
     * \param[in] plane orientation of the contour.
     * \param[in] slice position of the contour.
     * \param[in] spacing spacing of the volume.
     *
     */
    BinaryMaskSPtr<unsigned char> EspinaCore_EXPORT rasterizeContourToMask(vtkPolyData *contour, const Plane plane, const Nm slice, const NmVector3 &spacing);
  }
}

#endif // ESPINA_VTKPOLYDATAUTILS_H_
