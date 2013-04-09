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
#ifndef SEEDGROWINGSEGMENTATION_H
#define SEEDGROWINGSEGMENTATION_H

#include <Core/Interfaces/IToolBar.h>
#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <Core/Interfaces/IDynamicMenu.h>

#include <GUI/ISettingsPanel.h>
#include <GUI/Pickers/ISelector.h>
#include <App/Tools/SeedGrowSegmentation/SeedGrowSegmentationTool.h>
#include <App/Tools/TubularSegmentation/TubularTool.h>
#include <GUI/ViewManager.h>

#include <QAction>

class QUndoStack;

class ActionSelector;
class DefaultVOIAction;
class ThresholdAction;

namespace EspINA
{
  //Forward declarations
  class SeedGrowSegmentationSettings;

  /// Seed Growing Segmentation Plugin
  class SegmentationToolBar
  : public IToolBar
  , public IFactoryExtension
  , public IFilterCreator
  , public IDynamicMenu
  {
    Q_OBJECT
    Q_INTERFACES
    (
      EspINA::IToolBar
      EspINA::IFactoryExtension
      EspINA::IFilterCreator
      EspINA::IDynamicMenu
    )

  public:
    SegmentationToolBar(EspinaModel *model,
                        QUndoStack  *undoStack,
                        ViewManager *vm,
                        QWidget     *parent=NULL);
    virtual ~SegmentationToolBar();

    virtual void initToolBar(EspinaModel *model,
                             QUndoStack  *undoStack,
                             ViewManager *viewManager);
    virtual void initFactoryExtension(EspinaFactory *factory);

    virtual FilterSPtr createFilter(const QString             &filter,
                                   const Filter::NamedInputs  &inputs,
                                   const ModelItem::Arguments &args);

    virtual QList<MenuEntry> menuEntries();

  protected slots:
    /// Change picker
    void changePicker(QAction *action);
    void cancelSegmentationOperation();
    void tubularActionStateChanged(bool segmenting);
    void showNodesInformation();
    void cancelTubularSegmentationOperation();

    virtual void reset();

    void batchMode();

  private:
    void addVoxelPicker(QAction *action, IPickerSPtr picker);
    void buildPickers();

  private:
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    ThresholdAction  *m_threshold;
    DefaultVOIAction *m_useDefaultVOI;
    ActionSelector   *m_pickerSelector;
    QAction          *m_tubularAction;

    TubularToolSPtr   m_tubularTool;

    QMap<QAction *, IPickerSPtr> m_pickers;
    SeedGrowSegmentationToolSPtr m_SGStool;

    SeedGrowSegmentationSettings *m_settings;
    ISettingsPanelPrototype m_SeedGrowSettingsPanel;
  };

} // namespace EspINA

#endif// SEEDGROWINGSEGMENTATION_H
