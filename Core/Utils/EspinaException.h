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

#include "Core/EspinaCore_Export.h"

// Qt
#include <QString>

// C++
#include <exception>
#include <signal.h>

class QTextStream;

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
      class EspinaCore_EXPORT EspinaException
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

          virtual const char* what() const noexcept override final;

          /** \brief Returns the additional information about the exception context, if any.
           *
           */
          virtual const char* details() const;

        private:
          std::string m_what; /** standard 'what' message. */
          std::string m_info; /** more detailed information about the exception context for debug purposes */
      };

      /** \brief Installs the signal handler and reserves the memory for an
       *         alternate stack for tracing.
       *
       */
      void EspinaCore_EXPORT installSignalHandler();

      /** \brief Installs the handler for unmanaged exceptions.
       *
       */
      void EspinaCore_EXPORT installExceptionHandler();

      /** \brief Helper function to trace the stack and print method names.
       * \param[in] stream text stream where the stack info will be written.
       *
       */
      void EspinaCore_EXPORT backtrace_stack_print(QTextStream &stream);

      extern const int STACK_SIZE;
      extern uint8_t alternate_stack[];

    } // namespace Utils
  } // namespace Core
} // namespace ESPINA

#endif // CORE_UTILS_ESPINAEXCEPTION_H_
