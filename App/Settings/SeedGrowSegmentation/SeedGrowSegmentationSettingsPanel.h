/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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


#ifndef ESPINA_SEED_GROW_SEGMENTATION_SETTINGS_PANEL_H
#define ESPINA_SEED_GROW_SEGMENTATION_SETTINGS_PANEL_H

#include <Support/Settings/SettingsPanel.h>
#include <Support/ViewManager.h>
#include "ui_SeedGrowSegmentationSettingsPanel.h"

namespace ESPINA
{

  class SeedGrowSegmentationSettings;

  class SeedGrowSegmentationsSettingsPanel
  : public SettingsPanel
  , public Ui::SeedGrowSegmentationSettingsPanel
  {
    Q_OBJECT
  public:
    explicit SeedGrowSegmentationsSettingsPanel(SeedGrowSegmentationSettings* settings,
                                                ViewManagerSPtr               viewManager);
    virtual ~SeedGrowSegmentationsSettingsPanel(){}

    virtual const QString shortDescription()
    { return tr("Seed Grow Segmentation"); }
    virtual const QString longDescription()
    { return tr("Seed Grow Segmentation"); }
    virtual const QIcon icon()
    { return QIcon(":/espina/bestPixelSelector.svg"); }

    virtual void acceptChanges();
    virtual void rejectChanges();
    virtual bool modified() const;

    virtual SettingsPanelPtr clone();

  public slots:
    void displayColor(int value);

  protected slots:
    void changeTaxonomicalCheck(int);
    void zValueChanged(int);

  private:
    SeedGrowSegmentationSettings *m_settings;
    ViewManagerSPtr               m_viewManager;
    bool                          m_zValueChanged;
  };

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_SETTINGS_PANEL_H
