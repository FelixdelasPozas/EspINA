/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_SLICE_SELECTOR_WIDGET_H
#define ESPINA_SLICE_SELECTOR_WIDGET_H

#include "GUI/EspinaGUI_Export.h"

#include <memory>
// Qt
#include <Core/Utils/Spatial.h>
#include <QObject>

namespace ESPINA
{

class RenderView;
  class SliceSelector;
  using SliceSelectorSPtr = std::shared_ptr<SliceSelector>;

  enum SliceSelectionTypes
  {
    None=0x0, From = 0x1, To = 0x2
  };
  Q_DECLARE_FLAGS(SliceSelectionType, SliceSelectionTypes)


  class EspinaGUI_EXPORT SliceSelector
  : public QObject
  {
  public:
    /** \brief SliceSelectorWidget class constructor.
     *
     */
    virtual ~SliceSelector()
    {}

    /** \brief Returns the left widget raw pointer.
     *
     */
    virtual QWidget *lowerWidget () const = 0;

    /** \brief Returns the right widget raw pointer.
     *
     */
    virtual QWidget *rightWidget() const = 0;

    /** \brief Returns a raw pointer to a new instance of the class.
     *
     */
    virtual SliceSelectorSPtr clone(RenderView *view, Plane plane) = 0;

  protected:
    /** \brief SliceSelectorWidget class constructor.
     *
     */
    explicit SliceSelector()
    {}
  };


  Q_DECLARE_OPERATORS_FOR_FLAGS(SliceSelectionType)
} // namespace ESPINA

#endif // ESPINA_SLICE_SELECTOR_WIDGET_H
