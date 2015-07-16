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

#ifndef ESPINA_CORE_ANALYSIS_EXTENSIBLE_H
#define ESPINA_CORE_ANALYSIS_EXTENSIBLE_H

#include <Core/EspinaTypes.h>
#include <QReadWriteLock>
#include <QMap>
#include <QVariant>

namespace ESPINA {
  namespace Core {
    namespace Analysis {

      struct Existing_Extension{};

      struct Extension_Not_Found{};

      template<typename E, typename T>
      class Extensions
      {
      public:
        using ExtensionSPtr  = std::shared_ptr<E>;
        using ExtensionSList = QList<ExtensionSPtr>;

        class Iterator
        {
        public:
          Iterator(const Extensions<E, T> *container, unsigned index)
          : m_containter(container)
          , m_index(index)
          {}

          bool operator !=(const Iterator &rhs)
          { return m_index != rhs.m_index; }

          const Iterator& operator++()
          {
            m_index++;
            return *this;
          }

          ExtensionSPtr &operator*() const
          {
            auto extensions = m_containter->m_extensions;
            return extensions[extensions.keys()[m_index]];
          }

        private:
          const Extensions<E, T> *m_containter;
          unsigned m_index;
        };

      public:
        Extensions(T *item)
        : m_item(item)
        {}
        /** \brief Adds a extension to extendible
         * \param[in] extension to be added to extendible object
         *
         * Extesion won't be available until requirements are satisfied
         *
         */
        void add(ExtensionSPtr extension);

        /** \brief Deletes an extension from extendible
         * \param[in] extension to be deleted
         *
         */
        void remove(ExtensionSPtr extension);

        /** \brief Deletes an extension from extendible
         * \param[in] extension type to be removed
         *
         */
        void remove(const typename E::Type &extension);

        /** \brief Check whether or not there is an extension with the given type.
         * \param[in] extension type
         *
         */
        bool hasExtension(const typename E::Type& extension) const;

        /** \brief Check whether or not there is an extension with the given type.
         * \param[in] extension type
         *
         */
        bool hasExtension(const std::shared_ptr<E>& extension) const;

        /** \brief Return the extension with the especified type.
         * \param[in] extension type
         */
        ExtensionSPtr operator[](const typename E::Type& extension) const;

        template<typename Extension>
        std::shared_ptr<Extension> get() const
        {
          Q_ASSERT(hasExtension(Extension::TYPE));

          auto base      = operator[](Extension::TYPE);
          auto extension = std::dynamic_pointer_cast<Extension>(base);

          Q_ASSERT(extension);

          return extension;
        };

        /** \brief Returns a list of extensions available for this extendible
         *
         */
        ExtensionSList extensions() const
        { return m_extensions.values(); }

        /** \brief Returns the list of information keys provided by extensible.
         *
         */
        virtual typename E::InformationKeyList availableInformation() const;

        bool hasInformation(const typename E::InformationKey &key) const
        { return availableInformation().contains(key); }

        /** \brief Returns the value of the specified information key.
         * \param[in] key information key.
         *
         */
        virtual QVariant information(const typename E::InformationKey& key) const;

        /** \brief Returns true if the information for a given key has been generated
         * \param[in] key information key
         *
         */
        bool isReady(const typename E::InformationKey &key) const;

        bool isEmpty() const
        { return m_extensions.isEmpty(); }

        /** \brief Return the number of extensions
         *
         */
        unsigned int size() const
        { return m_extensions.size(); }

        Iterator begin() const
        { return Iterator(this, 0); }

        Iterator end() const
        { return Iterator(this, m_extensions.size()); }

      private:
        using ExtensionSMap = QMap<typename E::Type, ExtensionSPtr>;

        QReadWriteLock m_lock;
        ExtensionSMap  m_extensions;

        T             *m_item;

        template<typename U, typename V> friend class ReadLockExtensions;
        template<typename U, typename V> friend class WriteLockExtensions;
      };


      template<typename E, typename T>
      class ReadLockExtensions
      {
      public:
        ReadLockExtensions(const Extensions<E, T> &extensions)
        : m_extensions(extensions)
        { const_cast<Extensions<E, T> *>(&m_extensions)->m_lock.lockForRead(); }

        ~ReadLockExtensions()
        { const_cast<Extensions<E, T> *>(&m_extensions)->m_lock.unlock(); }

        typename Extensions<E, T>::Iterator begin() const
        { return m_extensions.begin(); }

        typename Extensions<E, T>::Iterator end() const
        { return m_extensions.end(); }

        typename Extensions<E, T>::ExtensionSPtr operator[] (const typename E::Type &type) const
        { return m_extensions[type]; }

        const Extensions<E, T> * operator ->() const
        { return &m_extensions; }

      private:
        const Extensions<E, T> &m_extensions;
      };

      template<typename E, typename T>
      class WriteLockExtensions
      {
      public:
        WriteLockExtensions(Extensions<E, T> &extensions)
        : m_extensions(extensions)
        { m_extensions.m_lock.lockForWrite(); }

        ~WriteLockExtensions()
        { m_extensions.m_lock.unlock(); }

        typename Extensions<E, T>::ExtensionSPtr operator[] (const typename E::Type &type)
        { return m_extensions[type]; }

        typename Extensions<E, T>::Iterator begin() const
        { return m_extensions.begin(); }

        typename Extensions<E, T>::Iterator end() const
        { return m_extensions.end(); }


        Extensions<E, T> * operator ->() const
        { return &m_extensions; }

      private:
        Extensions<E, T> &m_extensions;
      };

      template<typename E, typename T>
      class Extensible
      {
      public:
        Extensible(T *item)
        : m_extensions(item)
        {}

        const ReadLockExtensions<E, T> readOnlyExtensions() const
        { return ReadLockExtensions<E, T>(m_extensions); }

        WriteLockExtensions<E, T> extensions()
        { return WriteLockExtensions<E, T>(m_extensions); }

        /** \brief Returns the value of the specified information key.
         * \param[in] key information key.
         *
         * WARNING: Do not use this method if you are already accessing via Read/WriteLockExtensions
         *
         */
        virtual QVariant information(const typename E::InformationKey& key) const;

      private:
        Extensions<E, T> m_extensions;
      };
    }
  }
}

#include <Core/Analysis/Extensible.cxx>

#endif // ESPINA_CORE_ANALYSIS_EXTENSIBLE_H
