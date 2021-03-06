/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ESPINA_MARCHING_CUBES_MESH_H
#define ESPINA_MARCHING_CUBES_MESH_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "RawMesh.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageConstantPad.h>
#include <vtkDiscreteMarchingCubes.h>

// Qt
#include <QMutex>

namespace ESPINA
{
  class EspinaCore_EXPORT MarchingCubesMesh
  : public RawMesh
  {
    public:
      /** \brief MarchingCubesMesh class constructor.
       * \param[in] output to obtain the volumetric data from
       *
       * NOTE: this data doesn't update the output to access its volumetric data
       */
      explicit MarchingCubesMesh(Output *output);

      /** \brief MarchingCubesMesh class virtual destructor.
       *
       */
      virtual ~MarchingCubesMesh()
      {};

      virtual bool isValid() const override;

      virtual vtkSmartPointer<vtkPolyData> mesh() const override;

      virtual TimeStamp lastModified() const override;

      virtual VolumeBounds bounds() const override;

    private:
      /** \brief Applies marching cubes algorithm to the volumetric data of its output to generate a mesh.
       *
       */
      void updateMesh();

      virtual QList<Data::Type> updateDependencies() const override;

      /** \brief Returns true if the mesh need to be updated.
       *
       */
      const bool needsUpdate() const;

    private:
      Output        *m_output;                 /** output this data belongs to.    */
      TimeStamp      m_lastVolumeModification; /** last time the mesh was updated. */
      mutable QMutex m_lock;                   /** protects internal data.         */
  };

} // namespace ESPINA

#endif // ESPINA_MARCHING_CUBES_MESH_H
