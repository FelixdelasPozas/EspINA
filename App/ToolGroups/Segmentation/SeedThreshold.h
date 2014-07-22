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


#ifndef ESPINA_THRESHOLD_ACTION
#define ESPINA_THRESHOLD_ACTION

#include <QWidgetAction>

class QSpinBox;
class QLabel;

namespace ESPINA
{
  class SeedThreshold
  : public QWidgetAction
  {
    Q_OBJECT
  public:
    explicit SeedThreshold(QObject* parent = nullptr);

    virtual QWidget* createWidget(QWidget* parent);

    int lowerThreshold()
    {return m_threshold[0];}

    int upperThreshold()
    {return m_threshold[m_symmetrical?1:0];}

    void setSymmetricalThreshold(bool symmetrical);

    bool isSymmetrical() const
    {return m_symmetrical;}

  public slots:
    void setLowerThreshold(int th);
    void setUpperThreshold(int th);

  signals:
    void lowerThresholdChanged(int);
    void upperThresholdChanged(int);

  private:
    QLabel   *m_lthLabel, *m_uthLabel;
    QSpinBox *m_lth, *m_uth;

    int  m_threshold[2];
    bool m_symmetrical;
  };

} // namespace ESPINA

#endif // ESPINA_THRESHOLD_ACTION