/*
 *
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_LIST_UTILS_H
#define ESPINA_LIST_UTILS_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <QList>

// C++
#include <memory>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      //-----------------------------------------------------------------------------
      template <typename T>
      QList<T *> EspinaCore_EXPORT createRawList(const T *item)
      {
        QList<T *> result;

        result << item;

        return result;
      }

      //-----------------------------------------------------------------------------
      template <typename T>
      QList<T *> EspinaCore_EXPORT createRawList(const std::shared_ptr<T> &item)
      {
        return createRawList<T>(item.get());
      }

      //-----------------------------------------------------------------------------
      template <typename T>
      QList<T *> EspinaCore_EXPORT rawList(const QList<std::shared_ptr<T>> &list)
      {
        QList<T *> result;

        for (auto item : list)
        {
          result << item.get();
        }

        return result;
      }

      //-----------------------------------------------------------------------------
      template <typename T, typename F>
      QList<T *> EspinaCore_EXPORT toRawList(const F &list)
      {
        QList<T *> result;

        for (auto item : list)
        {
          result << item.get();
        }

        return result;
      }

      //-----------------------------------------------------------------------------
      template <typename T, typename F>
      QList<std::shared_ptr<T>> EspinaCore_EXPORT toList(const F &list)
      {
        QList<std::shared_ptr<T>> result;

        for (auto item : list)
        {
          result << item;
        }

        return result;
      }

      //-----------------------------------------------------------------------------
      template <typename T, typename F>
      QList<T *> EspinaCore_EXPORT toList(const QList<F *> &list)
      {
        QList<T *> result;

        for (auto item : list)
        {
          result << dynamic_cast<T *>(item);
        }

        return result;
      }

      //-----------------------------------------------------------------------------
      template <typename T>
      QSet<T *> EspinaCore_EXPORT toRawSet(const QList<std::shared_ptr<T>> &list)
      {
        QSet<T> result;

        for(auto item: list)
        {
          result.insert(item.get());
        }

        return result;
      }
    }
  }
}

#endif // ESPINA_LIST_UTILS_H
