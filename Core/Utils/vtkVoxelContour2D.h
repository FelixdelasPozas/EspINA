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

#ifndef ESPINA_VTKVOXELCONTOUR2D_H_
#define ESPINA_VTKVOXELCONTOUR2D_H_

#include "Core/EspinaCore_Export.h"

// VTK
#include <vtkPolyDataAlgorithm.h>

/** \Class vtkVoxelContour2D
 * \brief Creates a 2D contour given a 3d volume and a slice between the bounds.
 *
 *  This filter only works for slices in the axial plane (extent[4] == extent[5]),
 *  it's not a generic marching cubes for 2D or 3d volumes.
 */
class EspinaCore_EXPORT vtkVoxelContour2D
: public vtkPolyDataAlgorithm
{
  public:
    vtkTypeMacro(vtkVoxelContour2D,vtkPolyDataAlgorithm);

    static vtkVoxelContour2D *New();

    /** \brief Overrides vtkPolyDataAlgorithm::PrintSelf().
     *
     */
    void PrintSelf(ostream& os, vtkIndent indent) override;

    // Description:
    // Get the output data object for a port on this algorithm.
    vtkPolyData *GetOutput();
    vtkPolyData *GetOutput(int);
    virtual void SetOutput(vtkDataObject* d) override;

    // this method is not recommended for use, but lots of old style filters use it
    vtkDataObject* GetInput();
    vtkDataObject* GetInput(int port);


    // see vtkAlgorithm for details
    virtual int ProcessRequest(vtkInformation *request,
                               vtkInformationVector **inputVector,
                               vtkInformationVector *outputVector) override;

    virtual int FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info) override;

    void SetInput(vtkDataObject*);
    void SetInput(int, vtkDataObject*);
    void AddInput(vtkDataObject*);
    void AddInput(int, vtkDataObject*);

    /** \brief Get the smallest of the spacing of the plane that contains the contour
     *        (used to compute contour width in ContourRepresentation).
     */
    double getMinimumSpacing() const;

  protected:
    /** \brief vtkVoxelContour2D class constructor.
     *
     */
    vtkVoxelContour2D();

    /** \brief vtkVoxelContour2D class destructor.
     *
     */
    ~vtkVoxelContour2D();

    /** \brief Request data.
     *
     */
    virtual int RequestDataObject(vtkInformation* request,
                                  vtkInformationVector** inputVector,
                                  vtkInformationVector* outputVector);

    virtual int RequestInformation(vtkInformation* request,
                                   vtkInformationVector** inputVector,
                                   vtkInformationVector* outputVector) override;

    virtual int RequestData(vtkInformation* request,
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector) override;

    virtual int RequestUpdateExtent(vtkInformation*,
                                    vtkInformationVector**,
                                    vtkInformationVector*) override;

  private:
    /** \brief Copy constructor not implemented.
     *
     */
    vtkVoxelContour2D(const vtkVoxelContour2D&);

    /** \brief Assignment operator not implemented.
     *
     */
    void operator=(const vtkVoxelContour2D&);

    double m_minSpacing;
};

#endif // ESPINA_VTKVOXELCONTOUR2D_H_
