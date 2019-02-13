/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Core/Analysis/Data/VolumetricData.hxx>

using namespace ESPINA;

//----------------------------------------------------------------------------
Output::ReadLockData<DefaultVolumetricData> ESPINA::readLockVolume(Output *output, DataUpdatePolicy policy)
{
  return outputReadLockData<DefaultVolumetricData>(output, policy);
}

//----------------------------------------------------------------------------
Output::ReadLockData<DefaultVolumetricData> ESPINA::readLockVolume(OutputSPtr output, DataUpdatePolicy policy)
{
  return readLockVolume(output.get(), policy);
}

//----------------------------------------------------------------------------
Output::WriteLockData<DefaultVolumetricData> ESPINA::writeLockVolume(Output *output, DataUpdatePolicy policy)
{
  return outputWriteLockData<DefaultVolumetricData>(output, policy);
}

//----------------------------------------------------------------------------
Output::WriteLockData<DefaultVolumetricData> ESPINA::writeLockVolume(OutputSPtr output, DataUpdatePolicy policy)
{
  return writeLockVolume(output.get(), policy);
}

//----------------------------------------------------------------------------
bool ESPINA::hasVolumetricData(OutputSPtr output)
{
  return output->hasData(DefaultVolumetricData::TYPE);
}
