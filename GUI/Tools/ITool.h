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


#ifndef ITOOL_H
#define ITOOL_H

#include <QObject>

#include <QCursor>
#include <boost/shared_ptr.hpp>

class QEvent;

namespace EspINA
{
  class EspinaRenderView;

  class ITool
  : public QObject
  {
  public:
    virtual QCursor cursor() const = 0;
    virtual bool filterEvent(QEvent *e, EspinaRenderView *view=NULL) = 0;
    virtual void setInUse(bool value) = 0;
    virtual void setEnabled(bool value) = 0;
    virtual bool enabled() const = 0;
    virtual void lostEvent(EspinaRenderView*) {};
  };

  typedef boost::shared_ptr<ITool> IToolSPtr;
} // namespace EspINA

#endif // ITOOL_H
