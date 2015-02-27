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

#ifndef ESPINA_TESTING_MODEL_PROFILER_H
#define ESPINA_TESTING_MODEL_PROFILER_H

#include <QAbstractItemModel>

namespace ESPINA
{
  namespace Testing
  {
    class ModelProfiler
    : public QObject
    {
      Q_OBJECT
    public:
      explicit ModelProfiler(QAbstractItemModel &model);

      unsigned numberOfRowsInsertedSignals() const
      { return m_numRIS; }

      unsigned numberOfRowsAboutToBeRemovedSignals() const
      { return m_numRATBRS; }

      unsigned numberOfRowsAboutToBeMovedSignals() const
      { return m_numRATBMS; }

      unsigned numberOfDataChangedSignals() const
      { return m_numDCS; }

      unsigned numberOfResetSignals() const
      { return m_numRS; }

      int totalNumberOfSignals() const
      { return numberOfRowsInsertedSignals()
             + numberOfDataChangedSignals()
             + numberOfRowsAboutToBeMovedSignals()
             + numberOfRowsAboutToBeRemovedSignals()
             + numberOfResetSignals();
      }

      void reset();


    private slots:
      void onRowsInserted();

      void onDataChanged();

      void onRowsAboutToBeMoved();

      void onRowsAboutToBeRemoved();

      void onReset();

    private:
      unsigned m_numRIS;
      unsigned m_numDCS;
      unsigned m_numRATBMS;
      unsigned m_numRATBRS;
      unsigned m_numRS;
    };

    bool checkExpectedNumberOfSignals(ModelProfiler &profiler,
                                      unsigned       expectedRIS,
                                      unsigned       expectedDCS,
                                      unsigned       expectedRATBMS,
                                      unsigned       expectedRATBRS);
  }
}

#endif // ESPINA_TESTING_MODEL_PROFILER_H
