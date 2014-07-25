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


#ifndef ESPINA_VIEW_3D_SETTINGS_PANEL_H
#define ESPINA_VIEW_3D_SETTINGS_PANEL_H

#include <Support/Settings/SettingsPanel.h>
#include <GUI/Widgets/RenderersSelector.h>
#include "ui_View3DSettingsPanel.h"

#include <GUI/View/View3D.h>

namespace ESPINA
{
  class EspinaFactory;

  class View3DSettingsPanel
  : public SettingsPanel
  , Ui::View3DSettingsPanel
  {
    Q_OBJECT
  public:
    explicit View3DSettingsPanel(View3D *view, const RendererSList &renderers);

    virtual const QString shortDescription()
    {return tr("3D View");}

    virtual const QString longDescription()
    {return tr("%1").arg(shortDescription());}

    virtual const QIcon icon()
    {return QIcon();}

    virtual void acceptChanges();

    virtual void rejectChanges();

    virtual bool modified() const;

    virtual SettingsPanelPtr clone();

  private:
    View3D              *m_view;
    const RendererSList &m_renderers;
    RenderersSelector   *m_rendererSelector;
  };

} // namespace ESPINA

#endif // ESPINA_VIEW_3D_SETTINGS_PANEL_H
