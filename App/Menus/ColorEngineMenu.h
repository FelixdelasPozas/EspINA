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


#ifndef ESPINA_COLOR_ENGINE_MENU_H
#define ESPINA_COLOR_ENGINE_MENU_H

#include <QMenu>
#include <Support/ViewManager.h>
#include <GUI/ColorEngines/MultiColorEngine.h>

namespace ESPINA
{
  class ColorEngineMenu
  : public QMenu
  {
    Q_OBJECT
  public:
    explicit ColorEngineMenu(ViewManagerSPtr vm, const QString &title, QWidget *parent = 0);
    virtual ~ColorEngineMenu();

    ColorEngineSPtr engine() const 
    {return m_engine;}

    void addColorEngine(const QString &title, ColorEngineSPtr engine);

    void restoreUserSettings();

  protected slots:
    void setColorEngine(QAction *action);

  signals:
    void colorEngineChanged(ColorEngineSPtr);

  private:
    ViewManagerSPtr      m_viewManager;
    MultiColorEngineSPtr m_engine;

    QMap<QAction *, ColorEngineSPtr>  m_availableEngines;
  };

} // namespace ESPINA

#endif // COLORENGINESETTINGS_H
