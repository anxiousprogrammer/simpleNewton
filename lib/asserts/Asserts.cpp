#include "Asserts.hpp"

//===========================================================================================================================================
//
//  This file is part of simpleNewton. simpleNewton is free software: you can 
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of 
//  the License, or (at your option) any later version.
//  
//  simpleNewton is distributed in the hope that it will be useful, but WITHOUT 
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
//  for more details.
//  
//  You should have received a copy of the GNU General Public License along
//  with simpleNewton (see LICENSE.txt). If not, see <http://www.gnu.org/licenses/>.
//
///   Hack CPP: changes to header won't recompile all source files using it if it were its own unit.
///   \file
///   \addtogroup asserts Asserts
///   \author Nitin Malapally (anxiousprogrammer) <nitin.malapally@gmail.com>
//
//===========================================================================================================================================

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace simpleNewton {

template class Asserts_CPPHackClass<int>;

}   // namespace simpleNewton
#endif
