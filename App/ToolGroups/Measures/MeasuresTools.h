/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

#include <Support/ToolGroup.h>
#include <Tools/Measure/MeasureTool.h>
#include <Tools/Ruler/RulerTool.h>

namespace EspINA
{
  
  class MeasuresTools
  : public ToolGroup
  {
    Q_OBJECT
    public:
      /* \brief MeasuresTools class constructor.
       *
       */
      explicit MeasuresTools(ViewManagerSPtr viewManager, QWidget* parent = nullptr);

      /* \brief MeasuresTools class destructor.
       *
       */
      virtual ~MeasuresTools();

      /* \brief Implements ToolGroup::setEnabled.
       *
       */
      virtual void setEnabled(bool value);

      /* \brief Implements ToolGroup::enabled.
       *
       */
      virtual bool enabled() const;

      /* \brief Implements ToolGroup::tools.
       *
       */
      virtual ToolSList tools();

    private:
      MeasureToolSPtr m_measure;
      RulerToolSPtr   m_ruler;
      bool            m_enabled;
  };

} /* namespace EspINA */

#endif /* MEASURESTOOLS_H_ */
