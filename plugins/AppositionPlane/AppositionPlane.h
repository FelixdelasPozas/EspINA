#ifndef APPOSITIONPLANE_H
#define APPOSITIONPLANE_H

#include <EspinaPlugin.h>
#include <selectionManager.h>

class Product;

//! Apposition Plane Plugin
class AppositionPlane
      : public QObject
{
public:
  AppositionPlane(QObject* parent=0);
  
  void onStartup();
  void onShutdown(){}
};

#endif// APPOSITIONPLANE_H