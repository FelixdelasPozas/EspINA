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

// ESPINA
#include <GUI/ColorEngines/MultiColorEngine.h>

// Qt
#include <QMenu>

namespace ESPINA
{
  class ColorEngineMenu
  : public QMenu
  {
    Q_OBJECT
  public:
    /** \brief ColorEngineMenu class constructor.
     * \param[in] title of the color engine.
     * \param[in] parent QObject.
     *
     */
    explicit ColorEngineMenu(const QString  &title,
                             GUI::ColorEngines::MultiColorEngineSPtr colorEngine);

    /** \brief ColorEngineMenu class virtual destructor.
     *
     */
    virtual ~ColorEngineMenu();

    /** \brief Adds a color engine to the menu.
     * \param[in] title for displaying the color engine in the menu
     * \param[in] engine color engine
     *
     */
    void addColorEngine(const QString &title, GUI::ColorEngines::ColorEngineSPtr engine);

    /** \brief Restores user settings for the menu.
     *
     */
    void restoreUserSettings();

  protected slots:
    /** \brief Toggles the color engine binded to the QAction passed as parameter.
     * \param[in] action color engine toggle action.
     *
     */
    void toggleColorEngine(QAction *action);

  signals:
    void colorEngineChanged(GUI::ColorEngines::ColorEngineSPtr);

  private:
    GUI::ColorEngines::MultiColorEngineSPtr m_engine;

    QMap<QAction *, GUI::ColorEngines::ColorEngineSPtr>  m_availableEngines;
  };

} // namespace ESPINA

#endif // COLORENGINESETTINGS_H
