/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef SELECTABLEVIEW_H
#define SELECTABLEVIEW_H

#include "common/selection/SelectionHandler.h"
#include "common/model/Filter.h"

class pqRenderViewBase;

/// Interface for Views where user can select data
class SelectableView
{
public:
  SelectableView(){}// : m_VOIWidget(NULL){}

  /// Set a selection to all elements which belong to regions
  /// and pass the filtering criteria
//   virtual void setSelection(SelectionFilters &filters, ViewRegions &regions) = 0;
  /// Position in display coordinates
  virtual void addPreview(Filter *filter) = 0;
  virtual void removePreview(Filter *filter) = 0;
  virtual void previewExtent(int VOI[6]) = 0;
  virtual bool pickChannel(int x, int y, double pickPos[3]) = 0;
  virtual void eventPosition(int &x, int &y) = 0;
  virtual SelectionHandler::MultiSelection select(SelectionHandler::SelectionFilters filters, SelectionHandler::ViewRegions regions) = 0;

  virtual pqRenderViewBase *view() = 0;

// protected:
//   virtual void setVOI(IVOI *voi) = 0;
//
// protected:
//   pq3DWidget *m_VOIWidget;
//   IVOI *m_voi;
};

#endif // SELECTABLEVIEW_H
