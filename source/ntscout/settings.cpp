//Copyright (C) <2014>  <RSX>

//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "settings.h"

#include <QThread>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QDebug>

quint8 Settings::tasks = QThread::idealThreadCount();
quint8 Settings::metrics = 0;
bool Settings::searchBots = false;
QPair <quint8, quint8> Settings::age = {18, 99};
QPair <quint8, quint8> Settings::pot = {0, 11};
QPair <quint32, quint32> Settings::sal = {0, 999999};
QPair <quint32, quint32> Settings::dmi = {0, 99999999};

bool Settings::read()
{
    QFile file("data/settings.dat");
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDataStream stream(&file);
    stream >> tasks;
    stream >> metrics;
    stream >> searchBots;
    stream >> age;
    stream >> pot;
    stream >> sal;
    stream >> dmi;
    return true;
}

bool Settings::save()
{
    QFile file("data/settings.dat");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    QDataStream stream(&file);
    stream << tasks;
    stream << metrics;
    stream << searchBots;
    stream << age;
    stream << pot;
    stream << sal;
    stream << dmi;
    return true;
}

