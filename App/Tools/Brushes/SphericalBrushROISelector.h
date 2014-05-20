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

#ifndef ESPINA_SPHERICAL_BRUSH_ROI_SELECTOR_H_
#define ESPINA_SPHERICAL_BRUSH_ROI_SELECTOR_H_

#include <Tools/Brushes/SphericalBrushSelector.h>

namespace EspINA
{
  
  class SphericalBrushROISelector
  : public SphericalBrushSelector
  {
    public:
      /* \brief SphericalBrushROISelector class constructor.
       *
       */
      explicit SphericalBrushROISelector();

      /* \brief SphericalBrushROISelector class virtual destructor.
       *
       */
      virtual ~SphericalBrushROISelector();

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

  using SphericalBrushROISelectorSPtr = std::shared_ptr<SphericalBrushROISelector>;

} // namespace EspINA

#endif // ESPINA_SPHERICAL_BRUSH_ROI_SELECTOR_H_
