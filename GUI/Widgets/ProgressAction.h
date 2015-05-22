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

#ifndef ESPINA_GUI_WIDGETS_PROGRESS_ACTION_H
#define ESPINA_GUI_WIDGETS_PROGRESS_ACTION_H

#include <QWidgetAction>

class QPushButton;

namespace ESPINA {
  namespace GUI {
    namespace Widgets {

      class ProgressAction
      : public QWidgetAction
      {
        Q_OBJECT
      public:
        explicit ProgressAction(const QString &icon, const QString &tooltip, QObject* parent);

        virtual QWidget* createWidget(QWidget* parent);

      public slots:
        void setProgress(int progress);

      signals:
        void progressChanged(int value);

        void progressVisibilityChanged(bool visible);

      private:
        QPushButton *createActionButton(QWidget *parent);

        void createProgress(QWidget *parent);

        inline int buttonSize() const;

        inline int iconSize() const;

        inline int progressHeight() const;

        inline int progressMargin() const;

        inline int progressWitdh() const;

        inline int progressVerticalPosition() const;

        inline bool displayProgress(int progress);

      private:
        int m_progress;
      };
    }
  }
}

#endif // ESPINA_GUI_WIDGETS_PROGRESS_ACTION_H
