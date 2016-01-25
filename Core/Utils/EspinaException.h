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

#ifndef CORE_UTILS_ESPINAEXCEPTION_H_
#define CORE_UTILS_ESPINAEXCEPTION_H_

// Qt
#include <QString>

// C++
#include <exception>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      /** \class EspinaException
       * \brief Class for debugging purposes.
       *
       */
      class EspinaException
      : public std::exception
      {
        public:
          /** \brief EspinaException class constructor.
           * \param[in] what standard exception message.
           * \param[in] info additional exception information about the exception context.
           *
           */
          explicit EspinaException(const QString &what, const QString &info);

          /** \brief EspinaException class destructor.
           *
           */
          virtual ~EspinaException();

          virtual const char *what() const noexcept override final;

          /** \brief Returns the additional information about the exception context, if any.
           *
           */
          const QString& details() const;

        private:
          const QString m_what; /** standard 'what' message. */
          const QString m_info; /** more detailed information about the exception context for debug purposes */
      };
    
    } // namespace Utils
  } // namespace Core
} // namespace ESPINA

#endif // CORE_UTILS_ESPINAEXCEPTION_H_