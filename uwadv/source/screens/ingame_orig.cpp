/*
   Underworld Adventures - an Ultima Underworld hacking project
   Copyright (c) 2002,2003,2004 Underworld Adventures Team

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
/*! \file ingame_orig.cpp

   \brief ingame user interface, from the original uw1

*/

// needed includes
#include "common.hpp"
#include "ingame_orig.hpp"
#include "files.hpp"
#include "renderer.hpp"
#include "underworld.hpp"
#include "audio.hpp"
#include "save_game.hpp"
#include "cutscene_view.hpp"
#include "debug.hpp"
/*
#include "conversation.hpp"
#include <sstream>
#include <iomanip>
*/


// constants

//! time to fade in/out
const double ua_ingame_orig_screen::fade_time = 0.5;


// ua_ingame_orig_screen methods

/*! Constructor; sets parent pointers for ingame controls */
ua_ingame_orig_screen::ua_ingame_orig_screen(ua_game_interface& game)
:ua_screen(game), vitality_flask(true), mana_flask(false),
 dragon_left(true), dragon_right(false)
{
   compass.set_parent(this);
   runeshelf.set_parent(this);
   spellarea.set_parent(this);
   vitality_flask.set_parent(this);
   mana_flask.set_parent(this);
   gargoyle_eyes.set_parent(this);
   dragon_left.set_parent(this);
   dragon_right.set_parent(this);
   command_buttons.set_parent(this);
   view3d.set_parent(this);
   powergem.set_parent(this);
   move_arrows.set_parent(this);
}

void ua_ingame_orig_screen::init()
{
   ua_screen::init();

   ua_trace("orig. ingame user interface started\n");

   // init all variables
   move_turn_left = move_turn_right = move_walk_forward =
      move_walk_backwards = move_run_forward = false;

   priority_cursor = false;

   gamemode = ua_mode_default;
   tickcount = 0;

   fade_state = 0;

   fadeout_action = ua_action_none;
   fadeout_param = 0;

   game.get_image_manager().load_list(img_objects, "objects");



   // set OpenGL flags
   glDisable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);



   ua_settings& settings = game.get_settings();

   // load keymap
   ua_trace("loading keymaps ...\n");
   {
      keymap.init(settings);

      // load keymap from uadata resources
      std::string keymap_name(settings.get_string(ua_setting_game_prefix));
      keymap_name.append("/keymap.cfg");

      ua_trace(" keymap: %s\n",keymap_name.c_str());
      keymap.load(game.get_files_manager().get_uadata_file(keymap_name.c_str()));

      // load custom keymap
      keymap_name = settings.get_string(ua_setting_custom_keymap);

      ua_trace(" keymap: %s",keymap_name.c_str());
      SDL_RWops* rwops = SDL_RWFromFile(keymap_name.c_str(),"rb");

      if (rwops!=NULL)
         keymap.load(rwops);
      else
         ua_trace(" => not available");

      ua_trace("\n");
   }


   register_keymap(&keymap);



   // TODO move to level changing code
   game.get_renderer().prepare_level(game.get_underworld().get_current_level());



   // init images/subwindows

   // load background
   {
      if (settings.get_gametype() == ua_game_uw2)
      {
         // uw2: no background for now
         img_background.get_image().create(320,200);
      }
      else
      {
         // uw1/uw_demo
         const char* mainscreenname = "data/main.byt";

         // replace name when using uw_demo
         if (settings.get_gametype() == ua_game_uw_demo)
            mainscreenname = "data/dmain.byt";

         game.get_image_manager().ua_image_manager::load(img_background.get_image(),
            mainscreenname, 0, 0, ua_img_byt);
      }

      img_background.init(game, 0,0);

      img_background.get_image().fill_rect(236,7, 83,114, 0); // panel area
      img_background.get_image().fill_rect(272,3, 10,4, 0);
      img_background.get_image().fill_rect(272,121, 10,18, 0);

      img_background.update();

      register_window(&img_background);
   }

   // init 3d view window
   view3d.init(game,54,20);
   register_window(&view3d);

   // init compass
   compass.init(game, 112,131);
   compass.add_border(img_background.get_image());
   compass.update();
   register_window(&compass);

   // init textscroll
   {
      unsigned int scrollwidth = 290;

      // adjust scroll width for uw_demo
      if (game.get_settings().get_gametype() == ua_game_uw_demo)
         scrollwidth = 218;

      textscroll.init(game, 15,169, scrollwidth,30, 42);
      //textscroll.init(game, 11,169, 299,29, 42);
      textscroll.set_color_code(ua_cc_black);

      textscroll.print("Welcome to the Underworld Adventures!\n"
         " \\3http://uwadv.sourceforge.net/\\0");

      register_window(&textscroll);

      // fill message scroll background area
      img_background.get_image().fill_rect(16,169, scrollwidth,30, 42);
   }

   // runeshelf
   runeshelf.init(game, 176,138);
   runeshelf.update_runeshelf();
   register_window(&runeshelf);

   // active spells
   spellarea.init(game, 52,136);
   spellarea.update_spell_area();
   register_window(&spellarea);

   // vitality/mana flasks
   vitality_flask.init(game, 248,125);
   vitality_flask.add_border(img_background.get_image());
   register_window(&vitality_flask);

   mana_flask.init(game, 284,125);
   mana_flask.add_border(img_background.get_image());
   register_window(&mana_flask);

   // gargoyle eyes
   gargoyle_eyes.init(game, 128,4);
   //gargoyle_eyes.add_border(img_background.get_image());
   gargoyle_eyes.update_eyes();
   register_window(&gargoyle_eyes);

   // left and right side dragons
   dragon_left.init(game, 36,65);
   register_window(&dragon_left);

   dragon_right.init(game, 204,65);
   register_window(&dragon_right);

   // init command buttons
   command_buttons.init(game, 0,0); // buttons are self-repositioning
   register_window(&command_buttons);

   // init panel
   panel.init(this,235,6);
   register_window(&panel);

   // init powergem
   powergem.init(game,4,139);
   powergem.add_border(img_background.get_image());
   powergem.update_gem();
   register_window(&powergem);

   // init move arrows
   move_arrows.init(game,107,154);
   register_window(&move_arrows);

   // init mouse cursor
   mousecursor.init(game,0);
   mousecursor.show(true);

   register_window(&mousecursor);

   resume();
}

void ua_ingame_orig_screen::suspend()
{
   ua_trace("suspending orig. ingame user interface\n\n");

   game.get_renderer().clear();


   // unregister ourselves
   game.get_underworld().register_callback();


}

void ua_ingame_orig_screen::resume()
{
   ua_trace("resuming orig. ingame user interface\n");


   // register us as callback
   game.get_underworld().register_callback(this);


   // setup fade-in
   fade_state = 0;
   fading.init(true, game.get_tickrate(), fade_time);

   if (fadeout_action == ua_action_conversation)
   {
      // after conversations, play "Dark Abyss"
      game.get_audio_manager().start_music(ua_music_uw1_dark_abyss,true);
   }
   else
   {
      // normal start, play "Descent"
      game.get_audio_manager().start_music(ua_music_uw1_descent,true);
   }
}

void ua_ingame_orig_screen::destroy()
{
   suspend();

   // only windows and other stuff has to be destroyed that wasn't registered
   // with ua_screen::register_window()



   ua_trace("orig. ingame user interface finished\n\n");
}

void ua_ingame_orig_screen::draw()
{
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   // setup 3d world
   ua_vector3d view_offset(0.0, 0.0, 0.0);
   game.get_renderer().setup_camera3d(view_offset);

   // set up viewport for small window
   {
      SDL_Surface* surf = SDL_GetVideoSurface();

      // calculate viewport for window at 52/68 to 226/182
      unsigned int xpos, ypos, width, height;
      xpos = unsigned((surf->w / 320.0) * 52.0);
      ypos = unsigned((surf->h / 200.0) * 68.0);
      width = unsigned((surf->w / 320.0) * (226.0-52.0));
      height = unsigned((surf->h / 200.0) * (182.0-68.0));

      glViewport(xpos,ypos, width,height);

      // render a const world
      game.get_renderer().render_underworld(game.get_underworld());

      // reset viewport
      glViewport(0,0, surf->w,surf->h);
   }

   // render 2d user interface
   {
      game.get_renderer().setup_camera2d();

      glEnable(GL_BLEND);

      if (fade_state == 0 || fade_state == 2)
      {
         // when fading in/out, lay a quad over the already rendered 3d view
         Uint8 alpha = fading.get_fade_value();
         if (alpha < 255)
         {
            glColor4ub(0,0,0,255-alpha);
            glBindTexture(GL_TEXTURE_2D,0);

            // draw quad
            glBegin(GL_QUADS);
            glVertex2i( 52,  68);
            glVertex2i(226,  68);
            glVertex2i(226, 182);
            glVertex2i( 52, 182);
            glEnd();
         }

         glColor3ub(alpha,alpha,alpha);
      }
      else
         glColor3ub(255,255,255);

      // draw all registered windows
      ua_screen::draw();

      glDisable(GL_BLEND);
   }
}

bool ua_ingame_orig_screen::process_event(SDL_Event& event)
{
   return ua_screen::process_event(event);
}

void ua_ingame_orig_screen::key_event(bool key_down, ua_key_value key)
{
   ua_player& pl = game.get_underworld().get_player();
   ua_ingame_game_mode last_gamemode = gamemode;

   switch(key)
   {
      // run forward keys
   case ua_key_run_forward:
   case ua_key_run_forward_easymove:
      move_run_forward = key_down;
      if (key_down)
      {
         pl.set_movement_factor(ua_move_walk,1.0);
         pl.set_movement_mode(ua_move_walk);
      }
      else
      {
         if (move_walk_backwards)
            pl.set_movement_factor(ua_move_walk,-0.4);
         else
            if (move_walk_forward)
               pl.set_movement_factor(ua_move_walk,0.6);
            else
               if (!view3d.get_mouse_move())
                  pl.set_movement_mode(0,ua_move_walk);
      }

      break;

      // walk forward keys
   case ua_key_walk_forward:
   case ua_key_walk_forward_easymove:
      move_walk_forward = key_down;
      if (key_down)
      {
         pl.set_movement_factor(ua_move_walk,0.6);
         pl.set_movement_mode(ua_move_walk);
      }
      else
      {
         if (move_walk_backwards)
            pl.set_movement_factor(ua_move_walk,-0.4);
         else
            if (move_run_forward)
               pl.set_movement_factor(ua_move_walk,1.0);
            else
               if (!view3d.get_mouse_move())
                  pl.set_movement_mode(0,ua_move_walk);
      }
      break;

      // turn left keys
   case ua_key_turn_left:
   case ua_key_turn_left_easymove:
      move_turn_left = key_down;
      if (key_down)
      {
         pl.set_movement_factor(ua_move_rotate,1.0);
         pl.set_movement_mode(ua_move_rotate);
      }
      else
      {
         if (move_turn_right)
            pl.set_movement_factor(ua_move_rotate,-1.0);
         else
            if (!view3d.get_mouse_move())
               pl.set_movement_mode(0,ua_move_rotate);
      }
      break;

      // turn right keys
   case ua_key_turn_right:
   case ua_key_turn_right_easymove:
      move_turn_right = key_down;
      if (key_down)
      {
         pl.set_movement_factor(ua_move_rotate,-1.0);
         pl.set_movement_mode(ua_move_rotate);
      }
      else
      {
         if (move_turn_left)
            pl.set_movement_factor(ua_move_rotate,1.0);
         else
            if (!view3d.get_mouse_move())
               pl.set_movement_mode(0,ua_move_rotate);
      }
      break;

      // slide left key
   case ua_key_slide_left:
      if (key_down)
      {
         pl.set_movement_factor(ua_move_slide,-1.0);
         pl.set_movement_mode(ua_move_slide);
      }
      else
         if (!view3d.get_mouse_move())
            pl.set_movement_mode(0,ua_move_slide);
      break;

      // slide right key
   case ua_key_slide_right:
      if (key_down)
      {
         pl.set_movement_factor(ua_move_slide,1.0);
         pl.set_movement_mode(ua_move_slide);
      }
      else
         if (!view3d.get_mouse_move())
            pl.set_movement_mode(0,ua_move_slide);
      break;

      // walk backwards keys
   case ua_key_walk_backwards:
   case ua_key_walk_backwards_easymove:
      move_walk_backwards = key_down;

      if (key_down)
      {
         pl.set_movement_factor(ua_move_walk,-0.4);
         pl.set_movement_mode(ua_move_walk);
      }
      else
      {
         if (move_walk_forward || move_run_forward)
            pl.set_movement_factor(ua_move_walk,move_run_forward ? 1.0 : 0.6);
         else
            if (!view3d.get_mouse_move())
               pl.set_movement_mode(0,ua_move_walk);
      }
      break;

      // look up key
   case ua_key_look_up:
      if (key_down)
      {
         pl.set_movement_factor(ua_move_lookup,1.0);
         pl.set_movement_mode(ua_move_lookup);
      }
      else
         pl.set_movement_mode(0,ua_move_lookup);
      break;

      // look down key
   case ua_key_look_down:
      if (key_down)
      {
         pl.set_movement_factor(ua_move_lookup,-1.0);
         pl.set_movement_mode(ua_move_lookup);
      }
      else
         pl.set_movement_mode(0,ua_move_lookup);
      break;

      // center look key
   case ua_key_center_view:
      if (key_down)
      {
         pl.set_movement_mode(0,ua_move_lookup);
         pl.set_movement_factor(ua_move_lookup,0.0);
         pl.set_angle_pan(0.0);
      }
      break;

   case ua_key_menu_up:
      if (key_down && gamemode == ua_mode_options)
         command_buttons.select_previous_button();
      break;

   case ua_key_menu_down:
      if (key_down && gamemode == ua_mode_options)
         command_buttons.select_next_button();
      break;

   case ua_key_menu_left:
   case ua_key_menu_right:
      // note: as we don't have menus with 2 columns, these are unused here
      break;

   case ua_key_menu_top_of_list:
   case ua_key_menu_top_of_list2:
      if (key_down && gamemode == ua_mode_options)
         command_buttons.select_previous_button(true);
      break;

   case ua_key_menu_bottom_of_list:
   case ua_key_menu_bottom_of_list2:
      if (key_down && gamemode == ua_mode_options)
         command_buttons.select_next_button(true);
      break;

   case ua_key_menu_press_button:
      if (key_down && gamemode == ua_mode_options)
         command_buttons.do_button_action();
      break;

      // combat keys
   case ua_key_combat_bash:
   case ua_key_combat_slash:
   case ua_key_combat_thrust:
      if (gamemode == ua_mode_fight)
      {
         if (key_down)
         {
            // start combat weapon drawback
            game.get_underworld().user_action(ua_action_combat_draw,
               key == ua_key_combat_bash ? 0 :
               key == ua_key_combat_slash ? 1 : 2);
         }
         else
         {
            // end combat weapon drawback
            game.get_underworld().user_action(ua_action_combat_release, 0);
         }
      }
      break;

      // options menu
   case ua_key_special_options:
      if (key_down && gamemode != ua_mode_options)
      {
         command_buttons.select_button(0);
         command_buttons.do_button_action();
      }
      break;

      // talk mode
   case ua_key_special_talk_mode:
      if (key_down || gamemode == ua_mode_options)
         break;

      if (gamemode != ua_mode_talk)
      {
         command_buttons.select_button(1);
         gamemode = ua_mode_talk;
      }
      else
      {
         command_buttons.select_button(-1);
         gamemode = ua_mode_default;
      }
      break;

      // get mode
   case ua_key_special_get_mode:
      if (key_down || gamemode == ua_mode_options)
         break;

      if (gamemode != ua_mode_get)
      {
         command_buttons.select_button(2);
         gamemode = ua_mode_get;
      }
      else
      {
         command_buttons.select_button(-1);
         gamemode = ua_mode_default;
      }
      break;

      // look mode
   case ua_key_special_look_mode:
      if (key_down || gamemode == ua_mode_options)
         break;

      if (gamemode != ua_mode_look)
      {
         command_buttons.select_button(3);
         gamemode = ua_mode_look;
      }
      else
      {
         command_buttons.select_button(-1);
         gamemode = ua_mode_default;
      }
      break;

      // fight mode
   case ua_key_special_fight_mode:
      if (key_down || gamemode == ua_mode_options)
         break;

      if (gamemode != ua_mode_fight)
      {
         /*bool ret = */game.get_underworld().user_action(ua_action_combat_enter);
         //if (ret)
         {
            command_buttons.select_button(4);
            gamemode = ua_mode_fight;
         }
      }
      else
      {
         command_buttons.select_button(-1);
         gamemode = ua_mode_default;
      }

      break;

      // use mode
   case ua_key_special_use_mode:
      if (key_down || gamemode == ua_mode_options)
         break;

      if (gamemode != ua_mode_use)
      {
         command_buttons.select_button(5);
         gamemode = ua_mode_use;
      }
      else
      {
         command_buttons.select_button(-1);
         gamemode = ua_mode_default;
      }
      break;

   case ua_key_special_flip_panel:
      break;

   case ua_key_special_cast_spell:
      if (!key_down)
         game.get_underworld().user_action(ua_action_clicked_spells);
      break;

   case ua_key_special_use_track:
      if (!key_down)
         game.get_underworld().user_action(ua_action_track_creatures);
      break;

   case ua_key_special_sleep:
      if (!key_down)
         game.get_underworld().user_action(ua_action_sleep);
      break;

      // quicksave key
   case ua_key_special_quicksave:
      if (key_down)
      {
         ua_trace("quicksaving ... ");
         //ui_print_string("quicksaving ...");

         schedule_action(ua_action_quicksave,false);
         ua_trace("done\n");
      }
      break;

      // quickload key
   case ua_key_special_quickload:
      if (key_down)
      {
         ua_trace("quickloading ... ");
         //ui_print_string("quickloading ...");

         schedule_action(ua_action_quickload,false);
         ua_trace("done\n");
      }
      break;

      // "load game" key
   case ua_key_game_restore_game:
      if (key_down)
         schedule_action(ua_action_load_game, true);
      break;

      // "save game" key
   case ua_key_game_save_game:
      if (key_down)
      {
         schedule_action(ua_action_save_game, true);
/*
         // TODO render savegame preview image
         do_screenshot(false,80,50);
         core->get_savegames_mgr().set_save_screenshot(
            screenshot_rgba,screenshot_xres,screenshot_yres);
*/
      }
      break;

      // returns from menu to game
   case ua_key_game_return_to_game:
      break;

      // "debugger" key
   case ua_key_ua_debug:
      if (key_down)
         game.get_debugger().start_debugger(&game);
      break;

      // exit screen key
   case ua_key_ua_return_menu:
      if (key_down)
         schedule_action(ua_action_exit, true);
      break;

      // "screenshot" key
   case ua_key_ua_screenshot:
      if (key_down)
      {
         // take a screenshot
//         do_screenshot(true,0,0);
//         save_screenshot();
      }
      break;

   default:
      break;
   }

   // leaving fight mode?
   if (last_gamemode != gamemode && last_gamemode == ua_mode_fight)
   {
      game.get_underworld().user_action(ua_action_combat_leave);
   }
}

void ua_ingame_orig_screen::tick()
{
   ua_screen::tick();

   // evaluate underworld;
   // only evaluate when the user is not in the options menu
   if (fadeout_action == ua_action_none && gamemode != ua_mode_options)
   {
      game.get_underworld().eval_underworld(double(tickcount)/game.get_tickrate());

      tickcount++;

      // do renderer-specific tick processing
      game.get_renderer().tick(game.get_tickrate());
   }

   // action to perform?
   if ((fade_state == 0 || fade_state == 2) && fading.tick())
   {
      fade_state++;

      // now it's time
      do_action(fadeout_action);

      fadeout_action = ua_action_none;
   }
}

void ua_ingame_orig_screen::schedule_action(ua_ingame_orig_action action, bool fadeout_before)
{
   fadeout_action = action;

   if (fadeout_before)
   {
      fade_state = 2;
      fading.init(false,game.get_tickrate(), fade_time);
   }
}

/*! \todo implement all actions */
void ua_ingame_orig_screen::do_action(ua_ingame_orig_action action)
{
   switch(action)
   {
   case ua_action_none:
      break;

      // leave the ingame_orig screen
   case ua_action_exit:
      game.remove_screen();
      break;

      // start "load game" screen
   case ua_action_load_game:
      game.replace_screen(new ua_save_game_screen(game,false),true);
      break;

      // start "save game" screen
   case ua_action_save_game:
      game.replace_screen(new ua_save_game_screen(game,false),true);
      break;

      // quickloading
   case ua_action_quickload:
      {
/*
         ua_savegame sg = game.get_savegames_manager().get_quicksave(
            false,game.get_underworld().get_player());
         game.get_underworld().load_game(sg);
*/
      }
      break;

      // quicksaving
   case ua_action_quicksave:
      {
/*
         // render savegame preview image
         do_screenshot(false,80,50);
         game.get_savegames_managergr().set_save_screenshot(
            screenshot_rgba,screenshot_xres,screenshot_yres);

         ua_savegame sg = game.get_savegames_manager().get_quicksave(
            true,game.get_underworld().get_player());
         game.get_underworld().save_game(sg);
*/
      }
      break;

      // starts conversation
   case ua_action_conversation:
      //game.replace_screen(new ua_conversation_screen(game,fadeout_param),true);
      break;

      // shows cutscene
   case ua_action_cutscene:
      game.replace_screen(new ua_cutscene_view_screen(game,fadeout_param),true);
      break;

   default:
      break;
   }
}

bool ua_ingame_orig_screen::get_move_state(ua_ingame_move_state movestate)
{
   bool state = false;
   switch(movestate)
   {
   case ua_move_turn_left: state = move_turn_left; break;
   case ua_move_turn_right: state = move_turn_right; break;
   case ua_move_walk_forward: state = move_walk_forward; break;
   case ua_move_run_forward: state = move_run_forward; break;
   case ua_move_walk_backwards: state = move_walk_backwards; break;
   default: break;
   }
   return state;
}

/*! Sets a new cursor image for the mouse cursor. There can be normal cursor
    images and prioritized cursor images. The last ones cannot be changed or
    resetted by unprioritized ones. Passing a negative index resets priority
    mode and sets the negated index as normal cursor index. If the cursor
    index is > 0x100, an object image is used for the cursor image.

    \param index cursor image index to set
    \param priority indicates if priority cursor image should be set
*/
void ua_ingame_orig_screen::set_cursor(int index, bool priority)
{
   if (!priority && priority_cursor)
      return; // ignore new non-priority cursor

   priority_cursor = priority;

   // reset priority cursor
   if (index < 0 && priority)
   {
      priority_cursor = false;
      index = -index;
   }

   if (index>0x100)
   {
      // change mouse cursor type to object image
      mousecursor.set_custom(img_objects[unsigned(index)-0x100]);
   }
   else
   {
      // set image from cursors
      mousecursor.set_type(unsigned(index));
   }
}

void ua_ingame_orig_screen::uw_notify(ua_underworld_notification notify,
   unsigned int param)
{
   switch(notify)
   {
   case ua_notify_update_powergem:
      powergem.update_gem();
      break;

   case ua_notify_update_gargoyle_eyes:
      gargoyle_eyes.update_eyes();
      break;

   case ua_notify_update_flasks:
      vitality_flask.update_flask();
      mana_flask.update_flask();
      break;

   case ua_notify_update_show_ingame_animation:
      break;

   case ua_notify_update_spell_area:
      spellarea.update_spell_area();
      break;

   case ua_notify_animate_dragons:
      //dragon_left.animate();
      //dragon_right.animate();
      break;

   case ua_notify_level_change:
      break;

   case ua_notify_select_target:
      //target_select_mode = true
      set_cursor(param+0x100,true);
      break;

   default:
      break;
   }
}

void ua_ingame_orig_screen::uw_print(const char* text)
{
   textscroll.print(text);
}

void ua_ingame_orig_screen::uw_start_conversation(unsigned int list_pos)
{
   fadeout_param = list_pos;
   schedule_action(ua_action_conversation, true);
}

/*
void ua_ingame_orig_screen::do_screenshot(bool with_menu, unsigned int xres, unsigned int yres)
{
   if (xres==0) xres = core->get_screen_width();
   if (yres==0) yres = core->get_screen_height();

   // set up camera and viewport
   glViewport(0,0,xres,yres);
   renderer.setup_camera(90.0,double(xres)/yres,16.0);

   glClear(GL_COLOR_BUFFER_BIT);

   // render scene
   if (with_menu)
      render();
   else
   {
      glDisable(GL_SCISSOR_TEST);
      renderer.render();
      glEnable(GL_SCISSOR_TEST);
   }

   // prepare screenshot
   screenshot_xres = xres;
   screenshot_yres = yres;

   screenshot_rgba.clear();
   screenshot_rgba.resize(xres*yres,0);

   // read in scanlines
   glReadBuffer(GL_BACK);

   for(int i=yres-1; i>=0 ;i--)
   {
      glReadPixels(0, i, xres, 1, GL_RGBA, GL_UNSIGNED_BYTE,
         &screenshot_rgba[(yres-1-i)*xres]);
   }

   // reset camera and viewport
   glViewport(0,0,core->get_screen_width(),core->get_screen_height());

   renderer.setup_camera(90.0,
      double(core->get_screen_width())/core->get_screen_height(),
      16.0);
}
*/
