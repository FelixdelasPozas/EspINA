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

#ifndef ESPINA_PIXEL_SELECTOR_H
#define ESPINA_PIXEL_SELECTOR_H

// ESPINA
#include "GUI/Selectors/Selector.h"

class QSize;

namespace ESPINA
{
  //------------------------------------------------------------------------
  class EspinaGUI_EXPORT PixelSelector
  : public Selector
  {
    public:
  		/** \brief PixelSelector class constructor.
  		 *
  		 */
      explicit PixelSelector()
      {};

  		/** \brief PixelSelector class destructor.
  		 *
  		 */
      virtual ~PixelSelector()
      {};

  		/** \brief Generate a selection when the user pushes a mouse button.
  		 * \param[in] pos, position of the mouse.
  		 * \param[in] view, view where the user presses the mouse button.
  		 *
  		 */
      virtual void onMouseDown(const QPoint &pos, RenderView* view);

  		/** \brief Overrides EventHandler::filterEvent().
  		 *
  		 */
      virtual bool filterEvent(QEvent* e, RenderView* view = nullptr) override;

  		/** \brief Returns the point picked by the user.
  		 *
  		 */
      virtual NmVector3 getPickPoint(RenderView *view);

  		/** \brief Generate a selection in the given view using the current mouse position.
  		 * \param[in] view, raw pointer of the RenderView to generate the selection.
  		 *
  		 */
      virtual Selector::Selection generateSelection(RenderView *view);

    protected:
  		/** \brief Transforms display coordinates to world coordinates.
  		 * \param[in] x, x coordinate.
  		 * \param[in] y, y coordinate.
  		 * \param[in] view, raw pointer of the RenderView to compute the world coordinates.
  		 * \param[out] point, result.
  		 * \param[in] inSlice, true to set the point in the current slice of the view, false otherwise.
  		 *
  		 */
      void transformDisplayToWorld(int x, int y, RenderView *view, NmVector3 &point, bool inSlice) const;

  		/** \brief Returns true if the selection given is valid.
  		 * \param[in] selection, list of selected items.
  		 *
  		 */
      bool validSelection(Selector::Selection selection);
  };

  using PixelSelectorSPtr = std::shared_ptr<PixelSelector>;

  //------------------------------------------------------------------------
  class EspinaGUI_EXPORT BestPixelSelector
  : public PixelSelector
  {
    Q_OBJECT
  public:
    /** \brief BestPixelSelector class constructor.
     *
     */
    explicit BestPixelSelector();

    /** \brief BestPixelSelector class constructor.
     *
     */
    virtual ~BestPixelSelector();

    virtual void onMouseDown(const QPoint &pos, RenderView* view) override;

    virtual NmVector3 getPickPoint(RenderView* view) override;

  public slots:
    /** \brief Sets the color of the best pixel for the selector.
     *
     */
    void setBestPixelValue(int value)
    { m_bestPixel = value; }

  private:
    QSize *m_window;
    int    m_bestPixel;
  };

  using BestPixelSelectorSPtr = std::shared_ptr<BestPixelSelector>;

} // namespace ESPINA

#endif // ESPINA_PIXEL_SELECTOR_H
