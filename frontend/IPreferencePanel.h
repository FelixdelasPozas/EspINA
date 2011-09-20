#ifndef IPREFERENCEPANEL_H
#define IPREFERENCEPANEL_H

#include <QWidget>
#include <QIcon>

class IPreferencePanel : public QWidget
{
public:
  virtual ~IPreferencePanel(){}
  
  virtual const QString shortDescription() = 0;
  virtual const QString longDescription() = 0;
  virtual const QIcon icon() = 0;  
  
  virtual QWidget *widget() = 0;
};

#endif// IPREFERENCEPANEL_H