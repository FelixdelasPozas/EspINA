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
    public:
      explicit PixelSelector()
      {};

      virtual ~PixelSelector()
      {};

      virtual void onMouseDown(const QPoint &pos, RenderView* view);

      virtual bool filterEvent(QEvent* e, RenderView* view = 0);

      virtual NmVector3 getPickPoint(RenderView *view);

      virtual Selector::Selection generateSelection(RenderView *);

    protected:
      void transformDisplayToWorld(int x, int y, RenderView *view, NmVector3 &point, bool inSlice) const;
      bool validSelection(Selector::Selection);
  };

  //------------------------------------------------------------------------
  class EspinaGUI_EXPORT BestPixelSelector
  : public PixelSelector
  {
  public:
    explicit BestPixelSelector();
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
