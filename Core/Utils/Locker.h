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

// Qt
#include <QReadWriteLock>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      /** \class Locker
       * \brief Auxiliary class to lock objects for read or read-write access.
       */
      class Locker
      {
        public:
          /** \brief Locker class constructor.
           * \param[in] locker read/write lock object to use.
           */
          explicit Locker(QReadWriteLock &locker)
          : m_locker(locker)
          {}

          /** \brief Locker virtual destructor.
           *
           */
          virtual ~Locker()
          { m_locker.unlock();}

        protected:
          QReadWriteLock &m_locker; /** object lock */
      };

      /** \class ReadLocker
       * \brief Implements a read-only Output::Locker.
       */
      class ReadLocker
      : public Locker
      {
        public:
          /** \brief ReadLocker class constructor.
           * \param[in] locker read/write lock object to use.
           *
           */
          explicit ReadLocker(QReadWriteLock &locker)
          : Locker(locker)
          { m_locker.lockForRead(); }
      };

      /** \class WriteLocker
       * \brief Implements a read-write Output::Locker
       *
       */
      class WriteLocker
      : public Locker
      {
        public:
          /** \brief WriteLocker class constructor.
           * \param[in] locker read/write lock object to use.
           *
           */
          explicit WriteLocker(QReadWriteLock &locker)
          : Locker(locker)
          { m_locker.lockForWrite(); }
      };
    
    } // namespace Utils
  } // namespace Core
} // namespace ESPINA

#endif // CORE_UTILS_LOCKER_H_
