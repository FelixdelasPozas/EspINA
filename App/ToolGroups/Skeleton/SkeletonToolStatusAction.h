/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SKELETON_TOOL_STATUS_ACTION_H_
#define ESPINA_SKELETON_TOOL_STATUS_ACTION_H_

// ESPINA
#include <GUI/View/Widgets/Skeleton/SkeletonWidget.h>

// Qt
#include <QWidgetAction>
#include <QLabel>

class QAction;

namespace ESPINA
{
  
  class SkeletonToolStatusAction
  : public QWidgetAction
  {
    Q_OBJECT
    public:
      /** \brief StatusAction class constructor.
       * \param[in] parent raw pointer of the QObject parent of this one.
       *
       */
      explicit SkeletonToolStatusAction(QObject *parent = nullptr);

      /** \brief StatusAction class virtual destructor.
       *
       */
      virtual ~SkeletonToolStatusAction();

      virtual QWidget* createWidget(QWidget* parent) override;

      /** \brief Resets the status to the default.
       *
       */
      void reset();

      void setEnabled(bool value);

      bool isEnabled()
      { return m_enabled; }

    public slots:
      /** \brief Configures the status icons and tooltips according to the given status.
       * \param[in] status Status value.
       */
      void setStatus(SkeletonWidget::Status status);

    private:
      QLabel *m_label;
      QLabel *m_createIcon;
      QLabel *m_modifyIcon;
      bool    m_enabled;
  };

} // namespace ESPINA

#endif // ESPINA_SKELETON_TOOL_STATUS_ACTION_H_
