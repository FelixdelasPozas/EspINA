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


#ifndef THRESHOLDACTION_H
#define THRESHOLDACTION_H

#include <qt4/QtGui/QWidgetAction>


class ThresholdAction
: public QWidgetAction
{
  Q_OBJECT

public:
  explicit ThresholdAction(QObject* parent);

  virtual QWidget* createWidget(QWidget* parent);

  int threshold() {return m_threshold;}

public slots:
  void setThreshold(int th) {m_threshold = th; emit thresholdChanged(th);}

signals:
  void thresholdChanged(int);

private:
  int m_threshold;
};

#endif // THRESHOLDACTION_H
