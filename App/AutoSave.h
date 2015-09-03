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

#include <QDir>
#include <QTimer>

class QString;
namespace ESPINA {

  class AutoSave
  : public QObject
  {
    Q_OBJECT

  public:
    AutoSave();

    void setPath(const QDir &path);

    QDir path() const
    { return m_path; }

    /** \brief Sets the time interval for auto save events
     *
     */
    void setInterval(const unsigned int minutes);

    unsigned int interval() const;

    /** \brief Resets time to next auto save event
     *
     */
    void resetCountDown();

    /** \brief Returns if it is possible to restore from currently auto saved analysis
     *
     */
    bool canRestore();

    /** \brief Restore currently auto saved analysis
     *
     */
    void restore();

    /** \brief Clear currently auto saved analysis
     *
     */
    void clear();

    bool isAutoSaveFile(const QString &filename);

  signals:
    void restoreFromFile(const QString);

    void saveToFile(const QString);

  private slots:
    void autoSave();

  private:
    QString autosaveFile() const;

  private:
    QDir   m_path;
    QTimer m_timer;
  };
}

#endif // ESPINA_AUTOSAVE_H
