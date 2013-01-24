/*
 * AppositionSurfaceSettings.cpp
 *
 *  Created on: Jan 16, 2013
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "AppositionSurfaceSettings.h"
#include <Core/EspinaSettings.h>

// Qt
#include <QSettings>
#include <QColorDialog>

namespace EspINA
{
  
  //-----------------------------------------------------------------------------
  AppositionSurfaceSettings::AppositionSurfaceSettings()
  {
    setupUi(this);

    QSettings settings(CESVIMA, ESPINA);
    settings.beginGroup("Apposition Surface");

    if (settings.contains("Automatic Computation For Synapses"))
      m_automaticComputation = settings.value("Automatic Computation For Synapses").toBool();
    else
    {
      m_automaticComputation = false;
      settings.setValue("Automatic Computation For Synapses", m_automaticComputation);
    }
    settings.sync();

    m_modified = false;
    defaultComputation->setChecked(m_automaticComputation);
    connect(defaultComputation, SIGNAL(stateChanged(int)), this, SLOT(changeDefaultComputation(int)));
  }

  //-----------------------------------------------------------------------------
  void AppositionSurfaceSettings::changeDefaultComputation(int value)
  {
    m_automaticComputation = (Qt::Checked == value ? true : false);
    m_modified = true;
  }

  //-----------------------------------------------------------------------------
  void AppositionSurfaceSettings::acceptChanges()
  {
    if (!m_modified)
      return;

    QSettings settings(CESVIMA, ESPINA);
    settings.beginGroup("Apposition Surface");
    settings.setValue("Automatic Computation For Synapses", m_automaticComputation);
    settings.sync();
  }

  //-----------------------------------------------------------------------------
  void AppositionSurfaceSettings::rejectChanges()
  {
  }

  //-----------------------------------------------------------------------------
  bool AppositionSurfaceSettings::modified() const
  {
    return m_modified;
  }

  //-----------------------------------------------------------------------------
  ISettingsPanel *AppositionSurfaceSettings::clone()
  {
    return new AppositionSurfaceSettings();
  }

} /* namespace EspINA */
