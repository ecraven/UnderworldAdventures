/*
   Underworld Adventures - an Ultima Underworld remake project
   Copyright (c) 2006 Michael Fink

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
/*! \file importtest.cpp

   \brief import functions test

*/

// needed includes
#include "unittest.hpp"
#include "settings.hpp"
#include "resourcemanager.hpp"
#include "import.hpp"
#include "properties.hpp"
#include "levellist.hpp"

namespace UnitTest
{

//! Import test
/*! Tests importing all kinds of original game data files. */
class TestImport: public UnitTestCase
{
public:
   // generate test suite
   CPPUNIT_TEST_SUITE(TestImport)
      CPPUNIT_TEST(TestObjectPropertiesImport)
      CPPUNIT_TEST(TestLevelListImport)
   CPPUNIT_TEST_SUITE_END()

protected:
   void TestObjectPropertiesImport();
   void TestLevelListImport();
};

// register test suite
CPPUNIT_TEST_SUITE_REGISTRATION(TestImport)

} // namespace UnitTest

// methods

using namespace UnitTest;


/*! Tests importing object properties */
void TestImport::TestObjectPropertiesImport()
{
   Base::Settings settings;
   Base::LoadSettings(settings);

   settings.SetValue(Base::settingGamePrefix, std::string("uw1"));
   settings.SetValue(Base::settingUnderworldPath, settings.GetString(Base::settingUw1Path));

   {
      Base::ResourceManager resourceManager(settings);

      // load properties
      Underworld::ObjectProperties objectProperties;

      Import::ImportProperties(resourceManager, objectProperties);

      // test if all property vectors are filled
      CPPUNIT_ASSERT(0x0200 == objectProperties.GetVectorCommonObjectProperties().size());
      CPPUNIT_ASSERT(0x0010 == objectProperties.GetVectorMeleeWeaponProperties().size());
      CPPUNIT_ASSERT(0x0010 == objectProperties.GetVectorRangedWeaponProperties().size());
      CPPUNIT_ASSERT(0x0020 == objectProperties.GetVectorArmourAndWearableProperties().size());
      CPPUNIT_ASSERT(0x0040 == objectProperties.GetVectorCritterProperties().size());
      CPPUNIT_ASSERT(0x0010 == objectProperties.GetVectorContainerProperties().size());
      CPPUNIT_ASSERT(0x0010 == objectProperties.GetVectorLightSourceProperties().size());
      CPPUNIT_ASSERT(0x0010 == objectProperties.GetVectorAnimatedObjectProperties().size());
   }

   settings.SetValue(Base::settingGamePrefix, std::string("uw2"));
   settings.SetValue(Base::settingUnderworldPath, settings.GetString(Base::settingUw2Path));

   {
      Base::ResourceManager resourceManager(settings);

      // load properties
      Underworld::ObjectProperties objectProperties;

      Import::ImportProperties(resourceManager, objectProperties);

      // test if all property vectors are filled
      CPPUNIT_ASSERT(0x0200 == objectProperties.GetVectorCommonObjectProperties().size());
      CPPUNIT_ASSERT(0x0010 == objectProperties.GetVectorMeleeWeaponProperties().size());
      CPPUNIT_ASSERT(0x0010 == objectProperties.GetVectorRangedWeaponProperties().size());
      CPPUNIT_ASSERT(0x0020 == objectProperties.GetVectorArmourAndWearableProperties().size());
      CPPUNIT_ASSERT(0x0040 == objectProperties.GetVectorCritterProperties().size());
      CPPUNIT_ASSERT(0x0010 == objectProperties.GetVectorContainerProperties().size());
      CPPUNIT_ASSERT(0x0010 == objectProperties.GetVectorLightSourceProperties().size());
      CPPUNIT_ASSERT(0x0010 == objectProperties.GetVectorAnimatedObjectProperties().size());
   }
}

/*! Tests loading level list */
void TestImport::TestLevelListImport()
{
   Base::Settings settings;
   Base::LoadSettings(settings);

#if 0
   // uw_demo loading
   settings.SetValue(Base::settingGamePrefix, std::string("uw1"));
   settings.SetValue(Base::settingUw1IsUwdemo, true);
   settings.SetValue(Base::settingUnderworldPath, "d:\\uwadv\\uw_demo\\");

   {
      Base::ResourceManager resourceManager(settings);
      Import::LevelImporter levelImporter(resourceManager);

      Underworld::LevelList levelList;
      levelImporter.LoadUwDemoLevel(levelList);

      UaAssert(levelList.GetNumLevels() == 1);
   }

   settings.SetValue(Base::settingUw1IsUwdemo, false);
#endif

   settings.SetValue(Base::settingGamePrefix, std::string("uw1"));
   settings.SetValue(Base::settingUnderworldPath, settings.GetString(Base::settingUw1Path));

   {
      Base::ResourceManager resourceManager(settings);
      Import::LevelImporter levelImporter(resourceManager);

      Underworld::LevelList levelList;
      levelImporter.LoadUw1Levels(levelList);

      UaAssert(levelList.GetNumLevels() == 9);
   }

   settings.SetValue(Base::settingGamePrefix, std::string("uw2"));
   settings.SetValue(Base::settingUnderworldPath, settings.GetString(Base::settingUw2Path));

   {
      Base::ResourceManager resourceManager(settings);
      Import::LevelImporter levelImporter(resourceManager);

      Underworld::LevelList levelList;
      levelImporter.LoadUw2Levels(levelList);

      UaAssert(levelList.GetNumLevels() == 80);
   }
}