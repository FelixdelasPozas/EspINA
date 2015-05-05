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

#ifndef ESPINA_BASIC_REPRESENTATION_SWITCH_H
#define ESPINA_BASIC_REPRESENTATION_SWITCH_H

#include <Support/Representations/RepresentationSwitch.h>
#include <GUI/Representations/RepresentationManager.h>

namespace ESPINA {

  class BasicRepresentationSwitch
  : public RepresentationSwitch
  {
    Q_OBJECT

  public:
    explicit BasicRepresentationSwitch(GUI::Representations::RepresentationManagerSPtr manager, ViewTypeFlags supportedViews, Timer &timer);

    virtual ViewTypeFlags supportedViews();

    virtual QWidget* widget();

    virtual void showRepresentations(TimeStamp t) override;

    virtual void hideRepresentations(TimeStamp t) override;

  private slots:
    void onButtonToggled(bool active);

    void onSwitchValue();

  private:
    GUI::Representations::RepresentationManagerSPtr m_manager;

    ViewTypeFlags m_flags;
  };
}

#endif // ESPINA_BASIC_REPRESENTATION_SWITCH_H
