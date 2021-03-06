//
// Underworld Adventures - an Ultima Underworld remake project
// Copyright (c) 2006,2019 Underworld Adventures Team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
/// \file TextFile.hpp
/// \brief text file class
//
#pragma once

#include "File.hpp"

namespace Base
{
   /// \brief Text file
   /// \details Represents a text file where strings can be read from and
   /// written to. Usage is very similar as File.
   /// The class only supports single-byte strings.
   class TextFile : public File
   {
   public:
      /// default ctor; doesn't open any text file
      TextFile()
      {
      }

      /// ctor; opens text file by name
      TextFile(const std::string& filename, FileOpenMode openMode)
         :File(filename, openMode)
      {
      }

      /// ctor; opens text file from rwops pointer
      TextFile(SDL_RWopsPtr rwops)
         :File(rwops)
      {
      }

      /// reads a whole string line from file
      void ReadLine(std::string& line);

      /// writes a whole string line to file, including newline character
      void WriteLine(const std::string& line);
   };

} // namespace Base
