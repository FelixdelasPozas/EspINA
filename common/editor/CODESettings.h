/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

    This program is free software: you can redistribute it and/or modify
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


#ifndef CODESETTINGS_H
#define CODESETTINGS_H

#include <QWidget>

class QSpinBox;
class MorphologicalEditionFilter;

class CODESettings
: public QWidget
{
  Q_OBJECT
public:
  explicit CODESettings(MorphologicalEditionFilter *filter, QString title);
  virtual ~CODESettings();

protected slots:
  void modifyFilter();

private:
  MorphologicalEditionFilter *m_filter;
  QSpinBox *m_spinbox;
};

#endif // CODESETTINGS_H
