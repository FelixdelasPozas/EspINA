/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_INFORMATION_COLOR_ENGINE_SWITCH_H
#define ESPINA_INFORMATION_COLOR_ENGINE_SWITCH_H

#include <Support/Widgets/ColorEngineSwitch.h>

#include <GUI/Types.h>

namespace ESPINA
{
  class InformationColorEngineSwitch
  : public Support::Widgets::ColorEngineSwitch
  {
    Q_OBJECT

  public:
    explicit InformationColorEngineSwitch(Support::Context& context);

  private:
    void createPropertySelector();

    void createColorRange();

    GUI::ColorEngines::InformationColorEngine *informationColorEngine() const;

    void updateSettings();

  private slots:
    void changeProperty();

  private:
    QString m_extensionType;
    QString m_informationTag;

    QLabel *m_property;
  };
}

#endif // ESPINA_INFORMATION_COLOR_ENGINE_SWITCH_H
