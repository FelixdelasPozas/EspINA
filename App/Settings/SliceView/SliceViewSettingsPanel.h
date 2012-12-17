/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <email>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SLICEVIEWSETTINGSPANEL_H
#define SLICEVIEWSETTINGSPANEL_H

#include "ui_SliceViewSettingsPanel.h"
#include <GUI/ISettingsPanel.h>
#include <GUI/QtWidget/SliceView.h>

namespace EspINA
{
  class SliceViewSettingsPanel
  : public ISettingsPanel
  , Ui::SliceViewSettingsPanel
  {
    Q_OBJECT
  public:
    explicit SliceViewSettingsPanel(SliceView::SettingsPtr settings);

    virtual const QString shortDescription();
    virtual const QString longDescription() {return tr("%1 Settings").arg(shortDescription());}
    virtual const QIcon icon() {return QIcon();}

    virtual void acceptChanges();

    virtual bool modified() const;

    virtual ISettingsPanelPtr clone();

  private:
    SliceView::SettingsPtr m_settings;
  };

} // namespace EspINA

#endif // SLICEVIEWSETTINGSPANEL_H
