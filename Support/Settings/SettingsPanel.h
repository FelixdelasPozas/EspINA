#ifndef ESPINA_ISETTINGSPANEL_H
#define ESPINA_ISETTINGSPANEL_H

#include "GUI/EspinaGUI_Export.h"

#include <QWidget>
#include <QIcon>

#include <Core/EspinaTypes.h>

namespace EspINA
{
  class SettingsPanel;

  using SettingsPanelPtr   = SettingsPanel *;
  using SettingsPanelList  = QList<SettingsPanelPtr>;
  using SettingsPanelSPtr  = std::shared_ptr<SettingsPanel>;
  using SettingsPanelSList = QList<SettingsPanelSPtr>;

  class EspinaGUI_EXPORT SettingsPanel
  : public QWidget
  {
  public:
    virtual ~SettingsPanel(){}

    virtual const QString shortDescription() = 0;
    virtual const QString longDescription() = 0;
    virtual const QIcon icon() = 0;

    virtual void addPanel(SettingsPanel *panel) {}

    virtual void acceptChanges()  = 0;
    virtual void rejectChanges()  = 0;
    virtual bool modified() const = 0;

    virtual SettingsPanelPtr clone() = 0;
  };
} // namespace EspINA

#endif// ISETTINGSPANEL_H
