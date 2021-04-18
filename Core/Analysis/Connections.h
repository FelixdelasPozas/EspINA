/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef CORE_ANALYSIS_CONNECTIONS_H_
#define CORE_ANALYSIS_CONNECTIONS_H_

// ESPINA
#include <Core/Types.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Utils/Vector3.hxx>

// Qt
#include <QMap>
#include <QList>
#include <QString>
#include <QVector3D>
#include <QReadWriteLock>

namespace ESPINA
{
  class Analysis;

  namespace Core
  {
    /** \struct Connection
     * \brief Defines a connection between two segmentations and it's connection point.
     *
     */
    struct EspinaCore_EXPORT Connection
    {
        PersistentSPtr segmentation1; /** segmentation object. */
        PersistentSPtr segmentation2; /** segmentation object. */
        NmVector3      point;         /** connection point.    */

        const static RelationName CONNECTS;
    };

    using Connections = QList<Connection>;


    class EspinaCore_EXPORT ConnectionStorage
    {
      public:
        /** \brief ConnectionStorage class constructor.
         *
         */
        explicit ConnectionStorage()
        {}

        /** \brief ConnectionStorage class virtual destructor.
         *
         */
        virtual ~ConnectionStorage()
        {}

        /** \brief Returns the name of the file where the connections data is stored in the temporal storage.
         *
         */
        static const QString connectionsFileName()
        { return "connections.bin"; }

      private:
        struct Connection
        {
            QString   segmentation1;
            QString   segmentation2;
            NmVector3 point;
        };

        using Connections = QList<Connection>;

        /** \breif Sets the temporal storage object for this class.
         * \param[in] storage temporal storage object.
         *
         */
        void setStorage(TemporalStorageSPtr storage)
        { m_storage = storage; }

        /** \brief Adds a connection and returns true if successful and false otherwise.
         * \param[in] segmentation1 segmentation object.
         * \param[in] segmentation2 segmentation object.
         * \param[in] point connection point in space.
         *
         */
        bool addConnection(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2, const NmVector3 &point);

        /** \brief Removes a connection and returns true if successful and false otherwise.
         * \param[in] segmentation1 segmentation object.
         * \param[in] segmentation2 segmentation object.
         * \param[in] point connection point in space.
         *
         */
        bool removeConnection(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2, const NmVector3 &point);

        /** \brief Removes all connections between the given segmentations.
         * \param[in] segmentation1 segmentation object.
         * \param[in] segmentation2 segmentation object.
         *
         */
        bool removeConnections(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2);

        /** \brief Removes a given segmentation connections.
         * \param[in] segmentation segmentation object.
         *
         */
        bool removeSegmentation(const PersistentSPtr segmentation);

        /** \brief Returns a list of connections for the given segmentation.
         * \param[in] segmentation segmentation object.
         *
         */
        ConnectionStorage::Connections connections(const PersistentSPtr segmentation) const;

        /** \brief Returns the list of connections between the given segmentations.
         * \param[in] segmentation1 segmentation object.
         * \param[in] segmentation2 segmentation object.
         *
         */
        ConnectionStorage::Connections connections(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2) const;

        /** \brief Saves the connections data to the temporal storage. Returns true if saved data to disk and false otherwise (empty storage).
         *
         */
        bool save() const;

        /** \brief Loads the connections data from the temporal storage. Returns true if loaded data from disk and false otherwise (no data on disk).
         *
         */
        bool load();

        /** \brief Removes all content from the class.
         *
         */
        void clear();

      private:
        friend class ESPINA::Analysis;

        /** \brief Checks if values are empty and removes if so.
         * \param[in] uuid1 identifier1.
         * \param[in] uuid2 identifier1.
         *
         */
        void checkAndRemoveIfEmtpy(const QString &uuid1, const QString &uuid2 = QString());

        mutable QReadWriteLock                         m_lock;    /** data protection read-write lock. */
        QMap<QString, QMap<QString, QList<QVector3D>>> m_data;    /** connection data.                 */
        TemporalStorageSPtr                            m_storage; /** temporal storage object.         */
    };
  } // namespace Core
} // namespace ESPINA

#endif // CORE_ANALYSIS_CONNECTIONS_H_
