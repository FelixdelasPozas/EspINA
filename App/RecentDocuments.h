/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef RECENTDOCUMENTS_H
#define RECENTDOCUMENTS_H

// Qt
#include <QObject>
#include <QStringList>

class QAction;
class RecentDocuments
: QObject
{
	public:
		/* \brief RecentDocuments class constructor.
		 *
		 */
		explicit RecentDocuments();

		/* \brief RecentDocuments class virtual destructor.
		 *
		 */
		virtual ~RecentDocuments();

		/* \brief Adds a document to the list.
		 * \param[in] path, path of the document.
		 *
		 */
		void addDocument(QString path);

		/* \brief Removes a document from the list.
		 * \param[in] path, path of the document.
		 *
		 */
		void removeDocument(QString path);

		/* \brief Updates the document list with the contents of the settings.
		 *
		 */
		void updateDocumentList(void);

		/* \brief Returns the list of actions.
		 *
		 */
		QList<QAction *> list() const
	  { return m_actionList; }

	private:
		/* \brief Updates the actions with the names of the documents.
		 *
		 */
		void updateActions();

	private:
		QStringList m_recentDocuments;
		QList<QAction *> m_actionList;
};

#endif // RECENTDOCUMENTS_H
