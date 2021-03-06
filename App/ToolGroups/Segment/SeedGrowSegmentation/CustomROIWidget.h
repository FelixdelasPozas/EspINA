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
#include <QWidget>
#include <QLabel>
#include <QSpinBox>

namespace ESPINA
{
  /** \class CustomROIWidget
   * \brief Implements a custom widget for the seedgrow segmentation tool to handle ROI values.
   *
   */
  class CustomROIWidget
  : public QWidget
  {
    Q_OBJECT

  public:
    /** \brief CustomROIWidget class constructor.
     * \param[in] parent, raw pointer to the parent of this object.
     *
     */
    explicit CustomROIWidget(QWidget* parent=nullptr);

    /** \brief Returns the value of the applyROI flag.
     *
     */
    bool applyROI()
    {return m_useROI;}

    /** \brief Sets the value of the ROI for a specified axis.
     * \param[in] axis, axis for the value.
     * \param[in] value, size value.
     */
    void setValue(Axis axis, long long value);

    /** \brief Returns the value of the ROI for a specified axis.
     * \param[in] axis for the value.
     *
     */
    long long value(Axis axis) const
    { return m_values[idx(axis)]; }

    /** \brief Enables/Disables the use of the ROI.
     * \param[in] enabled boolean value.
     *
     */
    void setApplyROI(bool enabled);

  private slots:
    /** \brief Modifies the GUI if the ROI is to be used.
     * \param[in] val true if the ROI values are going to be used.
     *
     */
    void onApplyROIChanged(bool val);

    /** \brief Updates the value of the ROI on the X axis.
     * \param[in] value type double.
     *
     */
    void onXSizeChanged(double value);

    /** \brief Updates the value of the ROI on the Y axis.
     * \param[in] value type double.
     *
     */
    void onYSizeChanged(double value);

    /** \brief Updates the value of the ROI on the Z axis.
     * \param[in] value type double.
     *
     */
    void onZSizeChanged(double value);

  signals:
    void useROI(bool);

  private:
    bool m_useROI;

    long long       m_values    [3]; /** ROI size values in each axis. */
    QLabel         *m_labelROI  [3]; /** label widgets.                */
    QDoubleSpinBox *m_spinBoxROI[3]; /** spinbox widgets.              */
  };

} // namespace ESPINA

#endif // ESPINA_CUSTOM_ROI_ACTION_H
