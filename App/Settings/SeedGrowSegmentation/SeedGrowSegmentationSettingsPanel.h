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
#include <Support/ViewManager.h>

// Qt
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
    /** brief SeedGrowSegmentationSettingsPanel class constructor.
     * \param[in] settings, raw pointer to a SeedGrowSegmentationSettings object.
     * \param[in] viewManager, view manager smart pointer.
     *
     */
    explicit SeedGrowSegmentationsSettingsPanel(SeedGrowSegmentationSettings* settings,
                                                ViewManagerSPtr               viewManager);

    /** brief SeedGrowSegmentationSettingsPanel class virtual destructor.
     *
     */
    virtual ~SeedGrowSegmentationsSettingsPanel()
    {}

    /** brief Overrides SerttingsPanel::shortDescription().
     *
     */
    virtual const QString shortDescription() override
    { return tr("Seed Grow Segmentation"); }

    /** brief Overrides SerttingsPanel::longDescription().
     *
     */
    virtual const QString longDescription() override
    { return tr("Seed Grow Segmentation"); }

    /** brief Overrides SerttingsPanel::icon().
     *
     */
    virtual const QIcon icon() override
    { return QIcon(":/espina/bestPixelSelector.svg"); }

    /** brief Overrides SerttingsPanel::acceptChanges().
     *
     */
    virtual void acceptChanges() override;

    /** brief Overrides SerttingsPanel::rejectChanges().
     *
     */
    virtual void rejectChanges() override;

    /** brief Overrides SerttingsPanel::modified().
     *
     */
    virtual bool modified() const override;

    /** brief Overrides SerttingsPanel::clone().
     *
     */
    virtual SettingsPanelPtr clone() override;

  public slots:
		/** brief Modifies the color of the pixel value.
		 *
		 */
    void displayColor(int value);

  protected slots:
  	/** brief Manages the change of state of the taxonomical checkbox.
  	 * \param[in] state, checkbox state.
  	 *
  	 */
    void changeTaxonomicalCheck(int state);

    /** brief Stores the Z value.
     * \param[in] value, unused value.
     *
     */
    void zValueChanged(int unused);

  private:
    SeedGrowSegmentationSettings *m_settings;
    ViewManagerSPtr               m_viewManager;
    bool                          m_zValueChanged;
  };

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_SETTINGS_PANEL_H
