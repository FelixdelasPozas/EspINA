/*
 * CountingFrameInteractorAdapter.h
 *
 *  Created on: 17/06/2013
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef COUNTINGFRAMEINTERACTORADAPTER_H_
#define COUNTINGFRAMEINTERACTORADAPTER_H_

#include "CountingFramePlugin_Export.h"

#include <vtkAbstractWidget.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetEventTranslator.h>

class QEvent;

namespace EspINA
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

}// namespace EspINA


#endif /* COUNTINGFRAMEINTERACTORADAPTER_H_ */
