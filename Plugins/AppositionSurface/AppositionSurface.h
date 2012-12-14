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

// Qt
#include <QColor>
#include <QSharedPointer>

class AppositionSurfaceExtension;
class AppositionSurfaceRenderer;

//! Apposition Surface Plugin
class AppositionSurface
: public QObject
, public IFactoryExtension
{
  Q_OBJECT
  Q_INTERFACES(IFactoryExtension)

public:
  explicit AppositionSurface();
  virtual ~AppositionSurface(){}

  virtual void initFactoryExtension(EspinaFactory* factory);

  class Settings;

  // to avoid using signals and declaring renderer and extensions as QObjects
  // every renderer and extensions registers with the plugin on creation
  // and unregisters on destruction
  void registerExtension(AppositionSurfaceExtension *);
  void unregisterExtension(AppositionSurfaceExtension *);
  void registerRenderer(AppositionSurfaceRenderer *);
  void unregisterRenderer(AppositionSurfaceRenderer *);

public slots:
  void propagateSettings();

private:
  QList<AppositionSurfaceRenderer *> m_renderersList;
  QList<AppositionSurfaceExtension *> m_extensionsList;
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
    virtual bool modified() const;
    virtual ISettingsPanel *clone();

  signals:
    void settingsChanged();

  public slots:
    void changeColor();
    void changeResolution(int);
    void changeIterations(int);
    void changeConverge(int);

  private:
    QColor m_color;
    int m_iterations;
    int m_resolution;
    bool m_converge;
    bool m_modified;
    AppositionSurface *m_plugin;
};

#endif// APPOSITIONSURFACE_H
