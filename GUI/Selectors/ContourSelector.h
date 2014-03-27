/*
 * ContourSelection.h
 *
 *  Created on: Aug 28, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef _CONTOURSELECTOR_H_
#define _CONTOURSELECTOR_H_

#include "EspinaGUI_Export.h"

#include <GUI/Pickers/ISelector.h>

class QCursor;

namespace EspINA
{
  class EspinaGUI_EXPORT ContourSelector
  : public ISelector
  {
	public:
	  explicit ContourSelector(ISelector *successor = NULL);
	  virtual ~ContourSelector();

	  virtual bool filterEvent(QEvent* e, EspinaRenderView *view = 0);
	  virtual QCursor cursor();
	private:
	  QCursor m_cursor;
  };

} // namespace EspINA

#endif /* _CONTOURSELECTOR_H_ */
