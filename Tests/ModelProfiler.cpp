/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "ModelProfiler.h"
#include <iostream>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

//------------------------------------------------------------------------
ModelProfiler::ModelProfiler(QAbstractItemModel &model)
: m_numRIS   {0}
, m_numRATBRS{0}
, m_numDCS   {0}
, m_numRS    {0}
{
  connect(&model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(onRowsInserted()));
  connect(&model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(onRowsAboutToBeRemoved()));
  connect(&model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(onDataChanged()));
  connect(&model, SIGNAL(modelAboutToBeReset()),
          this, SLOT(onReset()));
}

//------------------------------------------------------------------------
void ModelProfiler::reset()
{
  m_numRIS = m_numRATBRS = m_numDCS = m_numRS = 0;
}

//------------------------------------------------------------------------
void ModelProfiler::onRowsInserted()
{
  ++m_numRIS;
}

//------------------------------------------------------------------------
void ModelProfiler::onRowsAboutToBeRemoved()
{
  ++m_numRATBRS;
}

//------------------------------------------------------------------------
void ModelProfiler::onDataChanged()
{
  ++m_numDCS;
}


//------------------------------------------------------------------------
void ModelProfiler::onReset()
{
  ++m_numRS;
}

//------------------------------------------------------------------------
bool ESPINA::Testing::checkExpectedNumberOfSignals(ModelProfiler &profiler,
                                                   unsigned       expectedRIS,
                                                   unsigned       expectedDCS,
                                                   unsigned       expectedRATBRS)
{
  bool error = false;

  auto numRIS = profiler.numberOfRowsInsertedSignals();
  if (numRIS != expectedRIS) {
    cerr << "Unexpected number of RIS: " << numRIS << " instead of " << expectedRIS << endl;
    error = true;
  }

  auto numDCS = profiler.numberOfDataChangedSignals();
  if (numDCS != expectedDCS) {
    cerr << "Unexpected number of DCS: " << numDCS << " instead of " << expectedDCS << endl;
    error = true;
  }

  auto numRATBRS = profiler.numberOfRowsAboutToBeRemovedSignals();
  if (numRATBRS != expectedRATBRS) {
    cerr << "Unexpected number of RATBRS: " << numRATBRS << " instead of " << expectedRATBRS << endl;
    error = true;
  }

  return error;
}