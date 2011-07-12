#ifndef SELECTEDPIXELLIST_H
#define SELECTEDPIXELLIST_H

#include "EspinaPlugin.h"
#include <iSegmentationPlugin.h>
#include "selectionManager.h"

class QMenu;//Forward declarations
class QSpinBox;
class Product;
class QString;

class SelectedPixelList 
  : public ISegmentationPlugin
  , public IFilterFactory
{
  Q_OBJECT

public:
  SelectedPixelList(QObject* parent);
  virtual EspinaFilter* createFilter(QString filter, ITraceNode::Arguments& args);
  
protected slots:
   void waitSeedSelection(bool wait);
   //! Abort current selection
   void abortSelection();
   //! Starts the segmentation filter putting a seed at @x, @y, @z.
   void startSegmentation(ISelectionHandler::Selection sel);

signals:
  void segmentationCreated(ProcessingTrace *);
  void productCreated(Segmentation *);
  void selectionAborted(ISelectionHandler *);
  
private:
  void buildUI();

private:
  ISelectionHandler* m_selector;
  QString m_output;
  QToolButton *m_segButton;
  
  
};

#endif // SELECTEDPIXELLIST_H
