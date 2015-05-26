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
#include <Support/Widgets/RefineTool.h>

#include <GUI/Widgets/SpinBoxAction.h>
#include <GUI/Widgets/NumericalInput.h>
#include <Filters/MorphologicalEditionFilter.h>
#include <GUI/ModelFactory.h>

namespace ESPINA {

  class CODEToolBase
  : public Support::Widgets::RefineTool
  {
    Q_OBJECT

  public:
    explicit CODEToolBase(const QString &name, const QString& icon, const QString& tooltip, Support::Context& context);

    /** \brief Sets the radius value.
     * \param[in] value value of the radius.
     */
    void setRadius(int value)
    { m_radius->setValue(value); }

    /** \brief Returns the value of the radius.
     *
     */
    int radius() const
    { return m_radius->value(); }

    virtual void abortOperation();

  private:
    void initOptionWidgets();

    virtual void onToolEnabled(bool enabled);

    virtual bool acceptsNInputs(int n) const override;

    virtual MorphologicalEditionFilterSPtr createFilter(InputSList inputs, const Filter::Type& type) = 0;

  private slots:
    void onApplyClicked();

    void onTaskFinished();

  private:
    struct TaskContext
    {
      MorphologicalEditionFilterSPtr Task;
      SegmentationAdapterPtr         Segmentation;
      QString                        Operation;
    };

    QString      m_name;
    Filter::Type m_type;

    QMap<MorphologicalEditionFilterPtr, TaskContext> m_executingTasks;
    GUI::Widgets::NumericalInput *m_radius;
  };

  template<typename T>
  class CODETool
  : public CODEToolBase
  {
  public:
    explicit CODETool(const QString& name, const QString& icon, const QString& tooltip, Support::Context& context)
    : CODEToolBase(name, icon, tooltip, context) {}

  private:
    virtual MorphologicalEditionFilterSPtr createFilter(InputSList inputs, const Filter::Type &type) override
    {
      return this->context().factory()->template createFilter<T>(inputs, type);
    }
  };


} // namespace ESPINA

#endif // ESPINA_CODE_TOOL_H
