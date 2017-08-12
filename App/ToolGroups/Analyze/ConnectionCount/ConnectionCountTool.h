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

#ifndef APP_TOOLGROUPS_ANALYZE_CONNECTIONCOUNTTOOL_H_
#define APP_TOOLGROUPS_ANALYZE_CONNECTIONCOUNTTOOL_H_

// ESPINA
#include <Support/Context.h>
#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
  class ConnectionCountDialog;

  /** \class ConnectionCountTool
   * \brief Tool to report the connected and unconnected synapses on a session.
   */
  class ConnectionCountTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief ConnectionCountTool class constructor.
       * \param[in] context application context.
       *
       */
      explicit ConnectionCountTool(Support::Context &context);

      /** \brief ConnectionCountTool class virtual destructor.
       *
       */
      virtual ~ConnectionCountTool();

    private slots:
      /** \brief Displays the dialog with the connections information.
       * \param[in] unused
       *
       */
      void onPressed(bool unused);

      /** \brief Makes the dialog pointer null.
       * \param[in] unused
       *
       */
      void onDialogClosed(QObject *unused)
      { m_dialog = nullptr; }

    private:
      ConnectionCountDialog *m_dialog;
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_ANALYZE_CONNECTIONCOUNTTOOL_H_
