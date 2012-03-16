#ifndef ESPINAPLUGIN_H
#define ESPINAPLUGIN_H

#include "espinaTypes.h"
#include <QToolButton>
#include <QModelIndex>
#include <QPushButton>

class EspinaFilter;
class Segmentation;
class Sample;
class pqView;
class pqPipelineSource;


class ISampleRepresentation : public QObject
{
  Q_OBJECT
  
public:
  typedef QString RepresentationId;
  
  ISampleRepresentation(Sample *sample) : m_sample(sample) {}
  virtual ~ISampleRepresentation(){}
  
  virtual QString id() = 0;
  //! Create a new representation in the given view
  virtual void render(pqView *view, ViewType type = VIEW_3D) = 0;
  virtual void clear(pqView *view, ViewType type = VIEW_3D) {}
  //! Returns the output port needed to connect it to other filters
  //! NOTE: This method must update internal properties if needed
  virtual pqPipelineSource *pipelineSource() = 0;
  
public slots:
  virtual void requestUpdate(bool force = false) = 0;
  
signals:
  void representationUpdated();

protected:
  Sample *m_sample;
};

//! Interface to extend sample behaviour
class ISampleExtension 
{
// public:
//   typedef QMap<QString, QVariant> InformationMap;
//   typedef QMap<QString, ISampleRepresentation *> RepresentationMap;
  
public:
  virtual ~ISampleExtension(){}
  
  virtual ExtensionId id() = 0;
  virtual void initialize(Sample *sample) = 0;
  virtual QStringList dependencies() {return QStringList();}
  virtual QStringList availableRepresentations() {return m_availableRepresentations;}
  virtual ISampleRepresentation *representation(QString rep) = 0;
  virtual QStringList availableInformations() {return m_availableInformations;}
  virtual QVariant information(QString info) = 0;
  virtual void setArguments(QString args) {}
  virtual QString getArguments() {return QString();};
  
  virtual Sample *sample() {return m_sample;}
  
  //! Prototype
  virtual ISampleExtension *clone() = 0;
  
protected:
  ISampleExtension() : m_sample(NULL), m_init(false){}
  
  Sample *m_sample;
  bool m_init; // Wheteher the extentation has been initialized or not
	       // In other words; if it has been linked to a segmentation
  QStringList m_availableRepresentations;
  QStringList m_availableInformations;
};

class ISegmentationRepresentation : public QObject
{
  Q_OBJECT
  
public:
  typedef QString RepresentationId;
  
  ISegmentationRepresentation(Segmentation *seg) : m_seg(seg) {}
  virtual ~ISegmentationRepresentation(){}
  
  virtual QString id() = 0;
  //! Create a new representation in the given view
  virtual void render(pqView *view) = 0;
  //! Returns the output port needed to connect it to other filters
  //! NOTE: This method must update internal properties if needed
  virtual pqPipelineSource *pipelineSource() = 0;
  
public slots:
  virtual void requestUpdate(bool force=false) = 0;
  
signals:
  void representationUpdated();

protected:
  Segmentation *m_seg;
};

// //Revisar!!! XXX MUERTEEEE DESTRUCCIOOOON
// class IViewWidget : public QPushButton
// {
//   Q_OBJECT
// public:
//   IViewWidget(QWidget* parent = 0) : QPushButton(parent) 
//   {
//     setCheckable(true);
//     setFlat(true);
//     setIconSize(QSize(22,22));
//     setMaximumSize(QSize(32,32));
//     connect(this,SIGNAL(toggled(bool)),this,SLOT(updateState(bool)));
//   }
//   virtual ~IViewWidget(){}
//   
//   virtual void renderInView(QModelIndex index, pqView* view) = 0;
// 
//   //! Prototype
//   virtual IViewWidget *clone() = 0;
//   
// public slots:
//   virtual void updateState(bool checked) = 0;
//   
// signals:
//   void updateRequired();
// };

// #include "processingTrace.h" //WARNING: Visibility?
// class IFilter;
// 
// class IFilterFactory
// {
// public:
//   virtual ~IFilterFactory(){}
//   virtual EspinaFilter* createFilter(QString filter, ITraceNode::Arguments &args) = 0;
//   
//   //QString pluginName() {return m_pluginName;}
// protected:
//   QString m_factoryName;
// };

// class IFileReader
// {
// public:
//   virtual ~IFileReader(){}
//   virtual void readFile(pqPipelineSource *proxy, const QString &filePath) = 0;
// };

#endif // ESPINAPLUGIN_H
