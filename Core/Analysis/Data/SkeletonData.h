/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SKELETON_DATA_H_
#define ESPINA_SKELETON_DATA_H_

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Data.h>
#include <Core/Utils/Bounds.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPolyData;

namespace ESPINA
{
  class EspinaCore_EXPORT SkeletonData
  : public Data
  {
    public:
      static const Data::Type TYPE;

    public:
      /** \brief SkeletonData class constructor.
       *
       */
      explicit SkeletonData();

      virtual Data::Type type() const
      { return TYPE; }

      virtual DataProxySPtr createProxy() const;

      Bounds bounds() const;

      /** \brief Returns the vtkPolyData smart pointer object.
       *
       */
      virtual vtkSmartPointer<vtkPolyData> skeleton() const = 0;
  };

  using SkeletonDataSPtr = std::shared_ptr<SkeletonData>;

  /** \brief Obtains and returns the SkeletonData smart pointer of the specified Output.
   * \param[in] output, Output object smart pointer.
   */
  SkeletonDataSPtr EspinaCore_EXPORT skeletonData(OutputSPtr output);

} // namespace ESPINA

#endif // ESPINA_SKELETON_DATA_H_
