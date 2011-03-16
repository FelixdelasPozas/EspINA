/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "espINAFactory.h"

#include "EspinaPlugin.h"
#include "traceNodes.h"

#include <QDebug>

EspINAFactory *EspINAFactory::m_instance = NULL;

EspINAFactory* EspINAFactory::instance()
{
  if (!m_instance)
    m_instance = new EspINAFactory();
  return m_instance;
}

Segmentation* EspINAFactory::CreateSegmentation(pqPipelineSource* source, int portNumber, QString parentHash)
{
  Segmentation *seg = new Segmentation(source,portNumber,parentHash);
  foreach(ISegmentationExtension *ext, m_extensions)
  {
    seg->addExtension(ext);
  }
  return seg;
}

void EspINAFactory::addSegmentationExtension(ISegmentationExtension* ext)
{
  qDebug() << "New Extension registered in Factory";
  m_extensions.append(ext->clone());
}



