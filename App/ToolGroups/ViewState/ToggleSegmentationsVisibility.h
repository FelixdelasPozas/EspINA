/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_SHOW_SEGMENTATIONS_H
#define ESPINA_SHOW_SEGMENTATIONS_H

// ESPINA
#include <Support/Widgets/Tool.h>
#include <Support/ViewManager.h>

// Qt
#include <QAction>

namespace ESPINA {

  class ToggleSegmentationsVisibility
  : public Tool
  {
    Q_OBJECT
		public:
			/** brief ToggleSegmentationsVisibility class constructor.
			 * \param[in] viewManager, view manager smart pointer.
			 *
			 */
			ToggleSegmentationsVisibility(ViewManagerSPtr viewManager);

			/** brief Implements Tool::actions().
			 *
			 */
			virtual QList< QAction* > actions() const;

			/** brief Implements Tool::enabled().
			 *
			 */
			virtual bool enabled() const;

			/** brief Implements Tool::setEnabled().
			 *
			 */
			virtual void setEnabled(bool value);

		public slots:
			/** brief Toggles the visibility action.
			 *
			 */
			void shortcut();

		private slots:
			/** brief Modifies the GUI and shows/hides segmentations based on parameter value.
			 * \param[in] visible, true to set all segmentations visible, false otherwise.
			 *
			 */
			void toggleVisibility(bool visible);

		private:
			ViewManagerSPtr m_viewManager;

			QAction m_toggle;
	};

  using ToggleSegmentationsVisibilitySPtr = std::shared_ptr<ToggleSegmentationsVisibility>;
}

#endif // ESPINA_SHOW_SEGMENTATIONS_H
