/*

    Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APPOSITIONSURFACESETTINGS_H_
#define APPOSITIONSURFACESETTINGS_H_

#include "AppositionSurfacePlugin_Export.h"

// ESPINA
#include <Support/Settings/SettingsPanel.h>
#include "ui_AppositionSurfaceSettings.h"

// Qt
#include <QColor>

namespace ESPINA
{
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceSettings
  : public Support::Settings::SettingsPanel
  , public Ui::AppositionSurfaceSettings
  {
    Q_OBJECT
  public:
    /** \brief AppositionSurfaceSettings class constructor.
     *
     */
    explicit AppositionSurfaceSettings();

    /** \brief AppositionSurfaceSettings class virtual destructor.
     *
     */
    virtual ~AppositionSurfaceSettings()
    {};

    virtual const QString shortDescription() { return tr("Synaptic Apposition Surface"); }
    virtual const QString longDescription()  { return tr("Synaptic Apposition Surface"); }
    virtual const QIcon icon()               { return QIcon(":/AppSurface.svg"); }

    virtual void acceptChanges();
    virtual void rejectChanges();
    virtual bool modified() const;

    virtual Support::Settings::SettingsPanelPtr clone();

  public slots:
    void changeDefaultComputation(int);

  private:
    bool m_automaticComputation;
    bool m_modified;
  };
} /* namespace ESPINA */

#endif /* APPOSITIONSURFACESETTINGS_H_ */
