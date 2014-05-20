/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_CIRCULAR_BRUSH_ROI_SELECTOR_H_
#define ESPINA_CIRCULAR_BRUSH_ROI_SELECTOR_H_

#include <App/Tools/Brushes/CircularBrushSelector.h>

namespace EspINA
{
  
  class CircularBrushROISelector
  : public CircularBrushSelector
  {
    public:
      /* \brief CircularBrushROISelector class constructor.
       *
       */
      explicit CircularBrushROISelector();

      /* \brief CircularBrushROISelector class virtual destructor.
       *
       */
      virtual ~CircularBrushROISelector();

      /* \brief Implements BrushSelector::filterEvent().
       *
       */
      virtual bool filterEvent(QEvent* e, RenderView* view = nullptr);

      /* \brief Method to set the bool to avoi erasing when there isnt a ROI
       *
       */
      void setHasROI(bool value)
      { m_hasROI = value; }

    protected:
      /* \brief Implements BrushSelector::startPreview().
       *
       */
      virtual void startPreview(RenderView *view);

      /* \brief Implements BrushSelector::updatePreview().
       *
       */
      virtual void updatePreview(NmVector3 center, RenderView *view);

      /* \brief Implements BrushSelector::stopStroke().
       *
       */
      virtual void stopStroke(RenderView* view);

    private:
      bool m_hasROI;
  };

  using CircularBrushROISelectorSPtr = std::shared_ptr<CircularBrushROISelector>;

} // namespace EspINA

#endif // ESPINA_CIRCULAR_BRUSH_ROI_SELECTOR_H_
