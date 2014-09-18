/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINA_CUSTOM_ROI_ACTION_H
#define ESPINA_CUSTOM_ROI_ACTION_H

// ESPINA
#include <Core/Utils/Spatial.h>

// Qt
#include <QWidgetAction>
#include <QLabel>
#include <QSpinBox>

namespace ESPINA
{

  class CustomROIWidget
  : public QWidgetAction
  {
    Q_OBJECT

  public:
    /* \brief CustomROIWidget class constructor.
     * \param[in] parent, raw pointer to the parent of this object.
     *
     */
    explicit CustomROIWidget(QObject* parent=nullptr);

    /* \brief Overrides QWidgetAction::createWidget().
     *
     */
    virtual QWidget* createWidget(QWidget* parent) override;

    /* \brief Overrides QWidgetAction::deleteWidget().
     *
     */
    virtual void deleteWidget(QWidget* widget) override;

    /* \brief Returns the value of the applyROI flag.
     *
     */
    bool applyROI()
    {return m_useROI;}

    /* \brief Sets the value of the ROI for a specified axis.
     * \param[in] axis, axis for the value.
     * \param[in] value, size value.
     */
    void setValue(Axis axis, unsigned int value);

    /* \brief Returns the value of the ROI for a specified axis.
     * \param[in] axis, axis for the value.
     */
    unsigned int value(Axis axis) const
    { return m_values[idx(axis)]; }

  private slots:
  	/* \brief Modifies the GUI if the ROI is to be used.
  	 * \param[in] val, true if the ROI values are going to be used.
  	 *
  	 */
    void onApplyROIChanged(bool val);

  	/* \brief Updates the value of the ROI on the X axis.
  	 *
  	 */
    void onXSizeChanged(int value);

  	/* \brief Updates the value of the ROI on the Y axis.
  	 *
  	 */
    void onYSizeChanged(int value);

  	/* \brief Updates the value of the ROI on the Z axis.
  	 *
  	 */
    void onZSizeChanged(int value);

  signals:
    void useROI(bool);

  private:
    bool    m_useROI;

    unsigned int m_values    [3];
    QLabel      *m_labelROI  [3];
    QSpinBox    *m_spinBoxROI[3];
  };

} // namespace ESPINA

#endif // ESPINA_CUSTOM_ROI_ACTION_H
