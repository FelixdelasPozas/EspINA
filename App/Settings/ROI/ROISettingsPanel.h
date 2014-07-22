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

#ifndef ROI_SETTINGS_PANEL_H_
#define ROI_SETTINGS_PANEL_H_

#include <Core/Utils/NmVector3.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/Settings/SettingsPanel.h>
#include <Support/ViewManager.h>
#include "ui_OrthogonalROISettings.h"

namespace ESPINA
{
  class ViewManager;
  class ROISettings;

  class ROISettingsPanel
  : public SettingsPanel
  , public Ui::OrthogonalROISettings
  {
    Q_OBJECT
  public:
    explicit ROISettingsPanel(ROISettings*     settings,
                              ModelAdapterSPtr model,
                              ViewManagerSPtr  viewManager);
    virtual ~ROISettingsPanel();

    virtual const QString shortDescription()
    { return tr("Orthogonal ROI"); }
    virtual const QString longDescription()
    { return tr("Orthogonal Region Of Interest"); }
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

  private:
    ModelAdapterSPtr    m_model;
    ROISettings*        m_settings;
    CategoryAdapterSPtr m_activeCategory;
    ViewManagerSPtr     m_viewManager;
  };

} // namespace ESPINA

#endif // ROI_SETTINGS_PANEL_H_
