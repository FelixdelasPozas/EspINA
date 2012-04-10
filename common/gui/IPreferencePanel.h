#ifndef ISETTINGS_PANEL_H
#define ISETTINGS_PANEL_H

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

  virtual QWidget *widget() = 0;
};

#endif// ISETTINGS_PANEL_H