/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_UTILS_REPRESENTATIONUTILS_H_
#define GUI_UTILS_REPRESENTATIONUTILS_H_

#include <Core/Utils/Spatial.h>
#include <GUI/EspinaGUI_Export.h>
#include <GUI/Representations/RepresentationState.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace RepresentationUtils
    {
      /** \brief Returns the plane value defined in the given state.
       * \param[in] state representation state object.
       *
       */
      Plane EspinaGUI_EXPORT plane(const RepresentationState &state);

      /** \brief Sets the plane in the given state.
       * \param[inout] state representation state object.
       * \param[in] plane orthogonal plane.
       *
       */
      void EspinaGUI_EXPORT setPlane(RepresentationState &state, const Plane plane);

      /** \brief Sets the plane for the given pool.
       * \param[inout] pool representation pool.
       * \param[in] plane orthogonal plane.
       *
       */
      void EspinaGUI_EXPORT setPlane(RepresentationPoolSPtr pool, const Plane plane);

      /** \brief Returns the depth value defined in the given state.
       * \param[in] state representation state object.
       *
       */
      Nm EspinaGUI_EXPORT segmentationDepth(const RepresentationState &state);

      /** \brief Sets the depth value in the given state.
       * \param[in] state representation state object.
       * \param[in] depth nm value.
       *
       */
      void EspinaGUI_EXPORT setSegmentationDepth(RepresentationState &state, const Nm depth);

      /** \brief Sets the depth value for the given pool.
       * \param[inout] pool representation pool.
       * \param[in] depth nm value.
       *
       */
      void EspinaGUI_EXPORT setSegmentationDepth(RepresentationPoolSPtr pool, const Nm depth);
    } // namespace RepresentationUtils
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_UTILS_REPRESENTATIONUTILS_H_
