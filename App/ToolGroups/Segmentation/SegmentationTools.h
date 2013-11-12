/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
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
 */
#ifndef ESPINA_SEGMENTATION_TOOLS_H
#define ESPINA_SEGMENTATION_TOOLS_H

#include <Support/ToolGroup.h>
#include <GUI/Model/ModelAdapter.h>

#include "SeedGrowSegmentationTool.h"

#include <QAction>

class QUndoStack;

namespace EspINA
{
  /// Seed Growing Segmentation Plugin
  class SegmentationTools
  : public ToolGroup
//   : public IToolBar
//   , public IFactoryExtension
//   , public IFilterCreator
//   , public IDynamicMenu
  {
    Q_OBJECT
//     Q_INTERFACES
//     (
//       EspINA::IFactoryExtension
//       EspINA::IFilterCreator
//       EspINA::IDynamicMenu
//     )

  public:
    SegmentationTools(ModelAdapterSPtr model,
                      ViewManagerSPtr  viewManager,
                      QUndoStack      *undoStack,
                      QWidget         *parent=nullptr);
    virtual ~SegmentationTools();

    virtual void setActiveTool(ToolSPtr tool);

    virtual void setEnabled(bool value);

    virtual bool enabled() const;

    virtual void setInUse(bool value);

    virtual ToolSList tools();

    virtual SelectorSPtr selector() const;

  private:
    SeedGrowSegmentationToolSPtr m_sgsTool;
//     virtual void initFactoryExtension(EspinaFactory *factory);
// 
//     virtual FilterSPtr createFilter(const QString             &filter,
//                                    const Filter::NamedInputs  &inputs,
//                                    const ModelItem::Arguments &args);
// 
//     virtual QList<MenuEntry> menuEntries();
// 
// 
//     virtual void resetToolbar();
// 
//     virtual void abortOperation() {};
// 
//     virtual void resetMenus() {}

//   protected slots:
//     /// Change picker
//     void changePicker(QAction *action);
//     void cancelSegmentationOperation();
//     void tubularActionStateChanged(bool segmenting);
//     void showNodesInformation();
//     void cancelTubularSegmentationOperation();
// 
//     void batchMode();
// 
//   private:
//     void addVoxelPicker(QAction *action, IPickerSPtr picker);
//     void buildPickers();
// 
//   private:
//     ModelAdapter *m_model;
//     QUndoStack  *m_undoStack;
//     ViewManager *m_viewManager;
// 
//     ThresholdAction  *m_threshold;
//     DefaultVOIAction *m_useDefaultVOI;
//     ActionSelector   *m_pickerSelector;
//     QAction          *m_tubularAction;
// 
//     TubularToolSPtr   m_tubularTool;
// 
//     QMap<QAction *, IPickerSPtr> m_pickers;
//     SeedGrowSegmentationToolSPtr m_SGStool;
// 
//     SeedGrowSegmentationSettings *m_settings;
//     ISettingsPanelPrototype m_SeedGrowSettingsPanel;
  };

} // namespace EspINA

#endif// ESPINA_SEGMENTATION_TOOLS_H
