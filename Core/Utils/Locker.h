/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef CORE_UTILS_LOCKER_H_
#define CORE_UTILS_LOCKER_H_

#include "Core/EspinaCore_Export.h"

// Qt
#include <QReadWriteLock>
#include <QMutex>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      /** \class WriteLocker
       * \brief Class to lock objects for read-write access.
       *
       */
      class EspinaCore_EXPORT WriteLocker
      {
        public:
          /** \brief Locker class constructor.
           * \param[in] locker lock object to use.
           *
           */
          explicit WriteLocker(QReadWriteLock &locker)
          : m_locker(locker)
          { m_locker.lockForWrite(); }

          /** \brief WriteLocker class virtual destructor.
           *
           */
          virtual ~WriteLocker()
          { m_locker.unlock(); }

        protected:
          QReadWriteLock &m_locker; /** object locker. */
      };

      /** \class ReadLocker
       * \brief Class to lock objects for read access.
       *
       */
      class EspinaCore_EXPORT ReadLocker
      {
        public:
          /** \brief ReadLocker class constructor.
           * \param[in] locker lock object to use.
           *
           */
          ReadLocker(QReadWriteLock &locker)
          : m_locker(locker)
          { const_cast<QReadWriteLock &>(m_locker).lockForRead(); }

          /** \brief ReadLocker class virtual destructor.
           *
           */
          virtual ~ReadLocker()
          { const_cast<QReadWriteLock &>(m_locker).unlock();}

        private:
          const QReadWriteLock &m_locker; /** object locker. */
      };

      /** \class MutexLocker
       *  \brief Class to locks objects prior using them.
       *
       */
      class EspinaCore_EXPORT MutexLocker
      {
        public:
          /** \brief MutexLocker class constructor
           * \param[in] mutex mutex object.
           * \param[in] tryFirst true to try to lock instead of directly locking and false to lock right away.
           *
           */
          explicit MutexLocker(QMutex &mutex, bool tryFirst = false)
          : m_mutex   (mutex)
          , m_isLocked{true}
          {
            if(tryFirst)
            {
              m_isLocked = m_mutex.tryLock();
            }
            else
            {
              m_mutex.lock();
            }
          }

          /** \brief Returns true if the mutex is locked and false otherwise.
           *
           */
          bool isLocked() const
          { return m_isLocked; }

          /** \brief MutexLocker class virtual destructor.
           *
           */
          virtual ~MutexLocker()
          { if(m_isLocked) m_mutex.unlock(); }

        private:
          QMutex &m_mutex;    /** mutex reference.                                */
          bool    m_isLocked; /** true if the lock is locked and false otherwise. */
      };

    } // namespace Utils
  } // namespace Core
} // namespace ESPINA

#endif // CORE_UTILS_LOCKER_H_
