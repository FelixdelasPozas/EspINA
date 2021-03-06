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

// ESPINA
#include <QList>

// C++
#include <memory>

// Qt
#include <QObject>
#include <QString>
#include <QRegExp>
#include <QtAlgorithms>
#include <QList>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      //-----------------------------------------------------------------------------
      template <typename T>
      QList<T *> createRawList(const T *item)
      {
        QList<T *> result;

        result << item;

        return result;
      }

      //-----------------------------------------------------------------------------
      template <typename T>
      QList<T *> createRawList(const std::shared_ptr<T> &item)
      {
        return createRawList<T>(item.get());
      }

      //-----------------------------------------------------------------------------
      template <typename T>
      QList<T *> rawList(const QList<std::shared_ptr<T>> &list)
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
      QList<T *> toRawList(const F &list)
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
      QList<std::shared_ptr<T>> toList(const F &list)
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
      QList<T *> toList(const QList<F *> &list)
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
      QSet<T *> toRawSet(const QList<std::shared_ptr<T>> &list)
      {
        QSet<T> result;

        for(auto item: list)
        {
          result.insert(item.get());
        }

        return result;
      }

      //-----------------------------------------------------------------------------
      template <typename T>
      bool lessThan(T left, T right)
      {
        auto lstring = left->data(Qt::DisplayRole).toString();
        auto rstring = right->data(Qt::DisplayRole).toString();
        auto lparts = lstring.split(' ');
        auto rparts = rstring.split(' ');

        // check same category
        if(lparts[0] != rparts[0])
        {
          return lstring < rstring;
        }

        // same category, check numbers
        QRegExp numExtractor("(\\d+)");
        numExtractor.setMinimal(false);

        if ((numExtractor.indexIn(lstring) == -1) || (numExtractor.indexIn(rstring) == -1))
        {
          return lstring < rstring;
        }

        // use the last number, we can't be sure that there is only one
        int pos      = 0;
        int numLeft  = 0;
        int numRight = 0;

        while ((pos = numExtractor.indexIn(lstring, pos)) != -1)
        {
          numLeft = numExtractor.cap(1).toInt();
          pos += numExtractor.matchedLength();
        }

        pos = 0;
        while ((pos = numExtractor.indexIn(rstring, pos)) != -1)
        {
          numRight = numExtractor.cap(1).toInt();
          pos += numExtractor.matchedLength();
        }

        if (numLeft == numRight)
        {
          return lstring < rstring;
        }

        // else not equal
        return numLeft < numRight;
      }

      //-----------------------------------------------------------------------------
      template <typename T>
      void sort(QList<std::shared_ptr<T>> &list)
      {
        qSort(list.begin(), list.end(), lessThan<std::shared_ptr<T>>);
      }

      //-----------------------------------------------------------------------------
      template <typename T>
      void sort(QList<T> &list)
      {
        qSort(list.begin(), list.end(), lessThan<T>);
      }
    }
  }
}

#endif // ESPINA_LIST_UTILS_H
