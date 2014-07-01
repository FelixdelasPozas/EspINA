/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_CODE_TOOL_H
#define ESPINA_CODE_TOOL_H

#include <GUI/Widgets/SpinBoxAction.h>

namespace EspINA {
  class CODETool
  : public QObject
  {
    Q_OBJECT

  public:
    CODETool(const QString&icon, const QString& tooltip);

    QList<QAction *> actions() const;

    void setEnabled(bool enabled);

    void setRadius(int value)
    { m_radius->setValue(value); }

    int radius() const
    { return m_radius->value(); }

  public slots:
    void toggleToolWidgets(bool visible);

  signals:
    void toggled(bool);
    void applyClicked();

  private:
    QAction*       m_toggle;
    SpinBoxAction* m_radius;
    QAction*       m_apply;
  };

} // namespace EspINA

#endif // ESPINA_CODE_TOOL_H
