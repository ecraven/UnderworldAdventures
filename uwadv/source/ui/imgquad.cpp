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
/*! \file imgquad.cpp

   \brief image quad implementation

*/

// needed includes
#include "common.hpp"
#include "imgquad.hpp"
#include "renderer.hpp"


// ua_image_quad methods

/*! Does image quad initialisation; ua_window::create() is called and needed
    texture(s) are init'ed. When there's no palette for the image yet, palette
    0 is retrieved and set.

    The method is virtual and can be derived to do window/control specific
    init'ing before or after calling this method.

    \param game reference to game interface
    \param xpos x position on ua_screen
    \param ypos y position
*/
void ua_image_quad::init(ua_game_interface& game, unsigned int xpos,
   unsigned int ypos)
{
   ua_window::create(xpos,ypos,image.get_xres(),image.get_yres());

   if (image.get_palette().get() == NULL)
      image.get_palette() = game.get_image_manager().get_palette(0);

   tex.init(&game.get_renderer().get_texture_manager(), 2);

   smooth_ui = game.get_settings().get_bool(ua_setting_ui_smooth);
}

/*! The method adds a border around the current quad image, based on the
    given image; the border pixels are taken from the window coordinates
    set during ua_window::create() that is called in ua_image_quad::init().
    The window is enlarged by 2 pixels in width and height and moved one
    pixel left and up. To update the quad image, paste new image contents
    to the coordinates destx = 1, desty = 1.

    The border is not added to the image if the image quad has to use
    split-textures to draw the quad.

    \param border_img the background image where borders are copied from
*/
void ua_image_quad::add_border(ua_image& border_img)
{
   if (split_textures)
      return; // no border support for large images

   ua_image img_temp = image;

   // enlarge the image
   image.create(image.get_xres()+2, image.get_yres()+2);

   // left border
   image.paste_rect(border_img, wnd_xpos-1,wnd_ypos-1, 1,wnd_height+2, 0,0);
   // right border
   image.paste_rect(border_img, wnd_xpos+wnd_width,wnd_ypos-1, 1,wnd_height+2, wnd_width+1,0);

   // top border
   image.paste_rect(border_img, wnd_xpos,wnd_ypos-1, wnd_width,1, 1,0);
   // bottom border
   image.paste_rect(border_img, wnd_xpos,wnd_ypos+wnd_height, wnd_width,1, 1,wnd_height+1);

   // image
   image.paste_rect(img_temp, 0,0, img_temp.get_xres(),img_temp.get_yres(), 1,1);

   wnd_xpos--;
   wnd_ypos--;

   has_border = true;
}

/*! Updates image quad texture(s) with changes from the quad image available
    through get_image(). The method determines if the texture will get larger
    than 256 in height or width and uses a split-texture approach then. This
    is required for older cards that only support textures up to 256x256.
*/
void ua_image_quad::update()
{
   wnd_width = image.get_xres();
   wnd_height = image.get_yres();

   split_textures = wnd_width > 254;

   if (split_textures)
   {
      // check if textures > 256 x 256 are supported
      GLint maxtexsize;
      glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxtexsize);

      split_textures = maxtexsize <= 256;
   }

   // upload image
   if (split_textures)
   {
      wnd_width += (wnd_width&1); // even width

      // split text image into two
      ua_image img_split1,img_split2;

      unsigned int texwidth = image.get_xres()/2;
      unsigned int texheight = image.get_yres();

      // create images
      img_split1.create(texwidth+1,texheight);
      img_split2.create(texwidth+1,texheight);

      img_split1.get_palette() = image.get_palette();
      img_split2.get_palette() = image.get_palette();

      // paste contents
      img_split1.paste_rect(image,0,0, texwidth+1,texheight, 0,0);
      img_split2.paste_rect(image,texwidth,0, texwidth,texheight, 0,0);

      img_split2.paste_rect(image,image.get_xres()-1,0, 1,texheight, texwidth,0); // copy border

      // upload it to the texture
      tex.convert(img_split1,0);
      tex.upload(0);

      tex.convert(img_split2,1);
      tex.upload(1);
   }
   else
   {
      // image is small enough
      tex.convert(image);
      tex.upload(0);
   }
}

/*! Cleans up texture(s) used for rendering the image quad. */
void ua_image_quad::destroy()
{
   tex.done();
}

/*! Draws the image quad. The method takes into account if a border was added
    with add_border(), and if a split-texture has to be used. */
void ua_image_quad::draw()
{
   double u = tex.get_tex_u(), v = tex.get_tex_v();
   tex.use(0);

   unsigned int quadwidth = wnd_width;
   unsigned int quadheight = wnd_height;
   double dx = 0.0, dy = 0.0;
   unsigned int dpx, dpy;

   unsigned int x0, x1, y0, y1;
   x0 = wnd_xpos;
   x1 = wnd_xpos+quadwidth;

   y0 = 200-wnd_ypos;
   y1 = 200-wnd_ypos-quadheight;


   if (split_textures)
   {
      dx = 1.22*0.5/quadwidth;
      quadwidth = wnd_width/2;
   }
   else
   if (has_border)
   {
      dx = 1.0/quadwidth;
      dy = 1.0/quadheight;
      x0++; x1--; y0--; y1++;
   }

   double u0 = dx, u1 = u-dx;
   double v0 = dy, v1 = v-dy;

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth_ui ? GL_LINEAR : GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth_ui ? GL_LINEAR : GL_NEAREST);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   // render (first) quad
   glBegin(GL_QUADS);
   glTexCoord2d(u0, v1); glVertex2d(x0,y1);
   glTexCoord2d(u1, v1); glVertex2d(x1,y1);
   glTexCoord2d(u1, v0); glVertex2d(x1,y0);
   glTexCoord2d(u0, v0); glVertex2d(x0,y0);
   glEnd();

   if (split_textures)
   {
      tex.use(1);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth_ui ? GL_LINEAR : GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth_ui ? GL_LINEAR : GL_NEAREST);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      // render second quad
      glBegin(GL_QUADS);
      glTexCoord2d(0.0, v );  glVertex2d(wnd_xpos+quadwidth,  200-wnd_ypos-quadheight);
      glTexCoord2d(u1,  v );  glVertex2d(wnd_xpos+quadwidth*2,200-wnd_ypos-quadheight);
      glTexCoord2d(u1,  0.0); glVertex2d(wnd_xpos+quadwidth*2,200-wnd_ypos);
      glTexCoord2d(0.0, 0.0); glVertex2d(wnd_xpos+quadwidth,  200-wnd_ypos);
      glEnd();
   }
}
