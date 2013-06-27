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

// EspINA
#include <Core/Interfaces/IToolBar.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>
#include <GUI/Pickers/ISelector.h>
#include <Tools/SegmentationRemover/SegmentationRemover.h>
#include <Tools/Measure/MeasureTool.h>
#include <Tools/Ruler/RulerTool.h>

// Qt
#include <QModelIndex>

class QUndoStack;
class QComboTreeView;

namespace EspINA
{
  class PixelSelector;
  class SegmentationRemover;
  class ViewManager;

  class MainToolBar
  : public IToolBar
  {
    Q_OBJECT
  public:
    explicit MainToolBar(EspinaModel *model,
                         QUndoStack  *undoStack,
                         ViewManager *viewManager,
                         QWidget     *parent = 0);
    virtual ~MainToolBar();

  virtual void resetToolbar();// slot

  public slots:
    void setShowSegmentations(bool visible);


  protected slots:
    void setActiveTaxonomy(const QModelIndex &index);
    void updateTaxonomy(TaxonomySPtr taxonomy);
    void removeSegmentation(bool active);
    void removeSegmentation(SegmentationPtr seg);
    void toggleCrosshair(bool);
    void abortRemoval();
    void toggleMeasureTool(bool);
    void toggleRuler(bool);
    void resetRootItem();
    void abortOperation();

  signals:
    void showSegmentations(bool);

  private:
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    QAction             *m_toggleSegVisibility;
    QAction             *m_removeSegmentation;
    QAction             *m_toggleCrosshair;
    QAction             *m_measureButton;
    QAction             *m_rulerButton;
    QComboTreeView      *m_taxonomySelector;

    SegmentationRemoverSPtr m_segRemover;
    MeasureToolSPtr         m_measureTool;
    RulerToolSPtr           m_rulerTool;
  };

} // namespace EspINA

#endif // MAINTOOLBAR_H
