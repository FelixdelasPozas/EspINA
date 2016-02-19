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

#ifndef ESPINA_INFORMATION_COLOR_ENGINE_SWITCH_H
#define ESPINA_INFORMATION_COLOR_ENGINE_SWITCH_H

#include <GUI/ColorEngines/InformationColorEngine.h>
#include <Support/Widgets/ColorEngineSwitch.h>

class QLabel;

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  /** \class InformationColorEngineSwitch
   * \brief Switch for coloring by property.
   *
   */
  class InformationColorEngineSwitch
  : public Support::Widgets::ColorEngineSwitch
  {
    Q_OBJECT

  public:
    /** \brief InformationColorEngineSwitch class constructor.
     * \param[in] context application context.
     *
     */
    explicit InformationColorEngineSwitch(Support::Context& context);

    using InformationKey = SegmentationExtension::InformationKey;

    virtual void restoreSettings(std::shared_ptr<QSettings> settings);

    virtual void saveSettings(std::shared_ptr<QSettings> settings);

  private:
    /** \brief Creates the property selection widget.
     *
     */
    void createPropertySelector();

    /** \brief Creates the color range widget.
     *
     */
    void createColorRange();

    /** \brief Helper method to return the color engine.
     *
     */
    GUI::ColorEngines::InformationColorEngine *informationColorEngine() const;

    /** \brief Updates the colors of the segmentations and the property selection label widget.
     *
     */
    void update();

    /** \brief Helper method to update the property selection label widget.
     *
     */
    void updateLink();

  private slots:
    /** \brief Displays the dialog to chenge the coloring property and manages the result.
     *
     */
    void changeProperty();

    /** \brief Launches the task to get the minimum and maximum values of the selected property.
     *
     */
    void updateRange();

    /** \brief Helper method to enable/disable the tool.
     *
     */
    void onToolToggled(bool checked);

    /** \brief Helper method to check for the returning value of the task and to inform the user of
     * any failure.
     */
    void onTaskFinished();

  private:
    InformationKey m_key;        /** coloring property.                                                                    */
    bool           m_needUpdate; /** true if the coloring values needs to be updated.                                      */
    TaskSPtr       m_task;       /** task to compute the maximum and minimum values of the property for all segmentations. */
    QLabel        *m_property;   /** property link label widget.                                                           */
  };

  //-----------------------------------------------------------------------------
  /** \class UpdateColorEngineTask
   * \brief Class for computing the minimum and maximum numerical value of a segmentation property
   *        for a given group of segmentations.
   */
  class UpdateColorEngineTask
  : public Task
  {
      Q_OBJECT
    public:
      /** \brief UpdateColorEngineTask class constructor.
       * \param[in] key segmentation extension information key whose value will be used as coloring value.
       * \param[in] colorEngine color engine.
       * \param[in] segmentations group of segmentation for coloring.
       * \param[in] factory model factory for creating segmentation extensions.
       * \param[in] scheduler application task scheduler.
       *
       */
        explicit UpdateColorEngineTask(const SegmentationExtension::InformationKey        key,
                                       ESPINA::GUI::ColorEngines::InformationColorEngine *colorEngine,
                                       SegmentationAdapterList                            segmentations,
                                       ModelFactorySPtr                                   factory,
                                       SchedulerSPtr                                      scheduler);

        /** \brief Returns true if the coloring has failed and false otherwise.
         *
         */
        bool hasFailed() const
        { return m_failed; }

        /** \brief Returns the error message if the coloring error if the task failed or an empty string otherwise.
         *
         */
        const QString error() const
        { return m_error; }

        /** \brief Returns the information key used for coloring.
         *
         */
        const QString key()
        { return m_key.value(); }

    private:
      virtual void run() override final;

    private:
      const SegmentationExtension::InformationKey        m_key;           /** property key.                                               */
      ESPINA::GUI::ColorEngines::InformationColorEngine *m_colorEngine;   /** color engine                                                */
      SegmentationAdapterList                            m_segmentations; /** segmentation to color.                                      */
      ModelFactorySPtr                                   m_factory;       /** factory to create segmentation extensions.                  */
      QString                                            m_error;         /** empty if the task doesn't fail and error message otherwise. */
      bool                                               m_failed;        /** true if the coloring fails, false otherwise.                */
  };
}

#endif // ESPINA_INFORMATION_COLOR_ENGINE_SWITCH_H
