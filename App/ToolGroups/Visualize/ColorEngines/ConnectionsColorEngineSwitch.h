/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_VISUALIZE_COLORENGINES_CONNECTIONSCOLORENGINESWITCH_H_
#define APP_TOOLGROUPS_VISUALIZE_COLORENGINES_CONNECTIONSCOLORENGINESWITCH_H_

// ESPINA
#include <Support/Widgets/ColorEngineSwitch.h>

class QLabel;

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      class ConnectionsColorEngine;
    }
  }

  /** \class ConnectionsColorEngineSwitch
   * \brief Switch for coloring by number of connections.
   *
   */
  class ConnectionsColorEngineSwitch
  : public Support::Widgets::ColorEngineSwitch
  {
      Q_OBJECT

    public:
      /** \brief ConnectionsColorEngineSwitch class constructor.
       * \param[in] context application context.
       *
       */
      explicit ConnectionsColorEngineSwitch(Support::Context& context);

      /** \brief ConnectionsColorEngineSwitch class virtual destructor.
       *
       */
      virtual ~ConnectionsColorEngineSwitch();

      virtual void restoreSettings(std::shared_ptr<QSettings> settings);

      virtual void saveSettings(std::shared_ptr<QSettings> settings);

    private slots:
      /** \brief Helper method to enable/disable the tool.
       *
       */
      void onToolToggled(bool checked);

      /** \brief Helper method to check for the returning value of the task and to inform the user of
       * any failure.
       */
      void onTaskFinished();

      /** \brief Helper method to update the maximum and minimum values of the coloring range.
       *
       */
      void updateRange();

      /** \brief Updates the range if checked, stores the need of updating otherwise.
       *
       */
      void onRangeModified();

    private:
      /** \brief Helper method to create the information widgets of the switch.
       *
       */
      void createWidgets();

      /** \brief Aborts the currently running task.
       *
       */
      void abortTask();

      bool     m_needUpdate; /** true if the coloring maximum and minimum values needs to be updated.                 */
      TaskSPtr m_task;       /** task to compute the maximum and minimum number of connections for all segmentations. */
      QLabel  *m_minLabel;   /** label for minimum value.                                                             */
      QLabel  *m_maxLabel;   /** label for minimum value.                                                             */
  };

  /** \class UpdateConnectionsRangeTask
   * \brief Class for computing the minimum and maximum numerical value of the number of connections
   *        in the current session.
   */
  class UpdateConnectionsRangeTask
  : public Task
  {
      Q_OBJECT
    public:
      /** \brief UpdateConnectionsRangeTask class constructor.
       * \param[in] segmentations group of segmentation for coloring.
       * \param[in] engine Connections color engine pointer.
       * \param[in] scheduler application task scheduler.
       *
       */
       explicit UpdateConnectionsRangeTask(SegmentationAdapterSList                   segmentations,
                                           GUI::ColorEngines::ConnectionsColorEngine *engine,
                                           SchedulerSPtr                              scheduler);

       /** \brief UpdateConnectionsRangeTask class virtual destructor.
        *
        */
       ~UpdateConnectionsRangeTask()
       {};

    private:
        virtual void run() override final;

    private:
        SegmentationAdapterSList                   m_segmentations; /** segmentations to color.   */
        GUI::ColorEngines::ConnectionsColorEngine *m_engine;        /** connections color engine. */
  };
}



#endif // APP_TOOLGROUPS_VISUALIZE_COLORENGINES_CONNECTIONSCOLORENGINESWITCH_H_
