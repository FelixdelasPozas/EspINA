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
#include <GUI/ISettingsPanel.h>

// plugin
#include "ui_AppositionSurfaceSettings.h"
#include "AppositionSurfaceExtension.h"
#include "AppositionSurfaceRenderer.h"

// Qt
#include <QColor>
#include <QSharedPointer>

namespace EspINA
{
  class AppositionSurfaceExtension;
  class AppositionSurfaceRenderer;

  //! Apposition Surface Plugin
  class AppositionSurface
  : public QObject
  , public IFactoryExtension
  {
    Q_OBJECT
    Q_INTERFACES
    (
      EspINA::IFactoryExtension
    )

  public:
    explicit AppositionSurface();
    virtual ~AppositionSurface();

    virtual void initFactoryExtension(EspinaFactoryPtr factory);

    class Settings;

    // to avoid using signals and declaring renderer as a QObject every
    // renderer registers with the plugin on creation and unregisters
    // on destruction
    void registerRenderer(AppositionSurfaceRenderer *);
    void unregisterRenderer(AppositionSurfaceRenderer *);

  public slots:
    void propagateSettings();

  private:
    EspinaFactory *m_factory;
    QList<AppositionSurfaceRenderer *> m_renderersList;

    ISettingsPanelPrototype        m_settings;
    AppositionSurfaceExtensionSPtr m_segmentationExtension;
    AppositionSurfaceRendererSPtr  m_renderer;
  };

  class AppositionSurface::Settings
  : public ISettingsPanel
  , public Ui::AppositionSurfaceSettings
  {
    Q_OBJECT
  public:
    explicit Settings(AppositionSurface *);
    virtual ~Settings()                      {};

    virtual const QString shortDescription() { return tr("Apposition Surface"); }
    virtual const QString longDescription()  { return tr("Apposition Surface Settings"); }
    virtual const QIcon icon()               { return QIcon(":/AppSurface.svg"); }

    virtual void acceptChanges();
    virtual void rejectChanges();
    virtual bool modified() const;
    virtual ISettingsPanel *clone();

  signals:
    void settingsChanged();

  public slots:
    void changeColor();

  private:
    QColor m_color;
    bool m_modified;

    AppositionSurface *m_plugin;
  };

} // namespace EspINA

#endif// APPOSITIONSURFACE_H
