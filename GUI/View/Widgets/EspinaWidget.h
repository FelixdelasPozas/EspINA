/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_WIDGET_H
#define ESPINA_WIDGET_H

#include "EspinaGUI_Export.h"

#include <memory>

namespace EspINA
{
  class RenderView;

  class EspinaGUI_EXPORT EspinaWidget
  {
  public:
    explicit EspinaWidget(){}
    virtual ~EspinaWidget(){}

    virtual void registerView(RenderView *) = 0;
    virtual void unregisterView(RenderView *) = 0;

    virtual void setEnabled(bool enable) = 0;
    virtual bool manipulatesSegmentations() { return false; };
  };

  using EspinaWidgetPtr  = EspinaWidget *;
  using EspinaWidgetSPtr = std::shared_ptr<EspinaWidget>;

} // namespace EspINA

#endif // ESPINA_WIDGET_H
