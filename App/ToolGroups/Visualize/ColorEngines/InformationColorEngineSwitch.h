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

namespace ESPINA
{
  class InformationColorEngineSwitch
  : public Support::Widgets::ColorEngineSwitch
  {
    Q_OBJECT

  public:
    explicit InformationColorEngineSwitch(Support::Context& context);

    using InformationKey = SegmentationExtension::InformationKey;

    virtual void restoreSettings(std::shared_ptr<QSettings> settings);

    virtual void saveSettings(std::shared_ptr<QSettings> settings);

  private:
    void createPropertySelector();

    void createColorRange();

    GUI::ColorEngines::InformationColorEngine *informationColorEngine() const;

    void update();

    void updateLink();

  private slots:
    void changeProperty();

    void updateRange();

    void onToolToggled(bool checked);

  private:
    InformationKey m_key;

    bool m_needUpdate;

    TaskSPtr m_task;

    QLabel *m_property;
  };
}

#endif // ESPINA_INFORMATION_COLOR_ENGINE_SWITCH_H