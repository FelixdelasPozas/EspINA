/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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
 */


#ifndef ESPINA_SEGMENTATION_INFORMATION_H
#define ESPINA_SEGMENTATION_INFORMATION_H

#include <Support/Widgets/DockWidget.h>
#include <Support/Factory/FilterRefinerRegister.h>

class QUndoStack;

namespace ESPINA
{
  class SegmentationInformation
  : public DockWidget
  , private Support::WithContext
  {
    Q_OBJECT
  public:
    explicit SegmentationInformation(Support::FilterRefinerRegister &filterRefiners,
                                     Support::Context               &context);
    virtual ~SegmentationInformation();

    virtual void showEvent(QShowEvent* event);

    virtual void hideEvent(QHideEvent* event);

    virtual void reset();

  private slots:
    void onSelectionChanged(SegmentationAdapterList selection);

    void onOutputModified();

    void manageTags();

    void onNotesModified();

  private:
    void showInformation(SegmentationAdapterPtr segmentation);

    void hideInformation();

    void showSegmentationName();

    void clearSegmentationName();

    void addRefineWidget();

    void removeRefineWidget();

    void showTags();

    void clearTags();

    void showNotes();

    void clearNotes();

  private:
    class UI;

  private:
    Support::FilterRefinerRegister &m_register;

    FilterSPtr             m_filter;
    SegmentationAdapterPtr m_segmentation;

    UI *m_gui;
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_INFORMATION_H
