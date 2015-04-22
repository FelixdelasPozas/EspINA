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

// ESPINA
#include <Core/Utils/NmVector3.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/Settings/SettingsPanel.h>
#include <Support/Context.h>

// Qt
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
    /** \brief ROISettingsPanel class constructor.
     * \param[in] settings, ROISettings object raw pointer.
     * \param[in] context ESPINA context
     *
     */
    explicit ROISettingsPanel(ROISettings            *settings,
                              Support::Context &context);

    /** \brief ROISettingsPanel class virtual destructor.
     *
     */
    virtual ~ROISettingsPanel();

    virtual const QString shortDescription() override
    { return tr("Orthogonal ROI"); }

    virtual const QString longDescription() override
    { return tr("Orthogonal Region Of Interest"); }

    virtual const QIcon icon() override
    { return QIcon(":/espina/voi.svg"); }

    virtual void acceptChanges() override;

    virtual void rejectChanges() override;

    virtual bool modified() const override;

    virtual SettingsPanelPtr clone() override;

  private:
    /** \brief Returns true if any of the values of ROI have changed.
     *
     */
    bool categoryROIModified() const;

    /** \brief Apply the new ROI values to the category.
     *
     */
    void writeCategoryProperties();

  private slots:
    /** \brief Updates category ROI values for the category.
     *
     */
    void updateCategoryROI(const QModelIndex &index);

  private:
    Support::Context &m_context;
    ROISettings*            m_settings;
    CategoryAdapterSPtr     m_activeCategory;
  };

} // namespace ESPINA

#endif // ROI_SETTINGS_PANEL_H_
