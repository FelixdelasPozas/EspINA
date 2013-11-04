/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "MeshProxy.h"

using namespace EspINA;

//-----------------------------------------------------------------------------
template<typename T>
MeshProxy<T>::MeshProxy(OutputSPtr *output)
: m_meshData(nullptr)
{
}

//-----------------------------------------------------------------------------
template<typename T>
bool MeshProxy<T>::setInternalData(MeshDataSPtr rhs)
{
  m_mesh = rhs;

  return m_mesh != nullptr;
}

//-----------------------------------------------------------------------------
template<typename T>
bool MeshProxy<T>::dumpSnapshot(const QString &prefix, Snapshot &snapshot) const
{
  bool dumped = false;

  if (m_meshRepresentation)
    dumped = m_meshRepresentation->dumpSnapshot(prefix, snapshot);

  return dumped;
}

//-----------------------------------------------------------------------------
// bool MeshProxy::fetchSnapshot(Filter *filter, const QString &prefix)
// {
//   bool fetched = false;
// 
//   if (m_meshRepresentation)
//     fetched = m_meshRepresentation->fetchSnapshot(filter, prefix);
// 
//   return fetched;
// }

//-----------------------------------------------------------------------------
template<typename T>
bool MeshProxy<T>::isValid() const
{
  bool res = false;

  if (m_meshRepresentation)
    res = m_meshRepresentation->isValid();

  return res;
}

//-----------------------------------------------------------------------------
template<typename T>
Bounds MeshProxy<T>::bounds()
{
  // TODO
//  EspinaRegion res;
//
//  if (m_meshRepresentation)
//    res = m_meshRepresentation->representationBounds();

  return Bounds();
}

//-----------------------------------------------------------------------------
template<typename T>
bool MeshProxy<T>::isEdited() const
{
  bool res = false;

  if (m_meshRepresentation)
    res = m_meshRepresentation->isEdited();

  return res;
}

//-----------------------------------------------------------------------------
template<typename T>
void MeshProxy<T>::clearEditedRegions()
{
  if (m_meshRepresentation)
    m_meshRepresentation->clearEditedRegions();
}

//-----------------------------------------------------------------------------
template<typename T>
void MeshProxy<T>::commitEditedRegions(bool withData) const
{
  if (m_meshRepresentation)
    m_meshRepresentation->commitEditedRegions(withData);
}

//-----------------------------------------------------------------------------
template<typename T>
void MeshProxy<T>::restoreEditedRegions(const QDir &cacheDir, const QString &outputId)
{
  if (m_meshRepresentation)
    m_meshRepresentation->restoreEditedRegions(cacheDir, outputId);
}

//-----------------------------------------------------------------------------
template<typename T>
vtkAlgorithmOutput *MeshProxy<T>::mesh()
{
  vtkAlgorithmOutput *res = NULL;

  if (m_meshRepresentation)
    res = m_meshRepresentation->mesh();

  return res;
}
