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

#ifndef ESPINA_MEASURES_TOOLS_H_
#define ESPINA_MEASURES_TOOLS_H_

// ESPINA
#include <Tools/Measure/MeasureTool.h>
#include <Tools/Ruler/RulerTool.h>
#include <ToolGroups/ToolGroup.h>

namespace ESPINA
{

  class AnalyzeToolGroup
  : public ToolGroup
  {
    Q_OBJECT
    public:
      /** \brief MeasuresTools class constructor.
       * \param[in] viewManager, view manager smart pointer.
       * \param[in] parent, QWidget raw pointer of the parent of this object.
       *
       */
      explicit AnalyzeToolGroup(ViewManagerSPtr viewManager, QWidget* parent = nullptr);

      /** \brief MeasuresTools class destructor.
       *
       */
      virtual ~AnalyzeToolGroup();
  };

} /* namespace ESPINA */

#endif /* MEASURESTOOLS_H_ */
