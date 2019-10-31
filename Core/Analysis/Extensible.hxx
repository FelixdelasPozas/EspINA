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

// ESPINA
#include <Core/Types.h>
#include <Core/Utils/Locker.h>
#include <Core/Utils/EspinaException.h>

// Qt
#include <QReadWriteLock>
#include <QMap>
#include <QVariant>

namespace ESPINA
{
  namespace Core
  {
    /** \class Extensions
     * \brief Manages item extensions. Class is templated over extended item T and extensions type E.
     *
     */
    template<typename E, typename T>
    class Extensions
    {
      public:
        using ExtensionSPtr  = std::shared_ptr<E>;
        using ExtensionSList = QList<ExtensionSPtr>;

        /** \class Iterator
         * \brief Item extensions iterator.
         *
         */
        class Iterator
        {
          public:
            /** \brief Extensible Iterator class constructor.
             * \param[in] container extension map.
             * \param[in] index current iterator index.
             *
             */
            Iterator(const Extensions<E, T> *container, unsigned index)
            : m_container(container)
            , m_index    (index)
            {}

            /** \brief Operator not equal.
             * \param[in] rhs right hand side iterator.
             *
             */
            bool operator !=(const Iterator &rhs)
            { return m_index != rhs.m_index; }

            /** \brief Operator ++
             *
             */
            const Iterator& operator++()
            {
              ++m_index;
              return *this;
            }

            /** \brief Operator ()
             *
             */
            ExtensionSPtr &operator*() const
            {
              auto extensions = m_container->m_extensions;
              return extensions[extensions.keys()[m_index]];
            }

          private:
            const Extensions<E, T> *m_container; /** extensions container map. */
            unsigned                m_index;     /** current iterator index.   */
        };

      public:
        /** \brief Extensions class constructor.
         * \param[in] item extended item pointer.
         *
         */
        Extensions(T *item)
        : m_lock{ QReadWriteLock::NonRecursive }
        , m_item(item)
        {}

        /** \brief Extensions class virtual destructor.
         *
         */
        virtual ~Extensions()
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

        /** \brief Returns the extension of the specified type in the template parameter.
         *
         */
        template<typename Extension>
        std::shared_ptr<Extension> get() const
        {
          Q_ASSERT(hasExtension(Extension::TYPE));

          auto base = operator[](Extension::TYPE);
          auto extension = std::dynamic_pointer_cast<Extension>(base);

          Q_ASSERT(extension);

          return extension;
        };

        /** \brief Returns a list of extensions available for this extendible
         *
         */
        ExtensionSList extensions() const
        { return m_extensions.values(); }

        /** \brief Returns a list of available extensions.
         *
         */
        const QList<typename E::Type> available() const
        { return m_extensions.keys(); }

        /** \brief Returns the list of information keys provided by extensible.
         *
         */
        virtual const typename E::InformationKeyList availableInformation() const;

        /** \brief Returns true if any of the extensions has an information corresponding to the given key.
         * \param[in] key information key.
         *
         */
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

        /** \brief Returns true if the extensions map is empty.
         *
         */
        bool isEmpty() const
        { return m_extensions.isEmpty(); }

        /** \brief Return the number of extensions
         *
         */
        unsigned int size() const
        { return m_extensions.size(); }

        /** \brief Begin extensions iterator.
         *
         */
        Iterator begin() const
        { return Iterator(this, 0); }

        /** \brief End extensions iterator.
         *
         */
        Iterator end() const
        { return Iterator(this, m_extensions.size()); }

      private:
        using ExtensionSMap = QMap<typename E::Type, ExtensionSPtr>;

        QReadWriteLock m_lock;       /** access lock.    */
        ExtensionSMap  m_extensions; /** extensions map. */

        T *m_item; /** extended item. */

        template<typename U, typename V> friend class ReadLockExtensions;
        template<typename U, typename V> friend class WriteLockExtensions;
    };

    /** \class ReadLockExtensions
     * \brief Locks extensions class for reading. Templated over extended item T and extension type E.
     *
     */
    template<typename E, typename T>
    class ReadLockExtensions: private Core::Utils::ReadLocker
    {
      public:
        /** \brief ReadLockExtensions class constructor.
         * \param[in] extensions extensions class object.
         *
         */
        ReadLockExtensions(const Extensions<E, T> &extensions)
        : Core::Utils::ReadLocker(const_cast<Extensions<E, T> *>(&extensions)->m_lock)
        , m_extensions           (extensions)
        {}

        /** \brief ReadLockExtensions class destructor.
         *
         */
        virtual ~ReadLockExtensions()
        {}

        /** \brief Extensions Iterator begin.
         *
         */
        typename Extensions<E, T>::Iterator begin() const
        { return m_extensions.begin(); }

        /** \brief Extensions Iterator end.
         *
         */
        typename Extensions<E, T>::Iterator end() const
        { return m_extensions.end(); }

        /** \brief Extensions operator []
         * \param[in] type extension type.
         *
         */
        typename Extensions<E, T>::ExtensionSPtr operator[](const typename E::Type &type) const
        { return m_extensions[type]; }

        /** \brief Extensions indirection operator.
         *
         */
        const Extensions<E, T> * operator ->() const
        { return &m_extensions; }

      private:
        const Extensions<E, T> &m_extensions; /** extensions map. */
    };

    /** \class WriteLockExtensions
     * \brief Locks extensions class for writing. Templated over extended item T and extension type E.
     *
     */
    template<typename E, typename T>
    class WriteLockExtensions
    : private Core::Utils::WriteLocker
    {
      public:
        /** \brief WriteLockExtensions class constructor.
         * \param[in] extensions extensions class object.
         *
         */
        WriteLockExtensions(Extensions<E, T> &extensions)
        : Core::Utils::WriteLocker(extensions.m_lock)
        , m_extensions            (extensions)
        {}

        /** \brief WriteLockExtensions class destructor.
         *
         */
        virtual ~WriteLockExtensions()
        {}

        /** \brief Extensions Iterator begin.
         *
         */
        typename Extensions<E, T>::Iterator begin() const
        { return m_extensions.begin(); }

        /** \brief Extensions Iterator end.
         *
         */
        typename Extensions<E, T>::Iterator end() const
        { return m_extensions.end(); }

        /** \brief Extensions operator []
         * \param[in] type extension type.
         *
         */
        typename Extensions<E, T>::ExtensionSPtr operator[](const typename E::Type &type)
        { return m_extensions[type]; }

        /** \brief Extensions indirection operator.
         *
         */
        Extensions<E, T> * operator ->() const
        { return &m_extensions; }

      private:
        Extensions<E, T> &m_extensions; /** extensions map. */
    };

    /** \class Extensible
     * \brief Defines an extensible class T extended by a group of extensions of type E.
     *
     */
    template<typename E, typename T>
    class Extensible
    {
      public:
        /** \brief Extensible class constructor.
         * \param[in] item model item to be extended.
         *
         */
        Extensible(T *item)
        : m_extensions(item)
        {}

        /** \brief Extensible class virtual destructor.
         *
         */
        virtual ~Extensible()
        {}

        /** \brief Returns the map of extensions in read-only mode. Locks extensions map for reading.
         *
         */
        const ReadLockExtensions<E, T> readOnlyExtensions() const
        { return ReadLockExtensions<E, T>(m_extensions); }

        /** \brief Returns the map of extensions in write mode. Locks extensions map for writing.
         *
         */
        WriteLockExtensions<E, T> extensions()
        { return WriteLockExtensions<E, T>(m_extensions); }

        /** \brief Returns the value of the specified information key.
         * \param[in] key information key.
         *
         * NOTE: Do not use this method if you are already accessing via Read/WriteLockExtensions
         *
         */
        virtual QVariant information(const typename E::InformationKey& key) const;

      private:
        Extensions<E, T> m_extensions; /** extensions map. */
    };

    //------------------------------------------------------------------
    template<typename E, typename T>
    void Extensions<E, T>::add(ExtensionSPtr extension)
    {
      if (hasExtension(extension))
      {
        auto what = QObject::tr("Attempt to add an existing extension, type: %1").arg(extension->type());
        auto details = QObject::tr("Extensions::add() -> Attempt to add an existing extension, type: %1").arg(extension->type());

        throw Core::Utils::EspinaException(what, details);
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
      if (!hasExtension(extension))
      {
        auto what = QObject::tr("Attempt to remove an unregistered extension, type: %1").arg(extension);
        auto details = QObject::tr("Extensions::remove() -> Attempt to obtain an unregistered extension, type: %1").arg(extension);

        throw Core::Utils::EspinaException(what, details);
      }

      m_extensions.remove(extension);
    }

    //------------------------------------------------------------------
    template<typename E, typename T>
    bool Extensions<E, T>::hasExtension(const typename E::Type& extension) const
    {
      return m_extensions.keys().contains(extension);
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
      if (!hasExtension(type))
      {
        auto what = QObject::tr("Attempt to obtain an unregistered extension, type: %1").arg(type);
        auto details = QObject::tr("Extensions::operator[] -> Attempt to obtain an unregistered extension, type: %1").arg(type);

        throw Core::Utils::EspinaException(what, details);
      }

      return m_extensions[type];
    }

    //------------------------------------------------------------------
    template<typename E, typename T>
    const typename E::InformationKeyList Extensions<E, T>::availableInformation() const
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
      if (hasExtension(key.extension()))
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
      for (auto extension : m_extensions.values())
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

#endif // ESPINA_CORE_ANALYSIS_EXTENSIBLE_H
