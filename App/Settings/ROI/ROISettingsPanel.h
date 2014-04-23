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

#ifndef ROI_SETTINGS_PANEL_H_
#define ROI_SETTINGS_PANEL_H_

#include <App/Tools/ROI/RectangularROI.h>
#include <Support/Settings/SettingsPanel.h>
#include "ui_RectangularROISettings.h"

namespace EspINA
{
  class ViewManager;

  class RectangularROI::ROISettingsPanel
  : public SettingsPanel
  , public Ui::RectangularROISettings
  {
    Q_OBJECT
  public:
    explicit ROISettingsPanel(ModelAdapterSPtr model,
                              RectangularROI::ROISettings *settings,
                              ViewManagerSPtr viewManager);
    virtual ~ROISettingsPanel();

    virtual const QString shortDescription()
    { return tr("Cuboid VOI"); }
    virtual const QString longDescription()
    { return tr("Cuboid Volume Of Interest"); }
    virtual const QIcon icon()
    { return QIcon(":/espina/voi.svg"); }

    virtual void acceptChanges();
    virtual void rejectChanges();
    virtual bool modified() const;

    virtual SettingsPanelPtr clone();

  private:
    bool categoryROIModified() const;
    void writeCategoryProperties();

  private slots:
    void updateCategoryROI(const QModelIndex &index);
    void zValueChanged(int);

  private:
    ModelAdapterSPtr m_model;
    RectangularROI::ROISettings *m_settings;
    CategoryAdapterSPtr m_activeCategory;
    ViewManagerSPtr m_viewManager;
    Nm m_zSpacing;

    bool m_zValueChanged;
    bool m_zTaxValueChanged;
  };

} // namespace EspINA

#endif // ROI_SETTINGS_PANEL_H_
