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

#ifndef APPOSITIONSURFACETOOLBAR_H
#define APPOSITIONSURFACETOOLBAR_H

// plugin
#include "Core/Extensions/AppositionSurfaceExtension.h"

// EspINA
#include <Core/Interfaces/IToolBar.h>
#include <GUI/ViewManager.h>
#include <GUI/ISettingsPanel.h>

namespace EspINA
{
  class AppositionSurfaceToolbar
  : public IToolBar
  {
    Q_OBJECT
  public:
    explicit AppositionSurfaceToolbar(EspinaModel *model,
                                      QUndoStack  *undoStack,
                                      ViewManager *viewManager);
    virtual ~AppositionSurfaceToolbar();


    // Re-implemented slots don't need to be so declared
    virtual void resetToolbar()   {}
    virtual void abortOperation() {}

  public slots:
    void selectionChanged(ViewManager::Selection selection, bool unused);

  private:
    static bool isSynapse(SegmentationPtr segmentation);

  private:
    EspinaModel                   *m_model;
    QUndoStack                    *m_undoStack;
    ViewManager                   *m_viewManager;
    QAction                       *m_action;
    ISettingsPanelPrototype        m_settings;
    AppositionSurfaceExtensionSPtr m_extension;
  };

} // namespace EspINA

#endif// APPOSITIONSURFACETOOLBAR_H
