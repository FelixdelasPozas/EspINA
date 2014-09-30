/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#include "GUI/EspinaGUI_Export.h"

// VTK
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// C++
#include <memory>

namespace ESPINA
{
  class RenderView;

  class EspinaGUI_EXPORT EspinaWidget
  {
  public:
  	/** \brief EspinaWidget class constructor.
  	 *
  	 */
    explicit EspinaWidget()
    {}

    /** \brief EspinaWidget class virtual destructor.
     *
     */
    virtual ~EspinaWidget()
    {}

    /** \brief Registers the specified view.
     * \brief view, raw pointer of the render view to register.
     *
     */
    virtual void registerView  (RenderView *view) = 0;

    /** \brief Unregisters the specified view.
     * \brief view, raw pointer of the render view to unregister.
     *
     */
    virtual void unregisterView(RenderView *view) = 0;

    /** \brief Enables/disables the widget.
     * \param[in] enable, true to enable, false otherwise.
     *
     */
    virtual void setEnabled(bool enable) = 0;

    /** \brief Returns true if the widget manipulates the segmentations in any way.
     *
     * Useful to disable the widget when the segmentation representations change.
     *
     */
    virtual bool manipulatesSegmentations() const
    { return false; };
  };

  using EspinaWidgetPtr  = EspinaWidget *;
  using EspinaWidgetSPtr = std::shared_ptr<EspinaWidget>;

} // namespace ESPINA

class vtkEspinaCommand
: public vtkCommand
{
  public:
    vtkTypeMacro(vtkEspinaCommand, vtkCommand);

    /** \brief vtkEspinaCommand destructor.
     *
     */
    virtual ~vtkEspinaCommand()
    {}

    /** \brief Sets the widget this vtkCommand executes to.
     * \param[in] widget EspinaWidget raw pointer.
     *
     */
    virtual void setWidget(ESPINA::EspinaWidgetPtr widget) = 0;

    /** \brief Implements vtkCommand::Execute.
     *
     */
    virtual void Execute(vtkObject *, unsigned long int, void*) = 0;
};

#endif // ESPINA_WIDGET_H
