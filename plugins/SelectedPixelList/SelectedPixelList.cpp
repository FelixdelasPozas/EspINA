#include "SelectedPixelList.h"

#include "espina.h"
#include "pixelSelector.h"

//GUI includes
#include <QApplication>
#include <QStyle>
#include <QHBoxLayout>
// #include <QLabel>
// #include <QSpinBox>
#include <QWidgetAction>
#include <QToolButton>
// #include <QMenu>
#include <QDebug>

#include "pqApplicationCore.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqUndoStack.h"
#include "pqPipelineFilter.h"
#include "pqFileDialog.h"
#include "pqCoreUtilities.h"

#include "vtkSMProxy.h"
#include "vtkSMInputProperty.h"


#include <espINAFactory.h>
#include <QBitmap>

//-----------------------------------------------------------------------------
SelectedPixelList::SelectedPixelList(QObject* parent)//: SeedGrowSegmentation(parent)
: ISegmentationPlugin(parent)
, m_selector(NULL)
, m_output("")
{
  buildUI();
}

//-----------------------------------------------------------------------------
EspinaFilter* SelectedPixelList::createFilter(QString filter, ITraceNode::Arguments& args)
{
  return NULL;
}

//-----------------------------------------------------------------------------
void SelectedPixelList::waitSeedSelection(bool wait)
{
  if (wait && m_selector)
  {
/*    QApplication::setOverrideCursor(QCursor(QPixmap(":crossRegion.svg")));
    else*/
    pqFileDialog dialog(NULL, pqCoreUtilities::mainWidget(), 
                        tr("File to save selected points"), 
                        QString(), QString());
    dialog.setFileMode(pqFileDialog::AnyFile);
    
    if( dialog.exec() == QDialog::Accepted )
    {
      m_output = dialog.getSelectedFiles().first(); //"SelectedPoints.csv";
      QFile::remove(m_output);
      QApplication::setOverrideCursor(Qt::CrossCursor);
      SelectionManager::instance()->setSelectionHandler(m_selector, Qt::CrossCursor);
      m_segButton->setChecked(true);
    }
  }else
  {
    SelectionManager::instance()->setSelectionHandler(NULL, Qt::ArrowCursor);
  }
}

//-----------------------------------------------------------------------------
void SelectedPixelList::abortSelection()
{
  QApplication::restoreOverrideCursor();
  m_segButton->setChecked(false);  
}

//-----------------------------------------------------------------------------
void SelectedPixelList::startSegmentation(ISelectionHandler::Selection sel)
{
  Point pixel = sel.first().first.first();
//   qDebug() << "SelectedPixelList: pixel selected" << pixel.x << pixel.y << pixel.z;
  
  QFile f(m_output);
  if( f.open(QIODevice::Append|QIODevice::WriteOnly) )
  {
    f.write(QString("%1, %2, %3\n").arg(pixel.x).arg(pixel.y).arg(pixel.z).toUtf8());
  }
  f.close();
}

//-----------------------------------------------------------------------------
void SelectedPixelList::buildUI()
{
  m_selector = new PixelSelector();
  m_selector->multiSelection = false;
  m_selector->filters << "EspINA_Sample";
  
  connect(m_selector, SIGNAL(selectionChanged(ISelectionHandler::Selection)),
          this, SLOT(startSegmentation(ISelectionHandler::Selection)));

  QAction* action = new QAction(
    QIcon(":pixelSelection.svg")
    , tr(""),
    m_selector);
    
  m_segButton = new QToolButton();
  m_segButton->setCheckable(true);
  m_segButton->setIcon(action->icon());

  QWidgetAction *mainWidget = new QWidgetAction(this);
  mainWidget->setDefaultWidget(m_segButton);
  
  connect(m_segButton, SIGNAL(triggered(QAction*)), 
          this, SLOT(changeSeedSelector(QAction *)));
  connect(m_segButton, SIGNAL(toggled(bool)), 
          this, SLOT(waitSeedSelection(bool)));
  
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------