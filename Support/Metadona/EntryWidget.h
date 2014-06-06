/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
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

#ifndef ESPINA_METADONA_ENTRY_WIDGET_H
#define ESPINA_METADONA_ENTRY_WIDGET_H

#include <Entry.h>

#include <QGroupBox>

class QLineEdit;
class QBoxLayout;

namespace EspINA {

  class EntryWidget
  : public QGroupBox
  {
  public:
    explicit EntryWidget(Metadona::Entry& entry, QWidget* parent = 0);

  private:
    void createIdInput(QBoxLayout* layout);

    void createFieldInputs (std::vector<Metadona::FieldSPtr>  entries, QBoxLayout* layout);

    void createStringInput (Metadona::StringField*     field, QBoxLayout* layout);

    void createDecimalInput(Metadona::DecimalField*    field, QBoxLayout* layout);

    void createIntegerInput(Metadona::IntegerField*    field, QBoxLayout* layout);

    void createEnumInput   (Metadona::SuggestionField* field, QBoxLayout* layout, bool enableSuggestions);

    void createGroupInput  (Metadona::GroupField*      field, QBoxLayout* layout);

    void createListInput   (Metadona::ListField*       field, QBoxLayout* layout);

  private:
    Metadona::Entry& m_entry;

    QLineEdit *m_idInput;

    std::vector<std::function<void()>> m_applyFunctors;
  };

} // namespace EspINA

#endif // ESPINA_METADONA_ENTRY_WIDGET_H
