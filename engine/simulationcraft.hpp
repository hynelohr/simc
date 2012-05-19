// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================
#ifndef SIMULATIONCRAFT_H
#define SIMULATIONCRAFT_H

// Platform Initialization ==================================================

#if defined( _MSC_VER ) || defined( __MINGW__ ) || defined( __MINGW32__ ) || defined( _WINDOWS ) || defined( WIN32 )
#  define WIN32_LEAN_AND_MEAN
#  define VC_EXTRALEAN
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  define DIRECTORY_DELIMITER "\\"
#  ifndef UNICODE
#    define UNICODE
#  endif
#else
#  define DIRECTORY_DELIMITER "/"
#  define SC_SIGACTION
#endif

#if ( _MSC_VER && _MSC_VER < 1600 )
#  include "../vs/stdint.h"
#else
#  include <stdint.h>
#endif

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <typeinfo>
#include <vector>

#if _MSC_VER || __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
// Use C++11
#include <array>
#include <type_traits>
#include <unordered_map>
#if ( _MSC_VER && _MSC_VER < 1600 )
namespace std {using namespace tr1; }
#endif
#else
// Use TR1
#include <tr1/array>
#include <tr1/functional>
#include <tr1/type_traits>
#include <tr1/unordered_map>
namespace std {using namespace tr1; }
#endif

#include "dbc/data_enums.hh"
#include "dbc/data_definitions.hh"
#include "utf8.h"

#if __BSD_VISIBLE
#  include <netinet/in.h>
#  if !defined(CLOCKS_PER_SEC)
#    define CLOCKS_PER_SEC 1000000
#  endif
#endif

#if _MSC_VER >= 1600 || __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
#define smart_ptr unique_ptr
#else
#define smart_ptr auto_ptr
#define static_assert( condition, message )
#endif

// GCC (and probably the C++ standard in general) doesn't like offsetof on non-POD types
#ifdef _MSC_VER
#define nonpod_offsetof(t, m) offsetof(t, m)
#else
#define nonpod_offsetof(t, m) ((size_t) ( (volatile char *)&((volatile t *)(size_t)0x10000)->m - (volatile char *)(size_t)0x10000 ))
#endif

#if defined(__GNUC__)
#  define likely(x)         __builtin_expect((x),1)
#  define unlikely(x)       __builtin_expect((x),0)
#else
#  define likely(x)        (x)
#  define unlikely(x)      (x)
#  define __attribute__(x)
#endif

#define SC_PACKED_STRUCT      __attribute__((packed))
#define PRINTF_ATTRIBUTE(a,b) __attribute__((format(printf,a,b)))

#define SC_MAJOR_VERSION "501"
#define SC_MINOR_VERSION "2"
#define SC_USE_PTR ( 0 )
#define SC_BETA ( 1 )
#ifndef M_PI
#define M_PI ( 3.14159265358979323846 )
#endif

// TODO: Integer time is only partially working.
#define SC_USE_INTEGER_TIME

// Forward Declarations =====================================================

struct absorb_buff_t;
struct action_callback_t;
struct action_priority_list_t;
struct action_state_t;
struct action_t;
struct alias_t;
struct attack_t;
struct benefit_t;
struct buff_t;
struct buff_creator_t;
struct buff_uptime_t;
struct callback_t;
struct cooldown_t;
struct cost_reduction_buff_t;
class  dbc_t;
struct debuff_t;
struct dot_t;
struct effect_t;
struct enchant_t;
struct event_t;
class  expr_t;
struct gain_t;
struct heal_t;
struct item_t;
struct js_node_t;
struct option_t;
struct pet_t;
struct player_t;
struct plot_t;
struct proc_t;
struct raid_event_t;
struct rating_t;
struct reforge_plot_data_t;
struct reforge_plot_t;
struct report_t;
struct rng_t;
struct sample_data_t;
struct scaling_t;
struct sim_t;
struct spell_data_t;
struct spell_id_t;
struct spell_t;
struct spelleffect_data_t;
struct stats_t;
struct stat_buff_t;
struct unique_gear_t;
struct uptime_t;
struct weapon_t;
struct xml_node_t;

// Type traits and metaprogramming tools ====================================

template <bool Condition,typename T>
struct enable_if { typedef T type; };
template <typename T>
struct enable_if<false,T> {};

template <typename T>
struct iterator_type
{ typedef typename T::iterator type; };

template <typename T>
struct iterator_type<const T>
{ typedef typename T::const_iterator type; };

// iterable enumeration templates ===========================================

/*
 * Enumeration types in C++ implicitly convert to int but not from int (e.g.,
 * "for ( attribute_type_e i = ATTR_STRENGTH; i < ATTR_MAX; ++i )" won't compile).
 * This is why so much of the code uses int when it really means an enum type.
 * Providing the kind of operations we want to use for enums lets us tighten up
 * our use of the type system and avoid accidentally passing some other thing
 * that converts to int when we really mean an enumeration type.
 *
 * The template functions tell the compiler it can perform prefix and postfix
 * operators ++ and -- on any type by converting it to int and back. The magic
 * with std::enable_if<> restricts those operations to types T for which the
 * type trait "is_iterable_enum<T>" is true. This trait gives us a way to
 * selectively control the functionality for a specific type T by specializing
 * is_iterable_enum<T> as std::true_type or std::false_type.
 */

// All enumerations are iterable by default.
template <typename T>
struct is_iterable_enum : public std::is_enum<T> {};

template <typename T>
inline typename enable_if<is_iterable_enum<T>::value,T&>::type
operator -- ( T& s )
{ return s = static_cast<T>( s - 1 ); }

template <typename T>
inline typename enable_if<is_iterable_enum<T>::value,T>::type
operator -- ( T& s, int )
{
  T tmp = s;
  --s;
  return tmp;
}

template <typename T>
inline typename enable_if<is_iterable_enum<T>::value,T&>::type
operator ++ ( T& s )
{ return s = static_cast<T>( s + 1 ); }

template <typename T>
inline typename enable_if<is_iterable_enum<T>::value,T>::type
operator ++ ( T& s, int )
{
  T tmp = s;
  ++s;
  return tmp;
}

// Enumerations =============================================================
// annex _e to enumerations

enum talent_format_e
{
  TALENT_FORMAT_NUMBERS=0,
  TALENT_FORMAT_ARMORY,
  TALENT_FORMAT_WOWHEAD,
  TALENT_FORMAT_UNCHANGED,
  TALENT_FORMAT_MAX
};

enum race_type_e
{
  RACE_NONE=0,
  // Target Races
  RACE_BEAST, RACE_DRAGONKIN, RACE_GIANT, RACE_HUMANOID, RACE_DEMON, RACE_ELEMENTAL,
  // Player Races
  RACE_NIGHT_ELF, RACE_HUMAN, RACE_GNOME, RACE_DWARF, RACE_DRAENEI, RACE_WORGEN,
  RACE_ORC, RACE_TROLL, RACE_UNDEAD, RACE_BLOOD_ELF, RACE_TAUREN, RACE_GOBLIN,
  RACE_PANDAREN, RACE_PANDAREN_ALLIANCE, RACE_PANDAREN_HORDE,
  RACE_MAX
};

enum player_type_e
{
  PLAYER_SPECIAL_SCALE=-1,
  PLAYER_NONE=0,
  DEATH_KNIGHT, DRUID, HUNTER, MAGE, MONK, PALADIN, PRIEST, ROGUE, SHAMAN, WARLOCK, WARRIOR,
  PLAYER_PET, PLAYER_GUARDIAN,
  ENEMY, ENEMY_ADD,
  PLAYER_MAX
};

enum pet_type_e
{
  PET_NONE=0,

  // Ferocity
  PET_CARRION_BIRD,
  PET_CAT,
  PET_CORE_HOUND,
  PET_DEVILSAUR,
  PET_DOG,
  PET_FOX,
  PET_HYENA,
  PET_MOTH,
  PET_RAPTOR,
  PET_SPIRIT_BEAST,
  PET_TALLSTRIDER,
  PET_WASP,
  PET_WOLF,

  PET_FEROCITY_TYPE,

  // Tenacity
  PET_BEAR,
  PET_BEETLE,
  PET_BOAR,
  PET_CRAB,
  PET_CROCOLISK,
  PET_GORILLA,
  PET_RHINO,
  PET_SCORPID,
  PET_SHALE_SPIDER,
  PET_TURTLE,
  PET_WARP_STALKER,
  PET_WORM,

  PET_TENACITY_TYPE,

  // Cunning
  PET_BAT,
  PET_BIRD_OF_PREY,
  PET_CHIMERA,
  PET_DRAGONHAWK,
  PET_MONKEY,
  PET_NETHER_RAY,
  PET_RAVAGER,
  PET_SERPENT,
  PET_SILITHID,
  PET_SPIDER,
  PET_SPOREBAT,
  PET_WIND_SERPENT,

  PET_CUNNING_TYPE,

  PET_HUNTER,

  PET_FELGUARD,
  PET_FELHUNTER,
  PET_IMP,
  PET_VOIDWALKER,
  PET_SUCCUBUS,
  PET_INFERNAL,
  PET_DOOMGUARD,
  PET_WILD_IMP,
  PET_WARLOCK,

  PET_GHOUL,
  PET_BLOODWORMS,
  PET_DANCING_RUNE_WEAPON,
  PET_DEATH_KNIGHT,

  PET_TREANTS,
  PET_DRUID,

  PET_WATER_ELEMENTAL,
  PET_MAGE,

  PET_SHADOWFIEND,
  PET_MINDBENDER,
  PET_PRIEST,

  PET_SPIRIT_WOLF,
  PET_FIRE_ELEMENTAL,
  PET_EARTH_ELEMENTAL,
  PET_SHAMAN,

  PET_ENEMY,

  PET_MAX
};

enum dmg_type_e { DMG_DIRECT=0, DMG_OVER_TIME=1, HEAL_DIRECT, HEAL_OVER_TIME, ABSORB };

enum stats_type_e { STATS_DMG, STATS_HEAL, STATS_ABSORB };

enum dot_behavior_type_e { DOT_CLIP, DOT_REFRESH, DOT_EXTEND };

enum attribute_type_e { ATTRIBUTE_NONE=0, ATTR_STRENGTH, ATTR_AGILITY, ATTR_STAMINA, ATTR_INTELLECT, ATTR_SPIRIT, ATTRIBUTE_MAX };

enum base_stat_type_e_e { BASE_STAT_STRENGTH=0, BASE_STAT_AGILITY, BASE_STAT_STAMINA, BASE_STAT_INTELLECT, BASE_STAT_SPIRIT,
                          BASE_STAT_HEALTH, BASE_STAT_MANA,
                          BASE_STAT_MELEE_CRIT_PER_AGI, BASE_STAT_SPELL_CRIT_PER_INT,
                          BASE_STAT_DODGE_PER_AGI,
                          BASE_STAT_MELEE_CRIT, BASE_STAT_SPELL_CRIT, BASE_STAT_MP5, BASE_STAT_SPI_REGEN, BASE_STAT_MAX
                        };

enum resource_type_e
{
  RESOURCE_NONE = 0,
  RESOURCE_HEALTH,
  RESOURCE_MANA,
  RESOURCE_RAGE,
  RESOURCE_FOCUS,
  RESOURCE_ENERGY,
  RESOURCE_MONK_ENERGY,
  RESOURCE_RUNIC_POWER,
  RESOURCE_SOUL_SHARD,
  RESOURCE_ECLIPSE,
  RESOURCE_HOLY_POWER,
  /* Unknown_2, */
  /* Unknown_3 */
  RESOURCE_CHI,
  RESOURCE_SHADOW_ORB,
  RESOURCE_BURNING_EMBER,
  RESOURCE_DEMONIC_FURY,
  /* Dummy resources for reporting */
  RESOURCE_RUNE,
  RESOURCE_RUNE_BLOOD,
  RESOURCE_RUNE_UNHOLY,
  RESOURCE_RUNE_FROST,
  RESOURCE_MAX
};

enum result_type_e
{
  RESULT_UNKNOWN=-1,
  RESULT_NONE=0,
  RESULT_MISS,  RESULT_DODGE, RESULT_PARRY,
  RESULT_BLOCK, RESULT_CRIT_BLOCK, RESULT_GLANCE, RESULT_CRIT, RESULT_HIT,
  RESULT_MAX
};

#define RESULT_HIT_MASK   ( (1<<RESULT_GLANCE) | (1<<RESULT_BLOCK) | (1<<RESULT_CRIT_BLOCK) | (1<<RESULT_CRIT) | (1<<RESULT_HIT) )
#define RESULT_CRIT_MASK  ( (1<<RESULT_CRIT) )
#define RESULT_MISS_MASK  ( (1<<RESULT_MISS) )
#define RESULT_BLOCK_MASK ( (1<<RESULT_BLOCK) )
#define RESULT_DODGE_MASK ( (1<<RESULT_DODGE) )
#define RESULT_PARRY_MASK ( (1<<RESULT_PARRY) )
#define RESULT_NONE_MASK  ( (1<<RESULT_NONE) )
#define RESULT_ALL_MASK  -1

enum proc_type_e
{
  PROC_NONE=0,
  PROC_DAMAGE,
  PROC_HEAL,
  PROC_TICK_DAMAGE,
  PROC_DIRECT_DAMAGE,
  PROC_DIRECT_HEAL,
  PROC_TICK_HEAL,
  PROC_ATTACK,
  PROC_SPELL,
  PROC_TICK,
  PROC_SPELL_AND_TICK,
  PROC_HEAL_SPELL,
  PROC_HARMFUL_SPELL,
  PROC_DAMAGE_HEAL,
  PROC_MAX
};

enum action_type_e { ACTION_USE=0, ACTION_SPELL, ACTION_ATTACK, ACTION_HEAL, ACTION_ABSORB, ACTION_SEQUENCE, ACTION_OTHER, ACTION_MAX };

enum school_type_e
{
  SCHOOL_NONE=0,
  SCHOOL_ARCANE,      SCHOOL_FIRE,        SCHOOL_FROST,       SCHOOL_HOLY,        SCHOOL_NATURE,
  SCHOOL_SHADOW,      SCHOOL_PHYSICAL,    SCHOOL_MAX_PRIMARY, SCHOOL_FROSTFIRE,
  SCHOOL_HOLYSTRIKE,  SCHOOL_FLAMESTRIKE, SCHOOL_HOLYFIRE,    SCHOOL_STORMSTRIKE, SCHOOL_HOLYSTORM,
  SCHOOL_FIRESTORM,   SCHOOL_FROSTSTRIKE, SCHOOL_HOLYFROST,   SCHOOL_FROSTSTORM,  SCHOOL_SHADOWSTRIKE,
  SCHOOL_SHADOWLIGHT, SCHOOL_SHADOWFLAME, SCHOOL_SHADOWSTORM, SCHOOL_SHADOWFROST, SCHOOL_SPELLSTRIKE,
  SCHOOL_DIVINE,      SCHOOL_SPELLFIRE,   SCHOOL_SPELLSTORM,  SCHOOL_SPELLFROST,  SCHOOL_SPELLSHADOW,
  SCHOOL_ELEMENTAL,   SCHOOL_CHROMATIC,   SCHOOL_MAGIC,       SCHOOL_CHAOS,       SCHOOL_BLEED,
  SCHOOL_DRAIN,
  SCHOOL_MAX
};

const int64_t SCHOOL_ATTACK_MASK = ( ( int64_t( 1 ) << SCHOOL_BLEED )        | ( int64_t( 1 ) << SCHOOL_PHYSICAL )     |
                                     ( int64_t( 1 ) << SCHOOL_HOLYSTRIKE )   | ( int64_t( 1 ) << SCHOOL_FLAMESTRIKE )  |
                                     ( int64_t( 1 ) << SCHOOL_STORMSTRIKE )  | ( int64_t( 1 ) << SCHOOL_FROSTSTRIKE )  |
                                     ( int64_t( 1 ) << SCHOOL_SHADOWSTRIKE ) | ( int64_t( 1 ) << SCHOOL_SPELLSTRIKE )  );
                                      // SCHOOL_CHAOS should probably be added here too.

const int64_t SCHOOL_SPELL_MASK  ( ( int64_t( 1 ) << SCHOOL_ARCANE )         | ( int64_t( 1 ) << SCHOOL_CHAOS )        |
                                   ( int64_t( 1 ) << SCHOOL_FIRE )           | ( int64_t( 1 ) << SCHOOL_FROST )        |
                                   ( int64_t( 1 ) << SCHOOL_FROSTFIRE )      | ( int64_t( 1 ) << SCHOOL_HOLY )         |
                                   ( int64_t( 1 ) << SCHOOL_NATURE )         | ( int64_t( 1 ) << SCHOOL_SHADOW )       |
                                   ( int64_t( 1 ) << SCHOOL_HOLYSTRIKE )     | ( int64_t( 1 ) << SCHOOL_FLAMESTRIKE )  |
                                   ( int64_t( 1 ) << SCHOOL_HOLYFIRE )       | ( int64_t( 1 ) << SCHOOL_STORMSTRIKE )  |
                                   ( int64_t( 1 ) << SCHOOL_HOLYSTORM )      | ( int64_t( 1 ) << SCHOOL_FIRESTORM )    |
                                   ( int64_t( 1 ) << SCHOOL_FROSTSTRIKE )    | ( int64_t( 1 ) << SCHOOL_HOLYFROST )    |
                                   ( int64_t( 1 ) << SCHOOL_FROSTSTORM )     | ( int64_t( 1 ) << SCHOOL_SHADOWSTRIKE ) |
                                   ( int64_t( 1 ) << SCHOOL_SHADOWLIGHT )    | ( int64_t( 1 ) << SCHOOL_SHADOWFLAME )  |
                                   ( int64_t( 1 ) << SCHOOL_SHADOWSTORM )    | ( int64_t( 1 ) << SCHOOL_SHADOWFROST )  |
                                   ( int64_t( 1 ) << SCHOOL_SPELLSTRIKE )    | ( int64_t( 1 ) << SCHOOL_DIVINE )       |
                                   ( int64_t( 1 ) << SCHOOL_SPELLFIRE )      | ( int64_t( 1 ) << SCHOOL_SPELLSTORM )   |
                                   ( int64_t( 1 ) << SCHOOL_SPELLFROST )     | ( int64_t( 1 ) << SCHOOL_SPELLSHADOW )  |
                                   ( int64_t( 1 ) << SCHOOL_ELEMENTAL )      | ( int64_t( 1 ) << SCHOOL_CHROMATIC )    |
                                   ( int64_t( 1 ) << SCHOOL_MAGIC ) );

const int64_t SCHOOL_MAGIC_MASK  ( ( int64_t( 1 ) << SCHOOL_ARCANE )         |
                                   ( int64_t( 1 ) << SCHOOL_FIRE )           | ( int64_t( 1 ) << SCHOOL_FROST )        |
                                   ( int64_t( 1 ) << SCHOOL_FROSTFIRE )      | ( int64_t( 1 ) << SCHOOL_HOLY )         |
                                   ( int64_t( 1 ) << SCHOOL_NATURE )         | ( int64_t( 1 ) << SCHOOL_SHADOW ) );
#define SCHOOL_ALL_MASK    ( int64_t( -1 ) )

#include "dbc/specialization.hpp"

enum weapon_type_e
{
  WEAPON_NONE=0,
  WEAPON_DAGGER,                                                                                   WEAPON_SMALL,
  WEAPON_BEAST,    WEAPON_SWORD,    WEAPON_MACE,     WEAPON_AXE,    WEAPON_FIST,                   WEAPON_1H,
  WEAPON_BEAST_2H, WEAPON_SWORD_2H, WEAPON_MACE_2H,  WEAPON_AXE_2H, WEAPON_STAFF,  WEAPON_POLEARM, WEAPON_2H,
  WEAPON_BOW,      WEAPON_CROSSBOW, WEAPON_GUN,      WEAPON_WAND,   WEAPON_THROWN,                 WEAPON_RANGED,
  WEAPON_MAX
};

enum glyph_type_e
{
  GLYPH_MAJOR=0,
  GLYPH_MINOR,
  GLYPH_PRIME,
  GLYPH_MAX
};

enum slot_type_e   // these enum values match armory settings
{
  SLOT_INVALID   = -1,
  SLOT_HEAD      = 0,
  SLOT_NECK      = 1,
  SLOT_SHOULDERS = 2,
  SLOT_SHIRT     = 3,
  SLOT_CHEST     = 4,
  SLOT_WAIST     = 5,
  SLOT_LEGS      = 6,
  SLOT_FEET      = 7,
  SLOT_WRISTS    = 8,
  SLOT_HANDS     = 9,
  SLOT_FINGER_1  = 10,
  SLOT_FINGER_2  = 11,
  SLOT_TRINKET_1 = 12,
  SLOT_TRINKET_2 = 13,
  SLOT_BACK      = 14,
  SLOT_MAIN_HAND = 15,
  SLOT_OFF_HAND  = 16,
  SLOT_RANGED    = 17,
  SLOT_TABARD    = 18,
  SLOT_MAX       = 19,
  SLOT_MIN       = 0
};

// Tiers 13..16 + PVP
#define N_TIER 5
#define MIN_TIER ( 13 )

// Caster 2/4, Melee 2/4, Tank 2/4, Heal 2/4
#define N_TIER_BONUS 8

typedef uint32_t set_bonus_description_t[N_TIER][N_TIER_BONUS];

enum set_type_e
{
  SET_NONE = 0,
  SET_T13_CASTER, SET_T13_2PC_CASTER, SET_T13_4PC_CASTER,
  SET_T13_MELEE,  SET_T13_2PC_MELEE,  SET_T13_4PC_MELEE,
  SET_T13_TANK,   SET_T13_2PC_TANK,   SET_T13_4PC_TANK,
  SET_T13_HEAL,   SET_T13_2PC_HEAL,   SET_T13_4PC_HEAL,
  SET_T14_CASTER, SET_T14_2PC_CASTER, SET_T14_4PC_CASTER,
  SET_T14_MELEE,  SET_T14_2PC_MELEE,  SET_T14_4PC_MELEE,
  SET_T14_TANK,   SET_T14_2PC_TANK,   SET_T14_4PC_TANK,
  SET_T14_HEAL,   SET_T14_2PC_HEAL,   SET_T14_4PC_HEAL,
  SET_T15_CASTER, SET_T15_2PC_CASTER, SET_T15_4PC_CASTER,
  SET_T15_MELEE,  SET_T15_2PC_MELEE,  SET_T15_4PC_MELEE,
  SET_T15_TANK,   SET_T15_2PC_TANK,   SET_T15_4PC_TANK,
  SET_T15_HEAL,   SET_T15_2PC_HEAL,   SET_T15_4PC_HEAL,
  SET_T16_CASTER, SET_T16_2PC_CASTER, SET_T16_4PC_CASTER,
  SET_T16_MELEE,  SET_T16_2PC_MELEE,  SET_T16_4PC_MELEE,
  SET_T16_TANK,   SET_T16_2PC_TANK,   SET_T16_4PC_TANK,
  SET_T16_HEAL,   SET_T16_2PC_HEAL,   SET_T16_4PC_HEAL,
  SET_PVP_CASTER, SET_PVP_2PC_CASTER, SET_PVP_4PC_CASTER,
  SET_PVP_MELEE,  SET_PVP_2PC_MELEE,  SET_PVP_4PC_MELEE,
  SET_PVP_TANK,   SET_PVP_2PC_TANK,   SET_PVP_4PC_TANK,
  SET_PVP_HEAL,   SET_PVP_2PC_HEAL,   SET_PVP_4PC_HEAL,
  SET_MAX
};
//static_assert( static_cast<int>( SET_MAX ) == ( 1 + N_TIER * 3 * N_TIER_BONUS / 2 ), "enum set_type_e must be structured correctly." );

enum gem_type_e
{
  GEM_NONE=0,
  GEM_META, GEM_PRISMATIC,
  GEM_RED, GEM_YELLOW, GEM_BLUE,
  GEM_ORANGE, GEM_GREEN, GEM_PURPLE,
  GEM_COGWHEEL,
  GEM_MAX
};

enum meta_gem_type_e
{
  META_GEM_NONE=0,
  META_AGILE_SHADOWSPIRIT,
  META_AUSTERE_EARTHSIEGE,
  META_AUSTERE_SHADOWSPIRIT,
  META_BEAMING_EARTHSIEGE,
  META_BRACING_EARTHSIEGE,
  META_BRACING_EARTHSTORM,
  META_BRACING_SHADOWSPIRIT,
  META_BURNING_SHADOWSPIRIT,
  META_CHAOTIC_SHADOWSPIRIT,
  META_CHAOTIC_SKYFIRE,
  META_CHAOTIC_SKYFLARE,
  META_DESTRUCTIVE_SHADOWSPIRIT,
  META_DESTRUCTIVE_SKYFIRE,
  META_DESTRUCTIVE_SKYFLARE,
  META_EFFULGENT_SHADOWSPIRIT,
  META_EMBER_SHADOWSPIRIT,
  META_EMBER_SKYFIRE,
  META_EMBER_SKYFLARE,
  META_ENIGMATIC_SHADOWSPIRIT,
  META_ENIGMATIC_SKYFLARE,
  META_ENIGMATIC_STARFLARE,
  META_ENIGMATIC_SKYFIRE,
  META_ETERNAL_EARTHSIEGE,
  META_ETERNAL_EARTHSTORM,
  META_ETERNAL_SHADOWSPIRIT,
  META_FLEET_SHADOWSPIRIT,
  META_FORLORN_SHADOWSPIRIT,
  META_FORLORN_SKYFLARE,
  META_FORLORN_STARFLARE,
  META_IMPASSIVE_SHADOWSPIRIT,
  META_IMPASSIVE_SKYFLARE,
  META_IMPASSIVE_STARFLARE,
  META_INSIGHTFUL_EARTHSIEGE,
  META_INSIGHTFUL_EARTHSTORM,
  META_INVIGORATING_EARTHSIEGE,
  META_MYSTICAL_SKYFIRE,
  META_PERSISTENT_EARTHSIEGE,
  META_PERSISTENT_EARTHSHATTER,
  META_POWERFUL_EARTHSIEGE,
  META_POWERFUL_EARTHSHATTER,
  META_POWERFUL_EARTHSTORM,
  META_POWERFUL_SHADOWSPIRIT,
  META_RELENTLESS_EARTHSIEGE,
  META_RELENTLESS_EARTHSTORM,
  META_REVERBERATING_SHADOWSPIRIT,
  META_REVITALIZING_SHADOWSPIRIT,
  META_REVITALIZING_SKYFLARE,
  META_SWIFT_SKYFIRE,
  META_SWIFT_SKYFLARE,
  META_SWIFT_STARFIRE,
  META_SWIFT_STARFLARE,
  META_THUNDERING_SKYFIRE,
  META_THUNDERING_SKYFLARE,
  META_TIRELESS_STARFLARE,
  META_TIRELESS_SKYFLARE,
  META_TRENCHANT_EARTHSIEGE,
  META_TRENCHANT_EARTHSHATTER,
  META_GEM_MAX
};

enum stat_type_e
{
  STAT_NONE=0,
  STAT_STRENGTH, STAT_AGILITY, STAT_STAMINA, STAT_INTELLECT, STAT_SPIRIT,
  STAT_HEALTH, STAT_MANA, STAT_RAGE, STAT_ENERGY, STAT_FOCUS, STAT_RUNIC,
  STAT_MAX_HEALTH, STAT_MAX_MANA, STAT_MAX_RAGE, STAT_MAX_ENERGY, STAT_MAX_FOCUS, STAT_MAX_RUNIC,
  STAT_SPELL_POWER, STAT_MP5,
  STAT_ATTACK_POWER, STAT_EXPERTISE_RATING, STAT_EXPERTISE_RATING2,
  STAT_HIT_RATING, STAT_HIT_RATING2,STAT_CRIT_RATING, STAT_HASTE_RATING,STAT_MASTERY_RATING,
  STAT_WEAPON_DPS, STAT_WEAPON_SPEED,
  STAT_WEAPON_OFFHAND_DPS, STAT_WEAPON_OFFHAND_SPEED,
  STAT_ARMOR, STAT_BONUS_ARMOR, STAT_RESILIENCE_RATING, STAT_DODGE_RATING, STAT_PARRY_RATING,
  STAT_BLOCK_RATING,
  STAT_ALL,
  STAT_MAX
};
#define check(x) static_assert( static_cast<int>( STAT_##x ) == static_cast<int>( ATTR_##x ), \
                                "stat_type_e and attribute_type_e must be kept in sync" )
check( STRENGTH );
check( AGILITY );
check( STAMINA );
check( INTELLECT );
check( SPIRIT );
#undef check

inline stat_type_e stat_from_attr( attribute_type_e a )
{
  // Assumes that ATTR_X == STAT_X
  return static_cast<stat_type_e>( a );
}

enum elixir_type_e
{
  ELIXIR_NONE=0,
  ELIXIR_ADEPT,
  ELIXIR_BATTLE,
  ELIXIR_DRAENIC_WISDOM,
  ELIXIR_FEL_STRENGTH,
  ELIXIR_GREATER_ARCANE,
  ELIXIR_GUARDIAN,
  ELIXIR_MAJOR_AGILITY,
  ELIXIR_MAJOR_FIRE_POWER,
  ELIXIR_MAJOR_FROST_POWER,
  ELIXIR_MAJOR_MAGEBLOOD,
  ELIXIR_MAJOR_SHADOW_POWER,
  ELIXIR_MAJOR_STRENGTH,
  ELIXIR_MASTERY,
  ELIXIR_MONGOOSE,
  ELIXIR_ONSLAUGHT,
  ELIXIR_SAGES,
  ELIXIR_MAX
};

enum flask_type_e
{
  FLASK_NONE=0,
  // cataclysm
  FLASK_DRACONIC_MIND,
  FLASK_FLOWING_WATER,
  FLASK_STEELSKIN,
  FLASK_TITANIC_STRENGTH,
  FLASK_WINDS,
  // mop
  FLASK_WARM_SUN,
  FLASK_FALLING_LEAVES,
  FLASK_EARTH,
  FLASK_WINTERS_BITE,
  FLASK_SPRING_BLOSSOMS,
  FLASK_MAX
};

enum food_type_e
{
  FOOD_NONE=0,
  FOOD_BAKED_ROCKFISH,
  FOOD_BASILISK_LIVERDOG,
  FOOD_BEER_BASTED_CROCOLISK,
  FOOD_BLACKBELLY_SUSHI,
  FOOD_CROCOLISK_AU_GRATIN,
  FOOD_DELICIOUS_SAGEFISH_TAIL,
  FOOD_FISH_CAKE,
  FOOD_FISH_FEAST,
  FOOD_FORTUNE_COOKIE,
  FOOD_GINSENG_CHICKEN_SOUP,
  FOOD_GREAT_PANDAREN_BANQUET,
  FOOD_GRILLED_DRAGON,
  FOOD_LAVASCALE_FILLET,
  FOOD_MUSHROOM_SAUCE_MUDFISH,
  FOOD_PANDAREN_BANQUET,
  FOOD_PANDAREN_MEATBALL,
  FOOD_RICE_PUDDING,
  FOOD_SEAFOOD_MAGNIFIQUE_FEAST,
  FOOD_SEVERED_SAGEFISH_HEAD,
  FOOD_SKEWERED_EEL,
  FOOD_MAX
};

enum position_type_e { POSITION_NONE=0, POSITION_FRONT, POSITION_BACK, POSITION_RANGED_FRONT, POSITION_RANGED_BACK, POSITION_MAX };

enum profession_type_e
{
  PROFESSION_NONE=0,
  PROF_ALCHEMY,
  PROF_MINING,
  PROF_HERBALISM,
  PROF_LEATHERWORKING,
  PROF_ENGINEERING,
  PROF_BLACKSMITHING,
  PROF_INSCRIPTION,
  PROF_SKINNING,
  PROF_TAILORING,
  PROF_JEWELCRAFTING,
  PROF_ENCHANTING,
  PROFESSION_MAX
};

enum role_type_e { ROLE_NONE=0, ROLE_ATTACK, ROLE_SPELL, ROLE_HYBRID, ROLE_DPS, ROLE_TANK, ROLE_HEAL, ROLE_MAX };

enum rng_type_e
{
  // Specifies the general class of RNG desired
  RNG_DEFAULT=0,     // Do not care/know where it will be used
  RNG_GLOBAL,        // Returns reference to global RNG on sim_t
  RNG_DETERMINISTIC, // Returns reference to global deterministic RNG on sim_t

  // Specifies a particular RNG desired
  RNG_STANDARD,          // Creates RNG using srand() and rand()
  RNG_MERSENNE_TWISTER,  // Creates RNG using SIMD oriented Fast Mersenne Twister
  RNG_MAX
};

enum save_type_e
{
  // Specifies the type of profile data to be saved
  SAVE_ALL=0,
  SAVE_GEAR,
  SAVE_TALENTS,
  SAVE_ACTIONS,
  SAVE_MAX
};

enum format_type_e
{
  FORMAT_NONE=0,
  FORMAT_CHAR_NAME,
  FORMAT_MAX
};

// Data Access ==============================================================
#ifndef MAX_LEVEL
#define MAX_LEVEL (85)
#endif

#ifndef NUM_SPELL_FLAGS
#define NUM_SPELL_FLAGS (10)
#endif

enum power_type_e
{
  POWER_HEALTH        = -2,
  POWER_MANA          = 0,
  POWER_RAGE          = 1,
  POWER_FOCUS         = 2,
  POWER_ENERGY        = 3,
  POWER_MONK_ENERGY   = 4,
  POWER_RUNE          = 5,
  POWER_RUNIC_POWER   = 6,
  POWER_SOUL_SHARDS   = 7,
  // Not yet used
  POWER_HOLY_POWER    = 9,
  // Not yet used (MoP Monk deprecated resource #1)
  // Not yet used
  POWER_CHI           = 12,
  POWER_SHADOW_ORB    = 13,
  POWER_BURNING_EMBER = 14,
  POWER_DEMONIC_FURY  = 15,
  // Helpers
  POWER_MAX           = 16,
  POWER_NONE          = 0xFFFFFFFF, // None.
  POWER_OFFSET        = 2,
};

enum rating_type_e
{
  RATING_DODGE = 0,
  RATING_PARRY,
  RATING_BLOCK,
  RATING_MELEE_HIT,
  RATING_RANGED_HIT,
  RATING_SPELL_HIT,
  RATING_MELEE_CRIT,
  RATING_RANGED_CRIT,
  RATING_SPELL_CRIT,
  RATING_MELEE_HASTE,
  RATING_RANGED_HASTE,
  RATING_SPELL_HASTE,
  RATING_EXPERTISE,
  RATING_MASTERY,
  RATING_MAX
};

// New stuff
enum snapshot_state_e
{
  STATE_HASTE         = 0x000001,
  STATE_CRIT          = 0x000002,
  STATE_AP            = 0x000004,
  STATE_SP            = 0x000008,

  STATE_MUL_DA        = 0x000010,
  STATE_MUL_TA        = 0x000020,
  STATE_TGT_CRIT      = 0x000100,
  STATE_TGT_MUL_DA    = 0x000200,
  STATE_TGT_MUL_TA    = 0x000400,

  STATE_USER_1        = 0x000800,
  STATE_USER_2        = 0x001000,
  STATE_USER_3        = 0x002000,
  STATE_USER_4        = 0x004000,
};

enum ready_type_e
{ READY_POLL=0, READY_TRIGGER=1 };

// Generic programming tools ================================================

template <typename T, std::size_t N>
inline std::size_t sizeof_array( const T ( & )[N] )
{ return N; }

template <typename T, std::size_t N>
inline std::size_t sizeof_array( const std::array<T,N>& )
{ return N; }

class noncopyable
{
public:
  noncopyable() {} // = default
  // noncopyable( noncopyable&& ) = default;
  // noncopyable& operator = ( noncopyable&& ) = default;
private:
  noncopyable( const noncopyable& ); // = delete
  noncopyable& operator = ( const noncopyable& ); // = delete
};

class nonmoveable : public noncopyable
{
private:
  // nonmoveable( nonmoveable&& ) = delete;
  // nonmoveable& operator = ( nonmoveable&& ) = delete;
};

// Adapted from (read "stolen") boost::checked_deleter
struct delete_disposer_t
{
  template <typename T>
  void operator () ( T* t ) const
  {
    typedef int force_T_to_be_complete[ sizeof( T ) ? 1 : -1 ];
    ( void )sizeof( force_T_to_be_complete );
    delete t;
  }
};

// Generic algorithms =======================================================

// Wrappers for std::fill, std::fill_n, and std::find that perform any type
// conversions for t at the callsite instead of per assignment in the loop body.
// (i.e., fill( T**, T**, 0 ) will deduce T* for the third argument, as opposed
//  to std::fill( T**, T**, 0 ) simply defaulting to int and failing to compile).
template <typename I>
inline void fill( I first, I last, typename std::iterator_traits<I>::value_type const& t )
{ std::fill( first, last, t ); }

template <typename I>
inline void fill_n( I first, typename std::iterator_traits<I>::difference_type n,
                    typename std::iterator_traits<I>::value_type const& t )
{ std::fill_n( first, n, t ); }

template <typename I>
inline I find( I first, I last, typename std::iterator_traits<I>::value_type const& t )
{ return std::find( first, last, t ); }

template <typename I, typename D>
void dispose( I first, I last, D disposer )
{
  while ( first != last )
    disposer( *first++ );
}

template <typename I>
inline void dispose( I first, I last )
{ dispose( first, last, delete_disposer_t() ); }

// Machinery for range-based generic algorithms =============================

namespace range { // ========================================================
template <typename T>
struct traits
{
  typedef typename iterator_type<T>::type iterator;
  static iterator begin( T& t ) { return t.begin(); }
  static iterator end( T& t ) { return t.end(); }
};

template <typename T, size_t N>
struct traits<T[N]>
{
  typedef T* iterator;
  static iterator begin( T ( &t )[N] ) { return &t[0]; }
  static iterator end( T ( &t )[N] ) { return begin( t ) + N; }
};

template <typename T>
struct traits< std::pair<T,T> >
{
  typedef T iterator;
  static iterator begin( const std::pair<T,T>& t ) { return t.first; }
  static iterator end( const std::pair<T,T>& t ) { return t.second; }
};

template <typename T>
struct traits< const std::pair<T,T> >
{
  typedef T iterator;
  static iterator begin( const std::pair<T,T>& t ) { return t.first; }
  static iterator end( const std::pair<T,T>& t ) { return t.second; }
};

template <typename T>
struct value_type
{
  typedef typename std::iterator_traits<typename traits<T>::iterator>::value_type type;
};

template <typename T>
inline typename traits<T>::iterator begin( T& t )
{ return traits<T>::begin( t ); }

template <typename T>
inline typename traits<const T>::iterator cbegin( const T& t )
{ return range::begin( t ); }

template <typename T>
inline typename traits<T>::iterator end( T& t )
{ return traits<T>::end( t ); }

template <typename T>
inline typename traits<const T>::iterator cend( const T& t )
{ return range::end( t ); }

// Range-based generic algorithms ===========================================

#if _MSC_VER && _MSC_VER < 1600
#pragma warning( disable : 4996 )
#endif
template <typename Range, typename Out>
inline Out copy( const Range& r, Out o )
{ return std::copy( range::begin( r ), range::end( r ), o ); }
#if _MSC_VER && _MSC_VER < 1600
#pragma warning( default : 4996 )
#endif

template <typename Range, typename D>
inline Range& dispose( Range& r, D disposer )
{ dispose( range::begin( r ), range::end( r ), disposer ); return r; }

template <typename Range>
inline Range& dispose( Range& r )
{ return dispose( r, delete_disposer_t() ); }

template <typename Range>
inline Range& fill( Range& r, typename range::value_type<Range>::type const& t )
{ std::fill( range::begin( r ), range::end( r ), t ); return r; }

template <typename Range>
inline typename range::traits<Range>::iterator
find( Range& r, typename range::value_type<Range>::type const& t )
{ return std::find( range::begin( r ), range::end( r ), t ); }

template <typename Range, typename F>
inline F for_each( Range& r, F f )
{ return std::for_each( range::begin( r ), range::end( r ), f ); }

template <typename Range, typename Out, typename Predicate>
inline Out remove_copy_if( Range& r, Out o, Predicate p )
{ return std::remove_copy_if( range::begin( r ), range::end( r ), o, p ); }

template <typename Range, typename Out, typename F>
inline Out transform( Range& r, Out o, F f )
{ return std::transform( range::begin( r ), range::end( r ), o, f ); }

template <typename Range, typename F>
inline typename range::traits<Range>::iterator transform_self( Range& r, F f )
{ return std::transform( range::begin( r ), range::end( r ), range::begin( r ), f ); }

template <typename Range1, typename Range2, typename Out>
inline Out set_difference( const Range1& left, const Range2& right, Out o )
{
  return std::set_difference( range::begin( left ), range::end( left ),
                              range::begin( right ), range::end( right ), o );
}

template <typename Range1, typename Range2, typename Out, typename Compare>
inline Out set_difference( const Range1& left, const Range2& right, Out o, Compare c )
{
  return std::set_difference( range::begin( left ), range::end( left ),
                              range::begin( right ), range::end( right ), o, c );
}

template <typename Range1, typename Range2, typename Out>
inline Out set_intersection( const Range1& left, const Range2& right, Out o )
{
  return std::set_intersection( range::begin( left ), range::end( left ),
                                range::begin( right ), range::end( right ), o );
}

template <typename Range1, typename Range2, typename Out, typename Compare>
inline Out set_intersection( const Range1& left, const Range2& right, Out o, Compare c )
{
  return std::set_intersection( range::begin( left ), range::end( left ),
                                range::begin( right ), range::end( right ), o, c );
}

template <typename Range1, typename Range2, typename Out>
inline Out set_union( const Range1& left, const Range2& right, Out o )
{
  return std::set_union( range::begin( left ), range::end( left ),
                         range::begin( right ), range::end( right ), o );
}

template <typename Range1, typename Range2, typename Out, typename Compare>
inline Out set_union( const Range1& left, const Range2& right, Out o, Compare c )
{
  return std::set_union( range::begin( left ), range::end( left ),
                         range::begin( right ), range::end( right ), o, c );
}

template <typename Range>
inline Range& sort( Range& r )
{ std::sort( range::begin( r ), range::end( r ) ); return r; }

template <typename Range, typename Comp>
inline Range& sort( Range& r, Comp c )
{ std::sort( range::begin( r ), range::end( r ), c ); return r; }

template <typename Range>
inline typename range::traits<Range>::iterator unique( Range& r )
{ return std::unique( range::begin( r ), range::end( r ) ); }

template <typename Range, typename Comp>
inline typename range::traits<Range>::iterator unique( Range& r, Comp c )
{ return std::unique( range::begin( r ), range::end( r ), c ); }

template <typename Range>
inline bool is_valid_utf8( const Range& r )
{ return utf8::is_valid( range::begin( r ), range::end( r ) ); }

template <typename Range>
inline typename range::traits<Range>::iterator max_element( Range& r )
{ return std::max_element( range::begin( r ), range::end( r ) ); }

} // namespace range ========================================================

// Adapter for container of owned pointers; automatically deletes the
// pointers on destruction.
template <typename Container>
class auto_dispose : public Container
{
private:
  void dispose_() { range::dispose( *this ); }
public:
  ~auto_dispose() { dispose_(); }
  void dispose()  { dispose_(); Container::clear(); }
};

template <typename To, typename From>
inline To debug_cast( From* ptr )
{
#ifdef NDEBUG
  return static_cast<To>( ptr );
#else
  To result = dynamic_cast<To>( ptr );
  assert( result );
  return result;
#endif
}

// timespan_t ===============================================================

// if timespan_t is in the global namespace, there's a name lookup issue with
// one of the Qt headers. Problem is avoided by defining in a sub-namespace
// and then lifting into the global namespace with a using declaration.
namespace timespan_adl_barrier { // =========================================

class timespan_t
{
#ifdef SC_USE_INTEGER_TIME
  // CAREFUL: Using int32_t implies that no overflowing happens during calculation.
  typedef int64_t time_t;

  static time_t native_to_milli ( time_t t ) { return t; }
  static double native_to_second( time_t t ) { return static_cast<double>( t ) * ( 1.0 / 1000 ); }
  static double native_to_minute( time_t t ) { return static_cast<double>( t ) * ( 1.0 / ( 60 * 1000 ) ); }

  template <typename Rep>
  static time_t milli_to_native ( Rep t ) { return static_cast<time_t>( t ); }
  template <typename Rep>
  static time_t second_to_native( Rep t ) { return static_cast<time_t>( t * 1000 ); }
  template <typename Rep>
  static time_t minute_to_native( Rep t ) { return static_cast<time_t>( t * ( 60 * 1000 ) ); }

#else // !SC_USE_INTEGER_TIME

  typedef double time_t;

  static time_t native_to_milli ( time_t t ) { return t * 1000.0; }
  static double native_to_second( time_t t ) { return t; }
  static double native_to_minute( time_t t ) { return t * ( 1.0 / 60 ); }

  template <typename Rep>
  static time_t milli_to_native ( Rep t ) { return static_cast<time_t>( t / 1000.0 ); }
  template <typename Rep>
  static time_t second_to_native( Rep t ) { return static_cast<time_t>( t ); }
  template <typename Rep>
  static time_t minute_to_native( Rep t ) { return static_cast<time_t>( t * 60 ); }
#endif

  time_t time;

  template <typename Rep>
  explicit timespan_t( Rep t ) : time( static_cast<time_t>( t ) ) {}

public:
  timespan_t() : time( 0 ) {}

  double total_minutes() const { return native_to_minute( time ); }
  double total_seconds() const { return native_to_second( time ); }
  time_t total_millis() const  { return native_to_milli ( time ); }

  template <typename Rep>
  static typename enable_if<std::is_arithmetic<Rep>::value,timespan_t>::type
  from_millis ( Rep millis )  { return timespan_t( milli_to_native ( millis ) ); }

  template <typename Rep>
  static typename enable_if<std::is_arithmetic<Rep>::value,timespan_t>::type
  from_seconds( Rep seconds ) { return timespan_t( second_to_native( seconds ) ); }

  template <typename Rep>
  static typename enable_if<std::is_arithmetic<Rep>::value,timespan_t>::type
  from_minutes( Rep minutes ) { return timespan_t( minute_to_native( minutes ) ); }

  bool operator==( timespan_t right ) const { return time == right.time; }
  bool operator!=( timespan_t right ) const { return time != right.time; }

  bool operator> ( timespan_t right ) const { return time >  right.time; }
  bool operator>=( timespan_t right ) const { return time >= right.time; }
  bool operator< ( timespan_t right ) const { return time <  right.time; }
  bool operator<=( timespan_t right ) const { return time <= right.time; }

  timespan_t& operator+=( timespan_t right )
  {
    time += right.time;
    return *this;
  }
  timespan_t& operator-=( timespan_t right )
  {
    time -= right.time;
    return *this;
  }

  template <typename Rep>
  typename enable_if<std::is_arithmetic<Rep>::value,timespan_t&>::type
  operator*=( Rep right )
  {
    time = static_cast<time_t>( time * right );
    return *this;
  }

  template <typename Rep>
  typename enable_if<std::is_arithmetic<Rep>::value,timespan_t&>::type
  operator/=( Rep right )
  {
    time = static_cast<time_t>( time / right );
    return *this;
  }

  friend timespan_t operator+( timespan_t right )
  { return right; }
  friend timespan_t operator-( timespan_t right )
  { return timespan_t( -right.time ); }

  friend timespan_t operator+( timespan_t left, timespan_t right )
  { return timespan_t( left.time + right.time ); }

  friend timespan_t operator-( timespan_t left, timespan_t right )
  { return timespan_t( left.time - right.time ); }

  template <typename Rep>
  friend typename enable_if<std::is_arithmetic<Rep>::value,timespan_t>::type
  operator*( timespan_t left, Rep right )
  { return timespan_t( left.time * right ); }

  template <typename Rep>
  friend typename enable_if<std::is_arithmetic<Rep>::value,timespan_t>::type
  operator*( Rep left, timespan_t right )
  { return timespan_t( left * right.time ); }

  template <typename Rep>
  friend typename enable_if<std::is_arithmetic<Rep>::value,timespan_t>::type
  operator/( timespan_t left, Rep right )
  { return timespan_t( left.time / right ); }

  friend double operator/( timespan_t left, timespan_t right )
  { return static_cast<double>( left.time ) / right.time; }

  typedef time_t native_t;
  static native_t to_native( timespan_t t ) { return t.time; }
  template <typename Rep>
  static timespan_t from_native( Rep t ) { return timespan_t( t ); }

  static timespan_t zero() { return timespan_t(); }
  static timespan_t max() { return timespan_t( std::numeric_limits<time_t>::max() ); }
  static timespan_t min()
  {
    if ( std::is_floating_point<time_t>::value )
      return timespan_t( -std::numeric_limits<time_t>::max() );
    else
      return timespan_t( std::numeric_limits<time_t>::min() );
  }
};

} // namespace timespan_adl_barrier =========================================

using timespan_adl_barrier::timespan_t;

// Cache Control ============================================================

namespace cache {

typedef int era_t;
static const era_t INVALID_ERA = -1;
static const era_t IN_THE_BEGINNING = 0;  // A time before any other possible era;
                                          // used to mark persistent caches at load.

enum behavior_e
{
  ANY,      // * Use any version present in the cache, retrieve if not present.
  CURRENT,  // * Use only current info from the cache; validate old versions as needed.
  ONLY,     // * Use any version present in the cache, fail if not present.
};

class cache_control_t
{
private:
  era_t current_era;
  behavior_e player_cache_behavior;
  behavior_e item_cache_behavior;

public:
  cache_control_t() :
    current_era( IN_THE_BEGINNING ),
    player_cache_behavior( CURRENT ),
    item_cache_behavior( ANY )
  {}

  era_t era() const { return current_era; }
  void advance_era() { ++current_era; }

  behavior_e cache_players() const { return player_cache_behavior; }
  void cache_players( behavior_e b ) { player_cache_behavior = b; }

  behavior_e cache_items() const { return item_cache_behavior; }
  void cache_items( behavior_e b ) { item_cache_behavior = b; }

  static cache_control_t singleton;
};

// Caching system's global notion of the current time.
inline era_t era()
{ return cache_control_t::singleton.era(); }

// Time marches on.
inline void advance_era()
{ cache_control_t::singleton.advance_era(); }

// Get/Set default cache behaviors.
inline behavior_e players()
{ return cache_control_t::singleton.cache_players(); }
inline void players( behavior_e b )
{ cache_control_t::singleton.cache_players( b ); }

inline behavior_e items()
{ return cache_control_t::singleton.cache_items(); }
inline void items( behavior_e b )
{ cache_control_t::singleton.cache_items( b ); }

}

struct stat_data_t
{
  double strength;
  double agility;
  double stamina;
  double intellect;
  double spirit;
  double base_health;
  double base_resource;
};

// Include DBC Module
#include "dbc/dbc.hpp"

// Options ==================================================================

enum option_type_e
{
  OPT_NONE=0,
  OPT_STRING,     // std::string*
  OPT_APPEND,     // std::string* (append)
  OPT_BOOL,       // int* (only valid values are 1 and 0)
  OPT_INT,        // int*
  OPT_FLT,        // double*
  OPT_TIMESPAN,   // time interval
  OPT_LIST,       // std::vector<std::string>*
  OPT_MAP,        // std::map<std::string,std::string>*
  OPT_FUNC,       // function pointer
  OPT_TALENT_RANK,   // talent rank
  OPT_SPELL_ENABLED, // spell enabled
  OPT_DEPRECATED,
  OPT_UNKNOWN
};

typedef bool ( *option_function_t )( sim_t* sim, const std::string& name, const std::string& value );

struct option_t
{
  const char* name;
  option_type_e type;
  void* address;

  void print( FILE* );
  void save ( FILE* );
  bool parse( sim_t*, const std::string& name, const std::string& value );

  static void add( std::vector<option_t>&, const char* name, option_type_e type, void* address );
  static void copy( std::vector<option_t>& opt_vector, const option_t* opt_array );
  static bool parse( sim_t*, std::vector<option_t>&, const std::string& name, const std::string& value );
  static bool parse( sim_t*, const char* context, std::vector<option_t>&, const std::string& options_str );
  static bool parse( sim_t*, const char* context, std::vector<option_t>&, const std::vector<std::string>& strings );
  static bool parse( sim_t*, const char* context, const option_t*,        const std::vector<std::string>& strings );
  static bool parse( sim_t*, const char* context, const option_t*,        const std::string& options_str );
  static bool parse_file( sim_t*, FILE* file );
  static bool parse_line( sim_t*, const char* line );
  static bool parse_token( sim_t*, const std::string& token );
  static option_t* merge( std::vector<option_t>& out, const option_t* in1, const option_t* in2 );
};

// Talent Translation =======================================================

#ifndef MAX_TALENT_ROWS
#define MAX_TALENT_ROWS ( 6 )
#endif

#ifndef MAX_TALENT_COLS
#define MAX_TALENT_COLS ( 3 )
#endif

#ifndef MAX_TALENT_SLOTS
#define MAX_TALENT_SLOTS ( MAX_TALENT_ROWS * MAX_TALENT_COLS )
#endif


// Utilities ================================================================

#ifdef _MSC_VER
// C99-compliant snprintf - MSVC _snprintf is NOT the same.

#undef vsnprintf
int vsnprintf_simc( char* buf, size_t size, const char* fmt, va_list ap );
#define vsnprintf vsnprintf_simc

#undef snprintf
inline int snprintf( char* buf, size_t size, const char* fmt, ... )
{
  va_list ap;
  va_start( ap, fmt );
  int rval = vsnprintf( buf, size, fmt, ap );
  va_end( ap );
  return rval;
}
#endif

namespace util {

  double ability_rank( int player_level, double ability_value, int ability_level, ... );
  int    ability_rank( int player_level, int    ability_value, int ability_level, ... );

  const char* attribute_type_string     ( attribute_type_e type );
  const char* dmg_type_string           ( dmg_type_e type );
  const char* dot_behavior_type_string  ( dot_behavior_type_e t );
  const char* elixir_type_string        ( elixir_type_e type );
  const char* flask_type_string         ( flask_type_e type );
  const char* food_type_string          ( food_type_e type );
  const char* gem_type_string           ( gem_type_e type );
  const char* meta_gem_type_string      ( meta_gem_type_e type );
  const char* player_type_string        ( player_type_e );
  const char* pet_type_string           ( pet_type_e type );
  const char* position_type_string      ( position_type_e );
  const char* profession_type_string    ( profession_type_e );
  const char* race_type_string          ( race_type_e );
  const char* role_type_string          ( role_type_e );
  const char* resource_type_string      ( resource_type_e );
  const char* result_type_string        ( result_type_e type );
  uint32_t    school_type_component     ( school_type_e s_type, school_type_e c_type );
  const char* school_type_string        ( school_type_e type );
  const char* armor_type_string         ( player_type_e ptype, slot_type_e );
  const char* set_bonus_string          ( set_type_e type );

  const char* slot_type_string          ( slot_type_e type );
  const char* stat_type_string          ( stat_type_e type );
  const char* stat_type_abbrev          ( stat_type_e type );
  const char* stat_type_wowhead         ( stat_type_e type );
  specialization_e translate_spec_str   ( player_type_e ptype, const std::string& spec_str );
  std::string specialization_string     ( specialization_e spec );
  resource_type_e translate_power_type  ( power_type_e );
  const char* weapon_type_string        ( weapon_type_e type );
  const char* weapon_class_string       ( inventory_type class_ );
  const char* weapon_subclass_string    ( item_subclass_weapon subclass );

  const char* set_item_type_string      ( int item_set );
  const char* item_quality_string       ( int item_quality );

  attribute_type_e parse_attribute_type ( const std::string& name );
  dmg_type_e parse_dmg_type             ( const std::string& name );
  elixir_type_e parse_elixir_type       ( const std::string& name );
  flask_type_e parse_flask_type         ( const std::string& name );
  food_type_e parse_food_type           ( const std::string& name );
  gem_type_e parse_gem_type             ( const std::string& name );
  meta_gem_type_e parse_meta_gem_type   ( const std::string& name );
  player_type_e parse_player_type       ( const std::string& name );
  pet_type_e parse_pet_type             ( const std::string& name );
  profession_type_e parse_profession_type( const std::string& name );
  position_type_e parse_position_type   ( const std::string& name );
  race_type_e parse_race_type           ( const std::string& name );
  role_type_e parse_role_type           ( const std::string& name );
  resource_type_e parse_resource_type   ( const std::string& name );
  result_type_e parse_result_type       ( const std::string& name );
  school_type_e parse_school_type       ( const std::string& name );
  set_type_e parse_set_bonus            ( const std::string& name );
  slot_type_e parse_slot_type           ( const std::string& name );
  stat_type_e parse_stat_type           ( const std::string& name );
  stat_type_e parse_reforge_type        ( const std::string& name );

  weapon_type_e parse_weapon_type       ( const std::string& name );

  int parse_item_quality                ( const std::string& quality );

  bool parse_origin( std::string& region, std::string& server, std::string& name, const std::string& origin );

  int class_id_mask( player_type_e type );
  int class_id( player_type_e type );
  unsigned race_mask( race_type_e type );
  unsigned race_id( race_type_e type );
  unsigned pet_mask( pet_type_e type );
  unsigned pet_id( pet_type_e type );
  player_type_e pet_class_type( pet_type_e type );

  const char* class_id_string( player_type_e type );
  player_type_e translate_class_id( int cid );
  player_type_e translate_class_str( std::string& s );
  race_type_e translate_race_id( int rid );
  stat_type_e translate_item_mod( item_mod_type stat_mod );
  slot_type_e translate_invtype( inventory_type inv_type );
  weapon_type_e translate_weapon_subclass( item_subclass_weapon weapon_subclass );
  profession_type_e translate_profession_id( int skill_id );
  gem_type_e translate_socket_color( item_socket_color );

  bool socket_gem_match( gem_type_e socket, gem_type_e gem );

  size_t string_split( std::vector<std::string>& results, const std::string& str, const char* delim, bool allow_quotes = false );
  int string_split( const std::string& str, const char* delim, const char* format, ... );
  void string_strip_quotes( std::string& str );
  std::string& replace_all( std::string& s, const char* from, char to );
  std::string& replace_all( std::string& s, char from, const char* to );

  template <typename T>
  std::string to_string( const T& t )
  { std::ostringstream s; s << t; return s.str(); }

  std::string to_string( double f );
  std::string to_string( double f, int precision );

  int64_t milliseconds();
  int64_t parse_date( const std::string& month_day_year );

  int printf( const char *format, ... ) PRINTF_ATTRIBUTE( 1,2 );
  int fprintf( FILE *stream, const char *format, ... ) PRINTF_ATTRIBUTE( 2,3 );
  int vfprintf( FILE *stream, const char *format, va_list fmtargs ) PRINTF_ATTRIBUTE( 2,0 );
  int vprintf( const char *format, va_list fmtargs ) PRINTF_ATTRIBUTE( 1,0 );

  std::string& str_to_utf8( std::string& str );
  std::string& str_to_latin1( std::string& str );
  std::string& urlencode( std::string& str );
  std::string& urldecode( std::string& str );

  std::string& format_text( std::string& name, bool input_is_utf8 );

  std::string& html_special_char_decode( std::string& str );

  bool str_compare_ci( const std::string& l, const std::string& r );
  std::string& glyph_name( std::string& n );
  bool str_in_str_ci ( const std::string& l, const std::string& r );
  bool str_prefix_ci ( const std::string& str, const std::string& prefix );

  double floor( double X, unsigned int decplaces = 0 );
  double ceil( double X, unsigned int decplaces = 0 );
  double round( double X, unsigned int decplaces = 0 );

  std::string& tolower( std::string& str );
  std::string encode_html( const std::string& );

  void tokenize( std::string& name, format_type_e = FORMAT_NONE );
  void inverse_tokenize( std::string& name );
  std::string inverse_tokenize( const std::string& name );

  bool is_number( const std::string& s );

  int snprintf( char* buf, size_t size, const char* fmt, ... ) PRINTF_ATTRIBUTE( 3,4 );
  void fuzzy_stats( std::string& encoding, const std::string& description );
};

// Spell information struct, holding static functions to output spell data in a human readable form

struct spell_info_t
{
  static std::string to_str( sim_t* sim, const spell_data_t* spell, int level=MAX_LEVEL );
  //static std::string to_str( sim_t* sim, uint32_t spell_id, int level=MAX_LEVEL );
  static std::string talent_to_str( sim_t* sim, const talent_data_t* talent, int level=MAX_LEVEL );
  static std::ostringstream& effect_to_str( sim_t* sim, const spell_data_t* spell, const spelleffect_data_t* effect, std::ostringstream& s, int level=MAX_LEVEL );
};


// Raid Event

struct raid_event_t
{
protected:
  sim_t* const sim;
  std::string name_str;
  int64_t num_starts;
  timespan_t first, last;
  timespan_t cooldown;
  timespan_t cooldown_stddev;
  timespan_t cooldown_min;
  timespan_t cooldown_max;
  timespan_t duration;
  timespan_t duration_stddev;
  timespan_t duration_min;
  timespan_t duration_max;
  double     distance_min;
  double     distance_max;
  timespan_t saved_duration;
  rng_t* rng;
  std::vector<player_t*> affected_players;

  raid_event_t( sim_t*, const std::string& );
private:
  virtual void _start() = 0;
  virtual void _finish() = 0;
public:
  virtual ~raid_event_t() {}

  timespan_t cooldown_time() const;
  timespan_t duration_time() const;
  void schedule();
  void reset();
  void start();
  void finish();
  void parse_options( option_t*, const std::string& options_str );
  static raid_event_t* create( sim_t* sim, const std::string& name, const std::string& options_str );
  static void init( sim_t* );
  static void reset( sim_t* );
  static void combat_begin( sim_t* );
  static void combat_end( sim_t* ) {}
  const char* name() const { return name_str.c_str(); }
};

// Gear Stats ===============================================================

namespace internal {
struct gear_stats_t
{
  std::array<double,ATTRIBUTE_MAX> attribute;
  std::array<double,RESOURCE_MAX> resource;
  double spell_power;
  double mp5;
  double attack_power;
  double expertise_rating;
  double expertise_rating2;
  double hit_rating;
  double hit_rating2;
  double crit_rating;
  double haste_rating;
  double weapon_dps;
  double weapon_speed;
  double weapon_offhand_dps;
  double weapon_offhand_speed;
  double armor;
  double bonus_armor;
  double dodge_rating;
  double parry_rating;
  double block_rating;
  double mastery_rating;
};
}

struct gear_stats_t : public internal::gear_stats_t
{
  typedef internal::gear_stats_t base_t;
  gear_stats_t() : base_t( base_t() ) {}

  void   add_stat( stat_type_e stat, double value );
  void   set_stat( stat_type_e stat, double value );
  double get_stat( stat_type_e stat ) const;
  void   print( FILE* );
  static double stat_mod( stat_type_e stat );
};

// Statistical Sample Data

struct sample_data_t
{
  // Analyzed Results
  double sum;
  double mean;
  double min;
  double max;
  double variance;
  double std_dev;
  double mean_std_dev;
  std::vector<int> distribution;
  const bool simple;
  const bool min_max;
private:
  std::vector<double> _data;
  int count;

  bool analyzed_basics;
  bool analyzed_variance;
  bool created_dist;
  bool is_sorted;
public:
  sample_data_t( bool s=true, bool mm=false );

  void reserve( std::size_t capacity )
  { if ( ! simple ) _data.reserve( capacity ); }
  void add( double x=0 );

  bool basics_analyzed() const { return analyzed_basics; }
  bool variance_analyzed() const { return analyzed_variance; }
  bool distribution_created() const { return created_dist; }
  bool sorted() const { return is_sorted; }
  int size() const { if ( simple ) return count; return ( int ) _data.size(); }

  void analyze(
    bool calc_basics=true,
    bool calc_variance=true,
    bool s=true,
    unsigned int create_dist=0 );
  void analyze_basics();
  void analyze_variance();
  void sort();
  void create_distribution( unsigned int num_buckets=50 );
  void clear() { count = 0; sum = 0; _data.clear(); distribution.clear(); }

  // Access functions
  double percentile( double ) const;
  const std::vector<double>& data() const { return _data; }
  void merge( const sample_data_t& );

  static double pearson_correlation( const sample_data_t&, const sample_data_t& );
};

// Actor Pair ===============================================================

struct actor_pair_t
{
  player_t* target;
  player_t* source;

  actor_pair_t( player_t* target, player_t* source )
    : target( target ), source( source )
  {}

  actor_pair_t( player_t* p = 0 )
    : target( p ), source( p )
  {}

  virtual ~actor_pair_t() {}
};

// Buffs ====================================================================

struct buff_creator_t
{
private:
  actor_pair_t _player;
  sim_t* const _sim;
  std::string _name;
  const spell_data_t* s_data;
  double _chance;
  double _default_value;
  int _max_stack;
  timespan_t _duration, _cooldown;
  int _quiet, _reverse, _activated;
  friend struct buff_t;
  friend struct debuff_t;
private:
  void init();
public:
  buff_creator_t( actor_pair_t, const std::string& name, const spell_data_t* = spell_data_t::nil() );
  buff_creator_t( actor_pair_t, uint32_t id, const std::string& name );
  buff_creator_t( sim_t*, const std::string& name, const spell_data_t* = spell_data_t::nil() );

  buff_creator_t& duration( timespan_t d )
  { _duration=d; return *this; }
  buff_creator_t& default_value( double v )
  { _default_value=v; return *this; }
  buff_creator_t& chance( double c )
  { _chance=c; return *this; }
  buff_creator_t& max_stack( unsigned ms )
  { _max_stack=ms; return *this; }
  buff_creator_t& cd( timespan_t t )
  { _cooldown=t; return *this; }
  buff_creator_t& reverse( bool r )
  { _reverse=r; return *this; }
  buff_creator_t& quiet( bool q )
  { _quiet=q; return *this; }
  buff_creator_t& activated( bool a )
  { _activated=a; return *this; }
  buff_creator_t& spell( const spell_data_t* s )
  { s_data=s; return *this; }

  operator buff_t* () const;
  operator debuff_t* () const;
};

struct stat_buff_creator_t
{
private:
  buff_creator_t bc;
  double _amount;
  stat_type_e _stat;
  friend struct stat_buff_t;
public:
  stat_buff_creator_t( buff_creator_t a ) :
    bc( a ), _amount( 0 ), _stat( STAT_NONE ) {}

  stat_buff_creator_t& amount( double a )
  { _amount=a; return *this; }
  stat_buff_creator_t& stat( stat_type_e s )
  { _stat=s; return *this; }

  operator stat_buff_t* () const;
};

struct absorb_buff_creator_t
{
private:
  buff_creator_t bc;
  stats_t* _absorb_source;
  friend struct absorb_buff_t;
public:
  absorb_buff_creator_t( buff_creator_t a ) :
    bc( a ),_absorb_source( 0 ) {}

  absorb_buff_creator_t& source( stats_t* s )
  { _absorb_source=s; return *this; }

  operator absorb_buff_t* () const;
};

struct cost_reduction_buff_creator_t
{
private:
  buff_creator_t bc;
  double _amount;
  school_type_e _school;
  bool _refreshes;
  friend struct cost_reduction_buff_t;
public:
  cost_reduction_buff_creator_t( buff_creator_t a ) :
    bc( a ), _amount( 0 ), _school( SCHOOL_NONE ), _refreshes( false )
  {}

  cost_reduction_buff_creator_t& amount( double a )
  { _amount=a; return *this; }
  cost_reduction_buff_creator_t& school( school_type_e s )
  { _school=s; return *this; }
  cost_reduction_buff_creator_t& refreshes( bool r )
  { _refreshes=r; return *this; }

  operator cost_reduction_buff_t* () const;
};

struct buff_t
{
  sim_t* const sim;
  player_t* const player;
  std::string name_str;
  const spell_data_t* s_data;
  // static values
private:
  int _max_stack;
public:
  double default_value;
  bool activated;
  bool reverse, constant, quiet, overridden;
  // dynamic values
  double current_value;
  int current_stack;
  timespan_t buff_duration, buff_cooldown;
  double default_chance;
  timespan_t last_start;
  timespan_t last_trigger;
  timespan_t iteration_uptime_sum;
  int64_t up_count, down_count, start_count, refresh_count;
  int64_t trigger_attempts, trigger_successes;
  double benefit_pct, trigger_pct, avg_start, avg_refresh;
  std::vector<timespan_t> stack_occurrence, stack_react_time;
  std::vector<buff_uptime_t*> stack_uptime;
  player_t* source;
  player_t* initial_source;
  event_t* expiration;
  event_t* delay;
  rng_t* rng;
  cooldown_t* cooldown;
  sample_data_t uptime_pct;
  sample_data_t start_intervals;
  sample_data_t trigger_intervals;

  virtual ~buff_t();

protected:
  buff_t( const buff_creator_t& params );
  friend struct buff_creator_t;
public:

  // Use check() inside of ready() methods to prevent skewing of "benefit" calculations.
  // Use up() where the presence of the buff affects the action mechanics.

  const spell_data_t& data() const { return *s_data; }
  int             check() const { return current_stack; }
  inline bool     up()    { if ( current_stack > 0 ) { up_count++; } else { down_count++; } return current_stack > 0; }
  inline int      stack() { if ( current_stack > 0 ) { up_count++; } else { down_count++; } return current_stack; }
  inline double   value() { if ( current_stack > 0 ) { up_count++; } else { down_count++; } return current_value; }
  timespan_t remains() const;
  bool   remains_gt( timespan_t time ) const;
  bool   remains_lt( timespan_t time ) const;
  bool   trigger  ( action_t*, int stacks=1, double value=-1.0, timespan_t duration = timespan_t::min() );
  virtual bool   trigger  ( int stacks=1, double value=-1.0, double chance=-1.0, timespan_t duration = timespan_t::min() );
  virtual void   execute ( int stacks=1, double value=-1.0, timespan_t duration = timespan_t::min() );
  virtual void   increment( int stacks=1, double value=-1.0, timespan_t duration = timespan_t::min() );
  void   decrement( int stacks=1, double value=-1.0 );
  void   extend_duration( player_t* p, timespan_t seconds );

  virtual void start    ( int stacks=1, double value=-1.0, timespan_t duration = timespan_t::min() );
  virtual void refresh  ( int stacks=0, double value=-1.0, timespan_t duration = timespan_t::min() );
  virtual void bump     ( int stacks=1, double value=-1.0 );
  virtual void override ( int stacks=1, double value=-1.0 );
  virtual bool may_react( int stacks=1 ) const;
  virtual int stack_react() const;
  virtual void expire();
  virtual void predict();
  virtual void reset();
  virtual void aura_gain();
  virtual void aura_loss();
  virtual void merge( const buff_t* other_buff );
  virtual void analyze();
  void init();
  virtual void combat_begin();
  virtual void combat_end();

  expr_t* create_expression( const std::string& type );
  std::string to_str() const;

  static buff_t* find( const std::vector<buff_t*>&, const std::string& name, player_t* source=0 );
  static buff_t* find(    sim_t*, const std::string& name );
  static buff_t* find( player_t*, const std::string& name, player_t* source=0 );

  const char* name() const { return name_str.c_str(); }
  int max_stack() const
  { return _max_stack; }
};

inline buff_creator_t::operator buff_t* () const
{ return new buff_t( *this ); }

struct stat_buff_t : public buff_t
{
  double amount;
  stat_type_e stat;

  virtual void bump     ( int stacks=1, double value=-1.0 );
  virtual void decrement( int stacks=1, double value=-1.0 );
  virtual void expire();

private:
  stat_buff_t( const stat_buff_creator_t& params );
  friend struct stat_buff_creator_t;
};

inline stat_buff_creator_t::operator stat_buff_t* () const
{ return new stat_buff_t( *this ); }


struct absorb_buff_t : public buff_t
{
  stats_t* absorb_source;

protected:
  absorb_buff_t( const absorb_buff_creator_t& params );
  friend struct absorb_buff_creator_t;
};

inline absorb_buff_creator_t::operator absorb_buff_t* () const
{ return new absorb_buff_t( *this ); }


struct cost_reduction_buff_t : public buff_t
{
  double amount;
  school_type_e school;
  bool refreshes;

private:
  cost_reduction_buff_t( const cost_reduction_buff_creator_t& params );
  friend struct cost_reduction_buff_creator_t;
public:
  virtual void bump     ( int stacks=1, double value=-1.0 );
  virtual void decrement( int stacks=1, double value=-1.0 );
  virtual void expire();
  virtual void refresh  ( int stacks=0, double value=-1.0, timespan_t duration = timespan_t::min() );
};

inline cost_reduction_buff_creator_t::operator cost_reduction_buff_t* () const
{ return new cost_reduction_buff_t( *this ); }

struct debuff_t : public buff_t
{
private:
  debuff_t( const buff_creator_t& params );
  friend struct buff_creator_t;
};

inline buff_creator_t::operator debuff_t* () const
{ return new debuff_t( *this ); }

typedef struct buff_t aura_t;

// Expressions ==============================================================

enum token_type_e
{
  TOK_UNKNOWN=0,
  TOK_PLUS,
  TOK_MINUS,
  TOK_MULT,
  TOK_DIV,
  TOK_ADD,
  TOK_SUB,
  TOK_AND,
  TOK_OR,
  TOK_NOT,
  TOK_EQ,
  TOK_NOTEQ,
  TOK_LT,
  TOK_LTEQ,
  TOK_GT,
  TOK_GTEQ,
  TOK_LPAR,
  TOK_RPAR,
  TOK_IN,
  TOK_NOTIN,
  TOK_NUM,
  TOK_STR,
  TOK_ABS,
  TOK_SPELL_LIST,
  TOK_FLOOR,
  TOK_CEIL
};

struct expr_token_t
{
  token_type_e type;
  std::string label;
};

struct expression_t
{
  static int precedence( token_type_e );
  static bool is_unary( token_type_e );
  static bool is_binary( token_type_e );
  static token_type_e next_token( action_t* action, const std::string& expr_str, int& current_index,
                                  std::string& token_str, token_type_e prev_token );
  static std::vector<expr_token_t> parse_tokens( action_t* action, const std::string& expr_str );
  static void print_tokens( const std::vector<expr_token_t>& tokens, sim_t* sim );
  static void convert_to_unary( std::vector<expr_token_t>& tokens );
  static bool convert_to_rpn( std::vector<expr_token_t>& tokens );
};

// Action expression types ==================================================

class expr_t
{
  std::string name_;

protected:
  template <typename T> static double coerce( T t ) { return t; }
  static double coerce( timespan_t t ) { return t.total_seconds(); }

  virtual double evaluate() = 0;

public:
  expr_t( const std::string& name ) : name_( name ) {}
  virtual ~expr_t() {}

  const std::string& name() const { return name_; }

  double eval() { return evaluate(); }
  bool success() { return eval() != 0; }

  static expr_t* parse( action_t*, const std::string& expr_str );
};

// Reference Expression - ref_expr_t
// Class Template to create a expression with a reference ( ref ) of arbitrary type T, and evaluate that reference
template <typename T>
class ref_expr_t : public expr_t
{
  const T& t;
  virtual double evaluate() { return coerce( t ); }

public:
  ref_expr_t( const std::string& name, const T& t_ ) : expr_t( name ), t( t_ ) {}
};

// Template to return a reference expression
template <typename T>
inline expr_t* make_ref_expr( const std::string& name, const T& t )
{ return new ref_expr_t<T>( name, t ); }

// Function Expression - fn_expr_t
// Class Template to create a function ( fn ) expression with arbitrary functor f, which gets evaluated
template <typename F>
class fn_expr_t : public expr_t
{
  F f;

  virtual double evaluate() { return coerce( f() ); }

public:
  fn_expr_t( const std::string& name, F f_ ) :
    expr_t( name ), f( f_ ) {}
};

// Template to return a function expression
template <typename F>
inline expr_t* make_fn_expr( const std::string& name, F f )
{ return new fn_expr_t<F>( name, f ); }

// Make member function expression - make_mem_fn_expr
// Template to return function expression that calls a member ( mem ) function ( fn ) f on reference t.
template <typename F, typename T>
inline expr_t* make_mem_fn_expr( const std::string& name, T& t, F f )
{ return make_fn_expr( name, std::bind( std::mem_fn( f ), &t ) ); }


// Spell query expression types =============================================

enum expr_data_type_e
{
  DATA_SPELL = 0,
  DATA_TALENT,
  DATA_EFFECT,
  DATA_TALENT_SPELL,
  DATA_CLASS_SPELL,
  DATA_RACIAL_SPELL,
  DATA_MASTERY_SPELL,
  DATA_SPECIALIZATION_SPELL,
  DATA_GLYPH_SPELL,
  DATA_SET_BONUS_SPELL
};

struct spell_data_expr_t
{
  std::string name_str;
  sim_t* sim;
  expr_data_type_e data_type;
  bool effect_query;

  int result_type_e;
  double result_num;
  std::vector<uint32_t> result_spell_list;
  std::string result_str;

  spell_data_expr_t( sim_t* sim, const std::string& n, expr_data_type_e dt = DATA_SPELL, bool eq = false, int t=TOK_UNKNOWN ) : name_str( n ), sim( sim ), data_type( dt ), effect_query( eq ), result_type_e( t ), result_num( 0 ), result_spell_list() {}
  spell_data_expr_t( sim_t* sim, const std::string& n, double       constant_value ) : name_str( n ), sim( sim ), data_type( DATA_SPELL ) { result_type_e = TOK_NUM; result_num = constant_value; }
  spell_data_expr_t( sim_t* sim, const std::string& n, std::string& constant_value ) : name_str( n ), sim( sim ), data_type( DATA_SPELL ) { result_type_e = TOK_STR; result_str = constant_value; }
  spell_data_expr_t( sim_t* sim, const std::string& n, std::vector<uint32_t>& constant_value ) : name_str( n ), sim( sim ), data_type( DATA_SPELL ) { result_type_e = TOK_SPELL_LIST; result_spell_list = constant_value; }
  virtual ~spell_data_expr_t() {}
  virtual int evaluate() { return result_type_e; }
  const char* name() { return name_str.c_str(); }

  virtual std::vector<uint32_t> operator|( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator&( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator-( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }

  virtual std::vector<uint32_t> operator<( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator>( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator<=( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator>=( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator==( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator!=( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }

  virtual std::vector<uint32_t> in( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> not_in( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }

  static spell_data_expr_t* parse( sim_t* sim, const std::string& expr_str );
  static spell_data_expr_t* create_spell_expression( sim_t* sim, const std::string& name_str );
};

class mutex_t : public noncopyable
{
private:
  class native_t;
  native_t* native_handle;

public:
  mutex_t();
  ~mutex_t();

  void lock();
  void unlock();
};

class thread_t : public noncopyable
{
private:
  class native_t;
  native_t* native_handle;

protected:
  thread_t();
  virtual ~thread_t();
public:
  virtual void run() = 0;

  void launch();
  void wait();

  static void sleep( timespan_t );
};

class auto_lock_t
{
private:
  mutex_t& mutex;
public:
  auto_lock_t( mutex_t& mutex_ ) : mutex( mutex_ ) { mutex.lock(); }
  ~auto_lock_t() { mutex.unlock(); }
};

// Simple freelist allocator for events =====================================

class event_freelist_t
{
private:
  struct free_event_t { free_event_t* next; };
  free_event_t* list;

public:
  event_freelist_t() : list( 0 ) {}
  ~event_freelist_t();

  void* allocate( std::size_t );
  void deallocate( void* );
};

// Simulation Setup =========================================================

struct option_tuple_t
{
  std::string scope, name, value;
  option_tuple_t( const std::string& s, const std::string& n, const std::string& v ) : scope( s ), name( n ), value( v ) {}
};

struct option_db_t : public std::vector<option_tuple_t>
{
  std::string auto_path;
  std::unordered_map<std::string,std::string> var_map;

  option_db_t();
  void add( const std::string& scope, const std::string& name, const std::string& value )
  {
    push_back( option_tuple_t( scope, name, value ) );
  }
  FILE* open_file( const std::string& name );
  bool parse_file( FILE* file );
  bool parse_token( const std::string& token );
  bool parse_line( const std::string& line );
  bool parse_args( const std::vector<std::string>& args );
};

struct player_description_t
{
  // Add just enough to describe a player

  // name, class, talents, glyphs, gear, professions, actions explicitly stored
  std::string name;
  // etc

  // flesh out API, these functions cannot depend upon sim_t
  // ideally they remain static, but if not then move to sim_control_t
  static void load_bcp    ( player_description_t& /*etc*/ );
  static void load_wowhead( player_description_t& /*etc*/ );
  static void load_chardev( player_description_t& /*etc*/ );
  static void load_rawr   ( player_description_t& /*etc*/ );
};

struct combat_description_t
{
  std::string name;
  int target_seconds;
  std::string raid_events;
  // etc
};

struct sim_control_t
{
  combat_description_t combat;
  std::vector<player_description_t> players;
  option_db_t options;

  bool parse_args( int argc, char** argv );
};

// Target Specific ==========================================================

struct target_specific_entry_t
{
  size_t index;
  std::string name;
  player_t* source;
  target_specific_entry_t( size_t i, const std::string& n, player_t* s ) : index(i), name(n), source(s) {}
};

struct target_specific_t
{
  size_t index;
  target_specific_t() : index(-1) {}
  target_specific_t( const std::string& name, player_t* source );
  void init(  const std::string& name, player_t* source );
  void*& operator[]( player_t* target );
};


struct target_data_t : public actor_pair_t
{
  target_data_t( player_t* t, player_t* s ) : actor_pair_t( t, s ) {}
  virtual ~target_data_t() {}
  virtual void reset() {}
};

// Simulation Engine ========================================================

struct sim_t : private thread_t
{
  sim_control_t* control;
  int         argc;
  char**      argv;
  sim_t*      parent;
  event_freelist_t free_list;
  player_t*   target;
  player_t*   target_list;
  player_t*   player_list;
  player_t*   active_player;
  int         num_players;
  int         num_enemies;
  int         max_player_level;
  int         canceled, iteration_canceled;
  timespan_t  queue_lag, queue_lag_stddev;
  timespan_t  gcd_lag, gcd_lag_stddev;
  timespan_t  channel_lag, channel_lag_stddev;
  timespan_t  queue_gcd_reduction;
  int         strict_gcd_queue;
  double      confidence;
  double      confidence_estimator;
  // Latency
  timespan_t  world_lag, world_lag_stddev;
  double      travel_variance, default_skill;
  timespan_t  reaction_time, regen_periodicity;
  timespan_t  current_time, max_time, expected_time;
  double      vary_combat_length;
  timespan_t  last_event;
  int         fixed_time;
  int64_t     events_remaining, max_events_remaining;
  int64_t     events_processed, total_events_processed;
  int         seed, id, iterations, current_iteration, current_slot;
  int         armor_update_interval, weapon_speed_scale_factors;
  int         optimal_raid, log, debug;
  int         save_profiles, default_actions;
  stat_type_e normalized_stat;
  std::string current_name, default_region_str, default_server_str, save_prefix_str,save_suffix_str;
  int         save_talent_str;
  talent_format_e talent_format;
  bool        input_is_utf8;
  std::vector<player_t*> actor_list;
  std::string main_target_str;

  // Target options
  double      target_death;
  double      target_death_pct;
  int         target_level;
  std::string target_race;
  int         target_adds;

  // Data access
  dbc_t       dbc;

  // Default stat enchants
  gear_stats_t enchant;

  std::unordered_map<std::string,std::string> var_map;
  std::vector<option_t> options;
  std::vector<std::string> party_encoding;
  std::vector<std::string> item_db_sources;

  // Random Number Generation
private:
  rng_t* default_rng_;     // == (deterministic_rng ? deterministic_rng : rng )
  rng_t* rng_list;
  int deterministic_rng;
public:
  rng_t* rng;
  rng_t* _deterministic_rng;
  int separated_rng, average_range, average_gauss;
  int convergence_scale;

  rng_t* default_rng() const { return default_rng_; }

  bool      roll( double chance ) const;
  double    range( double min, double max );
  double    averaged_range( double min, double max );
  double    gauss( double mean, double stddev );
  timespan_t gauss( timespan_t mean, timespan_t stddev );
  double    real() const;
  rng_t*    get_rng( const std::string& name, int type=RNG_DEFAULT );

  // Timing Wheel Event Management
  event_t** timing_wheel;
  int    wheel_seconds, wheel_size, wheel_mask, timing_slice;
  double wheel_granularity;

  // Raid Events
  std::vector<raid_event_t*> raid_events;
  std::string raid_events_str;
  std::string fight_style;

  // Buffs and Debuffs Overrides
  struct overrides_t
  {
    // Buff overrides
    int attack_haste;
    int attack_power_multiplier;
    int critical_strike;
    int mastery;
    int spell_haste;
    int spell_power_multiplier;
    int stamina;
    int str_agi_int;

    // Debuff overrides
    int slowed_casting;
    int magic_vulnerability;
    int mortal_wounds;
    int physical_vulnerability;
    int ranged_vulnerability;
    int weakened_armor;
    int weakened_blows;

    // Misc stuff needs resolving
    int bloodlust;
    int honor_among_thieves;
  } overrides;

  // Auras
  struct auras_t
  {
    // Raid-wide auras from various classes
    aura_t* attack_haste;
    aura_t* attack_power_multiplier;
    aura_t* critical_strike;
    aura_t* mastery;
    aura_t* spell_haste;
    aura_t* spell_power_multiplier;
    aura_t* stamina;
    aura_t* str_agi_int;

    // Honor Among Thieves hackery
    aura_t* honor_among_thieves;
  } auras;

  // Auras and De-Buffs
  std::vector<buff_t*> buff_list;
  timespan_t aura_delay;

  // Global aura related delay
  timespan_t default_aura_delay;
  timespan_t default_aura_delay_stddev;

  cooldown_t* cooldown_list;

  // Reporting
  report_t*  report;
  scaling_t* scaling;
  plot_t*    plot;
  reforge_plot_t* reforge_plot;
  timespan_t elapsed_cpu;
  double     iteration_dmg, iteration_heal;
  sample_data_t raid_dps, total_dmg, raid_hps, total_heal, simulation_length;
  int        report_progress;
  int        bloodlust_percent;
  timespan_t bloodlust_time;
  std::string reference_player_str;
  std::vector<player_t*> players_by_dps;
  std::vector<player_t*> players_by_hps;
  std::vector<player_t*> players_by_name;
  std::vector<player_t*> targets_by_name;
  std::vector<std::string> id_dictionary;
  std::vector<timespan_t> iteration_timeline;
  std::vector<int> divisor_timeline;
  std::string output_file_str, html_file_str;
  std::string xml_file_str, xml_stylesheet_file_str;
  std::vector<std::string> error_list;
  FILE* output_file;
  int debug_exp;
  int report_precision;
  int report_pets_separately;
  int report_targets;
  int report_details;
  int report_rng;
  int hosted_html;
  int print_styles;
  int report_overheal;
  int save_raid_summary;
  int statistics_level;
  int separate_stats_by_actions;
  int report_raid_summary;

  struct report_information_t
  {
    bool charts_generated;
    std::vector<std::string> dps_charts, hps_charts, gear_charts, dpet_charts;
    std::string timeline_chart;
    report_information_t() { charts_generated = false; }
  } report_information;

  std::vector<target_specific_entry_t> target_specifics;

  // Multi-Threading
  int threads;
  std::vector<sim_t*> children;
  int thread_index;
  virtual void run() { iterate(); }

  // Spell database access
  spell_data_expr_t* spell_query;
  unsigned           spell_query_level;

  sim_t( sim_t* parent = 0, int thrdID = 0 );
  virtual ~sim_t();

  int       main( const std::vector<std::string>& args );
  void      cancel();
  double    progress( std::string& phase );
  void      combat( int iteration );
  void      combat_begin();
  void      combat_end();
  void      add_event( event_t*, timespan_t delta_time );
  void      reschedule_event( event_t* );
  void      flush_events();
  void      cancel_events( player_t* );
  event_t*  next_event();
  void      reset();
  bool      init();
  void      analyze_player( player_t* p );
  void      analyze();
  void      merge( sim_t& other_sim );
  void      merge();
  bool      iterate();
  void      partition();
  bool      execute();
  void      print_options();
  void      create_options();
  bool      parse_option( const std::string& name, const std::string& value );
  bool      parse_options( int argc, char** argv );
  bool      setup( sim_control_t* );
  bool      time_to_think( timespan_t proc_time );
  timespan_t total_reaction_time ();
  double    iteration_adjust();
  player_t* find_player( const std::string& name ) const;
  player_t* find_player( int index ) const;
  cooldown_t* get_cooldown( const std::string& name );
  void      use_optimal_buffs_and_debuffs( int value );
  expr_t* create_expression( action_t*, const std::string& name );
  int       errorf( const char* format, ... ) PRINTF_ATTRIBUTE( 2,3 );

  size_t target_specific_index( const std::string& name, player_t* source )
  {
    size_t size = target_specifics.size();
    for( size_t i=0; i < size; i++ )
      if( target_specifics[ i ].name == name &&
          target_specifics[ i ].source == source )
        return i;
    target_specifics.push_back( target_specific_entry_t( size, name, source ) );
    return size;
  }

  static option_t* get_class_option( void* );
};

// Scaling ==================================================================

struct scaling_t
{
  sim_t* sim;
  sim_t* baseline_sim;
  sim_t* ref_sim;
  sim_t* delta_sim;
  sim_t* ref_sim2;
  sim_t* delta_sim2;
  stat_type_e scale_stat;
  double scale_value;
  double scale_delta_multiplier;
  int    calculate_scale_factors;
  int    center_scale_delta;
  int    positive_scale_delta;
  int    scale_lag;
  double scale_factor_noise;
  int    normalize_scale_factors;
  int    smooth_scale_factors;
  int    debug_scale_factors;
  std::string scale_only_str;
  stat_type_e current_scaling_stat;
  int num_scaling_stats, remaining_scaling_stats;
  double    scale_haste_iterations, scale_expertise_iterations, scale_crit_iterations, scale_hit_iterations, scale_mastery_iterations;
  std::string scale_over;
  std::string scale_over_player;

  // Gear delta for determining scale factors
  gear_stats_t stats;

  scaling_t( sim_t* s );

  void init_deltas();
  void analyze();
  void analyze_stats();
  void analyze_ability_stats( stat_type_e, double, player_t*, player_t*, player_t* );
  void analyze_lag();
  void normalize();
  void derive();
  double progress( std::string& phase );
  void create_options();
  bool has_scale_factors();
  double scale_over_function( sim_t* s, player_t* p );
  double scale_over_function_error( sim_t* s, player_t* p );
};

// Plot =====================================================================

struct plot_t
{
  sim_t* sim;
  std::string dps_plot_stat_str;
  double dps_plot_step;
  int    dps_plot_points;
  int    dps_plot_iterations;
  int    dps_plot_debug;
  stat_type_e current_plot_stat;
  int    num_plot_stats, remaining_plot_stats, remaining_plot_points;
  bool   dps_plot_positive;

  plot_t( sim_t* s );

  void analyze();
  void analyze_stats();
  double progress( std::string& phase );
  void create_options();
};

// Reforge Plot =============================================================

struct reforge_plot_t
{
  sim_t* sim;
  sim_t* current_reforge_sim;
  std::string reforge_plot_stat_str;
  std::string reforge_plot_output_file_str;
  FILE* reforge_plot_output_file;
  std::vector<stat_type_e> reforge_plot_stat_indices;
  int    reforge_plot_step;
  int    reforge_plot_amount;
  int    reforge_plot_iterations;
  int    reforge_plot_debug;
  int    current_stat_combo;
  int    num_stat_combos;

  reforge_plot_t( sim_t* s );

  void generate_stat_mods( std::vector<std::vector<int> > &stat_mods,
                           const std::vector<stat_type_e> &stat_indices,
                           int cur_mod_stat,
                           std::vector<int> cur_stat_mods );
  void analyze();
  void analyze_stats();
  double progress( std::string& phase );
  void create_options();
};

struct reforge_plot_data_t
{
  double value;
  double error;
};

// Event ====================================================================

struct event_t : public noncopyable
{
private:
  static void cancel_( event_t* e );
  static void early_( event_t* e );

  static void* operator new( std::size_t ) throw(); // DO NOT USE!

public:
  event_t*  next;
  sim_t* const sim;
  player_t* const player;
  uint32_t  id;
  timespan_t time;
  timespan_t reschedule_time;
  int       canceled;
  const char* const name;
  event_t( sim_t* s, player_t* p=0, const char* n="unknown" ) :
    next( 0 ), sim( s ), player( p ), time( timespan_t::zero() ), reschedule_time( timespan_t::zero() ), canceled( 0 ), name( n )
  { }
  timespan_t occurs()  const { return ( reschedule_time != timespan_t::zero() ) ? reschedule_time : time; }
  timespan_t remains() const { return occurs() - sim -> current_time; }
  void reschedule( timespan_t new_time );
  virtual void execute() = 0;
  virtual ~event_t() {}

  // T must be implicitly convertible to event_t* --
  // basically, a pointer to a type derived from event_t.
  template <typename T> static void cancel( T& e )
  { if ( e ) { cancel_( e ); e = 0; } }
  template <typename T> static void early( T& e )
  { if ( e ) { early_( e ); e = 0; } }

  // Simple free-list memory manager.
  static void* operator new( std::size_t size, sim_t* sim )
  { return sim -> free_list.allocate( size ); }

  static void operator delete( void* p )
  {
    event_t* e = static_cast<event_t*>( p );
    e -> sim -> free_list.deallocate( e );
  }

  static void operator delete( void* p, sim_t* sim )
  { sim -> free_list.deallocate( p ); }
};

// Gear Rating Conversions ==================================================

namespace internal {
struct rating_t
{
  double  spell_haste,  spell_hit,  spell_crit;
  double attack_haste, attack_hit, attack_crit;
  double ranged_haste, ranged_hit,ranged_crit;
  double expertise;
  double dodge, parry, block;
  double mastery;
};
}

struct rating_t : public internal::rating_t
{
  typedef internal::rating_t base_t;
  rating_t() : base_t( base_t() ) {}

  void init( sim_t*, dbc_t& pData, int level, player_type_e type );
  static double interpolate( int level, double val_60, double val_70, double val_80, double val_85 = -1 );
  static double get_attribute_base( sim_t*, dbc_t& pData, int level, player_type_e class_type, race_type_e race, base_stat_type_e_e stat_type_e );
};

// Weapon ===================================================================

struct weapon_t
{
  weapon_type_e type;
  school_type_e school;
  double damage, dps;
  double min_dmg, max_dmg;
  timespan_t swing_time;
  slot_type_e slot;
  int    buff_type;
  double buff_value;
  double bonus_dmg;

  weapon_type_e group() const;
  timespan_t normalized_weapon_speed() const;
  double proc_chance_on_swing( double PPM, timespan_t adjusted_swing_time=timespan_t::zero() ) const;

  weapon_t( weapon_type_e t=WEAPON_NONE,
            double d=0,
            timespan_t st=timespan_t::from_seconds( 2.0 ),
            school_type_e s=SCHOOL_PHYSICAL ) :
    type( t ), school( s ), damage( d ), min_dmg( d ), max_dmg( d ),
    swing_time( st ), slot( SLOT_INVALID ), buff_type( 0 ), buff_value( 0 ),
    bonus_dmg( 0 ) {}
};

// Item =====================================================================

struct item_t
{
  sim_t* sim;
  player_t* player;
  slot_type_e slot;
  int quality, ilevel;
  bool unique, unique_enchant, unique_addon, is_heroic, is_lfr, is_ptr, is_matching_type, is_reforged;
  stat_type_e reforged_from;
  stat_type_e reforged_to;

  // Option Data
  std::string option_name_str;
  std::string option_id_str;
  std::string option_stats_str;
  std::string option_gems_str;
  std::string option_enchant_str;
  std::string option_addon_str;
  std::string option_equip_str;
  std::string option_use_str;
  std::string option_weapon_str;
  std::string option_heroic_str;
  std::string option_lfr_str;
  std::string option_armor_type_str;
  std::string option_reforge_str;
  std::string option_random_suffix_str;
  std::string option_ilevel_str;
  std::string option_quality_str;
  std::string option_data_source_str;
  std::string options_str;

  // Armory Data
  std::string armory_name_str;
  std::string armory_id_str;
  std::string armory_stats_str;
  std::string armory_gems_str;
  std::string armory_enchant_str;
  std::string armory_addon_str;
  std::string armory_weapon_str;
  std::string armory_heroic_str;
  std::string armory_lfr_str;
  std::string armory_armor_type_str;
  std::string armory_reforge_str;
  std::string armory_ilevel_str;
  std::string armory_quality_str;
  std::string armory_random_suffix_str;

  // Encoded Data
  std::string id_str;
  std::string encoded_name_str;
  std::string encoded_stats_str;
  std::string encoded_gems_str;
  std::string encoded_enchant_str;
  std::string encoded_addon_str;
  std::string encoded_equip_str;
  std::string encoded_use_str;
  std::string encoded_weapon_str;
  std::string encoded_heroic_str;
  std::string encoded_lfr_str;
  std::string encoded_armor_type_str;
  std::string encoded_reforge_str;
  std::string encoded_ilevel_str;
  std::string encoded_quality_str;
  std::string encoded_random_suffix_str;

  // Extracted data
  gear_stats_t base_stats,stats;
  struct special_effect_t
  {
    std::string name_str, trigger_str;
    proc_type_e trigger_type;
    int64_t trigger_mask;
    stat_type_e stat;
    school_type_e school;
    int max_stacks;
    double stat_amount, discharge_amount, discharge_scaling;
    double proc_chance;
    timespan_t duration, cooldown, tick;
    bool cost_reduction;
    bool no_player_benefits;
    bool no_debuffs;
    bool no_refresh;
    bool chance_to_discharge;
    unsigned int override_result_type_es_mask;
    unsigned result_type_es_mask;
    bool reverse;
    int aoe;
    special_effect_t() :
      trigger_type( PROC_NONE ), trigger_mask( 0 ), stat( STAT_NONE ), school( SCHOOL_NONE ),
      max_stacks( 0 ), stat_amount( 0 ), discharge_amount( 0 ), discharge_scaling( 0 ),
      proc_chance( 0 ), duration( timespan_t::zero() ), cooldown( timespan_t::zero() ),
      tick( timespan_t::zero() ), cost_reduction( false ), no_player_benefits( false ), no_debuffs( false ),
      no_refresh( false ), chance_to_discharge( false ), override_result_type_es_mask( 0 ), result_type_es_mask( 0 ), reverse( false ), aoe( 0 ) {}
    bool active() { return stat || school; }
  } use, equip, enchant, addon;

  item_t() : sim( 0 ), player( 0 ), slot( SLOT_INVALID ), quality( 0 ), ilevel( 0 ), unique( false ), unique_enchant( false ),
    unique_addon( false ), is_heroic( false ), is_lfr( false ), is_ptr( false ), is_matching_type( false ), is_reforged( false ) {}
  item_t( player_t*, const std::string& options_str );
  bool active() const;
  bool heroic() const;
  bool lfr() const;
  bool ptr() const;
  bool reforged() const;
  bool matching_type();
  const char* name() const;
  const char* slot_name() const;
  const char* armor_type();
  weapon_t* weapon() const;
  bool init();
  bool parse_options();
  void encode_options();
  bool decode_stats();
  bool decode_gems();
  bool decode_enchant();
  bool decode_addon();
  bool decode_special( special_effect_t&, const std::string& encoding );
  bool decode_weapon();
  bool decode_heroic();
  bool decode_lfr();
  bool decode_armor_type();
  bool decode_reforge();
  bool decode_random_suffix();
  bool decode_ilevel();
  bool decode_quality();

  static bool download_slot( item_t& item,
                             const std::string& item_id,
                             const std::string& enchant_id,
                             const std::string& addon_id,
                             const std::string& reforge_id,
                             const std::string& rsuffix_id,
                             const std::string gem_ids[ 3 ] );
  static bool download_item( item_t&, const std::string& item_id );
  static bool download_glyph( player_t* player, std::string& glyph_name, const std::string& glyph_id );
  static gem_type_e parse_gem( item_t&            item,
                               const std::string& gem_id );
};

// Item database ============================================================

struct item_database_t
{
  static bool     download_slot(      item_t& item,
                                      const std::string& item_id,
                                      const std::string& enchant_id,
                                      const std::string& addon_id,
                                      const std::string& reforge_id,
                                      const std::string& rsuffix_id,
                                      const std::string gem_ids[ 3 ] );
  static bool     download_item(      item_t& item, const std::string& item_id );
  static bool     download_glyph(     player_t* player, std::string& glyph_name, const std::string& glyph_id );
  static gem_type_e parse_gem(        item_t& item, const std::string& gem_id );
  static bool     initialize_item_sources( const item_t& item, std::vector<std::string>& source_list );

  static int      random_suffix_type( const item_t& item );
  static int      random_suffix_type( const item_data_t* );
  static uint32_t armor_value(        const item_t& item, unsigned item_id );
  static uint32_t armor_value(        const item_data_t*, const dbc_t& );
  static uint32_t weapon_dmg_min(     const item_t& item, unsigned item_id );
  static uint32_t weapon_dmg_min(     const item_data_t*, const dbc_t& );
  static uint32_t weapon_dmg_max(     const item_t& item, unsigned item_id );
  static uint32_t weapon_dmg_max(     const item_data_t*, const dbc_t& );

  static bool     load_item_from_data( item_t& item, const item_data_t* item_data );
  static bool     parse_gems(         item_t&            item,
                                      const item_data_t* item_data,
                                      const std::string  gem_ids[ 3 ] );
  static bool     parse_enchant(      item_t& item, const std::string& enchant_id );
};

// Set Bonus ================================================================

struct set_bonus_t
{
  std::array<int,SET_MAX> count;
  int tier13_2pc_caster() const; int tier13_2pc_melee() const; int tier13_2pc_tank() const; int tier13_2pc_heal() const;
  int tier13_4pc_caster() const; int tier13_4pc_melee() const; int tier13_4pc_tank() const; int tier13_4pc_heal() const;
  int tier14_2pc_caster() const; int tier14_2pc_melee() const; int tier14_2pc_tank() const; int tier14_2pc_heal() const;
  int tier14_4pc_caster() const; int tier14_4pc_melee() const; int tier14_4pc_tank() const; int tier14_4pc_heal() const;
  int tier15_2pc_caster() const; int tier15_2pc_melee() const; int tier15_2pc_tank() const; int tier15_2pc_heal() const;
  int tier15_4pc_caster() const; int tier15_4pc_melee() const; int tier15_4pc_tank() const; int tier15_4pc_heal() const;
  int tier16_2pc_caster() const; int tier16_2pc_melee() const; int tier16_2pc_tank() const; int tier16_2pc_heal() const;
  int tier16_4pc_caster() const; int tier16_4pc_melee() const; int tier16_4pc_tank() const; int tier16_4pc_heal() const;
  int pvp_2pc_caster() const; int pvp_2pc_melee() const; int pvp_2pc_tank() const; int pvp_2pc_heal() const;
  int pvp_4pc_caster() const; int pvp_4pc_melee() const; int pvp_4pc_tank() const; int pvp_4pc_heal() const;
  int decode( player_t*, item_t& item ) const;
  bool init( player_t* );

  set_bonus_t();

  expr_t* create_expression( player_t*, const std::string& type );
};

struct set_bonus_array_t
{
private:
  const spell_data_t* default_value;
  const spell_data_t* set_bonuses[ SET_MAX ];
  player_t* p;

  const spell_data_t* create_set_bonus( uint32_t spell_id );

public:
  set_bonus_array_t( player_t* p, const uint32_t a_bonus[ N_TIER ][ N_TIER_BONUS ] );

  bool              has_set_bonus( set_type_e s ) const;
  const spell_data_t* set( set_type_e s ) const;
};

// Player ===================================================================

struct player_t : public noncopyable
{
  // static values
  sim_t* const sim;
  const player_type_e type;
  std::string name_str;

  player_t*   next;
  int         index;
  // (static) attributes - things which should not change during combat
  race_type_e       race;
  role_type_e       role;
  int               level;
  int               party, member;
  ready_type_e      ready_type;
  specialization_e  spec;
  bool              bugs, scale_player, has_dtr;
  double      dtr_proc_chance;

  // dynamic attributes - things which change during combat
  player_t*   target;
  position_type_e position;
  int         active_pets;
  int         initialized;
  bool        potion_used;

  std::string talents_str, glyphs_str, id_str, target_str;
  std::string region_str, server_str, origin_str;
  std::string race_str, professions_str, position_str;
  timespan_t  gcd_ready, base_gcd, started_waiting;
  rating_t    rating;
  std::vector<pet_t*> pet_list;
  int         invert_scaling;
  timespan_t  reaction_mean,reaction_stddev,reaction_nu;
  std::vector<absorb_buff_t*> absorb_buffs;
  double      avg_ilvl;

  struct p_vengeance_t
  {
    bool   enabled, was_attacked;
    double damage, value, max;
  } vengeance;

  // Latency
  timespan_t  world_lag, world_lag_stddev;
  timespan_t  brain_lag, brain_lag_stddev;
  bool        world_lag_override, world_lag_stddev_override;

  int    events;

  // Data access
  dbc_t       dbc;

  // Option Parsing
  std::vector<option_t> options;

  // Shift automatically out of stance/form
  bool autoUnshift;

  // Talent Parsing
  std::array<uint32_t,MAX_TALENT_SLOTS> talent_list;

  // Glyph Parsing
  std::vector<const spell_data_t*> glyph_list;

  // Profs
  std::array<int,PROFESSION_MAX> profession;

  // Haste
  double spell_haste, attack_haste;

  struct base_initial_current_t
  {
    std::array<double,ATTRIBUTE_MAX> attribute;

    double mastery, mastery_rating, haste_rating;

    double spell_hit, spell_crit, mp5;
    // Attack Mechanics
    double attack_power, attack_hit, attack_expertise, attack_crit;
    // Defense Mechanics
    double armor, bonus_armor, miss, dodge, parry, block, block_reduction;
  };

  struct initial_current_extended_t : base_initial_current_t
  {
    std::array<double,ATTRIBUTE_MAX> attribute_multiplier;
    double spell_power_multiplier, attack_power_multiplier, armor_multiplier;

    double spell_power_per_intellect, spell_crit_per_intellect;
    double attack_power_per_strength, attack_power_per_agility, attack_crit_per_agility;
    double dodge_per_agility, parry_rating_per_strength;

    double mp5_per_spirit, mp5_from_spirit_multiplier, health_per_stamina;
    double skill, distance;
    bool sleeping;

    std::array<double,SCHOOL_MAX> resource_reduction;

    initial_current_extended_t();
  };
  base_initial_current_t base;
  initial_current_extended_t initial, current;

  // Spell Mechanics
  double base_spell_power;
  std::array<double,SCHOOL_MAX + 1> initial_spell_power;
  std::array<double,SCHOOL_MAX + 1> spell_power;
  double mana_regen_base;
  double base_energy_regen_per_second;
  double base_focus_regen_per_second;
  double base_chi_regen_per_second;
  timespan_t last_cast;

  // Defense Mechanics
  event_t* target_auto_attack;
  double diminished_dodge_capi, diminished_parry_capi, diminished_kfactor;
  double armor_coeff;
  double half_resistance_rating;
  std::array< int, SCHOOL_MAX > spell_resistance;

  // Weapons
  weapon_t main_hand_weapon;
  weapon_t off_hand_weapon;

  // Main, offhand, and ranged attacks
  attack_t* main_hand_attack;
  attack_t*  off_hand_attack;

  // Resources
  struct resources_t
  {
    std::array<double,RESOURCE_MAX> base, initial, max, current,
                                    base_multiplier, initial_multiplier;
    std::array<int, RESOURCE_MAX> infinite_resource;

    resources_t()
    {
      range::fill( base, 0.0 );
      range::fill( initial, 0.0 );
      range::fill( max, 0.0 );
      range::fill( current, 0.0 );
      range::fill( base_multiplier, 1.0 );
      range::fill( initial_multiplier, 1.0 );
      range::fill( infinite_resource, 0 );
    }

    double pct( resource_type_e rt ) const
    { return current[ rt ] / max[ rt ]; }

    bool is_infinite( resource_type_e rt ) const
    { return infinite_resource[ rt ] != 0; }
  } resources;

  // Consumables
  std::string flask_str, elixirs_str, food_str;
  int elixir_guardian;
  int elixir_battle;
  int flask;
  int food;

  // Events
  action_t* executing;
  action_t* channeling;
  event_t*  readying;
  event_t*  off_gcd;
  bool      in_combat;
  bool      action_queued;

  // Delay time used by "cast_delay" expression to determine when an action
  // can be used at minimum after a spell cast has finished, including GCD
  timespan_t cast_delay_reaction;
  timespan_t cast_delay_occurred;

  // Callbacks
  struct callbacks_t
  {
    std::vector<action_callback_t*> all_callbacks;
    std::array< std::vector<action_callback_t*>, RESULT_MAX > attack;
    std::array< std::vector<action_callback_t*>, RESULT_MAX > spell;
    std::array< std::vector<action_callback_t*>, RESULT_MAX > harmful_spell;
    std::array< std::vector<action_callback_t*>, RESULT_MAX > heal;
    std::array< std::vector<action_callback_t*>, RESULT_MAX > absorb;
    std::array< std::vector<action_callback_t*>, RESULT_MAX > tick;

    std::array< std::vector<action_callback_t*>, SCHOOL_MAX > direct_damage;
    std::array< std::vector<action_callback_t*>, SCHOOL_MAX > tick_damage;
    std::array< std::vector<action_callback_t*>, SCHOOL_MAX > direct_heal;
    std::array< std::vector<action_callback_t*>, SCHOOL_MAX > tick_heal;

    std::array< std::vector<action_callback_t*>, RESOURCE_MAX > resource_gain;
    std::array< std::vector<action_callback_t*>, RESOURCE_MAX > resource_loss;

    virtual ~callbacks_t()
    { range::sort( all_callbacks ); dispose( all_callbacks.begin(), range::unique( all_callbacks ) ); }

    void reset();

    void register_resource_gain_callback( resource_type_e,     action_callback_t* );
    void register_resource_loss_callback( resource_type_e,     action_callback_t* );
    void register_attack_callback       ( int64_t result_mask, action_callback_t* );
    void register_spell_callback        ( int64_t result_mask, action_callback_t* );
    void register_tick_callback         ( int64_t result_mask, action_callback_t* );
    void register_heal_callback         ( int64_t result_mask, action_callback_t* );
    void register_absorb_callback       ( int64_t result_mask, action_callback_t* );
    void register_harmful_spell_callback( int64_t result_mask, action_callback_t* );
    void register_tick_damage_callback  ( int64_t result_mask, action_callback_t* );
    void register_direct_damage_callback( int64_t result_mask, action_callback_t* );
    void register_tick_heal_callback    ( int64_t result_mask, action_callback_t* );
    void register_direct_heal_callback  ( int64_t result_mask, action_callback_t* );
  } callbacks;

  virtual void register_callbacks();

  // Action Priority List
  std::vector<action_t*> action_list;
  std::string action_list_str;
  std::string choose_action_list;
  std::string action_list_skip;
  std::string modify_action;
  int         action_list_default;
  cooldown_t* cooldown_list;
  std::vector<dot_t*> dot_list;
  std::map<std::string,int> action_map;
  std::vector<action_priority_list_t*> action_priority_list;
  std::vector<action_t*> precombat_action_list;
  action_priority_list_t* active_action_list;
  action_priority_list_t* restore_action_list;
  std::map<std::string,std::string> alist_map;

  // Reporting
  int       quiet;
  action_t* last_foreground_action;
  timespan_t iteration_fight_length,arise_time;
  sample_data_t fight_length, waiting_time, executed_foreground_actions;
  timespan_t iteration_waiting_time;
  int       iteration_executed_foreground_actions;
  std::array< double, RESOURCE_MAX > resource_lost, resource_gained;
  double    rps_gain, rps_loss;
  sample_data_t deaths;
  double    deaths_error;

  // Buffed snapshot_stats (for reporting)
  struct buffed_stats_t
  {
    std::array< double, ATTRIBUTE_MAX > attribute;
    std::array< double, RESOURCE_MAX > resource;

    double spell_power, spell_hit, spell_crit, mp5;
    double attack_power,  attack_hit,  mh_attack_expertise,  oh_attack_expertise, attack_crit;
    double armor, miss, crit, dodge, parry, block;
    double spell_haste, attack_haste, attack_speed;
    double mastery;
  } buffed;

  std::vector<buff_t*> buff_list;
  proc_t*   proc_list;
  gain_t*   gain_list;
  std::vector<stats_t*> stats_list;
  benefit_t* benefit_list;
  uptime_t* uptime_list;
  std::array< std::vector<double>, STAT_MAX > dps_plot_data;
  std::vector<std::vector<reforge_plot_data_t> > reforge_plot_data;

  struct resource_timeline_t
  {
    resource_type_e type;
    std::vector<double> timeline;

    resource_timeline_t( resource_type_e t = RESOURCE_NONE ) : type( t ) {}
  };

  // Health/primary/secondary makes 3 resources, does any class/spec need more?
  // Druid: health/mana/energy/rage
  std::array<resource_timeline_t,4> resource_timelines;
  size_t resource_timeline_count;

  // Damage
  double iteration_dmg, iteration_dmg_taken; // temporary accumulators
  double dps_error, dpr, dtps_error;
  sample_data_t dmg;
  sample_data_t compound_dmg;
  sample_data_t dps;
  sample_data_t dpse;
  sample_data_t dtps;
  sample_data_t dmg_taken;
  std::vector<double> timeline_dmg;
  std::vector<double> dps_convergence_error;
  double dps_convergence;

  // Heal
  double iteration_heal,iteration_heal_taken; // temporary accumulators
  double hps_error,hpr;
  sample_data_t heal;
  sample_data_t compound_heal;
  sample_data_t hps;
  sample_data_t hpse;
  sample_data_t htps;
  sample_data_t heal_taken;

  struct report_information_t
  {
    bool charts_generated, buff_lists_generated;
    std::string action_sequence;
    std::string action_dpet_chart, action_dmg_chart, time_spent_chart;
    std::array<std::string, RESOURCE_MAX> timeline_resource_chart, gains_chart;
    std::string timeline_dps_chart, timeline_dps_error_chart, timeline_resource_health_chart;
    std::string distribution_dps_chart, scaling_dps_chart, scale_factors_chart;
    std::string reforge_dps_chart, dps_error_chart, distribution_deaths_chart;
    std::string gear_weights_lootrank_link, gear_weights_wowhead_link, gear_weights_wowreforge_link;
    std::string gear_weights_pawn_std_string, gear_weights_pawn_alt_string;
    std::string save_str;
    std::string save_gear_str;
    std::string save_talents_str;
    std::string save_actions_str;
    std::string comment_str;
    std::string thumbnail_url;
    std::string html_profile_str;
    std::vector<buff_t*> buff_list, dynamic_buffs, constant_buffs;

    report_information_t() : charts_generated(), buff_lists_generated() {}
  } report_information;

  // Gear
  std::string items_str, meta_gem_str;
  std::vector<item_t> items;
  gear_stats_t stats, initial_stats, gear, enchant, temporary;
  set_bonus_t set_bonus;
  set_bonus_array_t* sets;
  meta_gem_type_e meta_gem;
  bool matching_gear;

  // Scale Factors
  gear_stats_t scaling;
  gear_stats_t scaling_normalized;
  gear_stats_t scaling_error;
  gear_stats_t scaling_delta_dps;
  gear_stats_t scaling_compare_error;
  double       scaling_lag, scaling_lag_error;
  std::array<bool,STAT_MAX> scales_with;
  std::array<double,STAT_MAX> over_cap;
  std::vector<stat_type_e> scaling_stats; // sorting vector

  // Movement & Position
  double base_movement_speed;
  double x_position, y_position;

  struct buffs_t
  {
    buff_t* beacon_of_light;
    buff_t* berserking;
    buff_t* blood_fury_ap;
    buff_t* blood_fury_sp;
    buff_t* bloodlust;
    buff_t* body_and_soul;
    buff_t* exhaustion;
    buff_t* grace;
    buff_t* guardian_spirit;
    buff_t* heroic_presence;
    buff_t* hymn_of_hope;
    buff_t* illuminated_healing;
    buff_t* innervate;
    buff_t* lifeblood;
    buff_t* mana_tide;
    buff_t* mongoose_mh;
    buff_t* mongoose_oh;
    buff_t* pain_supression;
    buff_t* power_infusion;
    buff_t* raid_movement;
    buff_t* self_movement;
    buff_t* stoneform;
    buff_t* stunned;
    buff_t* tricks_of_the_trade;
    buff_t* unholy_frenzy;
    buff_t* weakened_soul;
  } buffs;

  struct potion_buffs_t
  {
    stat_buff_t* earthen;
    stat_buff_t* golemblood;
    stat_buff_t* speed;
    stat_buff_t* tolvir;
    stat_buff_t* volcanic;

    // new mop potions
    stat_buff_t* virmens_bite; // agi
    stat_buff_t* mogu_power; // str
    stat_buff_t* jinyu; // int
    stat_buff_t* mountains; // armor
  } potion_buffs;

  struct debuffs_t
  {
    debuff_t* bleeding;
    debuff_t* casting;
    debuff_t* flying;
    debuff_t* forbearance;
    debuff_t* invulnerable;
    debuff_t* vulnerable;

    // MoP debuffs
    debuff_t* slowed_casting;
    debuff_t* magic_vulnerability;
    debuff_t* mortal_wounds;
    debuff_t* physical_damage;
    debuff_t* physical_vulnerability;
    debuff_t* ranged_vulnerability;
    debuff_t* weakened_blows;
    debuff_t* weakened_armor;

    // Class specific "general" debuffs
    debuff_t* shattering_throw;
  } debuffs;

  struct gains_t
  {
    gain_t* arcane_torrent;
    gain_t* blessing_of_might;
    gain_t* chi_regen;
    gain_t* dark_rune;
    gain_t* energy_regen;
    gain_t* essence_of_the_red;
    gain_t* focus_regen;
    gain_t* glyph_of_innervate;
    gain_t* hymn_of_hope;
    gain_t* innervate;
    gain_t* mana_potion;
    gain_t* mana_spring_totem;
    gain_t* mp5_regen;
    gain_t* restore_mana;
    gain_t* spellsurge;
    gain_t* vampiric_embrace;
    gain_t* vampiric_touch;
    gain_t* water_elemental;
  } gains;

  struct procs_t
  {
    proc_t* hat_donor;
  } procs;

  rng_t* rng_list;

  struct rngs_t
  {
    rng_t* lag_ability;
    rng_t* lag_brain;
    rng_t* lag_channel;
    rng_t* lag_gcd;
    rng_t* lag_queue;
    rng_t* lag_reaction;
    rng_t* lag_world;
  } rngs;

  struct uptimes_t
  {
    uptime_t* primary_resource_cap;
  } uptimes;

  std::vector<void*> target_specific_ptrs;
  target_specific_t target_data;

  player_t( sim_t* sim, player_type_e type, const std::string& name, race_type_e race_type_e = RACE_NONE );

  virtual ~player_t();

  virtual const char* name() const { return name_str.c_str(); }

  virtual void init();
  virtual void init_talents();
  virtual void init_glyphs();
  virtual void init_base() = 0;
  virtual void init_items();
  virtual void init_meta_gem( gear_stats_t& );
  virtual void init_core();
  virtual void init_position();
  virtual void init_race();
  virtual void init_racials();
  virtual void init_spell();
  virtual void init_attack();
  virtual void init_defense();
  virtual void init_weapon( weapon_t* );
  virtual void init_unique_gear();
  virtual void init_enchant();
  virtual void init_resources( bool force = false );
  virtual void init_professions();
  virtual void init_professions_bonus();
  virtual std::string init_use_item_actions( const std::string& append = std::string() );
  virtual std::string init_use_profession_actions( const std::string& append = std::string() );
  virtual std::string init_use_racial_actions( const std::string& append = std::string() );
  virtual void init_actions();
  virtual void init_rating();
  virtual void init_scaling();
  virtual void init_spells();
  virtual void init_buffs();
  virtual void init_gains();
  virtual void init_procs();
  virtual void init_uptimes();
  virtual void init_benefits();
  virtual void init_rng();
  virtual void init_stats();
  virtual void init_values();
  virtual void init_target();

  virtual void reset();
  virtual void combat_begin();
  virtual void combat_end();
  virtual void merge( player_t& other );

  virtual double energy_regen_per_second() const;
  virtual double focus_regen_per_second() const;
  virtual double chi_regen_per_second() const;
  virtual double composite_attack_haste() const;
  virtual double composite_attack_speed() const;
  virtual double composite_attack_power() const;
  virtual double composite_attack_crit( const weapon_t* = 0 ) const;
  virtual double composite_attack_expertise( const weapon_t* = 0 ) const;
  virtual double composite_attack_hit() const;

  virtual double composite_spell_haste() const;
  virtual double composite_spell_power( school_type_e school ) const;
  virtual double composite_spell_crit() const;
  virtual double composite_spell_hit() const;
  virtual double composite_mp5() const;
  virtual double composite_mastery() const;

  virtual double composite_armor()                 const;
  virtual double composite_armor_multiplier()      const;
  virtual double composite_spell_resistance( school_type_e school ) const;
  virtual double composite_tank_miss( school_type_e school ) const;
  virtual double composite_tank_dodge()            const;
  virtual double composite_tank_parry()            const;
  virtual double composite_tank_block()            const;
  virtual double composite_tank_block_reduction()  const;
  virtual double composite_tank_crit_block()            const;
  virtual double composite_tank_crit( school_type_e school ) const;

  virtual double diminished_dodge()             const;
  virtual double diminished_parry()             const;

  virtual double composite_attack_power_multiplier() const;
  virtual double composite_spell_power_multiplier() const;

  virtual double matching_gear_multiplier( attribute_type_e /* attr */ ) const { return 0; }

  virtual double composite_player_multiplier   ( school_type_e, const action_t* a = NULL ) const;
  virtual double composite_player_dd_multiplier( school_type_e, const action_t* /* a */ = NULL ) const { return 1; }
  virtual double composite_player_td_multiplier( school_type_e, const action_t* a = NULL ) const;

  virtual double composite_player_heal_multiplier( school_type_e school ) const;
  virtual double composite_player_dh_multiplier( school_type_e /* school */ ) const { return 1; }
  virtual double composite_player_th_multiplier( school_type_e school ) const;

  virtual double composite_player_absorb_multiplier( school_type_e school ) const;

  virtual double composite_movement_speed() const;

  virtual double composite_attribute( attribute_type_e attr ) const;
  virtual double composite_attribute_multiplier( attribute_type_e attr ) const;

  double get_attribute( attribute_type_e a ) const;
  double strength() const { return get_attribute( ATTR_STRENGTH ); }
  double agility() const { return get_attribute( ATTR_AGILITY ); }
  double stamina() const { return get_attribute( ATTR_STAMINA ); }
  double intellect() const { return get_attribute( ATTR_INTELLECT ); }
  double spirit() const { return get_attribute( ATTR_SPIRIT ); }

  virtual void      interrupt();
  virtual void      halt();
  virtual void      moving();
  virtual void      stun();
  virtual void      clear_debuffs();
  virtual void      trigger_ready();
  virtual void      schedule_ready( timespan_t delta_time=timespan_t::zero(), bool waiting=false );
  virtual void      arise();
  virtual void      demise();
  virtual timespan_t available() const { return timespan_t::from_seconds( 0.1 ); }
  action_t* execute_action();

  virtual std::string print_action_map( const int iterations, int precision ) const;

  virtual void   regen( timespan_t periodicity=timespan_t::from_seconds( 0.25 ) );
  virtual double resource_gain( resource_type_e resource_type, double amount, gain_t* g=0, action_t* a=0 );
  virtual double resource_loss( resource_type_e resource_type, double amount, gain_t* g=0, action_t* a=0 );
  virtual void   recalculate_resource_max( resource_type_e resource_type );
  virtual bool   resource_available( resource_type_e resource_type, double cost ) const;
  void collect_resource_timeline_information();
  virtual resource_type_e primary_resource() const { return RESOURCE_NONE; }
  virtual role_type_e   primary_role() const;
  specialization_e primary_tree() const { return spec; }
  virtual const char* primary_tree_name() const;
  virtual stat_type_e normalize_by() const;

  virtual double health_percentage() const;
  virtual timespan_t time_to_die() const;
  timespan_t total_reaction_time() const;

  virtual void stat_gain( stat_type_e stat, double amount, gain_t* g=0, action_t* a=0, bool temporary=false );
  virtual void stat_loss( stat_type_e stat, double amount, gain_t* g=0, action_t* a=0, bool temporary=false );

  virtual void cost_reduction_gain( school_type_e school, double amount, gain_t* g=0, action_t* a=0 );
  virtual void cost_reduction_loss( school_type_e school, double amount, action_t* a=0 );

  virtual double assess_damage( double amount, school_type_e, dmg_type_e, result_type_e, action_t* a=0 );
  virtual double target_mitigation( double amount, school_type_e, dmg_type_e, result_type_e, action_t* a=0 );

  struct heal_info_t { double actual, amount; };
  virtual heal_info_t assess_heal( double amount, school_type_e, dmg_type_e, result_type_e, action_t* a=0 );

  virtual void  summon_pet( const std::string& name, timespan_t duration=timespan_t::zero() );
  virtual void dismiss_pet( const std::string& name );

  virtual bool ooc_buffs() { return true; }

  bool is_moving() { return buffs.raid_movement -> check() || buffs.self_movement -> check(); }

  virtual bool parse_talents_old_armory( const std::string& talent_string );
  virtual bool parse_talents_numbers( const std::string& talent_string );
  virtual bool parse_talents_armory( const std::string& talent_string );
  virtual bool parse_talents_wowhead( const std::string& talent_string );

  virtual void create_talents_wowhead();
  virtual void create_talents_armory();
  virtual void create_talents_numbers();

  void replace_spells();

  const spell_data_t* find_glyph( const std::string& name );
  const spell_data_t* find_racial_spell( const std::string& name, const std::string& token = std::string(), race_type_e s = RACE_NONE );
  const spell_data_t* find_class_spell( const std::string& name, const std::string& token = std::string(), specialization_e s = SPEC_NONE );
  const spell_data_t* find_pet_spell( const std::string& name, const std::string& token = std::string() );
  const spell_data_t* find_talent_spell( const std::string& name, const std::string& token = std::string() );
  const spell_data_t* find_glyph_spell( const std::string& name, const std::string& token = std::string() );
  const spell_data_t* find_specialization_spell( const std::string& name, const std::string& token = std::string(), specialization_e s = SPEC_NONE );
  const spell_data_t* find_mastery_spell( specialization_e s, const std::string& token = std::string(), uint32_t idx = 0 );
  const spell_data_t* find_spell( const std::string& name, const std::string& token = std::string(), specialization_e s = SPEC_NONE );
  const spell_data_t* find_spell( const unsigned int id, const std::string& token = std::string() );

  virtual expr_t* create_expression( action_t*, const std::string& name );
  expr_t* create_resource_expression( const std::string& name );

  virtual void create_options();
  void recreate_talent_str( talent_format_e format = TALENT_FORMAT_NUMBERS );
  virtual bool create_profile( std::string& profile_str, save_type_e=SAVE_ALL, bool save_html=false );


  // FIXME: Temporary "default" talent choices. (Overridden by each class/spec)
  virtual std::string set_default_talents() const { return "000000"; };
  virtual std::string set_default_glyphs()  const { return ""; };


  virtual void copy_from( player_t* source );

  virtual action_t* create_action( const std::string& name, const std::string& options );
  virtual void      create_pets() { }
  virtual pet_t*    create_pet( const std::string& /* name*/,  const std::string& /* type */ = std::string() ) { return 0; }
  virtual pet_t*    find_pet  ( const std::string& name );

  virtual int decode_set( const item_t& item ) const { ( void )item; assert( item.name() ); return SET_NONE; }

  void recalculate_haste();

  virtual void armory_extensions( const std::string& /* region */, const std::string& /* server */, const std::string& /* character */,
                                  cache::behavior_e /* behavior */=cache::players() )
  {}

  // Class-Specific Methods

  static player_t* create( sim_t* sim, const std::string& type, const std::string& name, race_type_e r = RACE_NONE );
  static player_t* create( sim_t* sim, const player_description_t& );

  // Raid-wide aura/buff/debuff maintenance
  static bool init        ( sim_t* sim );
  static void debuff_init ( sim_t* sim );
  static void combat_begin( sim_t* sim );
  static void combat_end  ( sim_t* sim );
  static void init_class_modules ( sim_t* sim );

  bool is_pet() const { return type == PLAYER_PET || type == PLAYER_GUARDIAN || type == ENEMY_ADD; }
  bool is_enemy() const { return type == ENEMY || type == ENEMY_ADD; }
  bool is_add() const { return type == ENEMY_ADD; }

  pet_t         * cast_pet         () { assert( is_pet()             ); return ( pet_t         * ) this; }

  bool      in_gcd() const { return gcd_ready > sim -> current_time; }
  bool      recent_cast() const;
  item_t*   find_item( const std::string& );
  action_t* find_action( const std::string& );
  bool      dual_wield() const { return main_hand_weapon.type != WEAPON_NONE && off_hand_weapon.type != WEAPON_NONE; }

  action_priority_list_t* find_action_priority_list( const std::string& name ) const;

  cooldown_t* find_cooldown( const std::string& name ) const;
  dot_t*      find_dot     ( const std::string& name, player_t* source ) const;
  buff_t*     find_buff    ( const std::string& name, player_t* source ) const;
  stats_t*    find_stats   ( const std::string& name );

  cooldown_t* get_cooldown( const std::string& name );
  dot_t*      get_dot     ( const std::string& name, player_t* source );
  gain_t*     get_gain    ( const std::string& name );
  proc_t*     get_proc    ( const std::string& name );
  stats_t*    get_stats   ( const std::string& name, action_t* action=0 );
  benefit_t*  get_benefit ( const std::string& name );
  uptime_t*   get_uptime  ( const std::string& name );
  rng_t*      get_rng     ( const std::string& name );
  double      get_player_distance( const player_t* p ) const;
  double      get_position_distance( double m=0, double v=0 ) const;
  action_priority_list_t* get_action_priority_list( const std::string& name );

  virtual target_data_t* create_target_data( player_t* /*target*/ ) { return NULL; }

  target_data_t* get_target_data( player_t* target )
  {
    target_data_t*& td = (target_data_t*&) target_data[ target ];
    if( ! td ) td = create_target_data( target );
    return td;
  }

  // Opportunity to perform any stat fixups before analysis
  virtual void pre_analyze_hook() {}

  /* New stuff */
  virtual double composite_player_vulnerability( school_type_e ) const;
  virtual double composite_spell_crit_vulnerability() const;
  virtual double composite_attack_crit_vulnerability() const;
  virtual double composite_ranged_attack_player_vulnerability() const;

  virtual void activate_action_list( action_priority_list_t* a );
};

// Pet ======================================================================

struct pet_t : public player_t
{
  std::string full_name_str;
  player_t* owner;
  double stamina_per_owner;
  double intellect_per_owner;
  bool summoned;
  pet_type_e pet_type;
  event_t* expiration;

private:
  void init_pet_t_();
public:
  pet_t( sim_t* sim, player_t* owner, const std::string& name, bool guardian=false );
  pet_t( sim_t* sim, player_t* owner, const std::string& name, pet_type_e pt, bool guardian=false );

  // Pets gain their owners' hit rating,
  // Also, heroic presence does not contribute to pet
  // expertise, so we use raw attack_hit.
  virtual double composite_attack_expertise( const weapon_t* ) const { return owner -> current.attack_hit + owner -> composite_attack_expertise(); }
  virtual double composite_attack_hit() const { return owner -> composite_attack_hit(); }
  virtual double composite_spell_hit() const  { return owner -> composite_spell_hit(); }
  virtual double composite_player_multiplier( school_type_e school, const action_t* a ) const;

  virtual double composite_attribute( attribute_type_e attr ) const;

  virtual void init_base();
  virtual void init_talents();
  virtual void init_target();
  virtual void reset();
  virtual void summon( timespan_t duration=timespan_t::zero() );
  virtual void dismiss();
  virtual bool ooc_buffs() { return false; }
  virtual double assess_damage( double amount, school_type_e, dmg_type_e, result_type_e, action_t* a=0 );
  virtual void combat_begin();

  virtual const char* name() const { return full_name_str.c_str(); }

  const spell_data_t* find_pet_spell( const std::string& name, const std::string& token = std::string() );
};

// Gain =====================================================================

struct gain_t
{
  std::array<double, RESOURCE_MAX> actual, overflow, count;

  const std::string name_str;
  gain_t* next;

  gain_t( const std::string& n ) :
    name_str( n ), next( NULL )
  {
    range::fill( actual, 0.0 );
    range::fill( overflow, 0.0 );
    range::fill( count, 0.0 );
  }
  void add( resource_type_e rt, double amount, double overflow_=0 )
  { actual[ rt ] += amount; overflow[ rt ] += overflow_; count[ rt ]++; }
  void merge( const gain_t& other )
  {
    for ( size_t i = 0; i < RESOURCE_MAX; i++ )
    { actual[ i ] += other.actual[ i ]; overflow[ i ] += other.overflow[ i ]; count[ i ] += other.count[ i ]; }
  }
  void analyze( const sim_t* sim )
  {
    for ( size_t i = 0; i < RESOURCE_MAX; i++ )
    { actual[ i ] /= sim -> iterations; overflow[ i ] /= sim -> iterations; count[ i ] /= sim -> iterations; }
  }
  const char* name() const { return name_str.c_str(); }
};

// Stats ====================================================================

struct stats_t
{
  const std::string name_str;
  sim_t* const sim;
  player_t* const player;
  stats_t* parent;
  // We should make school and type const or const-like, and either stricly define when, where and who defines the values,
  // or make sure that it is equal to the value of all it's actions.
  school_type_e school;
  stats_type_e type;

  std::vector<action_t*> action_list;
  gain_t resource_gain;
  // Flags
  bool analyzed;
  bool quiet;
  bool background;

  // Variables used both during combat and for reporting
  double num_executes, num_ticks;
  double num_direct_results, num_tick_results;
  timespan_t total_execute_time, total_tick_time;
  double portion_amount;
  sample_data_t total_intervals;
  timespan_t last_execute;
  double iteration_actual_amount, iteration_total_amount;
  sample_data_t actual_amount, total_amount, portion_aps, portion_apse;
  std::vector<stats_t*> children;
  double compound_actual;
  double opportunity_cost;

  struct stats_results_t
  {
    sample_data_t actual_amount, total_amount,fight_actual_amount, fight_total_amount,count,avg_actual_amount;
    int iteration_count;
    double iteration_actual_amount, iteration_total_amount,pct, overkill_pct;

    stats_results_t( sim_t* );
    void analyze( double num_results );
    void merge( const stats_results_t& other );
    void combat_end();
  };
  std::vector<stats_results_t> direct_results;
  std::vector<stats_results_t>   tick_results;

  std::vector<double> timeline_amount;

  // Reporting only
  std::array<double,RESOURCE_MAX> resource_portion, apr, rpe;
  double rpe_sum, compound_amount, overkill_pct;
  double aps, ape, apet, etpe, ttpt;
  timespan_t total_time;
  std::string aps_distribution_chart;

  std::string timeline_aps_chart;

  // Scale factor container
  gear_stats_t scaling;
  gear_stats_t scaling_error;

  stats_t( const std::string& name, player_t* );

  void add_child( stats_t* child );
  void consume_resource( const resource_type_e resource_type, const double resource_amount );
  void add_result( const double act_amount, const double tot_amount, const dmg_type_e dmg_type, const result_type_e result );
  void add_tick   ( timespan_t time );
  void add_execute( timespan_t time );
  void combat_begin();
  void combat_end();
  void reset();
  void analyze();
  void merge( const stats_t* other );
};

// Action ===================================================================

struct action_t
{
  const spell_data_t* s_data;
  sim_t* const sim;
  const action_type_e type;
  std::string name_str;
  player_t* const player;
  player_t* target;
  school_type_e school;

  uint32_t id;
  result_type_e result;
  resource_type_e resource_current;
  int aoe, pre_combat;
  bool dual, callbacks, special, channeled, background, sequence, use_off_gcd;
  bool direct_tick, repeating, harmful, proc, item_proc, proc_ignores_slot;
  bool may_trigger_dtr, discharge_proc, auto_cast, initialized;
  bool may_hit, may_miss, may_dodge, may_parry, may_glance, may_block, may_crush, may_crit;
  bool tick_may_crit, tick_zero, hasted_ticks;
  bool no_buffs, no_debuffs;
  bool stateless;
  dot_behavior_type_e dot_behavior;
  timespan_t ability_lag, ability_lag_stddev;
  double rp_gain;
  timespan_t min_gcd, trigger_gcd;
  double range;
  double weapon_power_mod, direct_power_mod, tick_power_mod;
  timespan_t base_execute_time;
  timespan_t base_tick_time;
  std::array< double, RESOURCE_MAX > base_costs;
  double base_dd_min, base_dd_max, base_td, base_td_init;
  double   base_dd_multiplier,   base_td_multiplier;
  double player_dd_multiplier, player_td_multiplier;
  double   base_multiplier,   base_hit,   base_crit;
  double player_multiplier, player_hit, player_crit;
  double target_multiplier, target_hit, target_crit;
  double   base_spell_power,   base_attack_power;
  double player_spell_power, player_attack_power;
  double target_spell_power, target_attack_power;
  double   base_spell_power_multiplier,   base_attack_power_multiplier;
  double player_spell_power_multiplier, player_attack_power_multiplier;
  double crit_multiplier, crit_bonus_multiplier, crit_bonus;
  double base_dd_adder, player_dd_adder, target_dd_adder;
  double base_ta_adder;
  double player_haste;
  double direct_dmg, tick_dmg;
  double snapshot_crit, snapshot_haste, snapshot_mastery;
  int num_ticks;
  weapon_t* weapon;
  double weapon_multiplier;
  double base_add_multiplier;
  double base_aoe_multiplier; // Static reduction of damage for AoE
  bool normalize_weapon_speed;
  rng_t* rng_result;
  rng_t* rng_travel;
  cooldown_t* cooldown;
  stats_t* stats;
  event_t* execute_event;
  event_t* travel_event;
  timespan_t time_to_execute, time_to_travel;
  double travel_speed, resource_consumed;
  int moving, wait_on_ready, interrupt, chain, cycle_targets;
  bool round_base_dmg;
  bool class_flag1;
  std::string if_expr_str;
  expr_t* if_expr;
  std::string interrupt_if_expr_str;
  expr_t* interrupt_if_expr;
  std::string sync_str;
  action_t* sync_action;
  char marker;
  std::string signature_str;
  std::string target_str;
  std::string label_str;
  timespan_t last_reaction_time;
  action_t* dtr_action;
  bool is_dtr_action;
  bool can_trigger_dtr;
  target_specific_t target_specific_dot;
  std::string action_list;

  action_t( action_type_e type, const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );
  virtual ~action_t();
  void init_dot( const std::string& dot_name );

  const spell_data_t& data() const { return ( *s_data ); }
  void   parse_spell_data( const spell_data_t& );
  void   parse_effect_data( const spelleffect_data_t& );

  virtual void   parse_options( option_t*, const std::string& options_str );
  virtual double cost() const;
  virtual double total_haste() const  { return haste(); }
  virtual double haste() const        { return 1.0; }
  virtual timespan_t gcd() const;
  virtual timespan_t execute_time() const { return base_execute_time; }
  virtual timespan_t tick_time( double haste ) const;
  virtual int    hasted_num_ticks( double haste, timespan_t d=timespan_t::min() ) const;
  virtual timespan_t travel_time() const;
  virtual void   player_buff();
  virtual void   player_tick() {}
  virtual void   target_debuff( player_t* t, dmg_type_e );
  virtual void   snapshot();
  virtual result_type_e calculate_result( double /* crit */, unsigned /* target_level */ ) { assert( false ); return RESULT_UNKNOWN; }
  virtual bool   result_is_hit ( result_type_e = RESULT_UNKNOWN ) const;
  virtual bool   result_is_miss( result_type_e = RESULT_UNKNOWN ) const;
  virtual double calculate_direct_damage( result_type_e, int chain_target, unsigned target_level,
                                          double attack_power,  double spell_power, double multiplier );
  virtual double calculate_tick_damage( result_type_e, double power, double multiplier );
  virtual double calculate_weapon_damage( double attack_power );
  virtual double armor() const;
  virtual double resistance() const;
  virtual void   consume_resource();
  virtual resource_type_e current_resource() const
  { return resource_current; }
  virtual void   execute();
  virtual void   tick( dot_t* d );
  virtual void   last_tick( dot_t* d );
  virtual void   impact( player_t*, result_type_e result, double dmg );
  virtual void   assess_damage( player_t* t, double amount, dmg_type_e, result_type_e impact_result );
  virtual void   additional_damage( player_t* t, double amount, dmg_type_e, result_type_e impact_result );
  virtual void   schedule_execute();
  virtual void   schedule_travel( player_t* t );
  virtual void   reschedule_execute( timespan_t time );
  virtual void   update_ready();
  virtual bool   usable_moving();
  virtual bool   ready();
  virtual void   init();
  virtual void   reset();
  virtual void   cancel();
  virtual void   interrupt_action();
  void   check_talent( int talent_rank );
  void   check_spec( specialization_e );
  void   check_race( race_type_e );
  void   check_spell( const spell_data_t* );
  const char* name() const { return name_str.c_str(); }

  virtual double   miss_chance( double /* hit */, int /* delta_level */ ) const { return 0; }
  virtual double  dodge_chance( double /* expertise */, int /* delta_level */ ) const { return 0; }
  virtual double  parry_chance( double /* expertise */, int /* delta_level */ ) const { return 0; }
  virtual double glance_chance( int /* delta_level */ ) const { return 0; }
  virtual double  block_chance( int /* delta_level */ ) const { return 0; }
  virtual double   crit_chance( double /* crit */, int /* delta_level */ ) const { return 0; }

  virtual double total_multiplier() const { return   base_multiplier * player_multiplier * target_multiplier; }
  virtual double total_crit() const       { return   base_crit       + player_crit       + target_crit;       }
  virtual double total_crit_bonus() const;

  virtual double total_spell_power() const  { return util::round( ( base_spell_power  + player_spell_power  + target_spell_power  ) * base_spell_power_multiplier  * player_spell_power_multiplier  ); }
  virtual double total_attack_power() const { return util::round( ( base_attack_power + player_attack_power + target_attack_power ) * base_attack_power_multiplier * player_attack_power_multiplier ); }
  virtual double total_power() const;

  // Some actions require different multipliers for the "direct" and "tick" portions.

  virtual double total_dd_multiplier() const { return total_multiplier() * base_dd_multiplier * player_dd_multiplier; }
  virtual double total_td_multiplier() const { return total_multiplier() * base_td_multiplier * player_td_multiplier; }

  virtual double bonus_damage() const { return base_dd_adder; }

  virtual expr_t* create_expression( const std::string& name );

  virtual double ppm_proc_chance( double PPM );

  dot_t* find_dot( player_t* t = 0 )
  {
    if( ! t ) t = target;
    return (dot_t*) target_specific_dot[ t ];
  }

  dot_t* get_dot( player_t* t = 0 )
  {
    if( ! t ) t = target;
    dot_t*& dot = (dot_t*&) target_specific_dot[ t ];
    if( ! dot ) dot = t -> get_dot( name_str, player );
    return dot;
  }

  void add_child( action_t* child ) { stats -> add_child( child -> stats ); }

  target_data_t* target_data( player_t* t=0 ) const { return player -> get_target_data( t ? t : target ); }

  virtual size_t available_targets( std::vector< player_t* >& ) const;
  virtual std::vector< player_t* > target_list() const;

  /* New stuff */
  action_state_t* state_cache;
  action_state_t* execute_state; /* State of the last execute() */
  uint32_t snapshot_flags;
  uint32_t update_flags;

  virtual action_state_t* new_state();
  virtual action_state_t* get_state( const action_state_t* = 0 );
  virtual void release_state( action_state_t* );
  virtual void schedule_travel_s( action_state_t* );
  virtual void impact_s( action_state_t* );

  virtual void   snapshot_state( action_state_t*, uint32_t );

  virtual double action_multiplier() const { return base_multiplier; }
  virtual double action_da_multiplier() const { return base_dd_multiplier; }
  virtual double action_ta_multiplier() const { return base_td_multiplier; }

  virtual double composite_hit() const { return base_hit; }
  virtual double composite_expertise() const { return 0.0; }
  virtual double composite_crit() const { return base_crit; }
  virtual double composite_haste() const { return 1.0; }
  virtual double composite_attack_power() const { return base_attack_power + player -> composite_attack_power(); }
  virtual double composite_attack_power_multiplier() const { return base_attack_power_multiplier * player -> composite_attack_power_multiplier(); }
  virtual double composite_spell_power() const { return base_spell_power + player -> composite_spell_power( school ); }
  virtual double composite_spell_power_multiplier() const { return base_spell_power_multiplier * player -> composite_spell_power_multiplier(); }
  virtual double composite_target_crit( player_t* /* target */ ) const { return 0.0; }
  virtual double composite_target_multiplier( player_t* target ) const { return target -> composite_player_vulnerability( school ); }
  virtual double composite_target_da_multiplier( player_t* target ) const { return composite_target_multiplier( target ); }
  virtual double composite_target_ta_multiplier( player_t* target ) const { return composite_target_multiplier( target ); }
  virtual double composite_da_multiplier() const
  {
    return action_multiplier() * action_da_multiplier() *
           player -> composite_player_multiplier( school, this ) *
           player -> composite_player_dd_multiplier( school, this );
  }
  virtual double composite_ta_multiplier() const
  {
    return action_multiplier() * action_ta_multiplier() *
           player -> composite_player_multiplier( school, this ) *
           player -> composite_player_td_multiplier( school, this );
  }
};

struct action_state_t
{
  // Source action, target actor
  action_t*       action;
  player_t*       target;
  // Results
  result_type_e   result;
  double          result_amount;
  // Snapshotted stats during execution
  double          haste;
  double          crit;
  double          target_crit;
  double          attack_power;
  double          spell_power;
  // Multipliers
  double          da_multiplier;
  double          ta_multiplier;
  double          target_da_multiplier;
  double          target_ta_multiplier;
  // Cache pointer
  action_state_t* next;

  action_state_t( action_t*, player_t* );
  virtual ~action_state_t() {};

  virtual void copy_state( const action_state_t* );

  virtual void debug() const;

  virtual double composite_crit() const
  {
    return crit + target_crit;
  }

  virtual double composite_power() const
  {
    return attack_power + spell_power;
  }

  virtual double composite_da_multiplier() const
  {
    return da_multiplier * target_da_multiplier;
  }

  virtual double composite_ta_multiplier() const
  {
    return ta_multiplier * target_ta_multiplier;
  }
};

// Attack ===================================================================

struct attack_t : public action_t
{
  attack_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Attack Overrides
  virtual double haste() const;
  virtual double total_haste() const { return swing_haste(); }
  virtual double swing_haste() const;
  virtual timespan_t execute_time() const;
  virtual void execute();
  virtual void   player_buff();
  int build_table( std::array<double,RESULT_MAX>& chances,
                   std::array<result_type_e,RESULT_MAX>& results,
                   unsigned level,
                   double /* attack_crit */ );
  virtual result_type_e calculate_result( double, unsigned );
  virtual void   init();

  virtual double   miss_chance( double hit, int delta_level ) const;
  virtual double  block_chance( int /* delta_level */ ) const { return 0.0; }
  virtual double  crit_block_chance( int delta_level ) const;
  virtual double   crit_chance( double crit, int delta_level ) const;

  /* New stuffs */
  virtual double composite_hit() const
  { return action_t::composite_hit() + player -> composite_attack_hit(); }
  virtual double composite_crit() const
  { return action_t::composite_crit() + player -> composite_attack_crit( weapon ); }
  virtual double composite_haste() const
  { return action_t::composite_haste() * player -> composite_attack_haste(); }
};

// Melee Attack ===================================================================

struct melee_attack_t : public attack_t
{
  double base_expertise, player_expertise, target_expertise;

  melee_attack_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Melee Attack Overrides
  virtual void   player_buff();
  virtual void   target_debuff( player_t* t, dmg_type_e );
  virtual double total_expertise() const;

  virtual double  dodge_chance( double /* expertise */, int delta_level ) const;
  virtual double  parry_chance( double /* expertise */, int delta_level ) const;
  virtual double glance_chance( int delta_level ) const;

  /* New stuffs */
  virtual double composite_expertise() const { return attack_t::composite_expertise() + player -> composite_attack_expertise( weapon ); }
};

// Ranged Attack ===================================================================

struct ranged_attack_t : public attack_t
{
  double base_expertise, player_expertise, target_expertise;

  ranged_attack_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Ranged Attack Overrides
  virtual void   player_buff();
  virtual void   target_debuff( player_t* t, dmg_type_e );
  virtual double total_expertise() const;

  virtual double  dodge_chance( double /* expertise */, int delta_level ) const;
  virtual double  parry_chance( double /* expertise */, int delta_level ) const;
  virtual double glance_chance( int delta_level ) const;
  virtual double composite_target_multiplier( player_t* target ) const { return target -> composite_ranged_attack_player_vulnerability(); }

  /* New stuffs */
  virtual double composite_expertise() const { return attack_t::composite_expertise() + player -> composite_attack_expertise( weapon ); }
};

// Spell Base ====================================================================

struct spell_base_t : public action_t
{
  spell_base_t( action_type_e at, const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );
  // Spell Overrides
  virtual double haste() const;
  virtual timespan_t gcd() const;
  virtual timespan_t execute_time() const;
  virtual void   player_buff();
  virtual result_type_e   calculate_result( double, unsigned );
  virtual void   execute();
  virtual double crit_chance( double crit, int delta_level ) const;
  virtual void   schedule_execute();
  virtual void   init();

  /* New stuffs */
  virtual double composite_crit() const
  { return action_t::composite_crit() + player -> composite_spell_crit(); }
  virtual double composite_haste() const { return action_t::composite_haste() * player -> composite_spell_haste(); }
};

// Harmful Spell ====================================================================

struct spell_t : public spell_base_t
{
public:
  spell_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Harmful Spell Overrides
  virtual void   player_buff();
  virtual void   target_debuff( player_t* t, dmg_type_e );
  virtual void   execute();
  virtual double miss_chance( double hit, int delta_level ) const;

  /* New stuffs */
  virtual double composite_hit() const { return action_t::composite_hit() + player -> composite_spell_hit(); }
};

// Heal =====================================================================

struct heal_t : public spell_base_t
{
  bool group_only;

  heal_t( const std::string& name, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  virtual void player_buff();
  virtual void execute();
  virtual void assess_damage( player_t* t, double amount, dmg_type_e, result_type_e );
  player_t* find_greatest_difference_player();
  player_t* find_lowest_player();
  virtual size_t available_targets( std::vector< player_t* >& ) const;
};

// Absorb ===================================================================

struct absorb_t : public spell_base_t
{
  absorb_t( const std::string& name, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  virtual void player_buff();
  virtual void execute();
  virtual void assess_damage( player_t* t, double amount,
                              dmg_type_e, result_type_e impact_result );
  virtual void impact_s( action_state_t* );
};

// Sequence =================================================================

struct sequence_t : public action_t
{
  std::vector<action_t*> sub_actions;
  int current_action;
  bool restarted;
  timespan_t last_restart;

  sequence_t( player_t*, const std::string& sub_action_str );

  virtual void schedule_execute();
  virtual void reset();
  virtual bool ready();
  void restart() { current_action = 0; restarted = true; last_restart = sim -> current_time; }
  bool can_restart()
  { return ! restarted || last_restart + timespan_t::from_millis( 1 ) < sim -> current_time; }
};

// Cooldown =================================================================

struct cooldown_t
{
  sim_t* const sim;
  player_t* const player;
  const std::string name_str;
  timespan_t duration;
  timespan_t ready;
  timespan_t reset_react;
  cooldown_t* next;
  int charges;
  int current_charge;

  struct recharge_event_t : event_t
  {
    cooldown_t* cooldown;

    recharge_event_t( player_t* p, cooldown_t* cd, timespan_t delay = timespan_t::zero() ) :
      event_t( p -> sim, p, ( cd -> name_str + "_recharge" ).c_str() ), cooldown( cd )
    {
      sim -> add_event( this, cd -> duration + delay );
    }

    virtual void execute()
    {
      assert( cooldown -> current_charge < cooldown -> charges );
      cooldown -> current_charge++;

      if ( cooldown -> current_charge < cooldown -> charges )
      {
        cooldown -> recharge_event = new ( sim ) recharge_event_t( player, cooldown );
      }
      else
      {
        cooldown -> recharge_event = 0;
      }
    }
  };

  recharge_event_t* recharge_event;
  cooldown_t( const std::string& n, player_t* p ) : sim( p->sim ), player( p ), name_str( n ), duration( timespan_t::zero() ), ready( ready_init() ), reset_react( timespan_t::zero() )
                                                                 , next( 0 ), charges( 1 ), current_charge( 1 ), recharge_event( 0 ) {}
  cooldown_t( const std::string& n, sim_t* s )    : sim( s ),      player( 0 ), name_str( n ), duration( timespan_t::zero() ), ready( ready_init() ), reset_react( timespan_t::zero() )
                                                                 , next( 0 ), charges( 1 ), current_charge( 1 ), recharge_event( 0 ) {}

  void adjust( timespan_t amount )
  {
    ready += amount;

    if ( remains() == timespan_t::zero() )
      reset( true );
  }
  void reset( bool early = false )
  {
    ready = ready_init();
    current_charge = charges;
    if ( early )
      reset_react = sim -> current_time + player -> total_reaction_time();
    else
      reset_react = timespan_t::zero();
    event_t::cancel( recharge_event );
  }
  void start( timespan_t override=timespan_t::min(), timespan_t delay=timespan_t::zero() )
  {
    reset_react = timespan_t::zero();
    if ( override >= timespan_t::zero() ) duration = override;
    if ( duration > timespan_t::zero() )
    {
      if ( charges > 1 )
      {
        assert( current_charge > 0 );
        current_charge--;

        if ( current_charge == charges - 1 )
        {
          recharge_event = new ( sim ) recharge_event_t( player, this, delay );
        }
        else if ( current_charge == 0 )
        {
          assert( recharge_event );
          ready = recharge_event -> time + timespan_t::from_millis( 1 );
        }
      }
      else
      {
        ready = sim -> current_time + duration + delay;
      }
    }
  }
  timespan_t remains() const
  {
    timespan_t diff = ready - sim -> current_time;
    if ( diff < timespan_t::zero() ) diff = timespan_t::zero();
    return diff;
  }
  const char* name() const { return name_str.c_str(); }

private:
  static timespan_t ready_init()
  { return timespan_t::from_seconds( -60 * 60 ); }
};

// DoT ======================================================================

struct dot_t
{
  sim_t* const sim;
  player_t* target;
  player_t* source;
  action_t* action;
  event_t* tick_event;
  int num_ticks, current_tick, added_ticks, ticking;
  timespan_t added_seconds;
  timespan_t ready;
  timespan_t miss_time;
  timespan_t time_to_tick;
  const std::string name_str;
  double prev_tick_amount;
  /* New stuff */
  action_state_t* state;

  dot_t( const std::string& n, player_t* target, player_t* source );

  void   cancel();
  void   extend_duration( int extra_ticks, bool cap=false );
  void   extend_duration_seconds( timespan_t extra_seconds );
  void   recalculate_ready();
  void   refresh_duration();
  void   reset();
  timespan_t remains() const;
  void   schedule_tick();
  int    ticks() const;

  expr_t* create_expression( action_t* action, const std::string& name_str, bool dynamic );

  const char* name() const { return name_str.c_str(); }
  /* New stuff */
  void schedule_new_tick();

};

// Action Callback ==========================================================

struct action_callback_t
{
  player_t* listener;
  bool active;
  bool allow_self_procs;
  bool allow_item_procs;
  bool allow_procs;

  action_callback_t( player_t* l, bool ap=false, bool aip=false, bool asp=false ) :
    listener( l ), active( true ), allow_self_procs( asp ), allow_item_procs( aip ), allow_procs( ap )
  {
    assert( l );
    l -> callbacks.all_callbacks.push_back( this );
  }
  virtual ~action_callback_t() {}
  virtual void trigger( action_t*, void* call_data=0 ) = 0;
  virtual void reset() {}
  virtual void activate() { active=true; }
  virtual void deactivate() { active=false; }

  static void trigger( const std::vector<action_callback_t*>& v, action_t* a, void* call_data=0 )
  {
    if ( a && ! a -> player -> in_combat ) return;

    std::size_t size = v.size();
    for ( std::size_t i=0; i < size; i++ )
    {
      action_callback_t* cb = v[ i ];
      if ( cb -> active )
      {
        if ( ! cb -> allow_item_procs && a && a -> item_proc ) return;
        if ( ! cb -> allow_procs && a && a -> proc ) return;
        cb -> trigger( a, call_data );
      }
    }
  }

  static void reset( const std::vector<action_callback_t*>& v )
  {
    std::size_t size = v.size();
    for ( std::size_t i=0; i < size; i++ )
    {
      v[ i ] -> reset();
    }
  }
};

// Action Priority List =====================================================

struct action_priority_list_t
{
  const std::string name_str;
  std::string action_list_str;
  player_t* const player;
  bool used;
  std::vector<action_t*> foreground_action_list;
  std::vector<action_t*> off_gcd_actions;
  action_priority_list_t( std::string name, player_t* p ) : name_str( name ), player( p ), used( false ),
    foreground_action_list( 0 ), off_gcd_actions( 0 )
  {}
};

// Player Ready Event =======================================================

struct player_ready_event_t : public event_t
{
  player_ready_event_t( sim_t* sim, player_t* p, timespan_t delta_time );
  virtual void execute();
};

struct player_gcd_event_t : public event_t
{
  player_gcd_event_t( sim_t* sim, player_t* p, timespan_t delta_time );
  virtual void execute();
};

// Action Execute Event =====================================================

struct action_execute_event_t : public event_t
{
  action_t* const action;
  action_execute_event_t( sim_t* sim, action_t* a, timespan_t time_to_execute );
  virtual void execute();
};

// DoT Tick Event ===========================================================

struct dot_tick_event_t : public event_t
{
  dot_t* const dot;
  dot_tick_event_t( sim_t* sim, dot_t* d, timespan_t time_to_tick );
  virtual void execute();
};

// Action Travel Event ======================================================

struct action_travel_event_t : public event_t
{
  action_t* const action;
  player_t* const target;
  result_type_e result;
  double damage;
  action_travel_event_t( sim_t* sim, player_t* t, action_t* a, timespan_t time_to_travel );
  virtual void execute();
};

struct stateless_travel_event_t : public event_t
{
  action_t* const action;
  action_state_t* const state;
  stateless_travel_event_t( sim_t* sim, action_t* a, action_state_t* state, timespan_t time_to_travel );
  virtual ~stateless_travel_event_t() { if ( unlikely( state && canceled ) ) action -> release_state( state ); }
  virtual void execute();
};

// Regen Event ==============================================================

struct regen_event_t : public event_t
{
  regen_event_t( sim_t* sim );
  virtual void execute();
};

// Unique Gear ==============================================================

struct unique_gear_t
{
  static void init( player_t* );

  static action_callback_t* register_stat_proc( proc_type_e, int64_t mask, const std::string& name, player_t*,
                                                stat_type_e, int max_stacks, double amount,
                                                double proc_chance, timespan_t duration, timespan_t cooldown,
                                                timespan_t tick=timespan_t::zero(), bool reverse=false );

  static action_callback_t* register_cost_reduction_proc( proc_type_e, int64_t mask, const std::string& name, player_t*,
                                                          school_type_e, int max_stacks, double amount,
                                                          double proc_chance, timespan_t duration, timespan_t cooldown,
                                                          bool refreshes=false, bool reverse=false );

  static action_callback_t* register_discharge_proc( proc_type_e, int64_t mask, const std::string& name, player_t*,
                                                     int max_stacks, school_type_e, double amount, double scaling,
                                                     double proc_chance, timespan_t cooldown, bool no_buffs, bool no_debuffs, int aoe,
                                                     unsigned int override_result_type_es_mask = 0, unsigned int results_types_mask = 0 );

  static action_callback_t* register_chance_discharge_proc( proc_type_e, int64_t mask, const std::string& name, player_t*,
                                                            int max_stacks, school_type_e, double amount, double scaling,
                                                            double proc_chance, timespan_t cooldown, bool no_buffs, bool no_debuffs, int aoe,
                                                            unsigned int override_result_type_es_mask = 0, unsigned int results_types_mask = 0 );

  static action_callback_t* register_stat_discharge_proc( proc_type_e, int64_t mask, const std::string& name, player_t*,
                                                          int max_stacks, stat_type_e, double stat_amount,
                                                          school_type_e, double discharge_amount, double discharge_scaling,
                                                          double proc_chance, timespan_t duration, timespan_t cooldown, bool no_buffs, bool no_debuffs, int aoe,
                                                          unsigned int override_result_type_es_mask = 0, unsigned int results_types_mask = 0 );

  static action_callback_t* register_stat_proc( item_t&, item_t::special_effect_t& );
  static action_callback_t* register_cost_reduction_proc( item_t&, item_t::special_effect_t& );
  static action_callback_t* register_discharge_proc( item_t&, item_t::special_effect_t& );
  static action_callback_t* register_chance_discharge_proc( item_t&, item_t::special_effect_t& );
  static action_callback_t* register_stat_discharge_proc( item_t&, item_t::special_effect_t& );

  static bool get_equip_encoding( std::string& encoding,
                                  const std::string& item_name,
                                  const bool         item_heroic,
                                  const bool         item_lfr,
                                  const bool         ptr,
                                  const std::string& item_id=std::string() );

  static bool get_use_encoding  ( std::string& encoding,
                                  const std::string& item_name,
                                  const bool         heroic,
                                  const bool         lfr,
                                  const bool         ptr,
                                  const std::string& item_id=std::string() );
};

// Enchants =================================================================

struct enchant_t
{
  static void init( player_t* );
  static bool get_encoding        ( std::string& name, std::string& encoding, const std::string& enchant_id, const bool ptr );
  static bool get_addon_encoding  ( std::string& name, std::string& encoding, const std::string& addon_id, const bool ptr );
  static bool get_reforge_encoding( std::string& name, std::string& encoding, const std::string& reforge_id );
  static int  get_reforge_id      ( stat_type_e stat_from, stat_type_e stat_to );
  static bool download        ( item_t&, const std::string& enchant_id );
  static bool download_addon  ( item_t&, const std::string& addon_id   );
  static bool download_reforge( item_t&, const std::string& reforge_id );
  static bool download_rsuffix( item_t&, const std::string& rsuffix_id );
};

// Consumable ===============================================================

namespace consumable {
  action_t* create_action( player_t*, const std::string& name, const std::string& options );
}

// Benefit ==================================================================

struct benefit_t : public noncopyable
{
private:
  int up, down;
public:
  double ratio;
  benefit_t* next;
  const std::string name_str;

  explicit benefit_t( const std::string& n ) :
    up( 0 ), down( 0 ),
    ratio( 0.0 ), name_str( n ) {}

  void update( int is_up ) { if ( is_up ) up++; else down++; }

  const char* name() const { return name_str.c_str(); }

  void analyze()
  {
    if ( up != 0 )
      ratio = 1.0 * up / ( down + up );
  }

  void merge( const benefit_t* other )
  { up += other -> up; down += other -> down; }
};

// Uptime ==================================================================

struct uptime_common_t
{
  timespan_t last_start;
  timespan_t iteration_uptime_sum;
  sample_data_t uptime_sum;
  sim_t* const sim;

  uptime_common_t( sim_t* s ) :
    last_start( timespan_t::min() ), iteration_uptime_sum( timespan_t::zero() ), uptime_sum( s -> statistics_level < 6 ), sim( s )
  {}

  void update( bool is_up )
  {
    if ( is_up )
    {
      if ( last_start < timespan_t::zero() )
        last_start = sim -> current_time;
    }
    else if ( last_start >= timespan_t::zero() )
    {
      iteration_uptime_sum += sim -> current_time - last_start;
      last_start = timespan_t::min();
    }
  }

  void combat_end()
  {
    uptime_sum.add( sim->current_time.total_seconds() ? iteration_uptime_sum.total_seconds() / sim->current_time.total_seconds() : 0.0 );
    iteration_uptime_sum = timespan_t::zero();
  }

  void reset() { last_start = timespan_t::min(); }

  void analyze()
  { uptime_sum.analyze(); }

  void merge( const uptime_common_t& other )
  { uptime_sum.merge( other.uptime_sum ); }
};

struct uptime_t : public uptime_common_t
{
  const std::string name_str;
  uptime_t* next;

  uptime_t( sim_t* s, const std::string& n ) :
    uptime_common_t( s ), name_str( n )
  {}

  const char* name() const { return name_str.c_str(); }
};

struct buff_uptime_t : public uptime_common_t
{ buff_uptime_t( sim_t* s ) : uptime_common_t( s ) {} };

// Proc =====================================================================

struct proc_t
{
  sim_t* const sim;
  player_t* const player;
  const std::string name_str;

  double count;
  timespan_t last_proc;
  sample_data_t interval_sum;
  proc_t* next;

  proc_t( sim_t* s, player_t* p, const std::string& n ) :
    sim( s ), player( p ), name_str( n ),
    count( 0.0 ), last_proc( timespan_t::zero() ), interval_sum( s -> statistics_level < 6 ),
    next( NULL )
  {}

  void occur()
  {
    count++;
    if ( last_proc > timespan_t::zero() && last_proc < sim -> current_time )
    {
      interval_sum.add( sim -> current_time.total_seconds() - last_proc.total_seconds() );
    }
    last_proc = sim -> current_time;
  }

  void merge( const proc_t& other )
  {
    count          += other.count;
    interval_sum.merge( other.interval_sum );
  }

  void analyze()
  {
    count /= sim -> iterations;
    interval_sum.analyze();
  }

  const char* name() const { return name_str.c_str(); }
};

// Report ===================================================================

struct report_t
{
  // In the end, the idea is to only have print_suite remaining, and completly separate the report from simulationcraft.hpp
  static void print_spell_query( sim_t*, unsigned level = MAX_LEVEL );
  static void print_profiles( sim_t* );
  static void print_text( FILE*, sim_t*, bool detail=true );

  static void print_suite( sim_t* );
};

// Log ======================================================================

struct log_t
{
  // Generic Output
  static void output( sim_t*, const char* format, ... ) PRINTF_ATTRIBUTE( 2,3 );

  // Combat Log (unsupported)
};

// Pseudo Random Number Generation ==========================================

struct rng_t
{
private:
  const std::string _name_str;
  rng_type_e _type;
  double actual_roll, actual_range, actual_gauss;
  uint64_t num_roll, num_range, num_gauss;
  double gauss_pair_value;
  bool   gauss_pair_use;
public:
  rng_t* next;
  // FIXME: Change rng-creation so that average_range and average_gauss can be protected again.
protected:
  rng_t( const std::string& n, rng_type_e );

public:
  virtual ~rng_t() {}

private:
  virtual double  _real() = 0;
  virtual void    _seed( uint32_t start ) = 0;
public:

  const std::string& name_str() const { return _name_str; }
  const char* name() const { return _name_str.c_str(); }
  void seed( uint32_t start = time( NULL ) ) { _seed( start ); }
  double real() { return _real(); }
  rng_type_e type() const { return _type; }
  bool    roll( double chance );
  double range( double min, double max );
  double gauss( double mean, double stddev, const bool truncate_low_end = false );
  double exgauss( double mean, double stddev, double nu_divisor, double nu_multiplier, double cutoff );
  std::string report( double confidence_estimator ) const;

  timespan_t range( timespan_t min, timespan_t max )
  {
    return timespan_t::from_native( range( ( double ) timespan_t::to_native( min ),
                                           ( double ) timespan_t::to_native( max ) ) );
  }

  timespan_t gauss( timespan_t mean, timespan_t stddev )
  {
    return timespan_t::from_native( gauss( ( double ) timespan_t::to_native( mean ),
                                           ( double ) timespan_t::to_native( stddev ) ) );
  }

  timespan_t exgauss( timespan_t mean, timespan_t stddev, timespan_t nu )
  {
    return timespan_t::from_seconds(
        exgauss( mean.total_seconds(), stddev.total_seconds(), nu.total_seconds(), nu.total_seconds(), 5.0 ) );
  }

  static double stdnormal_cdf( double u );
  static double stdnormal_inv( double p );

  static rng_t* create( const std::string& name, rng_type_e=RNG_STANDARD );
};

// String utils =============================================================

std::string tolower( const std::string& src );
std::string proper_option_name( const std::string& full_name );
#if 0 // UNUSED
std::string trim( const std::string& src );
void replace_char( std::string& str, char old_c, char new_c  );
void replace_str( std::string& str, const std::string& old_str, const std::string& new_str  );
bool str_to_float( const std::string& src, double& dest );
#endif // UNUSED

// Wowhead  =================================================================

namespace wowhead {
  player_t* download_player( sim_t* sim,
                                    const std::string& region,
                                    const std::string& server,
                                    const std::string& name,
                                    const std::string& spec,
                                    cache::behavior_e b=cache::players() );
  bool download_slot( item_t&,
                             const std::string& item_id,
                             const std::string& enchant_id,
                             const std::string& addon_id,
                             const std::string& reforge_id,
                             const std::string& rsuffix_id,
                             const std::string gem_ids[ 3 ],
                             bool ptr=false,
                             cache::behavior_e b=cache::items() );
  bool download_item( item_t&, const std::string& item_id,
                             bool ptr=false, cache::behavior_e b=cache::items() );
  bool download_glyph( player_t* player, std::string& glyph_name, const std::string& glyph_id,
                              bool ptr=false, cache::behavior_e b=cache::items() );
  gem_type_e parse_gem( item_t& item, const std::string& gem_id,
                               bool ptr=false, cache::behavior_e b=cache::items() );
}

// CharDev  =================================================================

namespace chardev {

  player_t* download_player( sim_t* sim, const std::string& id, cache::behavior_e b=cache::players() );
}

// MMO Champion =============================================================

struct mmo_champion_t
{
  static bool download_slot( item_t&,
                             const std::string& item_id,
                             const std::string& enchant_id,
                             const std::string& addon_id,
                             const std::string& reforge_id,
                             const std::string& rsuffix_id,
                             const std::string gem_ids[ 3 ],
                             cache::behavior_e b=cache::items() );
  static bool download_item( item_t&, const std::string& item_id,
                             cache::behavior_e b=cache::items() );
  static bool download_glyph( player_t* player, std::string& glyph_name,
                              const std::string& glyph_id, cache::behavior_e b=cache::items() );
  static gem_type_e parse_gem( item_t& item, const std::string& gem_id,
                               cache::behavior_e b=cache::items() );
};

// Rawr =====================================================================

struct rawr_t
{
  static player_t* load_player( sim_t*, const std::string& character_filename );
  static player_t* load_player( sim_t*, const std::string& character_filename, const std::string& character_xml );
};

// Blizzard Community Platform API ==========================================

namespace bcp_api
{
bool download_guild( sim_t* sim,
                     const std::string& region,
                     const std::string& server,
                     const std::string& name,
                     const std::vector<int>& ranks,
                     int player_type_e = PLAYER_NONE,
                     int max_rank=0,
                     cache::behavior_e b=cache::players() );
player_t* download_player( sim_t*,
                           const std::string& region,
                           const std::string& server,
                           const std::string& name,
                           const std::string& talents=std::string( "active" ),
                           cache::behavior_e b=cache::players() );
bool download_item( item_t&, const std::string& item_id, cache::behavior_e b=cache::items() );
bool download_glyph( player_t* player, std::string& glyph_name, const std::string& glyph_id,
                     cache::behavior_e b=cache::items() );
bool download_slot( item_t& item,
                    const std::string& item_id,
                    const std::string& enchant_id,
                    const std::string& addon_id,
                    const std::string& reforge_id,
                    const std::string& rsuffix_id,
                    const std::string gem_ids[ 3 ],
                    cache::behavior_e b=cache::items() );
gem_type_e parse_gem( item_t& item, const std::string& gem_id, cache::behavior_e b=cache::items() );
}

// Wowreforge ===============================================================

namespace wowreforge
{
player_t* download_player( sim_t* sim, const std::string& id, cache::behavior_e b=cache::players() );
};

// HTTP Download  ===========================================================

struct http_t
{
private:
  static void format_( std::string& encoded_url, const std::string& url );
public:
  struct proxy_t
  {
    std::string type;
    std::string host;
    int port;
  };
  static proxy_t proxy;

  static void cache_load();
  static void cache_save();
  static bool clear_cache( sim_t*, const std::string& name, const std::string& value );

  static bool get( std::string& result, const std::string& url, cache::behavior_e b,
                   const std::string& confirmation=std::string() );

  static std::string& format( std::string& encoded_url, const std::string& url )
  { format_( encoded_url, url ); return encoded_url; }
  static std::string& format( std::string& url )
  { format_( url, url ); return url; }
};

// XML ======================================================================

struct xml_t
{
  static const char* get_name( xml_node_t* node );
  static xml_node_t* get_child( xml_node_t* root, const std::string& name );
  static xml_node_t* get_node ( xml_node_t* root, const std::string& path );
  static xml_node_t* get_node ( xml_node_t* root, const std::string& path, const std::string& parm_name, const std::string& parm_value );
  static int  get_children( std::vector<xml_node_t*>&, xml_node_t* root, const std::string& name = std::string() );
  static int  get_nodes   ( std::vector<xml_node_t*>&, xml_node_t* root, const std::string& path );
  static int  get_nodes   ( std::vector<xml_node_t*>&, xml_node_t* root, const std::string& path, const std::string& parm_name, const std::string& parm_value );
  static bool get_value( std::string& value, xml_node_t* root, const std::string& path = std::string() );
  static bool get_value( int&         value, xml_node_t* root, const std::string& path = std::string() );
  static bool get_value( double&      value, xml_node_t* root, const std::string& path = std::string() );
  static xml_node_t* get( sim_t* sim, const std::string& url, cache::behavior_e b,
                          const std::string& confirmation=std::string() );
  static xml_node_t* create( sim_t* sim, const std::string& input );
  static xml_node_t* create( sim_t* sim, FILE* input );
  static void print( xml_node_t* root, FILE* f=0, int spacing=0 );
};

// Java Script ==============================================================

struct js_t
{
  static js_node_t* get_child( js_node_t* root, const std::string& name );
  static js_node_t* get_node ( js_node_t* root, const std::string& path );
  static int  get_children( std::vector<js_node_t*>&, js_node_t* root );
  static int  get_value( std::vector<std::string>& value, js_node_t* root, const std::string& path = std::string() );
  static bool get_value( std::string& value, js_node_t* root, const std::string& path = std::string() );
  static bool get_value( int&         value, js_node_t* root, const std::string& path = std::string() );
  static bool get_value( double&      value, js_node_t* root, const std::string& path = std::string() );
  static js_node_t* create( sim_t* sim, const std::string& input );
  static js_node_t* create( sim_t* sim, FILE* input );
  static void print( js_node_t* root, FILE* f=0, int spacing=0 );
  static const char* get_name( js_node_t* root );
};

// Handy Actions ============================================================

struct wait_action_base_t : public action_t
{
  wait_action_base_t( player_t* player, const std::string& name ) :
    action_t( ACTION_OTHER, name, player )
  { trigger_gcd = timespan_t::zero(); }

  virtual void execute()
  { player -> iteration_waiting_time += time_to_execute; }
};

// Wait For Cooldown Action =================================================

struct wait_for_cooldown_t : public wait_action_base_t
{
  cooldown_t* wait_cd;
  action_t* a;
  wait_for_cooldown_t( player_t* player, const std::string& cd_name );
  virtual bool usable_moving() { return a -> usable_moving(); }
  virtual timespan_t execute_time() const;
};

// Inlines ==================================================================

// buff_t inlines

inline buff_t* buff_t::find( sim_t* s, const std::string& name ) 
{ 
  return find( s -> buff_list, name );
}
inline buff_t* buff_t::find( player_t* p, const std::string& name, player_t* source ) 
{ 
  return find( p -> buff_list, name, source );
}

// sim_t inlines

inline double sim_t::real() const                { return default_rng_ -> real(); }
inline bool   sim_t::roll( double chance ) const { return default_rng_ -> roll( chance ); }
inline double sim_t::range( double min, double max )
{ 
  return default_rng_ -> range( min, max );
}
inline double sim_t::averaged_range( double min, double max )
{
  if ( average_range ) return ( min + max ) / 2.0;

  return default_rng_ -> range( min, max );
}

// target_specific_t inlines

inline target_specific_t::target_specific_t( const std::string& name, player_t* source )
{
  index = source -> sim -> target_specific_index( name, source );
}
inline void target_specific_t::init(  const std::string& name, player_t* source )
{
  index = source -> sim -> target_specific_index( name, source );
}
inline void*& target_specific_t::operator[]( player_t* target )
{
  std::vector<void*>& v = target -> target_specific_ptrs;
  size_t size = v.size();
  if( index >= size ) 
  {
    v.resize( index+1 );
    for( size_t i=size; i < index; i++ ) 
      v[i] = NULL;
  }
  return v[ index ];
}

#ifdef WHAT_IF

#define AOE_CAP 10

struct ticker_t // replacement for dot_t, handles any ticking buff/debuff
{
  dot_t stuff
  double crit;
  double haste;
  double mastery;
};

struct sim_t
{
  ...
  actor_t* actor_list;
  std::vector<player_t*> player_list;
  std::vector<mob_t*> mob_list;
  ...
  int get_aura_slot( const std::string& n, actor_t* source );
  ...
};

struct actor_t
{
  items, stats, action list, resource management...
  actor_t( const std::string& n, int type ) {}
  virtual actor_t*  choose_target();
  virtual void      execute_action() { choose_target(); execute_first_ready_action(); }
  virtual debuff_t* get_debuff( int aura_slot );
  virtual ticker_t* get_ticker( int aura_slot );
  virtual bool      is_ally( actor_t* other );
};

struct player_t : public actor_t
{
  scaling, current_target ( actor ), pet_list ...
  player_t( const std::string& n, int type ) : actor_t( n, type ) { sim.player_list.push_back( this ); }
};

struct pet_t : public actor_t
{
  owner, summon, dismiss...
  pet_t( const std::string& n, int type ) : actor_t( n, type ) {}
};

struct enemy_t : public actor_t
{
  health_by_time, health_per_player, arrise_at_time, arise_at_percent, ...
  enemy_t( const std::string& n ) : actor_t( n, ACTOR_ENEMY ) { sim.enemy_list.push_back( this ); }
};

struct action_t
{
  actor_t, ...
  action_t( const std::string& n );
  virtual int execute();
  virtual void schedule_execute();
  virtual double execute_time();
  virtual double haste();
  virtual double gcd();
  virtual bool ready();
  virtual void cancel();
  virtual void reset();
};

struct result_t
{
  actor_t* target;
  int type;
  bool hit;  // convenience
  bool crit; // convenience, two-roll (blocked crits, etc)
  double amount;
  ticker_t ticker;
};

struct ability_t : public action_t
{
  spell_data, resource, weapon, two_roll, self_cast, aoe, base_xyz (no player_xyz or target_xyz),
  direct_sp_coeff, direct_ap_coeff, tick_sp_coeff, tick_ap_coeff,
  harmful, healing, callbacks, std::vector<result_t> results,
  NO binary, NO repeating, NO direct_dmg, NO tick_dmg
  ability_t( spell_data_t* s ) : action_t( s -> name() ) {}
  virtual void  execute()
  {
    actor_t* targets[ AOE_CAP ];
    int num_targets = area_of_effect( targets );
    results.resize( num_targets );
    for ( int i=0; i < num_targets; i++ )
    {
      calculate_result( results[ i ], targets[ i ] );
      // "result" callbacks
    }
    consume_resource();
    for ( int i=0; i < num_targets; i++ )
    {
      schedule_travel( results[ i ] );
    }
    update_ready();
    // "cast" callbacks
  }
  virtual void impact( result_t& result )
  {
    if ( result.hit )
    {
      if ( result.amount > 0 )
      {
        if ( harmful )
        {
          assess_damage( result );
        }
        else if ( healing )
        {
          assess_healing( result );
        }
      }
      if ( num_ticks > 0 )
      {
        ticker_t* ticker = get_ticker( result.target );  // caches aura_slot
        ticker -> trigger( this, result.ticker );
        // ticker_t::trigger() handles dot work in existing action_t::impact()
      }
    }
    else
    {
      // miss msg
      stat -> add_result( result );
    }
  }
  virtual void   tick         ( ticker_t* );
  virtual void   last_tick    ( ticker_t* );
  virtual void   schedule_tick( ticker_t* );
  virtual int    calculate_num_ticks( double haste, double duration=0 );
  virtual double cost();
  virtual double haste();
  virtual bool   ready();
  virtual void   cancel();
  virtual void   reset();
  virtual void   consume_resource();
  virtual result_type_e calculate_attack_roll( actor_t* target );
  virtual result_type_e calculate_spell_roll( actor_t* target );
  virtual result_type_e calculate_roll( actor_t* target )
  {
    if ( weapon )
      return calculate_attack_roll( target );
    else
      return calculate_spell_roll( target );
  }
  virtual void calculate_result( result_t& result, actor_t* target )
  {
    result.type = calculate_roll( target );
    result.hit  = ( roll == ? );
    result.crit = ( roll == ? ) || ( two_roll );
    result.amount = calculate_direct_amount( target );
    if ( result.hit && num_ticks )
    {
      calculate_ticker( &( result.ticker ), target );
    }
    return result;
  }
  virtual void calculate_ticker( ticker_t* ticker, target )
  {
    if ( target ) ticker -> target = target;
    ticker -> amount  = calculate_tick_amount( ticker -> target );
    ticker -> crit    = calculate_crit_chance( ticker -> target );
    ticker -> haste   = calculate_haste      ( ticker -> target );
    ticker -> mastery = calculate_mastery    ( ticker -> target );
  }
  virtual void refresh_ticker( ticker_t* ticker )
  {
    calculate_ticker( ticker );
    ticker -> current_tick = 0;
    ticker -> added_ticks = 0;
    ticker -> added_time = 0;
    ticker -> num_ticks = calculate_num_ticks( ticker -> haste );
    ticker -> recalculate_finish();
  }
  virtual void extend_ticker_by_time( ticker_t* ticker, double extra_time )
  {
    int    full_tick_count   = ticker -> ticks() - 1;
    double full_tick_remains = ticker -> finish - ticker -> tick_event -> time;

    ticker -> haste = calculate_haste( ticker -> target );
    ticker -> num_ticks += calculate_num_ticks( ticker -> haste, full_tick_remains ) - full_tick_count;
    ticker -> recalculate_finish();
  }
  virtual void extend_ticker_by_ticks( ticker_t* ticker, double extra_ticks )
  {
    calculate_ticker( ticker );
    ticker -> added_ticks += extra_ticks;
    ticker -> num_ticks   += extra_ticks;
    ticker -> recalculate_finish();
  }
  virtual double calculate_weapon_amount( actor_t* target );
  virtual double calculate_direct_amount( actor_t* target );
  virtual double calculate_tick_amount  ( actor_t* target );
  virtual double calculate_power( actor_t* target )
  {
    return AP_multiplier * AP() + SP_multiplier * SP();
  }
  virtual double calculate_haste( actor_t* target )
  {
    if ( weapon )
      return calculate_attack_haste( target );
    else
      return calculate_spell_haste( target );
  }
  virtual double calculate_mastery( actor_t* target )
  {
    if ( weapon )
      return calculate_attack_mastery( target );
    else
      return calculate_spell_mastery( target );
  }
  virtual double calculate_miss_chance( actor_t* target )
  {
    if ( weapon )
      return calculate_attack_miss_chance( target );
    else
      return calculate_spell_miss_chance( target );
  }
  virtual double calculate_dodge_chance( actor_t* target )
  {
    if ( weapon )
      return calculate_attack_dodge_chance( target );
    else
      return 0;
  }
  virtual double calculate_parry_chance( actor_t* target )
  {
    if ( weapon )
      return calculate_attack_parry_chance( target );
    else
      return 0;
  }
  virtual double calculate_glance_chance( actor_t* target )
  {
    if ( weapon && auto_attack )
      return calculate_attack_glance_chance( target );
    else
      return 0;
  }
  virtual double calculate_block_chance( actor_t* target )
  {
    if ( weapon )
      return calculate_attack_block_chance( target );
    else
      return 0;
  }
  virtual double calculate_crit_chance( actor_t* target )
  {
    if ( weapon )
      return calculate_attack_crit_chance( target );
    else
      return calculate_spell_crit_chance( target );
  }
  virtual double calculate_crit_chance( ticker_t* ticker );
  virtual double calculate_source_multiplier();
  virtual double calculate_direct_multiplier() { return calculate_source_multiplier(); } // include crit bonus here
  virtual double calculate_tick_multiplier  () { return calculate_source_multiplier(); }
  virtual double calculate_target_multiplier( actor_t* target );
  virtual int       area_of_effect( actor_t* targets[] ) { targets[ 0 ] = self_cast ? actor : actor -> current_target; return 1; }
  virtual result_t& result(); // returns 0th "result", asserts if aoe
  virtual double    travel_time( actor_t* target );
  virtual void      schedule_travel( result_t& result );
  virtual void assess_damage( result_t& result )
  {
    result.amount *= calculate_target_multiplier( result.target ); // allows for action-specific target multipliers
    result.target -> assess_damage( result ); // adjust result as needed for flexibility
    stat -> add_result( result );
  }
  virtual void assess_healing( result_t& result )
  {
    result.amount *= calculate_target_multiplier( result.target ); // allows for action-specific target multipliers
    result.target -> assess_healing( result ); // adjust result as needed for flexibility
    stat -> add_result( result );
  }
};

#endif

#endif // SIMULATIONCRAFT_H
