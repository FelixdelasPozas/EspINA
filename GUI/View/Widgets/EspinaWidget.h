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

// VTK
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// C++
#include <memory>

namespace EspINA
{
  class RenderView;

  class EspinaGUI_EXPORT EspinaWidget
  {
  public:
    explicit EspinaWidget(){}
    virtual ~EspinaWidget(){}

    virtual void registerView  (RenderView *view) = 0;
    virtual void unregisterView(RenderView *view) = 0;

    virtual void setEnabled(bool enable) = 0;
    virtual bool manipulatesSegmentations() const { return false; };
  };

  using EspinaWidgetPtr  = EspinaWidget *;
  using EspinaWidgetSPtr = std::shared_ptr<EspinaWidget>;

} // namespace EspINA

class vtkEspinaCommand
: public vtkCommand
{
  public:
    vtkTypeMacro(vtkEspinaCommand, vtkCommand);

    /* \brief vtkEspinaCommand destructor.
     *
     */
    virtual ~vtkEspinaCommand()
    {}

    /* \brief Sets the widget this vtkCommand executes to.
     *
     */
    virtual void setWidget(EspINA::EspinaWidgetPtr widget) = 0;

    /* \brief Implements vtkCommand::Execute.
     *
     */
    virtual void Execute(vtkObject *, unsigned long int, void*) = 0;
};

#endif // ESPINA_WIDGET_H
