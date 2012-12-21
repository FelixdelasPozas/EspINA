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

#include <Core/Model/EspinaModel.h>
#include <GUI/ISettingsPanel.h>
#include <GUI/Pickers/IPicker.h>
#include <Tools/SeedGrowSegmentation/SeedGrowSegmentationTool.h>

class QAction;
class QUndoStack;

class ActionSelector;
class DefaultVOIAction;
class ThresholdAction;

namespace EspINA
{
  //Forward declarations
  class SeedGrowSegmentationSettings;
  class ViewManager;

  /// Seed Growing Segmenation Plugin
  class SeedGrowSegmentation
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

    class SettingsPanel;

  public:
    SeedGrowSegmentation(EspinaModelSPtr model,
                         QUndoStack     *undoStack,
                         ViewManager    *vm,
                         QWidget        *parent=NULL);
    virtual ~SeedGrowSegmentation();

    virtual void initToolBar(EspinaModelSPtr model,
                             QUndoStack     *undoStack,
                             ViewManager    *viewManager);
    virtual void initFactoryExtension(EspinaFactoryPtr factory);

    virtual FilterSPtr createFilter(const QString              &filter,
                                   const Filter::NamedInputs  &inputs,
                                   const ModelItem::Arguments &args);

  protected slots:
    /// Change picker
    void changePicker(QAction *action);
    void cancelSegmentationOperation();

    virtual void reset();

    void batchMode();

  private:
    void addVoxelPicker(QAction *action, IPickerSPtr picker);
    void buildPickers();

  private:
    EspinaModelSPtr m_model;
    QUndoStack     *m_undoStack;
    ViewManager    *m_viewManager;

    ThresholdAction  *m_threshold;
    DefaultVOIAction *m_useDefaultVOI;
    ActionSelector   *m_pickerSelector;

    QMap<QAction *, IPickerSPtr> m_pickers;
    SeedGrowSegmentationToolSPtr m_tool;

    SeedGrowSegmentationSettings *m_settings;
    ISettingsPanelPrototype m_settingsPanel;
  };

} // namespace EspINA

#endif// SEEDGROWINGSEGMENTATION_H
