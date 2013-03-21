/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
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


#ifndef SEEDGROWSEGMENTATIONPREFERENCES_H
#define SEEDGROWSEGMENTATIONPREFERENCES_H

#include <GUI/ISettingsPanel.h>
#include "ui_SettingsPanel.h"

#include "Toolbars/Segmentation/SegmentationToolBar.h"

namespace EspINA
{
  class ViewManager;

  class SeedGrowSegmentationsSettingsPanel
  : public ISettingsPanel
  , public Ui::SettingsPanel
  {
    Q_OBJECT
  public:
    explicit SeedGrowSegmentationsSettingsPanel(SeedGrowSegmentationSettings *settings, ViewManager *viewManager);
    virtual ~SeedGrowSegmentationsSettingsPanel(){}

    virtual const QString shortDescription()
    { return tr("Seed Grow Segmentation"); }
    virtual const QString longDescription()
    { return tr("Seed Grow Segmentation Settings"); }
    virtual const QIcon icon()
    { return QIcon(":/espina/bestPixelSelector.svg"); }

    virtual void acceptChanges();
  virtual void rejectChanges();
    virtual bool modified() const;

    virtual ISettingsPanelPtr clone();

  public slots:
    void displayColor(int value);

  protected slots:
    void changeTaxonomicalCheck(int);
    void zValueChanged(int);

  private:
    SeedGrowSegmentationSettings *m_settings;
    ViewManager                  *m_viewManager;
    bool                          m_zValueChanged;
  };

} // namespace EspINA

#endif // SEEDGROWSEGMENTATIONPREFERENCES_H
