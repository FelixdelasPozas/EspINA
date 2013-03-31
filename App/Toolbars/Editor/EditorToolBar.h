/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef EDITORTOOLBAR_H
#define EDITORTOOLBAR_H

#include <Core/Interfaces/IToolBar.h>
#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IFilterCreator.h>

#include "App/Tools/Brushes/Brush.h"

// EspINA
#include <Core/Model/Segmentation.h>
#include <GUI/ISettingsPanel.h>

class ActionSelector;
class QAction;
class QUndoStack;


namespace EspINA
{
  class ITool;
  class Brush;
  class ViewManager;
  class ContourSelector;
  class ContourWidget;
  class FreeFormSource;

  class EditorToolBar
  : public IToolBar
  , public IFactoryExtension
  , public IFilterCreator
  {
    Q_OBJECT
    Q_INTERFACES
    (
      EspINA::IToolBar
      EspINA::IFactoryExtension
      EspINA::IFilterCreator
    )

    class FreeFormCommand;
    class CODECommand;// Close/Open/Dilate/Erode Command

  public:
    class Settings;
    class SettingsPanel;

  public:
    explicit EditorToolBar(EspinaModel *model,
                           QUndoStack  *undoStack,
                           ViewManager *viewManager,
                           QWidget     *parent = 0);
    virtual ~EditorToolBar();

    virtual void initToolBar(EspinaModel *model,
                             QUndoStack  *undoStack,
                             ViewManager *viewManager);

    virtual void initFactoryExtension(EspinaFactory *factory);

    virtual FilterSPtr createFilter(const QString              &filter,
                                    const Filter::NamedInputs  &inputs,
                                    const ModelItem::Arguments &args);

  protected slots:
    void changeCircularBrushMode(Brush::BrushMode mode);
    void changeSphericalBrushMode(Brush::BrushMode mode);
    void changeContourMode(Brush::BrushMode mode);
    void changeDrawTool(QAction *action);
    void cancelDrawOperation();
    void changeSplitTool(QAction *action);
    void cancelSplitOperation();
    void combineSegmentations();
    void substractSegmentations();
    void closeSegmentations();
    void openSegmentations();
    void dilateSegmentations();
    void erodeSegmentations();
    void fillHoles();
    void updateAvailableOperations();

    virtual void reset();

  private:
    void initDrawTools();
    void initSplitTools();
    void initMorphologicalTools();
    void initCODETools();
    void initFillTool();

  private:
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    // GUI
    ActionSelector *m_drawToolSelector;
    QMap<QAction *, IToolSPtr> m_drawTools;

    ActionSelector *m_splitToolSelector;
    QMap<QAction *, IToolSPtr> m_splitTools;

    QAction *m_addition;
    QAction *m_substraction;

    QAction *m_erode;
    QAction *m_dilate;
    QAction *m_open;
    QAction *m_close;

    QAction *m_fill;
    ContourWidget *m_contourWidget;

    Settings      *m_settings;
    ISettingsPanelPrototype editorSettings;
  };

} // namespace EspINA


#endif // EDITORTOOLBAR_H
