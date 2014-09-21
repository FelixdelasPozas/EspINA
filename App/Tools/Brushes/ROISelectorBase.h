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

#ifndef ESPINA_ROI_SELECTOR_BASE_H_
#define ESPINA_ROI_SELECTOR_BASE_H_

#include <GUI/Selectors/BrushSelector.h>

namespace ESPINA
{

  class ROISelectorBase
  : public BrushSelector
  {
    public:
      /** \brief ROISelectorBase class constructor.
       *
       */
      explicit ROISelectorBase();

      /** \brief ROISelectorBase class virtual destructor.
       *
       */
      virtual ~ROISelectorBase();

      /** \brief Implements BrushSelector::filterEvent().
       *
       */
      virtual bool filterEvent(QEvent* e, RenderView* view = nullptr);

      /** \brief Method to set the bool to avoid erasing when there isn't a ROI
       *
       */
      void setHasROI(bool value)
      { m_hasROI = value; }

    protected:
      /** \brief Implements BrushSelector::startPreview().
       *
       */
      virtual void startPreview(RenderView *view);

      /** \brief Implements BrushSelector::updatePreview().
       *
       */
      virtual void updatePreview(BrushShape shape, RenderView *view);

      /** \brief Implements BrushSelector::stopStroke().
       *
       */
      virtual void stopStroke(RenderView* view);

    private:
      bool m_hasROI;
  };

} // namespace ESPINA

#endif // ESPINA_ROI_SELECTOR_BASE_H_
