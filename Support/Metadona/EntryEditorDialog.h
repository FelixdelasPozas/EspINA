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

#ifndef ESPINA_ENTRY_EDITOR_DIALOG_H
#define ESPINA_ENTRY_EDITOR_DIALOG_H

#include <QDialog>

#include <Support/ui_EntryEditorDialog.h>

#include <Coordinator.h>
#include <functional>

namespace EspINA
{
  class EntryEditorDialog
  : public QDialog
  , private Ui::EntryEditorDialog
  {
   Q_OBJECT

  public:
    explicit EntryEditorDialog(Metadona::Entry& entry,
                               QWidget*         parent = 0,
                               Qt::WindowFlags  f = 0);
    virtual ~EntryEditorDialog();

  private slots:
    void apply();

  private:
    void createIdInput(QBoxLayout* layout);

    void createFieldInputs(std::vector<Metadona::FieldSPtr > entries, QBoxLayout* layout);

    void createStringInput(Metadona::StringField* field, QBoxLayout* layout);

    void createDecimalInput(Metadona::DecimalField* field, QBoxLayout* layout);

    void createIntegerInput(Metadona::IntegerField* field, QBoxLayout* layout);

    void createEnumInput(Metadona::SuggestionField* field, QBoxLayout* layout, bool enableSuggestions);

    void createGroupInput(Metadona::GroupField* field, QBoxLayout* layout);

    void createListInput(Metadona::ListField* field, QBoxLayout* layout);

  private:
    Metadona::Entry& m_entry;

    QLineEdit *m_idInput;

    std::vector<std::function<void()>> m_applyFunctors;
 };
}

#endif // ESPINA_ENTRY_EDITOR_DIALOG_H
