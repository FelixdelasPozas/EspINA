/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef ESPINA_CIRCULAR_BRUSH_SELECTOR_H
#define ESPINA_CIRCULAR_BRUSH_SELECTOR_H

#include <GUI/Selectors/BrushSelector.h>

namespace EspINA
{

  class CircularBrushSelector
  : public BrushSelector
  {
    Q_OBJECT
    public:
      explicit CircularBrushSelector();
      virtual ~CircularBrushSelector();

    protected slots:
      virtual BrushSelector::BrushShape createBrushShape(ViewItemAdapterPtr item,
                                                         NmVector3 center,
                                                         Nm radius,
                                                         Plane plane);

  };

  using CircularBrushSelectorSPtr = std::shared_ptr<CircularBrushSelector>;
}

#endif // ESPINA_CIRCULAR_BRUSH_SELECTOR_H
