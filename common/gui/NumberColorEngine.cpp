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
#include "NumberColorEngine.h"

#include "common/model/Segmentation.h"

#include <pqServer.h>
#include <pqApplicationCore.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>
#include <pqScalarsToColors.h>
#include <pqLookupTableManager.h>

vtkSMProxy* NumberColorEngine::lut(const Segmentation* seg)
{
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  pqServer *server =  pqApplicationCore::instance()->getActiveServer();
  QString lutName = seg->taxonomy()->qualifiedName();
  if (seg->selected())
    lutName.append("_selected");

  pqLookupTableManager *lutManager = pqApplicationCore::instance()->getLookupTableManager();
  pqScalarsToColors *lut = lutManager->getLookupTable(server,lutName,4,0);
  if (lut)
  {
    double alpha = (seg->selected()?1.0:0.7);
    QColor c = color(seg);
    double colors[8] = {0,0,0,0,255, c.redF()*alpha,c.greenF()*alpha,c.blueF()*alpha};
    vtkSMPropertyHelper(lut->getProxy(), "RGBPoints").Set(colors, 8);
    lut->getProxy()->UpdateVTKObjects();
  }

  return lut->getProxy();
}

QColor NumberColorEngine::color(const Segmentation* seg)
{
  if (seg)
    return QColor((seg->number()*25)%255,
		  (seg->number()*73)%255,
		  (seg->number()*53)%255);
  else
    return Qt::red;
}

