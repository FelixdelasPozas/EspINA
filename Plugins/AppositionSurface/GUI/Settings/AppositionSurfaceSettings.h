/*
 * AppositionSurfaceSettings.h
 *
 *  Created on: Jan 16, 2013
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef APPOSITIONSURFACESETTINGS_H_
#define APPOSITIONSURFACESETTINGS_H_

#include "AppositionSurfacePlugin_Export.h"

// ESPINA
#include <Support/Settings/SettingsPanel.h>
#include "ui_AppositionSurfaceSettings.h"

// Qt
#include <QColor>

namespace ESPINA
{
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceSettings
  : public SettingsPanel
  , public Ui::AppositionSurfaceSettings
  {
    Q_OBJECT
  public:
    explicit AppositionSurfaceSettings();
    virtual ~AppositionSurfaceSettings() {};

    virtual const QString shortDescription() { return tr("Synaptic Apposition Surface"); }
    virtual const QString longDescription()  { return tr("Synaptic Apposition Surface"); }
    virtual const QIcon icon()               { return QIcon(":/AppSurface.svg"); }

    virtual void acceptChanges();
    virtual void rejectChanges();
    virtual bool modified() const;

    virtual SettingsPanelPtr clone();

  public slots:
    void changeDefaultComputation(int);

  private:
    bool m_automaticComputation;
    bool m_modified;
  };
} /* namespace ESPINA */

#endif /* APPOSITIONSURFACESETTINGS_H_ */
