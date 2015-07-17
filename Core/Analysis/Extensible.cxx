/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "Extensible.hxx"

namespace ESPINA
{
  namespace Core
  {
    namespace Analysis
    {
      //------------------------------------------------------------------
      template<typename E, typename T>
      void Extensions<E, T>::add(ExtensionSPtr extension)
      {
        if (m_extensions.contains(extension->type()))
        {
          throw Existing_Extension();
        }

        extension->setExtendedItem(m_item);

        m_extensions.insert(extension->type(), extension);
      }

      //------------------------------------------------------------------
      template<typename E, typename T>
      void Extensions<E, T>::remove(ExtensionSPtr extension)
      {
        remove(extension->type());
      }

      //------------------------------------------------------------------
      template<typename E, typename T>
      void Extensions<E, T>::remove(const typename E::Type &extension)
      {
        if (!m_extensions.contains(extension))
        {
          throw Extension_Not_Found();
        }

        m_extensions.remove(extension);
      }

      //------------------------------------------------------------------
      template<typename E, typename T>
      bool Extensions<E, T>::hasExtension(const typename E::Type& extension) const
      {
        return m_extensions.contains(extension);
      }

      //------------------------------------------------------------------
      template<typename E, typename T>
      bool Extensions<E, T>::hasExtension(const std::shared_ptr<E>& extension) const
      {
        return hasExtension(extension->type());
      }

      //------------------------------------------------------------------
      template<typename E, typename T>
      typename Extensions<E, T>::ExtensionSPtr Extensions<E, T>::operator[](const typename E::Type& type) const
      {
        if (!m_extensions.contains(type))
        {
          throw Extension_Not_Found();
        }

        return m_extensions[type];
      }

      //------------------------------------------------------------------
      template<typename E, typename T>
      typename E::InformationKeyList Extensions<E, T>::availableInformation() const
      {
        typename E::InformationKeyList list;

        for (auto extension : m_extensions.values())
        {
          list << extension->availableInformation();
        }

        return list;
      }

      //------------------------------------------------------------------
      template<typename E, typename T>
      QVariant Extensions<E, T>::information(const typename E::InformationKey& key) const
      {
        if (m_extensions.contains(key.extension()))
        {
          auto extension = m_extensions[key.extension()];

          if (extension->hasInformation(key))
          {
            return extension->information(key);
          }
        }

        return QVariant();
      }

      //------------------------------------------------------------------
      template<typename E, typename T>
      bool Extensions<E, T>::isReady(const typename E::InformationKey& key) const
      {
        for(auto extension: m_extensions.values())
        {
          if (extension->hasInformation(key))
          {
            return extension->isReady(key);
          }
        }

        return false;
      }

      //------------------------------------------------------------------
      template<typename E, typename T>
      QVariant Extensible<E, T>::information(const typename E::InformationKey& key) const
      {
        typename Extensions<E, T>::ExtensionSPtr extension;

        {
          auto extensions = readOnlyExtensions();

          if (extensions->hasExtension(key.extension()))
          {
            extension = extensions[key.extension()];
          }
        }

        QVariant information;

        if (extension && extension->hasInformation(key))
        {
          information = extension->information(key);
        }

        return information;
      }

    }
  }
}