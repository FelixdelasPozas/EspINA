/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef HUESELECTOR_H_
#define HUESELECTOR_H_

#include "GUI/EspinaGUI_Export.h"

// Qt includes
#include <QWidget>

namespace ESPINA
{
class EspinaGUI_EXPORT HueSelector
: public QWidget
{
	Q_OBJECT
	public:
	  /* \brief HueSelector class constructor.
	   * \param[in] parent, raw pointer of the QWidget parent of this one.
	   *
	   */
		HueSelector(QWidget* parent = nullptr);

		/* \brief HueSelector class destructor.
		 *
		 */
		~HueSelector();

	public slots:
	  /* \brief Sets the hue to the given value and updates the UI.
	   * \param[in] h, new hue value.
	   *
	   */
		void setHueValue(int h);

	signals:
		void newHsv(int h, int s, int v);

	protected:
		/* \brief Overrides QWidget::paintEvent().
		 *
		 */
		void paintEvent(QPaintEvent*) override;

		/* \brief Overrides QWidget::mouseMoveEvent().
		 *
		 */
		void mouseMoveEvent(QMouseEvent *) override;

		/* \brief Overrides QWidget::mousePressEvent().
		 *
		 */
		void mousePressEvent(QMouseEvent *) override;

	private:
		int val;
		int hue;
		int sat;

		/* \brief Computes the equivalent value from the x value of the slider of the widget.
		 * \param[in] y, slider value.
		 *
		 */
		int x2val(int y);

		/* \brief Computes the equivalent value from the hue value to the x coodinate of the slider.
		 * \param[in] y, hue value.
		 *
		 */
		int val2x(int val);

		/* \brief Sets the value of the slider of the widget.
		 *
		 */
		void setVal(int v);

		QPixmap *pix;
};

} // namespace ESPINA

#endif // HUESELECTOR_H_
