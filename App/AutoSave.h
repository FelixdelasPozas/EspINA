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

#ifndef ESPINA_AUTOSAVE_H
#define ESPINA_AUTOSAVE_H

// Qt
#include <QDir>
#include <QTimer>

class QString;
namespace ESPINA
{
  /** \class AutoSave
   * \brief Auto save properties.
   *
   */
  class AutoSave
  : public QObject
  {
      Q_OBJECT
    public:
      static const QString PATH;
      static const QString INTERVAL;
      static const QString INTHREAD;

    public:
      /** \brief AutoSave class constructor.
       *
       */
      AutoSave();

      /** \brief AutoSave class destructor.
       *
       */
      ~AutoSave();

      /** \brief Sets the path for the auto save file.
       * \param[in] path file system path for the auto save file.
       *
       */
      void setPath(const QDir &path);

      /** \brief Returns the path for the auto save file.
       *
       */
      QDir path() const
      { return m_path; }

      /** \brief Sets the time interval for auto save events
       * \param[in] minutes time interval in minutes.
       *
       */
      void setInterval(const unsigned int minutes);

      /** \brief Returns the time interval for auto save events.
       *
       */
      const int interval() const;

      /** \brief Resets time to next auto save event
       *
       */
      void resetCountDown();

      /** \brief Returns if it is possible to restore from currently auto saved analysis
       *
       */
      bool canRestore() const;

      /** \brief Restore currently auto saved analysis
       *
       */
      void restore();

      /** \brief Clear currently auto saved analysis
       *
       */
      void clear();

      /** \brief Returns true if the given filename is equal to the auto save file.
       *
       */
      const bool isAutoSaveFile(const QString &filename);

      /** \brief Returns the date of the auto-save file or an empty string if it doesn't exist.
       *
       */
      const QString autoSaveDate() const;

      /** \brief Returns the time of the auto-save file or an empty string if it doesn't exists.
       *
       */
      const QString autoSaveTime() const;

      /** \brief Returns true if the auto save must be done in a separate thread.
       *
       */
      const bool autoSaveInThread() const
      { return m_inThread; }

      /** \brief Sets if the auto save must be done in a saparate thread.
       * \param[in] value true to use a thread to save data and false otherwise.
       *
       */
      void setSaveInThread(const bool value);

    signals:
      void restoreFromFile(const QString);

      void saveToFile(const QString);

    private slots:
      /** \brief Resets the timer and emits the save signal.
       *
       */
      void autoSave();

    private:
      QString autosaveFile() const;

    private:
      QDir   m_path;     /** auto save file path.                              */
      QTimer m_timer;    /** timer.                                            */
      bool   m_inThread; /** true to use a thread to save and false otherwise. */
  };
}

#endif // ESPINA_AUTOSAVE_H
