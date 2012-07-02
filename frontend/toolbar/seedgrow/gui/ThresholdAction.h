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

#include <QWidgetAction>


class ThresholdAction
: public QWidgetAction
{
  Q_OBJECT

public:
  explicit ThresholdAction(QObject* parent);

  virtual QWidget* createWidget(QWidget* parent);

  int lowerThreshold() {return m_threshold[0];}
  int upperThreshold() {return m_threshold[1];}

public slots:
  void setLowerThreshold(int th);
  void setUpperThreshold(int th);

signals:
  void lowerThresholdChanged(int);
  void upperThresholdChanged(int);

private:
  int m_threshold[2];
};

#endif // THRESHOLDACTION_H