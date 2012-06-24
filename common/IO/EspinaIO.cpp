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
#include "vtkSegReader.h"

#include <common/model/EspinaFactory.h>

#include <common/cache/Cache.h>
#include <sstream>
#include <QDebug>
#include <model/Taxonomy.h>
#include <EspinaCore.h>
#include <QApplication>

#include <vtkSmartPointer.h>

static const QString SEG = "seg";

//-----------------------------------------------------------------------------
bool readSegFile(const QFileInfo file)
{
  const QFileInfo cachePath = file.path() + "/" + file.baseName();
  Cache::instance()->setWorkingDirectory(cachePath);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  qDebug() << "Opening Seg File:" << file.absoluteFilePath();
  qDebug() << "Parent Directory:" << cachePath.absolutePath();

  vtkSmartPointer<vtkSegReader> reader = vtkSmartPointer<vtkSegReader>::New();
  reader->SetFileName(file.filePath().toAscii());
  reader->Update();

  QString TaxonomySerialization(reader->GetTaxonomy());

  std::istringstream trace(reader->GetTrace());
  //     qDebug() << TraceSerialization;

  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();
  try
  {
    Taxonomy *taxonomy = IOTaxonomy::loadXMLTaxonomy(TaxonomySerialization);
    model->addTaxonomy(taxonomy);
    if (model->taxonomy()->elements().size() == 0)
    {
      QApplication::restoreOverrideCursor();
      return false;
    }
    taxonomy->print(3);

    model->loadSerialization(trace);
  }
  catch (char *str)
  {
    qWarning() << "Espina: Unable to load" << file.absolutePath() << str;
    QApplication::restoreOverrideCursor();
    return false;
  }
  QApplication::restoreOverrideCursor();
  return true;
}


