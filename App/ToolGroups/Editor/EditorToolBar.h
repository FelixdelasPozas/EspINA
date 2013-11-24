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

#ifndef ESPINA_EDITOR_TOOL_H
#define ESPINA_EDITOR_TOOL_H

#include <Support/Tool.h>
#include <Support/ViewManager.h>

#include <GUI/Widgets/ActionSelector.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/ModelFactory.h>

class ActionSelector;
class QAction;
class QUndoStack;

namespace EspINA
{
  class ContourSelector;
  class ContourWidget;
  class FreeFormSource;
  class EditorToolBarSettings;

  class EditorToolBar
  : public Tool
  , public FilterFactory
  {
    Q_OBJECT

    class FreeFormCommand;
    class CODECommand;// Close/Open/Dilate/Erode Command

  public:
    class SettingsPanel;

  public:
    explicit EditorToolBar(ModelAdapterSPtr model,
                           ModelFactorySPtr factory,
                           ViewManagerSPtr  viewManager,
                           QUndoStack      *undoStack);
    virtual ~EditorToolBar();

    virtual FilterTypeList providedFilters() const;

    virtual FilterSPtr createFilter(OutputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const;

    virtual void initFactoryExtension(EspinaFactory *factory);

    virtual void setEnabled(bool value);

    virtual bool enabled() const;

    virtual QList<QAction *> actions() const;

  protected slots:
    void changeCircularBrushMode(Brush::BrushMode mode);
    void changeSphericalBrushMode(Brush::BrushMode mode);
    void changeContourMode(Brush::BrushMode mode);
    void changeDrawTool(QAction *action);
    void cancelDrawOperation();
    void changeSplitTool(QAction *action);
    void cancelSplitOperation();
    void mergeSegmentations();
    void subtractSegmentations();
    void closeSegmentations();
    void openSegmentations();
    void dilateSegmentations();
    void erodeSegmentations();
    void fillHoles();
    void updateAvailableOperations();
    void startContourOperation();

  private:
    void initDrawTools();
    void initSplitTools();
    void initMorphologicalTools();
    void initCODETools();
    void initFillTool();

  private:
    ModelAdapterSPtr m_model;
    ModelFactorySPtr m_factory;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;

    // GUI
    ActionSelector *m_drawToolSelector;
    QMap<QAction *, ToolSPtr> m_drawTools;

    ActionSelector *m_splitToolSelector;
    QMap<QAction *, ToolSPtr> m_splitTools;

    QAction *m_addition;
    QAction *m_subtract;

    QAction *m_erode;
    QAction *m_dilate;
    QAction *m_open;
    QAction *m_close;

    QAction *m_discTool;
    QAction *m_sphereTool;
    QAction *m_contourTool;

    QAction *m_fill;
    ContourWidget *m_contourWidget;

    EditorToolBarSettings  *m_settings;
    ISettingsPanelPrototype editorSettings;
  };

} // namespace EspINA


#endif // ESPINA_EDITOR_TOOL_H
