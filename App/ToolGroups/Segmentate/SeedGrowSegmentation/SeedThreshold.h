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


#ifndef ESPINA_THRESHOLD_ACTION
#define ESPINA_THRESHOLD_ACTION

#include <QWidgetAction>

class QSpinBox;
class QLabel;

namespace ESPINA
{
  class SeedThreshold
  : public QWidgetAction
  {
    Q_OBJECT
  public:
    /** \brief SeedThreshold class constructor.
     * \param[in] parent raw pointer of the parent of this object.
     */
    explicit SeedThreshold(QObject* parent = nullptr);

    virtual QWidget* createWidget(QWidget* parent) override;

    /** \brief Returns lower threshold value.
     *
     */
    int lowerThreshold()
    {return m_threshold[0];}

    /** \brief Returns upper threshold value.
     *
     */
    int upperThreshold()
    {return m_threshold[m_symmetrical?1:0];}

    /** \brief Set if the threshold is symmetric.
     * \param[in] symmetrical true if symmetrical, false otherwise.
     *
     */
    void setSymmetricalThreshold(bool symmetrical);

    /** \brief Returns true if the threshold is symmetrical.
     *
     */
    bool isSymmetrical() const
    {return m_symmetrical;}

  public slots:
    /** \brief Sets the lower threshold value.
     * \param[in] th lower threshold value.
     */
    void setLowerThreshold(int th);

    /** \brief Sets the upper threshold value.
     * \param[in] th upper threshold value.
     */
    void setUpperThreshold(int th);

  signals:
    void lowerThresholdChanged(int);
    void upperThresholdChanged(int);

  private:
    QLabel   *m_lthLabel, *m_uthLabel;
    QSpinBox *m_lth, *m_uth;

    int  m_threshold[2];
    bool m_symmetrical;
  };

} // namespace ESPINA

#endif // ESPINA_THRESHOLD_ACTION
