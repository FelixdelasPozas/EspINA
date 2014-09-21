/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_SMOOTHED_MESH_REPRESENTATION_H_
#define ESPINA_SMOOTHED_MESH_REPRESENTATION_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "MeshRepresentation.h"

class vtkDecimatePro;

namespace ESPINA
{
  class EspinaGUI_EXPORT SmoothedMeshRepresentation
  : public MeshRepresentationBase
  {
    Q_OBJECT
  public:
    static const Representation::Type TYPE;

  public:
    /** \brief SmoothedMeshRepresentation class constructor.
     * \param[in] mesh, MeshData smart pointer of the data to represent.
     * \param[in] view, render view pointer where the representation will be shown.
     *
     */
    explicit SmoothedMeshRepresentation(MeshDataSPtr mesh,
                                        RenderView *view);

    /** \brief SmoothedMeshRepresentation class virtual destructor.
     *
     */
    virtual ~SmoothedMeshRepresentation()
    {};

    /** \brief Implements Representation::settingsWidget().
     *
     */
    virtual RepresentationSettings *settingsWidget();

    /** \brief Implements Representation::updateRepresentation().
     *
     */
    virtual void updateRepresentation();

  protected:
    /** \brief Implements Representation::cloneImplementation(View3D*);
     *
     */
    virtual RepresentationSPtr cloneImplementation(View3D *view);

  private:
    /** \brief Implements MeshRepresentation::initializePipeline().
     *
     */
    void initializePipeline();

  private:
    vtkSmartPointer<vtkDecimatePro> m_decimate;
  };

  using SmoothedMeshRepresentationSPtr  = std::shared_ptr<SmoothedMeshRepresentation>;
  using SmoothedMeshRepresentationSList = QList<SmoothedMeshRepresentationSPtr>;

} /* namespace ESPINA */
#endif /* ESPINA_SMOOTHED_MESH_REPRESENTATION_H_ */
