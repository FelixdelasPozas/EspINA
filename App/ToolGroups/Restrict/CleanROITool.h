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
#ifndef ESPINA_CLEAN_ROI_H
#define ESPINA_CLEAN_ROI_H

// ESPINA
#include <Support/Widgets/ProgressTool.h>
#include <Support/Context.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoStack>

class QAction;
namespace ESPINA
{
  class RestrictToolGroup;

  class CleanROITool
  : public Support::Widgets::ProgressTool
  {
    Q_OBJECT
  public:
    /** \brief CleanROITool class constructor.
     * \param[in] context ESPINA context
     * \param[in] toolGroup ROIToolsGroup raw pointer containing the ROI accumulator.
     */
    explicit CleanROITool(Support::Context &context,
                          RestrictToolGroup *toolGroup);

    /** \brief CleanROITool class virtual destructor.
     *
     */
    virtual ~CleanROITool();

  protected slots:
    void onROIChanged();
    void cancelROI();

  private:
    virtual void onToolEnabled(bool enabled);

  private:
    Support::Context &m_context;
    RestrictToolGroup *m_toolGroup;
  };

  using CleanROIToolSPtr = std::shared_ptr<CleanROITool>;

} // namespace ESPINA

#endif // ESPINA_CLEAN_ROI_H
