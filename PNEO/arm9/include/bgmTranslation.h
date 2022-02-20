#pragma once
#include "bgmNames.h"
#include "soundbank.h"

#define MOD_NONE -1

constexpr int BGMIndexForName( unsigned p_name ) {
   switch( p_name ) {
   default: return -1;
   case BGM_NONE: return MOD_NONE;
   case BGM_LITTLEROOT_TOWN: return MOD_LITTLEROOT_TOWN;
   case BGM_OLDALE_TOWN: return MOD_OLDALE_TOWN;
   case BGM_PETALBURG_CITY: return MOD_PETALBURG_CITY;
   case BGM_RUSTBORO_CITY: return MOD_RUSTBORO_CITY;
   case BGM_DEWFORD_TOWN: return MOD_DEWFORD_TOWN;
   case BGM_SLATEPORT_CITY: return MOD_SLATEPORT_CITY;
   case BGM_VERDANTURF_TOWN: return MOD_VERDANTURF_TOWN;
   case BGM_FALLARBOR_TOWN: return MOD_FALLARBOR_TOWN;
   case BGM_FORTREE_CITY: return MOD_FORTREE_CITY;
   case BGM_LILYCOVE_CITY: return MOD_LILYCOVE_CITY;
   case BGM_NEW_LILYCOVE_CITY: return MOD_NEW_LILYCOVE_CITY;
   case BGM_SOOTOPOLIS_CITY: return MOD_SOOTOPOLIS_CITY;
   case BGM_CLIFFELTA_CITY: return MOD_CLIFFELTA_CITY;
   case BGM_EX07: return MOD_EX07;
   case BGM_ROUTE_101: return MOD_ROUTE_101;
   case BGM_ROUTE_104: return MOD_ROUTE_104;
   case BGM_ROUTE_110: return MOD_ROUTE_110;
   case BGM_ROUTE_113: return MOD_ROUTE_113;
   case BGM_ROUTE_119: return MOD_ROUTE_119;
   case BGM_ROUTE_120: return MOD_ROUTE_120;
   case BGM_ROUTE_123: return MOD_ROUTE_123;
   case BGM_ROUTE_135: return MOD_ROUTE_135;
   case BGM_ROUTE_38: return MOD_ROUTE_38;
   case BGM_ROUTE_10: return MOD_ROUTE_10;
   case BGM_POKEMON_CENTER: return MOD_POKEMON_CENTER;
   case BGM_POKEMON_GYM: return MOD_POKEMON_GYM;
   case BGM_POKEMON_MART: return MOD_POKEMON_MART;
   case BGM_PROF_BIRCH_LAB: return MOD_PROF_BIRCH_LAB;
   case BGM_CAVE_FORESTS: return MOD_CAVE_FORESTS;
   case BGM_EX03: return MOD_EX03;
   case BGM_TRAINER_SCHOOL: return MOD_TRAINER_SCHOOL;
   case BGM_ABANDONED_SHIP: return MOD_ABANDONED_SHIP;
   case BGM_MARINE_SCIENCE_MUSEUM: return MOD_MARINE_SCIENCE_MUSEUM;
   case BGM_TRICK_HOUSE: return MOD_TRICK_HOUSE;
   case BGM_NEW_MAUVILLE: return MOD_NEW_MAUVILLE;
   case BGM_MT_CHIMNEY: return MOD_MT_CHIMNEY;
   case BGM_METEOR_FALLS: return MOD_METEOR_FALLS;
   case BGM_SAFARI_ZONE: return MOD_SAFARI_ZONE;
   case BGM_LILYCOVE_MUSEUM: return MOD_LILYCOVE_MUSEUM;
   case BGM_MT_PYRE_PEAK: return MOD_MT_PYRE_PEAK;
   case BGM_TEAM_AM_HIDEOUT: return MOD_TEAM_AM_HIDEOUT;
   case BGM_SHOAL_CAVE: return MOD_SHOAL_CAVE;
   case BGM_VICTORY_ROAD: return MOD_VICTORY_ROAD;
   case BGM_SEALED_CHAMBER: return MOD_SEALED_CHAMBER;
   case BGM_CRYSTAL_CAVERN: return MOD_CRYSTAL_CAVERN;
   case BGM_BATTLE_FRONTIER: return MOD_BATTLE_FRONTIER;
   case BGM_BATTLE_TOWER: return MOD_BATTLE_TOWER;
   case BGM_BATTLE_ARENA: return MOD_BATTLE_ARENA;
   case BGM_BATTLE_FACTORY: return MOD_BATTLE_FACTORY;
   case BGM_BATTLE_PALACE: return MOD_BATTLE_PALACE;
   case BGM_BATTLE_PYRAMID_PEAK: return MOD_BATTLE_PYRAMID_PEAK;
   case BGM_BATTLE_WILD: return MOD_BATTLE_WILD;
   case BGM_BATTLE_WILD_ALT: return MOD_BATTLE_WILD_ALT;
   case BGM_BATTLE_REGI: return MOD_BATTLE_REGI;
   case BGM_BATTLE_RAYQUAZA: return MOD_BATTLE_RAYQUAZA;
   case BGM_BATTLE_RAIKOU_ENTEI_SUICUNE: return MOD_BATTLE_RAIKOU_ENTEI_SUICUNE;
   case BGM_BATTLE_EX01: return MOD_BATTLE_EX01;
   case BGM_BATTLE_DIALGA_PALKIA: return MOD_BATTLE_DIALGA_PALKIA;
   case BGM_BATTLE_TRAINER: return MOD_BATTLE_TRAINER;
   case BGM_BATTLE_MAY_BRENDAN: return MOD_BATTLE_MAY_BRENDAN;
   case BGM_BATTLE_TEAM_AM: return MOD_BATTLE_TEAM_AM;
   case BGM_BATTLE_GYM_LEADER: return MOD_BATTLE_GYM_LEADER;
   case BGM_BATTLE_CHAMPION: return MOD_BATTLE_CHAMPION;
   case BGM_BATTLE_FRONTIER_BRAIN: return MOD_BATTLE_FRONTIER_BRAIN;
   case BGM_VICTORY_GYM_LEADER: return MOD_VICTORY_GYM_LEADER;
   case BGM_VICTORY_TEAM_AM_GRUNT: return MOD_VICTORY_TEAM_AM_GRUNT;
   case BGM_VICTORY_TRAINER: return MOD_VICTORY_TRAINER;
   case BGM_VICTORY_WILD: return MOD_VICTORY_WILD;
   case BGM_ENCOUNTER_BRENDAN: return MOD_ENCOUNTER_BRENDAN;
   case BGM_ENCOUNTER_MAY: return MOD_ENCOUNTER_MAY;
   case BGM_ENCOUNTER_TEAM_AM: return MOD_ENCOUNTER_TEAM_AM;
   case BGM_ENCOUNTER_ELECTRIC: return MOD_ENCOUNTER_ELECTRIC;
   case BGM_ENCOUNTER_HIKER: return MOD_ENCOUNTER_HIKER;
   case BGM_ENCOUNTER_SAILOR: return MOD_ENCOUNTER_SAILOR;
   case BGM_ENCOUNTER_TUBER: return MOD_ENCOUNTER_TUBER;
   case BGM_ENCOUNTER_TWINS: return MOD_ENCOUNTER_TWINS;
   case BGM_ENCOUNTER_YOUNGSTER: return MOD_ENCOUNTER_YOUNGSTER;
   case BGM_ENCOUNTER_SHIRONA: return MOD_ENCOUNTER_SHIRONA;
   case BGM_HELP_PROF_BIRCH: return MOD_HELP_PROF_BIRCH;
   case BGM_SHOW_ME_AROUND: return MOD_SHOW_ME_AROUND;
   case BGM_CYCLING: return MOD_CYCLING;
   case BGM_SURFING: return MOD_SURFING;
   case BGM_SURFING_ALT: return MOD_SURFING_ALT;
   case BGM_SAILING: return MOD_SAILING;
   case BGM_DIVING: return MOD_DIVING;
   case BGM_UNWAVERING_EMOTIONS: return MOD_UNWAVERING_EMOTIONS;
   case BGM_DESERT: return MOD_DESERT;
   case BGM_CABLE_CAR: return MOD_CABLE_CAR;
   case BGM_EVOLVING: return MOD_EVOLVING;
   case BGM_MYSTERY_GIFT: return MOD_MYSTERY_GIFT;
   case BGM_OS_BADGE: return MOD_OS_BADGE;
   case BGM_OS_SYMBOL: return MOD_OS_SYMBOL;
   case BGM_OS_TM: return MOD_OS_TM;
   case BGM_OS_EVOLVED: return MOD_OS_EVOLVED;
   case BGM_OS_HEAL_PKMN: return MOD_OS_HEAL_PKMN;
   case BGM_OS_LEVEL_UP: return MOD_OS_LEVEL_UP;
   case BGM_OS_PKMN_CAPTURE: return MOD_OS_PKMN_CAPTURE;
   case BGM_EX01: return MOD_EX01;
   case BGM_EX02: return MOD_EX02;
   case BGM_MT_MOON: return MOD_MT_MOON;
   case BGM_ECRUTEAK_CITY: return MOD_ECRUTEAK_CITY;
   case BGM_EX04: return MOD_EX04;
   case BGM_EX05: return MOD_EX05;
   case BGM_LAKE_CAVE: return MOD_LAKE_CAVE;
   case BGM_DISTORTION_WORLD: return MOD_DISTORTION_WORLD;
   case BGM_EX06: return MOD_EX06;
   case BGM_POKE_RADAR: return MOD_POKE_RADAR;
   }
}
