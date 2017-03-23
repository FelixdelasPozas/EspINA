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

#ifndef ESPINA_CODE_TOOL_H
#define ESPINA_CODE_TOOL_H

// ESPINA
#include <Support/Widgets/EditTool.h>

#include <Filters/MorphologicalEditionFilter.h>
#include <GUI/Widgets/SpinBoxAction.h>
#include <GUI/Widgets/NumericalInput.h>
#include <GUI/Widgets/ToolButton.h>
#include <GUI/ModelFactory.h>

namespace ESPINA
{
  /** \class CODEToolBase
   * \brief Base class for morphological tool buttons
   * .
   */
  class CODEToolBase
  : public Support::Widgets::EditTool
  {
      Q_OBJECT
    public:
      /** \brief CODEToolBase class constructor.
       * \param[in] type morphological filter type.
       * \param[in] toolId tool's id.
       * \param[in] name tool's name.
       * \param[in] icon tool's icon id from resources.
       * \param[in] tooltip tool's default tooltip.
       * \param[in] context application context.
       *
       */
      explicit CODEToolBase(const Filter::Type type, const QString &toolId, const QString &name, const QString& icon, const QString& tooltip, Support::Context& context);

      /** \brief CODEToolBase class virtual destructor.
       *
       */
      virtual ~CODEToolBase();

      /** \brief Sets the radius value.
       * \param[in] value value of the radius.
       *
       */
      void setRadius(int value)
      { m_radius->setValue(value); }

      /** \brief Returns the value of the radius.
       *
       */
      int radius() const
      { return m_radius->value(); }

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

    private:
      /** \brief Helper method to initiate the configuration widgets.
       *
       */
      void initOptionWidgets();

      virtual bool acceptsNInputs(int n) const override;

      /** \brief Helper method to create and return the morphological filter.
       *
       */
      virtual MorphologicalEditionFilterSPtr createFilter(InputSList inputs, const Filter::Type& type) = 0;

    private slots:
      /** \brief Launches the operation.
       *
       */
      void onApplyClicked();

      /** \brief Gets the results of the finished filter.
       *
       */
      void onTaskFinished();

    private:
      /** \brief Aborts currently executing tasks.
       *
       */
      void abortTasks();

      /** \struct TaskContext
       * \brief Data from a currently running tasks.
       */
      struct TaskContext
      {
        MorphologicalEditionFilterSPtr Task;         /** running task.            */
        SegmentationAdapterPtr         Segmentation; /** input segmentation.      */
        QString                        Operation;    /** morphological operation. */
      };

      Filter::Type  m_type; /** morphological filter type. */
      const QString m_name; /** morphological filter name. */

      QMap<MorphologicalEditionFilterPtr, TaskContext> m_executingTasks; /** maps filter<->context. */
      GUI::Widgets::NumericalInput                    *m_radius;         /** radius widget.         */
      GUI::Widgets::ToolButton                        *m_apply;          /** apply button widget.   */
  };

  /** \class CODETool
   * \brief Implements a CODE morphological tool (Close-Open-Dilate-Erode) templated over type.
   *
   */
  template<typename T>
  class CODETool
  : public CODEToolBase
  {
    public:
      /** \brief CODETool class constructor.
       * \param[in] type morphological filter type.
       * \param[in] toolId tool's id.
       * \param[in] name tool's name.
       * \param[in] icon tool's icon id from resources.
       * \param[in] tooltip tool's default tooltip.
       * \param[in] context application context.
       *
       */
      explicit CODETool(const Filter::Type type, const QString &toolId, const QString& name, const QString& icon, const QString& tooltip, Support::Context& context)
      : CODEToolBase(type, toolId, name, icon, tooltip, context)
      {}

    private:
      virtual MorphologicalEditionFilterSPtr createFilter(InputSList inputs, const Filter::Type &type) override
      {
        return this->getFactory()->template createFilter<T>(inputs, type);
      }
  };


} // namespace ESPINA

#endif // ESPINA_CODE_TOOL_H
