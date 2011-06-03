#include "EspinaSaveDataReaction.h"

// class pqServer;
// class pqActiveObjects;
// class pqOutputPort;
// class vtkSMWriterFactory;
// class vtkSMProxyManager;
// class vtkSmartPointer;
// class vtkSMProxy;

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqOptions.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "pqTestUtility.h"
#include "pqWriterDialog.h"
#include "vtkSmartPointer.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMWriterFactory.h"

#include <QMessageBox>
#include <QDebug>
//-----------------------------------------------------------------------------
bool EspinaSaveDataReaction::saveActiveData(const QString& filename)
{
  pqServer* server = pqActiveObjects::instance().activeServer();
  // TODO: also is there's a pending accept.
  pqOutputPort* port = pqActiveObjects::instance().activePort();
  if (!server || !port)
    {
    qCritical("No active source located.");
    return false;
    }

  vtkSMWriterFactory* writerFactory =
    vtkSMProxyManager::GetProxyManager()->GetWriterFactory();
  vtkSmartPointer<vtkSMProxy> proxy;
  proxy.TakeReference(writerFactory->CreateWriter(filename.toAscii().data(),
      vtkSMSourceProxy::SafeDownCast(port->getSource()->getProxy()),
      port->getPortNumber()));
  vtkSMSourceProxy* writer = vtkSMSourceProxy::SafeDownCast(proxy);
  if (!writer)
    {
    qCritical() << "Failed to create writer for: " << filename;
    return false;
    }

  if (writer->IsA("vtkSMPSWriterProxy") && port->getServer()->getNumberOfPartitions() > 1)
    {
    //pqOptions* options = pqApplicationCore::instance()->getOptions();
    // To avoid showing the dialog when running tests.
    if (!pqApplicationCore::instance()->testUtility()->playingTest())
      {
      QMessageBox::StandardButton result =
        QMessageBox::question(
          pqCoreUtilities::mainWidget(),
          "Serial Writer Warning",
          "This writer will collect all of the data to the first node before "
          "writing because it does not support parallel IO. This may cause the "
          "first node to run out of memory if the data is large. "
          "Are you sure you want to continue?",
          QMessageBox::Ok | QMessageBox::Cancel,
          QMessageBox::Cancel);
      if (result == QMessageBox::Cancel)
        {
        return false;
        }
      }
    }
  /*
  pqWriterDialog dialog(writer);

  // Check to see if this writer has any properties that can be configured by
  // the user. If it does, display the dialog.
  if (dialog.hasConfigurableProperties())
    {
    dialog.exec();
    if(dialog.result() == QDialog::Rejected)
      {
      // The user pressed Cancel so don't write
      return false;
      }
    }*/
  writer->UpdateVTKObjects();
  writer->UpdatePipeline();
  return true;
}