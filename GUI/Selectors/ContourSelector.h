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

#ifndef ESPINA_CONTOUR_SELECTOR_H_
#define ESPINA_CONTOUR_SELECTOR_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Selectors/Selector.h>

class QCursor;

namespace ESPINA
{
  class EspinaGUI_EXPORT ContourSelector
  : public Selector
  {
	public:
  	/** \brief ContourSelector class constructor.
  	 *
  	 */
	  explicit ContourSelector()
	  : m_cursor(Qt::CrossCursor)
	  {}

  	/** \brief ContourSelector class destructor.
  	 *
  	 */
	  virtual ~ContourSelector()
	  {}

  	/** \brief Overrides EventHandler::filterEvent().
  	 *
  	 */
	  virtual bool filterEvent(QEvent* e, RenderView *view = nullptr) override;

	  /** \brief Overrides EventHandler::cursor() const.
	   *
	   */
	  virtual QCursor cursor() const override;
	private:
	  QCursor m_cursor;
  };

} // namespace ESPINA

#endif /* _CONTOURSELECTOR_H_ */
