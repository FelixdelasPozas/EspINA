#ifndef ISETTINGSPANEL_H
#define ISETTINGSPANEL_H

#include <QWidget>
#include <QIcon>

class ISettingsPanel : public QWidget
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

  virtual ISettingsPanel *widget() = 0;
};

#endif// ISETTINGSPANEL_H