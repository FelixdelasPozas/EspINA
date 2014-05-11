/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>
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
 *
 */

#ifndef ESPINA_PIXEL_SELECTOR_H
#define ESPINA_PIXEL_SELECTOR_H

#include "GUI/Selectors/Selector.h"

class QSize;

namespace EspINA
{
  //------------------------------------------------------------------------
  class EspinaGUI_EXPORT PixelSelector
  : public Selector
  {
    protected:
      struct Invalid_Resolution_Exception {};

    public:
      explicit PixelSelector(NmVector3 resolution = NmVector3{1,1,1}) throw(Invalid_Resolution_Exception)
      : m_resolution{resolution}
      {
        if(!validResolution(resolution))
          throw Invalid_Resolution_Exception();
      }

      virtual ~PixelSelector(){}

      virtual void onMouseDown(const QPoint &pos, RenderView* view);

      virtual bool filterEvent(QEvent* e, RenderView* view = 0);

      virtual NmVector3 getPickPoint(RenderView *view);

      virtual Selector::Selection generateSelection(RenderView *);

      void setResolution(NmVector3 resolution) throw(Invalid_Resolution_Exception)
      {
        if(!validResolution(resolution))
          throw Invalid_Resolution_Exception();

        m_resolution = resolution;
      }

    protected:
      bool validResolution(NmVector3 resolution)
      { return (resolution[0] > 0) && (resolution[1] > 0) && (resolution[2] > 0); }

      NmVector3 m_resolution;
  };

  //------------------------------------------------------------------------
  class EspinaGUI_EXPORT BestPixelSelector
  : public PixelSelector
  {
  public:
    explicit BestPixelSelector(NmVector3 resolution = NmVector3{1,1,1});
    virtual ~BestPixelSelector();

    void setBestPixelValue(int value)
    {m_bestPixel = value;}

    virtual void onMouseDown(const QPoint &pos, RenderView* view);

    virtual NmVector3 getPickPoint(RenderView* view);

  private:
    QSize *m_window;
    int    m_bestPixel;
  };

} // namespace EspINA

#endif // ESPINA_PIXEL_SELECTOR_H
