/*
   Underworld Adventures - an Ultima Underworld hacking project
   Copyright (c) 2002,2003 Underworld Adventures Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id$

*/
/*! \file imgquad.hpp

   \brief image quad class

*/
//! \ingroup userinterface

//@{

// include guard
#ifndef uwadv_imgquad_hpp_
#define uwadv_imgquad_hpp_

// needed includes
#include "window.hpp"
#include "game_interface.hpp"
#include "image.hpp"
#include "texture.hpp"


// classes

//! image quad class
/*! The ua_image_quad class is a ua_window that draws an ua_image on an OpenGL
    screen managed by ua_screen. It supports images up to 320x200 (the
    standard screen size) and supports adding a border to properly render the
    image quad on top of a background image in OpenGL.

    The image quad can be registered using ua_screen::register_window() just
    like any ua_window object. The underlying image can be retrieved via
    get_image().
*/
class ua_image_quad: public ua_window
{
public:
   //! ctor
   ua_image_quad():split_textures(false),has_border(false){}

   //! initializes image quad window
   virtual void init(ua_game_interface& game, unsigned int xpos, unsigned int ypos);

   //! returns image to draw
   ua_image& get_image();

   //! adds border to image, taking border pixels from given image
   void add_border(ua_image& border_img);

   //! updates internal texture when image was changed
   void update();

   // virtual functions from ua_window
   virtual void destroy();
   virtual void draw();

protected:
   //! the image to draw
   ua_image image;

   //! indicates if image has border
   bool has_border;

   //! image quad texture
   ua_texture tex;

   //! indicates if we use two textures to draw the quad
   bool split_textures;
};


// inline methods

inline ua_image& ua_image_quad::get_image()
{
   return image;
}


#endif
//@}
