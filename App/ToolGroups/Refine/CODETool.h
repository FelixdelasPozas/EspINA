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

// ESPINA
#include <Support/Widgets/Tool.h>
#include <GUI/Widgets/SpinBoxAction.h>
#include <GUI/Widgets/NumericalInput.h>

namespace ESPINA {
  class CODETool
  : public Tool
  {
    Q_OBJECT

  public:
    /** \brief CODETool class constructor.
     * \param[in] icon tool icon.
     * \param[in] tooltip tooltip of the tool.
     *
     */
    explicit CODETool(const QString&icon, const QString& tooltip);

    virtual QList<QAction *> actions() const override;

    virtual void abortOperation() override;

    /** \brief Sets the radius value.
     * \param[in] value value of the radius.
     */
    void setRadius(int value)
    { m_radius->setValue(value); }

    /** \brief Returns the value of the radius.
     *
     */
    int radius() const
    { return m_radius->value(); }

  public slots:
    /** \brief Hide/show the tool widgets.
     * \param[in] visible true to set visible, false otherwise.
     */
    void toggleToolWidgets(bool visible);

  signals:
    void toggled(bool);
    void applyClicked();

  private:
    virtual void onToolEnabled(bool enabled);

    void initOptionWidgets();

  private:
    QAction       *m_toggle;
    QWidgetAction *m_nestedOptions;
    GUI::Widgets::NumericalInput *m_radius;
  };

} // namespace ESPINA

#endif // ESPINA_CODE_TOOL_H
