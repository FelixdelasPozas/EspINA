/*
 Copyright (C) 2013  Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef COUNTINGFRAMEINTERACTORADAPTER_H_
#define COUNTINGFRAMEINTERACTORADAPTER_H_

#include "CountingFramePlugin_Export.h"

#include <vtkAbstractWidget.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetEventTranslator.h>

class QEvent;

namespace ESPINA
{
  template<class T>
  class CountingFramePlugin_EXPORT CountingFrameInteractorAdapter
  : public T
  {
  public:
    bool ProcessEventsHandler(long unsigned int event)
    {
        this->EventCallbackCommand->SetAbortFlag(0);
        this->CallbackMapper->DebugOn();
        unsigned long int widgetEvent = this->CallbackMapper->GetEventTranslator()->GetTranslation(event);
        this->CallbackMapper->InvokeCallback(widgetEvent);

        return this->EventCallbackCommand->GetAbortFlag();
    }
  };

}// namespace ESPINA


#endif /* COUNTINGFRAMEINTERACTORADAPTER_H_ */
