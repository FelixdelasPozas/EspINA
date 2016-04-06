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

#include "ProgressAction.h"
#include "Styles.h"
#include "ToolButton.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QProgressBar>

using namespace ESPINA::GUI::Widgets;

//------------------------------------------------------------------------
ProgressAction::ProgressAction(const QString &icon, const QString &tooltip, QObject* parent)
: ProgressAction{QIcon(icon), tooltip, parent}
{
}

//------------------------------------------------------------------------
ProgressAction::ProgressAction(const QIcon &icon, const QString &tooltip, QObject* parent)
: QWidgetAction(parent)
, m_progress(100)
{
  setIcon(icon);
  setToolTip(tooltip);
}

//------------------------------------------------------------------------
QWidget* ProgressAction::createWidget(QWidget* parent)
{
  auto widget = new QWidget(parent);
  auto layout = new QVBoxLayout;

  layout->setMargin(0);
  widget->setLayout(layout);

  auto button = createActionButton(widget);

  layout->addWidget(button);

  createProgress(button);

  return widget;
}

//------------------------------------------------------------------------
void ProgressAction::setActionIcon(const QIcon& icon)
{
  setIcon(icon);

  emit iconChanged(icon);
}

//------------------------------------------------------------------------
void ProgressAction::setActionToolTip(const QString& tooltip)
{
  setToolTip(tooltip);

  emit toolChanged(tooltip);
}

//------------------------------------------------------------------------
void ProgressAction::setProgress(int progress)
{
  if (m_progress != progress)
  {
    bool visibilityChange = displayProgress(m_progress) != displayProgress(progress);

    m_progress = progress;

    emit progressChanged(progress);

    if (visibilityChange)
    {
      emit progressVisibilityChanged(displayProgress(progress));
    }
  }
}

//------------------------------------------------------------------------
void ProgressAction::setActionEnabled(bool enabled)
{
  setEnabled(enabled);

  emit actionEnabled(enabled);
}

//------------------------------------------------------------------------
void ProgressAction::setActionChecked(bool checked)
{
  if(isChecked() == checked) return;

  setChecked(checked);

  emit actionChecked(checked);
}

//------------------------------------------------------------------------
QPushButton* ProgressAction::createActionButton(QWidget* parent)
{
  auto action = Styles::createToolButton(icon(), toolTip(), parent);

  action->setEnabled(isEnabled());
  action->setCheckable(isCheckable());
  action->setChecked(isChecked());

  connect(action, SIGNAL(toggled(bool)),
          this,   SLOT(setChecked(bool)));

  connect(action, SIGNAL(clicked(bool)),
          this,   SIGNAL(triggered(bool)));

  connect(this,   SIGNAL(actionEnabled(bool)),
          action, SLOT(setEnabled(bool)));

  connect(this,   SIGNAL(actionChecked(bool)),
          action, SLOT(setChecked(bool)));

  connect(this,   SIGNAL(iconChanged(QIcon)),
          action, SLOT(changeIcon(QIcon)));

  connect(this,   SIGNAL(toolChanged(QString)),
          action, SLOT(changeTooltip(QString)));

  return action;
}

//------------------------------------------------------------------------
void ProgressAction::createProgress(QWidget* parent)
{
  auto progress = new QProgressBar(parent);

  progress->setStyleSheet(
    "QProgressBar {"
     "border: 1px solid #0333c9;"
     "border-radius: 5px;"
    "}"
    "QProgressBar::chunk {"
     "background-color: #0282c9;"
     "width: 1px;"
     "margin: 0px;"
    "}");
  progress->setValue(m_progress);
  progress->setVisible(displayProgress(m_progress));
  progress->setTextVisible(false);
  progress->setGeometry(progressMargin(),
                        progressVerticalPosition(),
                        progressWitdh(),
                        progressHeight());

  connect(this,     SIGNAL(progressVisibilityChanged(bool)),
          progress, SLOT(setVisible(bool)));

  connect(this,     SIGNAL(progressChanged(int)),
          progress, SLOT(setValue(int)));
}

//------------------------------------------------------------------------
constexpr int ProgressAction::progressHeight()
{
  return 0.2*Styles::buttonSize();
}

//------------------------------------------------------------------------
constexpr int ProgressAction::progressMargin()
{
  return 3;
}

//------------------------------------------------------------------------
constexpr int ProgressAction::progressWitdh()
{
  return Styles::buttonSize() - 2*progressMargin();
}

//------------------------------------------------------------------------
constexpr int ProgressAction::progressVerticalPosition()
{
  return 0.6*Styles::buttonSize();
}

//------------------------------------------------------------------------
bool ProgressAction::displayProgress(int progress)
{
  return 0 <= progress && progress < 100;
}
