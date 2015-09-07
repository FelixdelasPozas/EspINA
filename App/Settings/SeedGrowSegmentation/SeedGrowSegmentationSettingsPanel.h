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

// ESPINA
#include <Support/Settings/SettingsPanel.h>
#include <GUI/Types.h>

// Qt
#include "ui_SeedGrowSegmentationSettingsPanel.h"

namespace ESPINA
{
  class SeedGrowSegmentationSettings;

  class SeedGrowSegmentationsSettingsPanel
  : public Support::Settings::SettingsPanel
  , public Ui::SeedGrowSegmentationSettingsPanel
  {
    Q_OBJECT
  public:
    /** \brief SeedGrowSegmentationSettingsPanel class constructor.
     * \param[in] settings SeedGrowSegmentation settings
     * \param[in] context ESPINA context
     *
     */
    explicit SeedGrowSegmentationsSettingsPanel(SeedGrowSegmentationSettings* settings);

    /** \brief SeedGrowSegmentationSettingsPanel class virtual destructor.
     *
     */
    virtual ~SeedGrowSegmentationsSettingsPanel()
    {}

    virtual const QString shortDescription() override
    { return tr("Grey Level Segmentation"); }

    virtual const QString longDescription() override
    { return tr("Create segmentation using similar grey level voxels."); }

    virtual const QIcon icon() override
    { return QIcon(":/espina/grey_level_segmentation.svg"); }

    virtual void acceptChanges() override;

    virtual void rejectChanges() override;

    virtual bool modified() const override;

    virtual Support::Settings::SettingsPanelPtr clone() override;

  private slots:
    /** \brief Manages the change of state of the taxonomical checkbox.
     * \param[in] state checkbox state.
     *
     */
    void changeTaxonomicalCheck(int state);

  private:
    SeedGrowSegmentationSettings *m_settings;

    GUI::Widgets::PixelValueSelector *m_pixelSelector;
  };

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_SETTINGS_PANEL_H
