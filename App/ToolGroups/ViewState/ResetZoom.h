/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_RESET_ZOOM_TOOL_H
#define ESPINA_RESET_ZOOM_TOOL_H

// EspINA
#include <Support/Tool.h>
#include <Support/ViewManager.h>

class QAction;

namespace EspINA
{

  class ResetZoom
  : public Tool
  {
    Q_OBJECT
    public:
      /* \brief ResetZoom class constructor.
       * \param[in] vm View manager.
       */
      explicit ResetZoom(ViewManagerSPtr vm);

      /* \brief ResetZoom class destructor.
       *
       */
      virtual ~ResetZoom();


      virtual QList<QAction *> actions() const;

      virtual bool enabled() const;

      virtual void setEnabled(bool value);

    public slots:
      /* \brief Slot to activate when the action gets triggered. Resets the views
       * via ViewManager.
       *
       */
      void resetViews();

    private:
      ViewManagerSPtr m_viewManager;
      QAction        *m_action;
      bool            m_enabled;
  };

  using ResetZoomSPtr = std::shared_ptr<ResetZoom>;

} // namespace EspINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_TOOL_H
