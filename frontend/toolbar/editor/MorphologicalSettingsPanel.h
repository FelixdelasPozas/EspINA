/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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

#ifndef MORPHOLOGICALFILTERSPREFERENCES_H
#define MORPHOLOGICALFILTERSPREFERENCES_H

#include <common/settings/ISettingsPanel.h>
#include "ui_MorphologicalSettingsPanel.h"

const QString RADIUS("MorphologicalFilters::Radius");

class MorphologicalFiltersPreferences: public ISettingsPanel, public Ui::MorphologicalSettingsPanel
{
    Q_OBJECT
    public:
        MorphologicalFiltersPreferences();
        virtual ~MorphologicalFiltersPreferences()
        {
        }

        virtual const QString shortDescription()
        {
            return "Morphological Filters";
        }
        virtual const QString longDescription()
        {
            return "Morphological Filters Settings";
        }
        virtual const QIcon icon()
        {
            return QIcon(":/close.png");
        }

        virtual void acceptChanges();

        virtual bool modified() const;

        virtual ISettingsPanel *clone();

    public slots:

    private:
        int m_radius;
};

#endif // SEEDGROWSEGMENTATIONPREFERENCES_H
