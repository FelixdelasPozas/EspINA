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


#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include "ui_RectangularVOISettings.h"
#include <Tools/VOI/RectangularVOI.h>
#include <GUI/ISettingsPanel.h>

namespace EspINA
{
  class RectangularVOI::SettingsPanel
  : public ISettingsPanel
  , public Ui::RectangularVOISettings
  {
    Q_OBJECT
  public:
    explicit SettingsPanel(EspinaModelPtr model,
                           RectangularVOI::Settings *settings);
    virtual ~SettingsPanel();

    virtual const QString shortDescription()
    { return tr("Cuboid VOI"); }
    virtual const QString longDescription()
    { return tr("Cuboid Volume Of Interest Settings"); }
    virtual const QIcon icon()
    { return QIcon(":/espina/voi.svg"); }

    virtual void acceptChanges();
    virtual void rejectChanges();
    virtual bool modified() const;

    virtual ISettingsPanelPtr clone();

  private:
    bool taxonomyVOIModified() const;
    void writeTaxonomyProperties();

  private slots:
    void updateTaxonomyVOI(const QModelIndex &index);

  private:
    EspinaModel *m_model;
    RectangularVOI::Settings *m_settings;
    TaxonomyElementPtr m_activeTaxonomy;
  };

} // namespace EspINA

#endif // SETTINGSPANEL_H
