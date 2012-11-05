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


#ifndef PLANARSPLITTOOL_H
#define PLANARSPLITTOOL_H

#include <common/tools/ITool.h>

class EspinaWidget;
class PlanarSplitTool
: public ITool
{
  Q_OBJECT
public:
  explicit PlanarSplitTool();

  virtual QCursor cursor() const;
  virtual bool filterEvent(QEvent* e, EspinaRenderView* view = 0);
  virtual bool enabled() const;
  virtual void setEnabled(bool value);
  virtual void setInUse(bool value);

signals:
  void splittingStopped();

private:
  bool m_inUse;
  bool m_enable;

  // TODO 2012-11-05: Use PlaneWidget
  EspinaWidget *m_widget;
};

#endif // PLANARSPLITTOOL_H
