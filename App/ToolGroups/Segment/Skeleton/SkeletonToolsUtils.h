/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_SEGMENT_SKELETON_SKELETONTOOLSUTILS_H_
#define APP_TOOLGROUPS_SEGMENT_SKELETON_SKELETONTOOLSUTILS_H_

// ESPINA
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <GUI/Model/CategoryAdapter.h>

namespace ESPINA
{
  namespace SkeletonToolsUtils
  {
    // shared between creation and modification skeleton tools.
    extern QMap<const QString, Core::SkeletonStrokes> STROKES;

    // icons for stroke types.
    static QStringList ICONS{":/espina/line.svg", ":/espina/dashed-line.svg"};

    /** \brief Returns the default strokes for the given category. Only Axon and Dendrite have strokes
     * other than the default one (named simply "Stroke").
     * \param[in] category category adapter object.
     *
     */
    Core::SkeletonStrokes defaultStrokes(const CategoryAdapterSPtr category);

    /** \brief Saves the strokes from the STROKES global variable to the settings object.
     * \param[in] settings shared settings object.
     *
     */
    void saveStrokes(std::shared_ptr<QSettings> settings);

    /** \brief Loads the strokes information in the settings object to the STROKES global variable.
     * \param[in] settings shared settings object.
     *
     */
    void loadStrokes(std::shared_ptr<QSettings> settings);
  }
}

#endif // APP_TOOLGROUPS_SEGMENT_SKELETON_SKELETONTOOLSUTILS_H_
