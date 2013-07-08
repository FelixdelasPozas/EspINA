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

#ifndef PIXELSELECTOR_H_
#define PIXELSELECTOR_H_

#include "EspinaGUI_Export.h"

#include "GUI/Pickers/ISelector.h"

class QSize;

namespace EspINA
{
  class EspinaGUI_EXPORT PixelSelector
  : public ISelector
  {
  public:
    explicit PixelSelector() {}
    virtual ~PixelSelector(){}

    virtual void onMouseDown(const QPoint &pos, EspinaRenderView* view);
    virtual bool filterEvent(QEvent* e, EspinaRenderView* view = 0);
    /// NOTE: It is user responsability to free the pointer returned
    virtual double *getPickPoint(EspinaRenderView *view);
    virtual ISelector::PickList generatePickList(EspinaRenderView*);
  };


  class EspinaGUI_EXPORT BestPixelSelector
  : public PixelSelector
  {
  public:
    explicit BestPixelSelector();
    virtual ~BestPixelSelector();

    void setBestPixelValue(int value) {m_bestPixel = value;}

    virtual void onMouseDown(const QPoint& pos, EspinaRenderView* view);
    /// NOTE: It is user responsability to free the pointer returned
    virtual double *getPickPoint(EspinaRenderView *view);

  private:
    QSize *m_window;
    int    m_bestPixel;
  };

} // namespace EspINA

#endif //PIXELSELECTOR_H_