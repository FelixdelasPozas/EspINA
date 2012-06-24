/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

#ifndef PQDATA_H
#define PQDATA_H

class vtkAlgorithmOutput;
// Forward declarations
class QString;
class Sample;
class pqFilter;
class vtkImageAlgorithmOutput;
class vtkImageAlgorithm;

/// Represent source's output in Paraview's pipeline
class pqData
{
public:
  /// Paraview pipeline data correspondig to @source's @portNumber output
  pqData(){}
  pqData(pqFilter *source, unsigned int portNumber);

  QString id() const;
  pqFilter *source() {return m_source;}
  vtkImageAlgorithm *algorithm();
  int portNumber() {return m_portNumber;}
  /// The output port where data can be retrieved
  vtkAlgorithmOutput *outputPort() const;

protected:
  pqFilter     *m_source;
  unsigned int  m_portNumber;
};

#endif // PQDATA_H