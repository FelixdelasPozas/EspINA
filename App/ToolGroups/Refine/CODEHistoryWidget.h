/*
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_CODE_HISTORY_WIDGET_H
#define ESPINA_CODE_HISTORY_WIDGET_H

// Qt
#include <QWidget>

// ESPINA
#include <Filters/MorphologicalEditionFilter.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/Context.h>

class QUndoStack;

namespace ESPINA
{

  namespace Ui
  {
    class CODEHistoryWidget;
  }

  class CODEHistoryWidget
  : public QWidget
  {
    Q_OBJECT
  public:
    explicit CODEHistoryWidget(const QString                  &title,
                               MorphologicalEditionFilterSPtr filter,
                               const Support::Context        &context);
    virtual ~CODEHistoryWidget();

  public slots:
    void setRadius(int value);

  signals:
    void radiusChanged(int);

  private slots:
    void onRadiusChanged(int value);
    void modifyFilter();

  private:
    const Support::Context &m_context;
    Ui::CODEHistoryWidget *m_gui;

    QString                        m_title;
    MorphologicalEditionFilterSPtr m_filter;
  };

} // namespace ESPINA

#endif // ESPINA_CODE_HISTORY_WIDGET_H
