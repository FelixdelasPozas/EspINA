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

// ESPINA
#include "EspinaException.h"

// QT
#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QTextStream>
#include <QDir>

// C++
#include <iostream>
#include <execinfo.h>
#include <cxxabi.h>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace ESPINA::Core::Utils;

static int const STACK_FRAMES = 40;

const int ESPINA::Core::Utils::STACK_SIZE = 8192;
uint8_t ESPINA::Core::Utils::alternate_stack[ESPINA::Core::Utils::STACK_SIZE];

//-----------------------------------------------------------------------------
EspinaException::EspinaException(const QString& what, const QString& info)
: m_what{what.toStdString().c_str()}
, m_info{info.toStdString().c_str()}
{
//  std::cerr << "ESPINA EXCEPTION" << std::endl;
//  std::cerr << what.toStdString().c_str() << std::endl;
//  std::cerr << info.toStdString().c_str() << std::endl;
//  std::cerr << std::flush;
}

//-----------------------------------------------------------------------------
EspinaException::~EspinaException()
{
}

//-----------------------------------------------------------------------------
const char* EspinaException::what() const noexcept
{
  return m_what.c_str();
}

//-----------------------------------------------------------------------------
const char* EspinaException::details() const
{
  return m_info.c_str();
}

//-----------------------------------------------------------------------------
void signalHandler(int signal, siginfo_t *siginfo, void *context)
{
  const char *signal_text = nullptr;

  switch(signal)
  {
    case SIGSEGV:
      signal_text = "SIGSEGV: segmentation fault";
      break;
    case SIGINT:
      signal_text = "SIGINT: interactive attention signal (ctrl+c ?)";
      break;
    case SIGFPE:
      switch(siginfo->si_code)
      {
        case FPE_INTDIV:
          signal_text = "SIGFPE: integer divide by zero";
          break;
        case FPE_INTOVF:
          signal_text = "SIGFPE: integer overflow";
          break;
        case FPE_FLTDIV:
          signal_text = "SIGFPE: floating-point divide by zero";
          break;
        case FPE_FLTOVF:
          signal_text = "SIGFPE: floating-point overflow";
          break;
        case FPE_FLTUND:
          signal_text = "SIGFPE: floating-point underflow";
          break;
        case FPE_FLTRES:
          signal_text = "SIGFPE: floating-point inexact result";
          break;
        case FPE_FLTINV:
          signal_text = "SIGFPE: floating-point invalid operation";
          break;
        case FPE_FLTSUB:
          signal_text = "SIGFPE: subscript out of range";
          break;
        default:
          signal_text = "SIGFPE: arithmetic exception";
          break;
      }
      break;
    case SIGILL:
      switch(siginfo->si_code)
      {
        case ILL_ILLOPC:
          signal_text = "SIGILL: illegal opcode";
          break;
        case ILL_ILLOPN:
          signal_text = "SIGILL: illegal operand";
          break;
        case ILL_ILLADR:
          signal_text = "SIGILL: illegal addressing mode";
          break;
        case ILL_ILLTRP:
          signal_text = "SIGILL: illegal trap";
          break;
        case ILL_PRVOPC:
          signal_text = "SIGILL: privileged opcode";
          break;
        case ILL_PRVREG:
          signal_text = "SIGILL: privileged register";
          break;
        case ILL_COPROC:
          signal_text = "SIGILL: coprocessor error";
          break;
        case ILL_BADSTK:
          signal_text = "SIGILL: internal stack error";
          break;
        default:
          signal_text = "SIGILL: illegal instruction";
          break;
      }
      break;
    case SIGTERM:
      signal_text = "SIGTERM: explicit termination request";
      break;
    case SIGABRT:
      signal_text = "SIGABRT: probably assert()";
      break;
    default:
      signal_text = "Unidentified signal.";
      break;
  }

  auto date     = QDate::currentDate();
  auto time     = QTime::currentTime();
  auto fileName = QObject::tr("EspINA-dump-%1_%2_%3-%4_%5_%6.txt").arg(date.year(),   4, 10, QChar('0'))
                                                                  .arg(date.month(),  2, 10, QChar('0'))
                                                                  .arg(date.day(),    2, 10, QChar('0'))
                                                                  .arg(time.hour(),   2, 10, QChar('0'))
                                                                  .arg(time.minute(), 2, 10, QChar('0'))
                                                                  .arg(time.second(), 2, 10, QChar('0'));
  QFile file{QDir::home().filePath(fileName)};
  if(file.open(QIODevice::Truncate|QIODevice::ReadWrite))
  {
    QTextStream out(&file);
    out << "-- ESPINA CRASH ------------------------------------------\n";
    out << "WHEN:" << date.toString() << " " << time.toString() << "\n";
    out << "SIGNAL: " << signal_text << "\n";
    ESPINA::Core::Utils::backtrace_stack_print(out);
  }

  std::_Exit(1);
}

//-----------------------------------------------------------------------------
void exceptionHandler()
{
  auto exptr = std::current_exception();

  const char *message = nullptr;
  const char *details = nullptr;

  try
  {
    std::rethrow_exception(exptr);
  }
  catch(const EspinaException &e)
  {
    message = e.what();
    details = e.details();
  }
  catch (const std::exception &e)
  {
    message = e.what();
  }
  catch(...)
  {
    message = "Unidentified exception.\n";
  }

  auto date     = QDate::currentDate();
  auto time     = QTime::currentTime();
  auto fileName = QObject::tr("EspINA-dump-%1_%2_%3-%4_%5_%6.txt").arg(date.year(),   4, 10, QChar('0'))
                                                                  .arg(date.month(),  2, 10, QChar('0'))
                                                                  .arg(date.day(),    2, 10, QChar('0'))
                                                                  .arg(time.hour(),   2, 10, QChar('0'))
                                                                  .arg(time.minute(), 2, 10, QChar('0'))
                                                                  .arg(time.second(), 2, 10, QChar('0'));
  QFile file{QDir::home().filePath(fileName)};
  if(file.open(QIODevice::Truncate|QIODevice::ReadWrite))
  {
    QTextStream out(&file);
    out << "-- ESPINA CRASH ------------------------------------------\n";
    out << "WHEN:" << date.toString() << " " << time.toString() << "\n";
    out << "EXCEPTION MESSAGE" << message << "\n";
    if(details) out << "EXCEPTION DETAILS: " << details << "\n";
    ESPINA::Core::Utils::backtrace_stack_print(out);
  }

  std::_Exit(1);
}

//-----------------------------------------------------------------------------
void ESPINA::Core::Utils::installSignalHandler()
{
  /* setup alternate stack */
  stack_t ss;
  ss.ss_sp = (void*) alternate_stack;
  ss.ss_size = STACK_SIZE;
  ss.ss_flags = 0;

  if (sigaltstack(&ss, NULL) != 0)
  {
    auto message = QObject::tr("Error setting the alternative stack for handling signals.");
    auto details = QObject::tr("installSignalHandler -> ") + message;

    throw EspinaException(message, details);
  }

  /* register signal handlers */
  struct sigaction sig_action;
  sig_action.sa_sigaction = signalHandler;
  sigemptyset(&sig_action.sa_mask);

  sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;

  if ((sigaction(SIGSEGV, &sig_action, NULL) != 0) ||
      (sigaction(SIGFPE,  &sig_action, NULL) != 0) ||
      (sigaction(SIGINT,  &sig_action, NULL) != 0) ||
      (sigaction(SIGILL,  &sig_action, NULL) != 0) ||
      (sigaction(SIGTERM, &sig_action, NULL) != 0) ||
      (sigaction(SIGABRT, &sig_action, NULL) != 0))
  {
    auto message = QObject::tr("Error setting the handlers for signals.");
    auto details = QObject::tr("installSignalHandler -> ") + message;

    throw EspinaException(message, details);
  }
}

//-----------------------------------------------------------------------------
void ESPINA::Core::Utils::installExceptionHandler()
{
  std::set_terminate(exceptionHandler);
}

//-----------------------------------------------------------------------------
void ESPINA::Core::Utils::backtrace_stack_print(QTextStream &stream)
{
  stream << "-- STACK TRACE -------------------------------------------\n";

  void *stack_traces[STACK_FRAMES];
  auto trace_size = backtrace(stack_traces, STACK_FRAMES);
  auto messages = backtrace_symbols(stack_traces, trace_size);

  size_t funcnamesize = 1024;
  char funcname[1024];
  for (int i = 0; i < trace_size - 1; i++)
  {
    char* begin_name = nullptr;
    char* begin_pos  = nullptr;
    char* end_pos    = nullptr;

    // find parentheses and +address offset surrounding the mangled name
    // ./module(function+0x15c) [0x8048a6d]
    for (char *p = messages[i]; *p; ++p)
    {
      if (*p == '(')
      {
        begin_name = p;
      }
      else
      {
        if (*p == '+')
        {
          begin_pos = p;
        }
        else
        {
          if (*p == ')' && (begin_pos || begin_name))
          {
            end_pos = p;
          }
        }
      }
    }

    if (begin_name && end_pos && (begin_name < end_pos))
    {
      *begin_name++ = '\0';
      *end_pos++ = '\0';
      if (begin_pos)
      {
        *begin_pos++ = '\0';
      }

      // apply __cxa_demangle with the current positions.
      int status = 0;
      auto ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
      auto func_name = begin_name;
      if (status == 0)
      {
        func_name = ret;
      }

      stream << messages[i] << " (";
      stream << ((func_name[0] != '\0') ? func_name : "unidentified") << ",";
      stream << (begin_pos ? begin_pos : "unknown");
      stream << ") " << end_pos << "\n";
    }
    else
    {
      // couldn't parse the line? print the whole line.
      stream << messages[i] << "\n";
    }
  }

  if (messages) free(messages);
}
