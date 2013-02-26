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

// EspINA
#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IToolBar.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <GUI/ISettingsPanel.h>
#include <GUI/ViewManager.h>

// plugin
#include "Core/Extensions/AppositionSurfaceExtension.h"
#include "GUI/Renderer/AppositionSurfaceRenderer.h"

namespace EspINA
{
  class AppositionSurface
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

  public:
    explicit AppositionSurface();
    virtual ~AppositionSurface();

    virtual void initToolBar(EspinaModel *model,
                             QUndoStack  *undoStack,
                             ViewManager *viewManager);

    virtual void initFactoryExtension(EspinaFactoryPtr factory);

    virtual FilterSPtr createFilter(const QString              &filter,
                                    const Filter::NamedInputs  &inputs,
                                    const ModelItem::Arguments &args);

  public slots:
    void selectionChanged(ViewManager::Selection selection, bool unused);
    void segmentationAdded(SegmentationSPtr);
    virtual void reset();

  private:
    EspinaFactory                 *m_factory;
    EspinaModel                   *m_model;
    QUndoStack                    *m_undoStack;
    ViewManager                   *m_viewManager;
    QAction                       *m_action;
    ISettingsPanelPrototype        m_settings;
    AppositionSurfaceRendererSPtr  m_renderer;
    AppositionSurfaceExtensionSPtr m_extension;
  };

} // namespace EspINA

#endif// APPOSITIONSURFACE_H
