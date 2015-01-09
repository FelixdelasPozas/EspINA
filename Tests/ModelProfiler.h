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

      int numberOfRowsInsertedSignals() const
      { return m_numRIS; }

      int numberOfRowsAboutToBeRemovedSignals() const
      { return m_numRATBRS; }

      int numberOfDataChangedSignals() const
      { return m_numDCS; }

      int numberOfResetSignals() const
      { return m_numRS; }

      int totalNumberOfSignals() const
      { return numberOfRowsInsertedSignals()
             + numberOfRowsAboutToBeRemovedSignals()
             + numberOfDataChangedSignals()
             + numberOfResetSignals();
      }

      void reset();


    private slots:
      void onRowsInserted();

      void onRowsAboutToBeRemoved();

      void onDataChanged();

      void onReset();

    private:
      int m_numRIS;
      int m_numRATBRS;
      int m_numDCS;
      int m_numRS;
    };
  }
}

#endif // ESPINA_TESTING_MODEL_PROFILER_H
