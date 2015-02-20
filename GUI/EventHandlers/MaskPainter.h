/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_MASK_PAINTER_H
#define ESPINA_MASK_PAINTER_H

#include <GUI/EventHandlers/PointTracker.h>
#include <Core/Utils/BinaryMask.hxx>
#include "StrokePainter.h"

namespace ESPINA
{
  class MaskPainter
  : public EventHandler
  {
    Q_OBJECT
  public:

  public:
    virtual bool filterEvent(QEvent *e, RenderView *view = nullptr);

    void setCanErase(bool value);

    void setDrawingMode(const DrawingMode mode);

    void setColor(const QColor &color);

    PointTrackerSPtr pointTracker() const;

    void setMaskProperties(const NmVector3 &spacing, const NmVector3 &origin=NmVector3());

  protected:
    explicit MaskPainter(PointTrackerSPtr handler);

    /** \brief This methods takes into account the shift modifier
     *
     */
    DrawingMode currentMode() const;

  signals:
    void drawingModeChanged(DrawingMode mode);

    void startPainting();

    void stopPaining(BinaryMaskSPtr<unsigned char> mask);

  private:
    void updateDrawingMode();

    /** \brief Returns true when the shift key is down in the keyboard.
     *
     */
    inline bool ShiftKeyIsDown() const;

    virtual void updateCursor(DrawingMode mode) = 0;

    virtual void onMaskPropertiesChanged(const NmVector3 &spacing, const NmVector3 &origin=NmVector3()) = 0;

  protected:
    NmVector3 m_origin;
    NmVector3 m_spacing;

    QColor    m_color;

  private:
    PointTrackerSPtr m_tracker;

    bool        m_canErase;
    DrawingMode m_mode;
  };

  using MaskPainterSPtr = std::shared_ptr<MaskPainter>;
}

#endif // ESPINA_MASKPAINTER_H
