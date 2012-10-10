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


#ifndef SLICEVIEWSELECTIONCOLORENGINE_H
#define SLICEVIEWSELECTIONCOLORENGINE_H


#include "common/gui/SliceView.h"

#include "common/colorEngines/ColorEngine.h"

class SliceView::SelectionColorEngine
: public ColorEngine
{
public:
    virtual QColor color(const Segmentation* seg);
    virtual vtkSmartPointer< vtkLookupTable > lut(const Segmentation* seg);
};

#endif // SLICEVIEWSELECTIONCOLORENGINE_H
