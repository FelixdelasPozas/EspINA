/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_CONTOUR_SELECTOR_H_
#define ESPINA_CONTOUR_SELECTOR_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Selectors/BrushSelector.h>
#include <GUI/View/Widgets/Contour/ContourWidget.h>

// Qt
#include <QObject>

namespace ESPINA
{
  class EspinaGUI_EXPORT ContourSelector
  : public BrushSelector
  {
    Q_OBJECT
    public:
      /** \brief ContourSelector class constructor.
       *
       */
      explicit ContourSelector();

      /** \brief ContourSelector class virtual destructor.
       *
       */
      virtual ~ContourSelector()
      {}

      virtual bool filterEvent(QEvent* e, RenderView *view = nullptr) override;

      /** \brief Sets the contour widget that has the vtkWidgets for the interaction.
       * \param[in] widget ContourWidget raw pointer.
       *
       */
      void setContourWidget(ContourWidgetPtr widget)
      { m_widget = widget; }

    protected slots:
      virtual BrushShape createBrushShape(ViewItemAdapterPtr item,
                                          NmVector3 center,
                                          Nm radius,
                                          Plane plane)
      { return BrushShape(); };

    protected:
      virtual void buildCursor() override;

      ContourWidgetPtr m_widget;
  };

  using ContourSelectorPtr  = ContourSelector *;
  using ContourSelectorSPtr = std::shared_ptr<ContourSelector>;

} // namespace ESPINA

#endif // ESPINA_CONTOUR_SELECTOR_H_
