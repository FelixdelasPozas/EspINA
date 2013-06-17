/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
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

#ifndef APPOSITIONSURFACE_H
#define APPOSITIONSURFACE_H

#include "AppositionSurfacePlugin_Export.h"

// Plugin
#include "Core/Extensions/AppositionSurfaceExtension.h"

// EspINA
#include <Core/Interfaces/IDynamicMenu.h>
#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <Core/Interfaces/IToolBarFactory.h>
#include <GUI/ISettingsPanel.h>
#include <GUI/ViewManager.h>

namespace EspINA
{
  class AppositionSurfacePlugin_EXPORT AppositionSurface
  : public QObject
  , public IToolBarFactory
  , public IFactoryExtension
  , public IFilterCreator
  , public IDynamicMenu
  {
    Q_OBJECT
    Q_INTERFACES
    (
      EspINA::IToolBarFactory
      EspINA::IFactoryExtension
      EspINA::IFilterCreator
      EspINA::IDynamicMenu
    )

  public:
    explicit AppositionSurface();
    virtual ~AppositionSurface();

    virtual void initToolBarFactory(EspinaModel *model,
                                    QUndoStack  *undoStack,
                                    ViewManager *viewManager);

    virtual void initFactoryExtension(EspinaFactory *factory);

    virtual QList<IToolBar *> toolBars() const;

    virtual FilterSPtr createFilter(const QString              &filter,
                                    const Filter::NamedInputs  &inputs,
                                    const ModelItem::Arguments &args);

    virtual QList<MenuEntry> menuEntries();

    virtual void resetMenus() {}

  public slots:
    void createSynapticAppositionSurfaceAnalysis();

    void segmentationAdded(SegmentationSPtr segmentation);

  private:
    static bool isSynapse(SegmentationPtr segmentation);

  private:
    EspinaFactory                 *m_factory;
    EspinaModel                   *m_model;
    QUndoStack                    *m_undoStack;
    ViewManager                   *m_viewManager;
    IToolBar                      *m_toolbar;
    ISettingsPanelPrototype        m_settings;
    AppositionSurfaceExtensionSPtr m_extension;
  };

} // namespace EspINA

#endif// APPOSITIONSURFACE_H
