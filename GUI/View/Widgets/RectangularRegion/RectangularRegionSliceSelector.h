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


#ifndef RECTANGULARREGIONSLICESELECTOR_H
#define RECTANGULARREGIONSLICESELECTOR_H

#include <GUI/Widgets/SliceSelectorWidget.h>

class QPushButton;

namespace EspINA
{
  class RectangularRegion;

  class EspinaGUI_EXPORT RectangularRegionSliceSelector
  : public SliceSelectorWidget
  {
    Q_OBJECT
  public:
    explicit RectangularRegionSliceSelector(RectangularRegion *region);
    virtual ~RectangularRegionSliceSelector();

    virtual void setPlane(const Plane plane);

    virtual QWidget *leftWidget () const;
    virtual QWidget *rightWidget() const;

    void setLeftLabel (const QString &label) {m_leftLabel  = label; update();}
    void setRightLabel(const QString &label) {m_rightLabel = label; update();}

    virtual SliceSelectorWidget *clone();

  protected slots:
    void update();
    void leftWidgetClicked();
    void rightWidgetClicked();

  private:
    RectangularRegion *m_region;

    QPushButton *m_leftWidget;
    QPushButton *m_rightWidget;

    QString m_leftLabel;
    QString m_rightLabel;
  };
}// namespace EspINA


#endif // RECTANGULARREGIONSLICESELECTOR_H
