/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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


#ifndef ESPINAINTERACTORADAPTER_H
#define ESPINAINTERACTORADAPTER_H

#include <vtkAbstractWidget.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetEventTranslator.h>

class QEvent;

template<class T>
class EspinaInteractorAdapter
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

//vtkStandardNewMacro(EspinaInteractorAdapter<T>);

#endif // ESPINAINTERACTORADAPTER_H
