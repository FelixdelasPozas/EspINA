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

#ifndef GUI_REPRESENTATIONS_SETTINGS_SEGMENTATIONSKELETONPOOLSETTINGS_H_
#define GUI_REPRESENTATIONS_SETTINGS_SEGMENTATIONSKELETONPOOLSETTINGS_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationPool.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace Representations
    {
      /** \class SegmentationSkeletonPoolSettins
       * \brief Implements the settings for the Skeleton representation, to be used by pools.
       *
       */
      class EspinaGUI_EXPORT SegmentationSkeletonPoolSettings
      : public PoolSettings
      {
        public:
          /** \brief SegmentationSkeletonPoolSettings class constructor.
           *
           */
          SegmentationSkeletonPoolSettings();

          /** \brief SegmentationSkeletonPoolSettings class virtual destructor.
           *
           */
          virtual ~SegmentationSkeletonPoolSettings()
          {};

          /** \brief Sets the skeleton representation opacity value.
           * \param[in] opacity value in [0,1]
           *
           */
          void setOpacity(const double opacity);

          /** \brief Returns the opacity value.
           *
           */
          double opacity() const;

          /** \brief Sets the skeleton representation width value.
           * \param[in] width integer value in [1-max].
           *
           */
          void setWidth(const int width);

          /** \brief Returns the width value.
           *
           */
          int width() const;

          /** \brief Enables/disables the showing of node annotations in the representation.
           * \param[in] value true to enable and false otherwise.
           *
           */
          void setShowAnnotations(bool value);

          /** \brief Returns the value of the node annotations visibility setting.
           *
           */
          bool showAnnotations() const;

          /** \brief Sets the size of the annotations' text.
           * \param[in] size Font size.
           *
           */
          void setAnnotationsSize(int size);

          /** \brief Returns the annotations size value.
           *
           */
          int annotationsSize() const;

          /** \brief Helper method to get the skeleton width value from a representation state object.
           * \param[in] state representation state object.
           *
           */
          static int getWidth(const RepresentationState &state);

          /** \brief Helper method to get the skeleton opacity value from a representation state object.
           * \param[in] state representation state object.
           *
           */
          static double getOpacity(const RepresentationState &state);

          /** \brief Helper method to get the show annotations value from a representation state object.
           * \param[in] state representation state object.
           *
           */
          static bool getShowAnnotations(const RepresentationState &state);

          /** \brief Helper method to get the annotations' text from a representation state object.
           * \param[in] state representation state object.
           *
           */
          static int getAnnotationsSize(const RepresentationState &state);

        private:
          static const QString WIDTH;   /** width setting identifier                   */
          static const QString SHOWIDS; /** annotations visibility setting identifier. */
          static const QString OPACITY; /** opacity setting identifier.                */
          static const QString IDSSIZE; /** annotations' text size.                    */
      };

    } // namespace Representations
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_REPRESENTATIONS_SETTINGS_SEGMENTATIONSKELETONPOOLSETTINGS_H_
