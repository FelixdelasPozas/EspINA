#ifndef ESPINAPLUGIN_H
#define ESPINAPLUGIN_H

#include "espinaTypes.h"
#include <QToolButton>

class Segmentation;
class pqView;

//! Interface to extend segmentation behaviour
class ISegmentationExtension {
public:
  
  virtual ExtensionId id() = 0;
  virtual void initialize(Segmentation *seg) = 0;
  virtual void addInformation(InformationMap &map) = 0;
  virtual void addRepresentations(RepresentationMap &map) = 0;
  
  virtual Segmentation *segmentation() {return m_seg;}
  
  //! Prototype
  virtual ISegmentationExtension *clone() = 0;
  
protected:
  ISegmentationExtension() : m_seg(NULL), m_init(false){}
  virtual ~ISegmentationExtension(){}
  
  Segmentation *m_seg;
  bool m_init; // Wheteher the extentation has been initialized or not
	       // In other words; if it has been linked to a segmentation
};

//TODO: Revisar!!! XXX MUERTEEEE DESTRUCCIOOOON
class IViewWidget : public QToolButton
{
  Q_OBJECT
public:
  IViewWidget(QWidget* parent = 0) : QToolButton(parent) 
  {
    setCheckable(true);
    connect(this,SIGNAL(toggled(bool)),this,SLOT(updateState(bool)));
  }
  virtual ~IViewWidget(){}
  
  virtual void renderInView(pqView* view) = 0;

  //! Prototype
  virtual IViewWidget *clone() = 0;
  
public slots:
  virtual void updateState(bool checked) = 0;
  
signals:
  void updateRequired();
};


class EspinaPlugin
{
public:

  virtual void LoadAnalisys(EspinaParamList& args) = 0;

protected:
  QString m_groupName, m_filterName;

};

#endif // ESPINAPLUGIN_H
