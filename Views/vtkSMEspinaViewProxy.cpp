/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "vtkSMEspinaViewProxy.h"

#include <vtkObjectFactory.h>
#include <vtkSMProxyManager.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMRepresentationProxy.h>

#include <QDebug>
#include <assert.h>

vtkStandardNewMacro(vtkSMEspinaViewProxy);

vtkSMRepresentationProxy* vtkSMEspinaViewProxy::CreateDefaultRepresentation(vtkSMProxy* source, int port)
{
  qDebug() << "vtkSMEspinaViewProxy: Cretating default representation for" << source;
  vtkSMProxyManager* pxm = this->GetProxyManager();
  // Update with time to avoid domains updating without time later.
  vtkSMSourceProxy* sproxy = vtkSMSourceProxy::SafeDownCast(source);
  if (sproxy)
    {
    double view_time = vtkSMPropertyHelper(this, "ViewTime").GetAsDouble();
    sproxy->UpdatePipeline(view_time);
    }
    vtkSMRepresentationProxy* repr = vtkSMRepresentationProxy::SafeDownCast(
      pxm->NewProxy("representations", "ChannelRepresentation"));
    assert(repr);
//     vtkSMPropertyHelper(repr, "UseXYPlane").Set(1);
  return repr;
}
