/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

//----------------------------------------------------------------------------
// File:    MainToolBar.h
// Purpose: Provide tool buttons to:
//          - Toggle Segmentation Visibility
//          - Select Active Taxonomy
//          - Remove Segmentation by clicking on them
//----------------------------------------------------------------------------
#ifndef MAINTOOLBAR_H
#define MAINTOOLBAR_H

#include <Core/Interfaces/IToolBar.h>

#include <Core/EspinaTypes.h>

#include <Core/Model/EspinaModel.h>
#include <GUI/Pickers/IPicker.h>

#include <Tools/SegmentationRemover/SegmentationRemover.h>
#include <Tools/Measure/MeasureTool.h>

#include <QModelIndex>

class QUndoStack;
class QComboTreeView;

namespace EspINA
{
  class PixelPicker;
  class SegmentationRemover;
  class ViewManager;

  class MainToolBar
  : public IToolBar
  {
    Q_OBJECT
    Q_INTERFACES
    (
      EspINA::IToolBar
    )

  public:
    explicit MainToolBar(EspinaModelSPtr model,
                         QUndoStack    *undoStack,
                         ViewManager   *vm,
                         QWidget       *parent = 0);
    virtual ~MainToolBar();

    virtual void initToolBar(EspinaModelSPtr model,
                             QUndoStack     *undoStack,
                             ViewManager    *viewManager);

  public slots:
    void setShowSegmentations(bool visible);

    virtual void reset();

  protected slots:
    void setActiveTaxonomy(const QModelIndex &index);
    void updateTaxonomy(QModelIndex left, QModelIndex right);
    void removeSegmentation(bool active);
    void removeSegmentation(SegmentationPtr seg);
    void toggleCrosshair(bool);
    void abortRemoval();
    void toggleMeasureTool(bool);

  signals:
    void showSegmentations(bool);

  private:
    EspinaModelSPtr m_model;
    QUndoStack     *m_undoStack;
    ViewManager    *m_viewManager;

    QAction             *m_toggleSegVisibility;
    QAction             *m_removeSegmentation;
    QAction             *m_toggleCrosshair;
    QAction             *m_measureButton;
    QComboTreeView      *m_taxonomySelector;

    SegmentationRemoverSPtr m_segRemover;
    MeasureToolSPtr         m_measureTool;
  };

} // namespace EspINA

#endif // MAINTOOLBAR_H
