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

#ifndef ESPINA_VIEW_2D_SETTINGS_PANEL_H
#define ESPINA_VIEW_2D_SETTINGS_PANEL_H

// ESPINA
#include <Support/Settings/SettingsPanel.h>
#include <GUI/View/View2D.h>

// Qt
#include "ui_View2DSettingsPanel.h"

namespace ESPINA
{
  class View2DSettingsPanel
  : public Support::Settings::SettingsPanel
  , Ui::View2DSettingsPanel
  {
    Q_OBJECT
  public:
    /** \brief View2DSettingsPanel class constructor.
     * \param[in] view 2D render view
     */
    explicit View2DSettingsPanel(View2D *view);

    virtual const QString shortDescription() override;

    virtual const QString longDescription() override
    {return tr("%1").arg(shortDescription());}

    virtual const QIcon icon() override
    {return QIcon();}

    virtual void acceptChanges() override;

    virtual void rejectChanges() override;

    virtual bool modified() const override;

    virtual Support::Settings::SettingsPanelPtr clone() override;

  private:
    View2D *m_view;
  };

} // namespace ESPINA

#endif // ESPINA_VIEW_2D_SETTINGS_PANEL_H
