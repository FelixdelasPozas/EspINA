/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINA_CUSTOM_ROI_ACTION_H
#define ESPINA_CUSTOM_ROI_ACTION_H

#include <QWidgetAction>

#include <QLabel>
#include <QSpinBox>
#include <Core/Utils/Spatial.h>

namespace ESPINA
{

  class CustomROIWidget
  : public QWidgetAction
  {
    Q_OBJECT

  public:
    explicit CustomROIWidget(QObject* parent=nullptr);

    virtual QWidget* createWidget(QWidget* parent);

    virtual void deleteWidget(QWidget* widget);

    bool applyROI()
    {return m_useROI;}

    void setValue(Axis axis, unsigned int value);

    unsigned int value(Axis axis) const
    { return m_values[idx(axis)]; }

  private slots:
    void onApplyROIChanged(bool val);
    void onXSizeChanged(int value);
    void onYSizeChanged(int value);
    void onZSizeChanged(int value);

  signals:
    void useROI(bool);

  private:
    bool    m_useROI;

    unsigned int m_values    [3];
    QLabel      *m_labelROI  [3];
    QSpinBox    *m_spinBoxROI[3];
  };

} // namespace ESPINA

#endif // ESPINA_CUSTOM_ROI_ACTION_H
