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

#include "EntryWidget.h"

#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

using namespace ESPINA;
using namespace Metadona;

//------------------------------------------------------------------------
EntryWidget::EntryWidget(Metadona::Entry& entry, QWidget* parent)
: QGroupBox(parent)
, m_entry(entry)
{
  setTitle(entry.id().c_str());

  auto layout = new QVBoxLayout(this);

  createIdInput(layout);

  createFieldInputs(m_entry.fields(), layout);

  setLayout(layout);
}

//------------------------------------------------------------------------
void EntryWidget::createIdInput(QBoxLayout* layout)
{
  auto horizontalLayout = new QHBoxLayout(this);

  auto label = new QLabel(tr("%1 Id:").arg(m_entry.name().c_str()), this);
  m_idInput  = new QLineEdit(m_entry.id().c_str(), this);

  horizontalLayout->addWidget(label);
  horizontalLayout->addWidget(m_idInput);

  layout->addLayout(horizontalLayout);
}

//------------------------------------------------------------------------
void EntryWidget::createFieldInputs(std::vector<Metadona::FieldSPtr> entries, QBoxLayout* layout)
{
  for (auto field : entries)
  {
    auto fieldPtr = field.get();

    if(field->type() == FieldTypes::STRING)
    {
      createStringInput(dynamic_cast<StringField*>(fieldPtr), layout);
    }
    else
    {
      if(field->type() == FieldTypes::INTEGER)
      {
        createIntegerInput(dynamic_cast<IntegerField*>(fieldPtr), layout);
      }
      else
      {
        if(field->type() == FieldTypes::DECIMAL)
        {
          createDecimalInput(dynamic_cast<DecimalField*>(fieldPtr), layout);
        }
        else
        {
          if(field->type() == FieldTypes::ENUM)
          {
            createEnumInput(dynamic_cast<SuggestionField*>(fieldPtr), layout, false);
          }
          else
          {
            if(field->type() == FieldTypes::SUGGESTION)
            {
              createEnumInput(dynamic_cast<SuggestionField*>(fieldPtr), layout, true);
            }
            else
            {
              if(field->type() == FieldTypes::GROUP)
              {
                createGroupInput(dynamic_cast<GroupField*>(fieldPtr), layout);
              }
              else
              {
                if(field->type() == FieldTypes::LIST)
                {
                  createListInput(dynamic_cast<ListField*>(fieldPtr), layout);
                }
                else
                {
                  throw Scheme_Validation_Exception();
                }
              }
            }
          }
        }
      }
    }
  }
}

//------------------------------------------------------------------------
void EntryWidget::createStringInput(::Metadona::StringField* field, QBoxLayout* layout)
{
  auto horizontalLayout = new QHBoxLayout(this);

  auto label = new QLabel(tr("%1:").arg(field->name().c_str()), this);
  auto input = new QLineEdit(field->value().c_str(), this);

  horizontalLayout->addWidget(label);
  horizontalLayout->addWidget(input);

  layout->addLayout(horizontalLayout);

  m_applyFunctors.push_back([field, input](){ field->setValue(input->text().toStdString()); });
}

//------------------------------------------------------------------------
void EntryWidget::createIntegerInput(::Metadona::IntegerField* field, QBoxLayout* layout)
{
  auto horizontalLayout = new QHBoxLayout(this);

  auto label = new QLabel(tr("%1:").arg(field->name().c_str()), this);
  auto input = new QLineEdit(QString::number(field->value()), this);

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
void EntryWidget::createDecimalInput(::Metadona::DecimalField* field, QBoxLayout* layout)
{
  auto horizontalLayout = new QHBoxLayout(this);

  auto label = new QLabel(tr("%1:").arg(field->name().c_str()), this);
  auto input = new QLineEdit(QString::number(field->value()), this);

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
void EntryWidget::createEnumInput(::Metadona::SuggestionField* field, QBoxLayout* layout, bool enableSuggestions)
{
  auto horizontalLayout = new QHBoxLayout(this);

  auto label = new QLabel(tr("%1:").arg(field->name().c_str()), this);
  auto input = new QComboBox(this);

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
void EntryWidget::createGroupInput(::Metadona::GroupField* field, QBoxLayout* layout)
{
  auto group          = new QGroupBox(tr("%1:").arg(field->name().c_str()), this);
  auto verticalLayout = new QVBoxLayout(this);

  createFieldInputs(field->entries(), verticalLayout);

  group->setLayout(verticalLayout);

  layout->addWidget(group);
}
//------------------------------------------------------------------------
void EntryWidget::createListInput(::Metadona::ListField* field, QBoxLayout* layout)
{
  auto listGroup      = new QGroupBox(tr("%1:").arg(field->name().c_str()), this);
  auto verticalLayout = new QVBoxLayout(this);

  createFieldInputs(field->entries(), verticalLayout);

  listGroup->setLayout(verticalLayout);

  layout->addWidget(listGroup);
}
