#ifndef ISETTINGSPANEL_H
#define ISETTINGSPANEL_H

#include "EspinaGUI_Export.h"

#include <QWidget>
#include <QIcon>

#include <Core/EspinaTypes.h>

namespace EspINA
{
  class ISettingsPanel;
  typedef ISettingsPanel * ISettingsPanelPtr;

  class EspinaGUI_EXPORT ISettingsPanel
  : public QWidget
  {
  public:
    virtual ~ISettingsPanel(){}

    virtual const QString shortDescription() = 0;
    virtual const QString longDescription() = 0;
    virtual const QIcon icon() = 0;

    virtual void addPanel(ISettingsPanel *panel){}

    virtual void acceptChanges()  = 0;
    virtual void rejectChanges()  = 0;
    virtual bool modified() const = 0;

    virtual ISettingsPanelPtr clone() = 0;
  };

  typedef QList<ISettingsPanelPtr>          ISettingsPanelList;
  typedef boost::shared_ptr<ISettingsPanel> ISettingsPanelPrototype;
  typedef QList<ISettingsPanelPrototype>    ISettingsPanelPrototypeList;

} // namespace EspINA

#endif// ISETTINGSPANEL_H
