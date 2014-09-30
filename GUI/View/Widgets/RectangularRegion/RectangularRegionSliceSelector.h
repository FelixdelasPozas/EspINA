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

#ifndef RECTANGULARREGIONSLICESELECTOR_H
#define RECTANGULARREGIONSLICESELECTOR_H

// ESPINA
#include <GUI/Widgets/SliceSelectorWidget.h>

class QPushButton;

namespace ESPINA
{
  class RectangularRegion;

  class EspinaGUI_EXPORT RectangularRegionSliceSelector
  : public SliceSelectorWidget
  {
    Q_OBJECT
  public:
    /** \brief RectangularRegionSliceSelector class constructor.
     *
     */
    explicit RectangularRegionSliceSelector(RectangularRegion *region);

    /** \brief RectangularRegionSliceSelector class destructor.
     *
     */
    virtual ~RectangularRegionSliceSelector();

    /** \brief Overrides SliceSelectorWidget::setPlane().
     *
     */
    virtual void setPlane(const Plane plane) override;

    /** \brief Implements SliceSelectorWidget::leftWidget().
     *
     */
    virtual QWidget *leftWidget () const;

    /** \brief Implements SliceSelectorWidget::rightWidget().
     *
     */
    virtual QWidget *rightWidget() const;

    /** \brief Sets the left widget label.
     * \param[in] label.
     */
    void setLeftLabel (const QString &label)
    { m_leftLabel  = label; update();}

		/** \brief Sets the right widget label.
		 * \param[in] label.
		 */
    void setRightLabel(const QString &label)
    {m_rightLabel = label; update();}

    /** \brief Implements SliceSelectorWidget::clone().
     *
     */
    virtual SliceSelectorWidget *clone();

  protected slots:
		/** \brief Update the widgets.
		 *
		 */
    void update();

		/** \brief Update the widgets when the left widgets has been clicked.
		 *
		 */
    void leftWidgetClicked();

		/** \brief Update the widgets when the right widgets has been clicked.
		 *
		 */
    void rightWidgetClicked();

  private:
    RectangularRegion *m_region;

    QPushButton *m_leftWidget;
    QPushButton *m_rightWidget;

    QString m_leftLabel;
    QString m_rightLabel;
  };
}// namespace ESPINA


#endif // RECTANGULARREGIONSLICESELECTOR_H
