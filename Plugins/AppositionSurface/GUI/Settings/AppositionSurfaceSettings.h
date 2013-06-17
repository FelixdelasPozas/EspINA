/*
 * AppositionSurfaceSettings.h
 *
 *  Created on: Jan 16, 2013
 *      Author: F�lix de las Pozas �lvarez
 */

#ifndef APPOSITIONSURFACESETTINGS_H_
#define APPOSITIONSURFACESETTINGS_H_

#include "AppositionSurfacePlugin_Export.h"

// EspINA
#include <GUI/ISettingsPanel.h>
#include "ui_AppositionSurfaceSettings.h"

// Qt
#include <QColor>

namespace EspINA
{
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceSettings
  : public ISettingsPanel
  , public Ui::AppositionSurfaceSettings
  {
    Q_OBJECT
  public:
    explicit AppositionSurfaceSettings();
    virtual ~AppositionSurfaceSettings() {};

    virtual const QString shortDescription() { return tr("Synaptic Apposition Surface"); }
    virtual const QString longDescription()  { return tr("Synaptic Apposition Surface Settings"); }
    virtual const QIcon icon()               { return QIcon(":/AppSurface.svg"); }

    virtual void acceptChanges();
    virtual void rejectChanges();
    virtual bool modified() const;
    virtual ISettingsPanel *clone();

  public slots:
    void changeDefaultComputation(int);

  private:
    bool m_automaticComputation;
    bool m_modified;
  };
} /* namespace EspINA */

#endif /* APPOSITIONSURFACESETTINGS_H_ */
