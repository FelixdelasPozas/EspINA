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
MeshProxy::MeshProxy(FilterOutput *output)
: MeshType(output)
{

}

//-----------------------------------------------------------------------------
bool MeshProxy::setInternalData(SegmentationRepresentationSPtr rhs)
{
  m_meshRepresentation = boost::dynamic_pointer_cast<MeshType>(rhs);

  return m_meshRepresentation != NULL;
}

//-----------------------------------------------------------------------------
bool MeshProxy::dumpSnapshot(const QString &prefix, Snapshot &snapshot) const
{
  bool dumped = false;

  if (m_meshRepresentation)
    dumped = m_meshRepresentation->dumpSnapshot(prefix, snapshot);

  return dumped;
}

//-----------------------------------------------------------------------------
bool MeshProxy::fetchSnapshot(Filter *filter, const QString &prefix)
{
  bool fetched = false;

  if (m_meshRepresentation)
    fetched = m_meshRepresentation->fetchSnapshot(filter, prefix);

  return fetched;
}

//-----------------------------------------------------------------------------
bool MeshProxy::isValid() const
{
  bool res = false;

  if (m_meshRepresentation)
    res = m_meshRepresentation->isValid();

  return res;
}

//-----------------------------------------------------------------------------
bool MeshProxy::isEdited() const
{
  bool res = false;

  if (m_meshRepresentation)
    res = m_meshRepresentation->isEdited();

  return res;
}

//-----------------------------------------------------------------------------
FilterOutput::NamedRegionList MeshProxy::editedRegions() const
{
  FilterOutput::NamedRegionList res;

  if (m_meshRepresentation)
    res = m_meshRepresentation->editedRegions();

  return res;
}

//-----------------------------------------------------------------------------
void MeshProxy::clearEditedRegions()
{
  if (m_meshRepresentation)
    m_meshRepresentation->clearEditedRegions();
}

//-----------------------------------------------------------------------------
void MeshProxy::dumpEditedRegions(const QString &prefix) const
{
  if (m_meshRepresentation)
    m_meshRepresentation->dumpEditedRegions(prefix);
}

//-----------------------------------------------------------------------------
void MeshProxy::restoreEditedRegion(Filter *filter, const EspinaRegion &region, const QString &prefix)
{
  if (m_meshRepresentation)
    m_meshRepresentation->restoreEditedRegion(filter, region, prefix);
}

//-----------------------------------------------------------------------------
vtkAlgorithmOutput *MeshProxy::mesh() const
{
  vtkAlgorithmOutput *res = NULL;

  if (m_meshRepresentation)
    res = m_meshRepresentation->mesh();

  return res;
}
