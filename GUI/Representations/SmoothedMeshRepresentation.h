/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef SMOOTHEDMESHREPRESENTATION_H_
#define SMOOTHEDMESHREPRESENTATION_H_

// EspINA
#include "IMeshRepresentation.h"

class vtkDecimatePro;

namespace EspINA
{
  
  class SmoothedMeshRepresentation
  : public IMeshRepresentation
  {
    Q_OBJECT
    public:
      explicit SmoothedMeshRepresentation(MeshRepresentationSPtr mesh,
                                          EspinaRenderView *view);
      virtual ~SmoothedMeshRepresentation() {};

      virtual GraphicalRepresentationSPtr clone(VolumeView *view);

      virtual void updateRepresentation();

    private slots:
      void updatePipelineConnections();

    private:
      void initializePipeline(VolumeView *view);

    private:
      vtkSmartPointer<vtkDecimatePro>                m_decimate;
  };

  typedef boost::shared_ptr<SmoothedMeshRepresentation> SmoothedMeshRepresentationSPtr;
  typedef QList<SmoothedMeshRepresentationSPtr> SmoothedMeshRepresentationSList;

} /* namespace EspINA */
#endif /* SMOOTHEDMESHREPRESENTATION_H_ */
