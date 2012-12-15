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

#include "AppositionSurface.h"

#include "AppositionSurfaceExtension.h"
#include "AppositionSurfaceRenderer.h"

// EspINA
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/EspinaSettings.h>

// Qt
#include <QColorDialog>
#include <QSettings>

//-----------------------------------------------------------------------------
AppositionSurface::AppositionSurface()
{
}

//-----------------------------------------------------------------------------
void AppositionSurface::initFactoryExtension(EspinaFactory* factory)
{
  factory->registerSettingsPanel(new AppositionSurface::Settings(this));

  QSettings settings(CESVIMA, ESPINA);
  settings.beginGroup("Apposition Surface");
  QColor color = settings.value("Surface Color").value<QColor>();

  SegmentationExtension::SPtr segExtension(new AppositionSurfaceExtension());
  factory->registerSegmentationExtension(segExtension);
  factory->registerRenderer(new AppositionSurfaceRenderer(color, this));
}

//-----------------------------------------------------------------------------
void AppositionSurface::registerRenderer(AppositionSurfaceRenderer *renderer)
{
  if (!m_renderersList.contains(renderer))
    m_renderersList.append(renderer);
}

//-----------------------------------------------------------------------------
void AppositionSurface::unregisterRenderer(AppositionSurfaceRenderer *renderer)
{
  if (m_renderersList.contains(renderer))
    m_renderersList.removeOne(renderer);
}

//-----------------------------------------------------------------------------
void AppositionSurface::propagateSettings()
{
  QSettings settings(CESVIMA, ESPINA);
  settings.beginGroup("Apposition Surface");
  QColor color = settings.value("Surface Color").value<QColor>();

  QList<AppositionSurfaceRenderer*>::iterator it = m_renderersList.begin();
  while (it != m_renderersList.end())
  {
    (*it)->SetColor(color);
    ++it;
  }
}

//-----------------------------------------------------------------------------
AppositionSurface::Settings::Settings(AppositionSurface *plugin)
: m_plugin(plugin)
{
  setupUi(this);

  connect(this, SIGNAL(settingsChanged()), m_plugin, SLOT(propagateSettings()));

  QSettings settings(CESVIMA, ESPINA);
  settings.beginGroup("Apposition Surface");

  if (settings.contains("Surface Color"))
    m_color = settings.value("Surface Color").value<QColor>();
  else
  {
    m_color = Qt::red;
    settings.setValue("Surface Color", m_color);
  }
  settings.sync();

  m_modified = false;

  QPixmap pixmap(19,19);
  pixmap.fill(QColor(m_color));
  colorButton->setIcon(QIcon(pixmap));

  connect(colorButton, SIGNAL(pressed()), this, SLOT(changeColor()));
}

//-----------------------------------------------------------------------------
void AppositionSurface::Settings::changeColor()
{
  QColorDialog colorDialog;
  colorDialog.setOptions(QColorDialog::DontUseNativeDialog);

  QColor color = colorDialog.getColor(m_color);

  if (color != m_color)
  {
    m_color = color;
    m_modified = true;

    QPixmap pixmap(19,19);
    pixmap.fill(QColor(color));
    colorButton->setIcon(QIcon(pixmap));
  }
}

//-----------------------------------------------------------------------------
void AppositionSurface::Settings::acceptChanges()
{
  QSettings settings(CESVIMA, ESPINA);
  settings.beginGroup("Apposition Surface");
  settings.setValue("Surface Color", m_color);
  settings.sync();

  emit settingsChanged();
}

//-----------------------------------------------------------------------------
bool AppositionSurface::Settings::modified() const
{
  return m_modified;
}

//-----------------------------------------------------------------------------
ISettingsPanel *AppositionSurface::Settings::clone()
{
  return new AppositionSurface::Settings(m_plugin);
}

Q_EXPORT_PLUGIN2(AppositionSurfacePlugin, AppositionSurface)


