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


#ifndef FILLEDCONTOUR_H
#define FILLEDCONTOUR_H

#include <tools/ITool.h>

class ContourSelector;

class FilledContour
: public ITool //NOTE Change to IVOI to use countour as VOI
{
public:
  explicit FilledContour();
  virtual ~FilledContour();

  virtual QCursor cursor() const;
  virtual bool filterEvent(QEvent* e, EspinaRenderView* view = 0);
  virtual void setEnabled(bool enable);
  virtual void setInteraction(bool enable);
  virtual bool interactive() const;

private:
  ContourSelector *m_picker;
};

#endif // FILLEDCONTOUR_H
