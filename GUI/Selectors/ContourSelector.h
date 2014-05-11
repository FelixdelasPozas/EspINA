/*
 * ContourSelection.h
 *
 *  Created on: Aug 28, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef ESPINA_CONTOUR_SELECTOR_H_
#define ESPINA_CONTOUR_SELECTOR_H_

#include "EspinaGUI_Export.h"
#include <GUI/Selectors/Selector.h>

class QCursor;

namespace EspINA
{
  class EspinaGUI_EXPORT ContourSelector
  : public Selector
  {
	public:
	  explicit ContourSelector()
	  : m_cursor(Qt::CrossCursor)
	  {}

	  virtual ~ContourSelector()
	  {}

	  virtual bool filterEvent(QEvent* e, RenderView *view = 0);
	  virtual QCursor cursor();
	private:
	  QCursor m_cursor;
  };

} // namespace EspINA

#endif /* _CONTOURSELECTOR_H_ */
