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


#ifndef ESPINA_DEFAULT_VOI_ACTION_H
#define ESPINA_DEFAULT_VOI_ACTION_H

#include <QWidgetAction>

#include <QLabel>
#include <QSpinBox>

namespace EspINA
{

  class ApplyROI
  : public QWidgetAction
  {
    Q_OBJECT

  public:
    explicit ApplyROI(QObject* parent=nullptr);

    virtual QWidget* createWidget(QWidget* parent);

    bool applyROI()
    {return m_useROI;}

  protected slots:
    void onValueChanged(bool val);

  signals:
    void useROI(bool);

  private:
    bool    m_useROI;

    QLabel   *m_labelROI[3];
    QSpinBox *m_spinBoxROI[3];
  };

} // namespace EspINA

#endif // ESPINA_DEFAULT_VOI_ACTION_H
