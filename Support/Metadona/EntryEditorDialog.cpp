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

#include "EntryEditorDialog.h"

#include "ui_EntryEditorDialog.h"
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>

using namespace EspINA;
using namespace Metadona;


//------------------------------------------------------------------------
EntryEditorDialog::EntryEditorDialog(Metadona::Entry& entry,
                                     QWidget*         parent,
                                     Qt::WindowFlags  f)
: QDialog(parent, f)
, m_entry(entry)
{
  setupUi(this);

  m_label->setText(QString("%1:").arg(entry.name().c_str()));

  connect(m_create, SIGNAL(clicked(bool)),
          this,     SLOT(apply()));

  connect(m_cancel, SIGNAL(clicked(bool)),
          this,     SLOT(reject()));

  auto layout = new QVBoxLayout();

  createIdInput(layout);

  createFieldInputs(m_entry.fields(), layout);

  delete m_centralWidget->layout();
  m_centralWidget->setLayout(layout);

  QApplication::setOverrideCursor(Qt::ArrowCursor);
}

//------------------------------------------------------------------------
EntryEditorDialog::~EntryEditorDialog()
{
  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void EntryEditorDialog::apply()
{
  m_entry.setId(m_idInput->text().toStdString());

  for (auto functor : m_applyFunctors)
  {
    functor();
  }

  accept();
}

//------------------------------------------------------------------------
void EntryEditorDialog::createIdInput(QBoxLayout* layout)
{
  auto horizontalLayout = new QHBoxLayout();

  auto label = new QLabel(tr("%1 Id:").arg(m_entry.name().c_str()));
  m_idInput  = new QLineEdit(m_entry.id().c_str());

  horizontalLayout->addWidget(label);
  horizontalLayout->addWidget(m_idInput);

  layout->addLayout(horizontalLayout);
}

//------------------------------------------------------------------------
void EntryEditorDialog::createFieldInputs(std::vector<Metadona::FieldSPtr> entries, QBoxLayout* layout)
{
  for (auto field : entries)
  {
    auto fieldPtr = field.get();

    switch (field->type())
    {
      case FieldType::STRING:
        createStringInput(dynamic_cast<StringField*>(fieldPtr), layout);
        break;
      case FieldType::INTEGER:
        createIntegerInput(dynamic_cast<IntegerField*>(fieldPtr), layout);
        break;
      case FieldType::DECIMAL:
        createDecimalInput(dynamic_cast<DecimalField*>(fieldPtr), layout);
        break;
      case FieldType::ENUM:
        createEnumInput(dynamic_cast<SuggestionField*>(fieldPtr), layout, false);
        break;
      case FieldType::SUGGESTION:
        createEnumInput(dynamic_cast<SuggestionField*>(fieldPtr), layout, true);
        break;
      case FieldType::GROUP:
        createGroupInput(dynamic_cast<GroupField*>(fieldPtr), layout);
        break;
      case FieldType::LIST:
        createListInput(dynamic_cast<ListField*>(fieldPtr), layout);
        break;
      default:
        throw Scheme_Validation_Exception();
    }
  }
}

//------------------------------------------------------------------------
void EntryEditorDialog::createStringInput(Metadona::StringField* field, QBoxLayout* layout)
{
  auto horizontalLayout = new QHBoxLayout();

  auto label = new QLabel(tr("%1:").arg(field->name().c_str()));
  auto input = new QLineEdit(field->value().c_str());

  horizontalLayout->addWidget(label);
  horizontalLayout->addWidget(input);

  layout->addLayout(horizontalLayout);

  m_applyFunctors.push_back([field, input](){ field->setValue(input->text().toStdString()); });
}

//------------------------------------------------------------------------
void EntryEditorDialog::createIntegerInput(Metadona::IntegerField* field, QBoxLayout* layout)
{
  auto horizontalLayout = new QHBoxLayout();

  auto label = new QLabel(tr("%1:").arg(field->name().c_str()));
  auto input = new QLineEdit(QString::number(field->value()));

  input->setValidator(new QIntValidator(0, 10000, this));

  horizontalLayout->addWidget(label);
  horizontalLayout->addWidget(input);

  if (field->units().empty())
  {
    m_applyFunctors.push_back([field, input](){
                               field->setValue(input->text().toInt());
                               });
  }
  else
  {
    QStringList units;
    for (auto unit : field->units())
    {
      units << unit.c_str();
    }

    auto unitSelector = new QComboBox();

    unitSelector->insertItems(0, units);

    horizontalLayout->addWidget(unitSelector);

    m_applyFunctors.push_back([field, input, unitSelector](){
                               field->setValue(input->text().toInt());
                               field->setUnit(unitSelector->currentText().toStdString());
                               });
  }

  layout->addLayout(horizontalLayout);

}

//------------------------------------------------------------------------
void EntryEditorDialog::createDecimalInput(Metadona::DecimalField* field, QBoxLayout* layout)
{
  auto horizontalLayout = new QHBoxLayout();

  auto label = new QLabel(tr("%1:").arg(field->name().c_str()));
  auto input = new QLineEdit(QString::number(field->value()));

  input->setValidator(new QDoubleValidator(0, 10000, 4, this));

  horizontalLayout->addWidget(label);
  horizontalLayout->addWidget(input);

  if (field->units().empty())
  {
    m_applyFunctors.push_back([field, input](){
                               field->setValue(input->text().toDouble());
                               });
  }
  else
  {
    QStringList units;
    for (auto unit : field->units())
    {
      units << unit.c_str();
    }

    auto unitSelector = new QComboBox();

    unitSelector->insertItems(0, units);

    horizontalLayout->addWidget(unitSelector);

    m_applyFunctors.push_back([field, input, unitSelector](){
                               field->setValue(input->text().toDouble());
                               field->setUnit(unitSelector->currentText().toStdString());
                               });
  }

  layout->addLayout(horizontalLayout);
}

//------------------------------------------------------------------------
void EntryEditorDialog::createEnumInput(Metadona::SuggestionField* field, QBoxLayout* layout, bool enableSuggestions)
{
  auto horizontalLayout = new QHBoxLayout();

  auto label = new QLabel(tr("%1:").arg(field->name().c_str()));
  auto input = new QComboBox();

  input->setEditable(enableSuggestions);
  input->setAutoCompletion(true);

  QStringList values;

  for (auto value : field->values())
  {
    values << value.c_str();
  }

  input->insertItems(0, values);

  horizontalLayout->addWidget(label);
  horizontalLayout->addWidget(input);

  layout->addLayout(horizontalLayout);

  m_applyFunctors.push_back([field, input](){ field->setValue(input->currentText().toStdString()); });
}

//------------------------------------------------------------------------
void EntryEditorDialog::createGroupInput(Metadona::GroupField* field, QBoxLayout* layout)
{
  auto group          = new QGroupBox(tr("%1:").arg(field->name().c_str()));
  auto verticalLayout = new QVBoxLayout();

  createFieldInputs(field->entries(), verticalLayout);

  group->setLayout(verticalLayout);

  layout->addWidget(group);
}
//------------------------------------------------------------------------
void EntryEditorDialog::createListInput(Metadona::ListField* field, QBoxLayout* layout)
{
  auto listGroup      = new QGroupBox(tr("%1:").arg(field->name().c_str()));
  auto verticalLayout = new QVBoxLayout();

  createFieldInputs(field->entries(), verticalLayout);

  listGroup->setLayout(verticalLayout);

  layout->addWidget(listGroup);
}
