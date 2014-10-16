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

#ifndef SEARCHVALUES_H
#define SEARCHVALUES_H

#include <QPair>

struct SearchValues {
    QPair<int,int> age;
    QPair<int,int> potential;
    QPair<int,int> salary;
    QPair<int,int> dmi;
    int countryid;
    int divCount;
    bool div[6];
    bool countrySet;
    bool nationalitySet;
};

#endif // SEARCHVALUES_H
