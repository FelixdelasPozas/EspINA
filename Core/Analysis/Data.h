/*

 Copyright (C) 2014 Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_DATA_H
#define ESPINA_DATA_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/Types.h"
#include <Core/Utils/VolumeBounds.h>
#include "Persistent.h"

// Qt
#include <QMutex>

// C++
#include <atomic>

namespace ESPINA
{
  class ChangeSignalDelayer;

  class Data;
  using DataSPtr  = std::shared_ptr<Data>;
  using DataSList = QList<DataSPtr>;

  class DataProxy;
  using DataProxySPtr = std::shared_ptr<DataProxy>;

  /** \class Data
   * \brief Implements the base class of every data object in EspINA
   *
   */
  class EspinaCore_EXPORT Data
  : public QObject
  {
      Q_OBJECT

      static TimeStamp s_tick;

    public:
      using Type = QString;

      enum class Access { READ, WRITE };

    public:
      /** \brief Data class constructor.
       *
       */
      virtual ~Data()
      {}

      /** \brief Returns the type of data.
       *
       */
      virtual Data::Type type() const = 0;

      /** \brief Creates a proxy for the data type.
       *
       */
      virtual DataSPtr createProxy() const = 0;

      /** \brief Returns the list of data types on which this
       *         data type relies on
       *
       */
      QList<Data::Type> dependencies() const;

      /** \brief Returns the time stamp of the last modification to the data.
       *
       */
      virtual TimeStamp lastModified() const
      { return m_timeStamp; }

      /** \brief Returns the list of bounds of the edited regions of the data.
       *
       */
      virtual BoundsList editedRegions() const
      { return m_editedRegions; }

      /** \brief Set current data edited regions
       *
       */
      virtual void setEditedRegions(const BoundsList &regions)
      { m_editedRegions = regions; }

      /** \brief Set context to look for data on fetch request
       * \param[in] storage temporal storage where data snasphots can be loaded from.
       * \param[in] path storage path where data snapshosts will be loaded from
       * \param[in] id identifier of stored data snapshosts
       *
       */
      virtual void setFetchContext(const TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds);

      /** \brief Copies the fetch context from the given data.
       * \param[in] data
       *
       */
      void copyFetchContext(DataSPtr data);

      /** \brief Recover data from Persistent Storage.
       *
       */
      bool fetchData();

      /** \brief Returns true if the data needs to be fetched from disk.
       *
       */
      virtual bool needFetch() const
      { return m_needFetch; }

      /** \brief Clears the edited regions list.
       *
       */
      virtual void clearEditedRegions()
      { m_editedRegions.clear(); }

      /** \brief Return the byte arrays needed to save this object between sessions.
       * \param[in] storage temporal storage where data snasphots can be loaded from
       * \param[in] path storage path where data snapshosts will be saved to
       * \param[in] id identifier to store data snapshosts
       *
       *  Temporal storage may be also used to store temporal files where snapshot generation.
       *
       *  NOTE: if data is dependent from another data it should be updated before saving as its
       *  assumed to be in sync when loading from disk. Thus this method is not const and cycled
       *  dependencies should be avoided.
       */
      virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) = 0;

      /** \brief Returns a snapshot object of the edited regions of the data.
       * \param[in] storage temporal storage where edited regions snasphots can be loaded from
       * \param[in] path storage path where edited regions snapshosts will be saved to
       * \param[in] id identifier to store edited regions snapshosts
       *
       *  Temporal storage may be also used to store temporal files where snapshot generation
       */
      virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) = 0;

      /** \brief Restore data edited regions from its snapshots.
       * \param[in] storage temporal storage where edited regions snasphots can be loaded from
       * \param[in] path storage path where edited regions snapshosts will be loaded from
       * \param[in] id identifier to store edited regions snapshosts
       *
       * PRE: Previously edited regions bounds have been restored
       */
      virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString &path, const QString &id) = 0;

      /** \brief Returns true if the object has been correctly initialized and contains data.
       *
       */
      virtual bool isValid() const = 0;

      /** \brief Returns true if the object is empty.
       *
       */
      virtual bool isEmpty() const = 0;

      /** \brief Returns the bounds of the contained data.
       *
       */
      virtual VolumeBounds bounds() const
      { return m_bounds; }

      /** \brief Sets the spacing of the data.
       *
       */
      virtual void setSpacing(const NmVector3& spacing) = 0;

      /** \brief Returns true if the object has been edited.
       *
       */
      bool isEdited() const
      { return !editedRegions().isEmpty(); }

      /** \brief Return memory usage in bytes, the amount of memory allocated by the object.
       *
       */
      virtual size_t memoryUsage() const = 0;

    signals:
      void dataChanged(); //former representationChanged

    protected:
      /** \brief Data class constructor.
       *
       */
      explicit Data()
      : m_needFetch{false}
      , m_timeStamp{s_tick++}
      , m_mutex    {QMutex::Recursive}
      {}

      /** \brief Increments the modification time and signals the modification of the data.
       *
       */
      void updateModificationTime()
      {
        m_timeStamp = s_tick++;
        emit dataChanged();
      }

      void addEditedRegion(const Bounds &bounds)
      { m_editedRegions << bounds; }

      /** \brief Returns true if the data has been fetched from disk and false otherwise.
       * \param[in] storage storage that can contain the data files.
       * \param[in] path file path that can contain the data files on storage.
       * \param[in] id data identifier.
       * \param[in] bounds data bounds.
       *
       */
      virtual bool fetchDataImplementation(TemporalStorageSPtr storage,
                                           const QString      &path,
                                           const QString      &id,
                                           const VolumeBounds &bounds) = 0;

    private:
      /** \brief Returns the list of data types on which this
       *         data type relies on
       *
       */
      virtual QList<Data::Type> updateDependencies() const = 0;

      /** \brief Auxiliary virtual method in case a data type needs to fix something wrong from
       * previous versions.
       */
      virtual void applyFixes()
      {};

    protected:
      QString             m_path;          /** path of data files stored on disk.    */
      QString             m_id;            /** data id.                              */
      TemporalStorageSPtr m_storage;       /** storage containing data files.        */
      VolumeBounds        m_bounds;        /** data bounds.                          */

    private:
      std::atomic<bool>   m_needFetch;     /** true if fetch from disk is necessary. */
      TimeStamp           m_timeStamp;     /** time stamp of last modification.      */
      BoundsList          m_editedRegions; /** list of edited regions bounds.        */

      mutable QMutex      m_mutex;         /** protection mutex.                     */

      template<typename T> friend class SignalBlocker;
  };

  enum class DataUpdatePolicy { Request, Ignore };

} // namespace ESPINA

#endif // ESPINA_DATA_H
