#ifndef ISETTINGSPANEL_H
#define ISETTINGSPANEL_H

#include <QWidget>
#include <QIcon>

#include <Core/EspinaTypes.h>

namespace EspINA
{
class ISettingsPanel
: public QWidget
{
public:
  virtual ~ISettingsPanel(){}

  virtual const QString shortDescription() = 0;
  virtual const QString longDescription() = 0;
  virtual const QIcon icon() = 0;  

  virtual void addPanel(ISettingsPanel *panel){}

  virtual void acceptChanges(){}; //TODO: Make abstract
  virtual void rejectChanges(){};
  virtual bool modified() const {return false;}

  virtual ISettingsPanelPtr clone() = 0;
};

} // namespace EspINA

#endif// ISETTINGSPANEL_H