/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "EspinaIO.h"

#include <common/model/EspinaFactory.h>

#include <common/cache/Cache.h>

#include <pqActiveObjects.h>
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqOutputPort.h>
#include <pqPipelineSource.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>
#include <vtkSMStringVectorProperty.h>

#include <QFileInfo>

#include <sstream>

#include <QDebug>
#include <model/Taxonomy.h>
#include <EspinaCore.h>
static const QString SEG = "seg";

//-----------------------------------------------------------------------------
EspinaIO::EspinaIO()
{
  EspinaFactory::instance()->registerReader(SEG, this);
}

//-----------------------------------------------------------------------------
EspinaIO::~EspinaIO()
{
}

QString parentDirectory(const QString path)
{
  return path.section('/',0,-2);
}

QString fileName(const QString path)
{
  return path.section('/',-1).section('.',0,-2);
}

//-----------------------------------------------------------------------------
void EspinaIO::readFile(const QString file )
{
  const QString cachePath = parentDirectory(file) + "/" + fileName(file);

//   qDebug() << "Opening Seg File:" << file;
//   qDebug() << "Parent Directory:" << cachePath;

  pqServer* server = pqActiveObjects::instance().activeServer();
  Q_ASSERT(server);
  pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  pqPipelineSource* reader = ob->createFilter("sources", "EspinaReader",
					      QMap<QString, QList<pqOutputPort*> >(),
					      pqApplicationCore::instance()->getActiveServer() );
  // Set the file name
  vtkSMPropertyHelper(reader->getProxy(), "FileName").Set(file.toStdString().c_str());
  reader->getProxy()->UpdateVTKObjects();

  QFileInfo path(cachePath);
  Cache::instance()->setWorkingDirectory(path);
  reader->updatePipeline(); //Update the pipeline to obtain the content of the file
  reader->getProxy()->UpdatePropertyInformation();

  vtkSMProperty *p;
  // Taxonomy
  p = reader->getProxy()->GetProperty("Taxonomy");
  vtkSMStringVectorProperty* TaxProp = vtkSMStringVectorProperty::SafeDownCast(p);
  QString TaxonomySerialization(TaxProp->GetElement(0));

  // Trace
  p = reader->getProxy()->GetProperty("Trace");
  vtkSMStringVectorProperty* TraceProp = vtkSMStringVectorProperty::SafeDownCast(p);
  std::istringstream trace(TraceProp->GetElement(0));
  //     qDebug() << TraceSerialization;

  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();
  try
  {
    Taxonomy *taxonomy = IOTaxonomy::loadXMLTaxonomy(TaxonomySerialization);
    model->addTaxonomy(taxonomy);
    // 	taxonomy->print(3);

    model->loadSerialization(trace);

    // Remove the proxy of the .seg file
    ob->destroy(reader);
  }
  catch (char *str)
  {
    qWarning() << "Espina: Unable to load" << file << str;
  }
}


