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
#include <Support/ViewManager.h>

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
    /* \brief ROISettingsPanel class constructor.
     * \param[in] settings, ROISettings object raw pointer.
     * \param[in] model, model adapter smart pointer.
     * \param[in] viewManager, view manager smart pointer.
     *
     */
    explicit ROISettingsPanel(ROISettings*     settings,
                              ModelAdapterSPtr model,
                              ViewManagerSPtr  viewManager);

    /* \brief ROISettingsPanel class virtual destructor.
     *
     */
    virtual ~ROISettingsPanel();

    /* \brief Overrides SettingsPanel::shortDescription().
     *
     */
    virtual const QString shortDescription() override
    { return tr("Orthogonal ROI"); }

    /* \brief Overrides SettingsPanel::longDescription().
     *
     */
    virtual const QString longDescription() override
    { return tr("Orthogonal Region Of Interest"); }

    /* \brief Overrides SettingsPanel::icon().
     *
     */
    virtual const QIcon icon() override
    { return QIcon(":/espina/voi.svg"); }

    /* \brief Overrides SettingsPanel::acceptChanges().
     *
     */
    virtual void acceptChanges() override;

    /* \brief Overrides SettingsPanel::rejectChanges().
     *
     */
    virtual void rejectChanges() override;

    /* \brief Overrides SettingsPanel::modified().
     *
     */
    virtual bool modified() const override;

    /* \brief Overrides SettingsPanel::clone().
     *
     */
    virtual SettingsPanelPtr clone() override;

  private:
    /* \brief Returns true if any of the values of ROI have changed.
     *
     */
    bool categoryROIModified() const;

    /* \brief Apply the new ROI values to the category.
     *
     */
    void writeCategoryProperties();

  private slots:
		/* \brief Updates category ROI values for the category.
		 *
		 */
    void updateCategoryROI(const QModelIndex &index);

  private:
    ModelAdapterSPtr    m_model;
    ROISettings*        m_settings;
    CategoryAdapterSPtr m_activeCategory;
    ViewManagerSPtr     m_viewManager;
  };

} // namespace ESPINA

#endif // ROI_SETTINGS_PANEL_H_
