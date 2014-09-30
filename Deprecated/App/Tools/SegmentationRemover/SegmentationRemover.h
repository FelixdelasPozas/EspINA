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


#ifndef SEGREMOVER_H
#define SEGREMOVER_H

#include <GUI/Tools/ITool.h>
#include <GUI/Pickers/ISelector.h>

namespace ESPINA
{
  class PixelSelector;

  class SegmentationRemover
  : public ITool
  {
    Q_OBJECT
  public:
    explicit SegmentationRemover();
    virtual ~SegmentationRemover();

    virtual QCursor cursor() const;
    virtual bool filterEvent(QEvent* e, EspinaRenderView* view = 0);
    virtual void setInUse(bool enable);
    virtual void setEnabled(bool enable);
    virtual bool enabled() const;

  private slots:
    void removeSegmentation(ISelector::PickList pickedSeg);

  signals:
    void removeSegmentation(SegmentationPtr);
    void removalAborted();

  private:
    PixelSelector *m_picker;
  };

  typedef boost::shared_ptr<SegmentationRemover> SegmentationRemoverSPtr;

} // namespace ESPINA

#endif // SEGREMOVER_H