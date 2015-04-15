/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "MeasureEventHandler.h"
#include <QKeyEvent>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets::Measures;

//----------------------------------------------------------------------------
bool MeasureEventHandler::filterEvent(QEvent *e, RenderView *view)
{
  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent *ke = reinterpret_cast<QKeyEvent*>(e);
    if (ke->key() == Qt::Key_Backspace)
    {
      emit clear();

      return true;
    }
  }

  return false;
}
