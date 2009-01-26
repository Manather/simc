// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simcraft.h"

namespace { // ANONYMOUS NAMESPACE ==========================================

static const char* class_color( int type )
{
  switch( type )
  {
  case PLAYER_NONE:  return "FFFFFF";
  case DEATH_KNIGHT: return "C41F3B";
  case DRUID:        return "FF7D0A";
  case HUNTER:       return "ABD473";
  case MAGE:         return "69CCF0";
  case PALADIN:      return "F58CBA";
  case PRIEST:       return "333333";
  case ROGUE:        return "FFF569";
  case SHAMAN:       return "2459FF";
  case WARLOCK:      return "9482CA";
  case WARRIOR:      return "C79C6E";
  default: assert(0);
  }
  return 0;
}

static const char* school_color( int type )
{
  switch( type )
  {
  case SCHOOL_ARCANE:    return class_color( DRUID );
  case SCHOOL_BLEED:     return class_color( WARRIOR );
  case SCHOOL_CHAOS:     return class_color( DEATH_KNIGHT );
  case SCHOOL_FIRE:      return class_color( DEATH_KNIGHT );
  case SCHOOL_FROST:     return class_color( MAGE );
  case SCHOOL_FROSTFIRE: return class_color( PALADIN );
  case SCHOOL_HOLY:      return class_color( ROGUE );
  case SCHOOL_NATURE:    return class_color( SHAMAN );
  case SCHOOL_PHYSICAL:  return class_color( WARRIOR );
  case SCHOOL_SHADOW:    return class_color( WARLOCK );
  default: assert(0);
  }
  return 0;
}

static const char* get_color( player_t* p )
{
  if( p -> is_pet() ) 
  {
    return class_color( p -> cast_pet() -> owner -> type );
  }
  return class_color( p -> type );
}

static unsigned char simple_encoding( int number )
{
  if( number < 0  ) number = 0;
  if( number > 61 ) number = 61;

  static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  return encoding[ number ];
}

#if 0
static const char* extended_encoding( int number )
{
  if( number < 0    ) number = 0;
  if( number > 4095 ) number = 4095;

  static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  int first  = number / 64;
  int second = number - ( first * 64 );

  std::string pair;

  pair = "";
  pair += encoding[ first  ];
  pair += encoding[ second ];

  return pair.c_str();
}
#endif

} // ANONYMOUS NAMESPACE =====================================================

// ==========================================================================
// Report
// ==========================================================================

// report_t::report_t =======================================================

report_t::report_t( sim_t* s ) :
  sim( s ),
  report_actions(1),
  report_attack_stats(1),
  report_chart(1),
  report_core_stats(1),
  report_dpr(1),
  report_dps(1),
  report_gains(1),
  report_miss(1),
  report_rps(1),
  report_name(1),
  report_performance(1),
  report_procs(1),
  report_raid_dps(1),
  report_scaling(1),
  report_spell_stats(1),
  report_statistics(1),
  report_tag(1),
  report_uptime(1),
  report_waiting(1)
{
}

// report_t::parse_option ===================================================

bool report_t::parse_option( const std::string& name,
			     const std::string& value )
{
  option_t options[] =
  {
    { "report_actions",          OPT_INT8,   &( report_actions          ) },
    { "report_attack_stats",     OPT_INT8,   &( report_attack_stats     ) },
    { "report_chart",            OPT_INT8,   &( report_chart            ) },
    { "report_core_stats",       OPT_INT8,   &( report_core_stats       ) },
    { "report_dpr",              OPT_INT8,   &( report_dpr              ) },
    { "report_dps",              OPT_INT8,   &( report_dps              ) },
    { "report_gains",            OPT_INT8,   &( report_gains            ) },
    { "report_miss",             OPT_INT8,   &( report_miss             ) },
    { "report_rps",              OPT_INT8,   &( report_rps              ) },
    { "report_name",             OPT_INT8,   &( report_name             ) },
    { "report_performance",      OPT_INT8,   &( report_performance      ) },
    { "report_procs",            OPT_INT8,   &( report_procs            ) },
    { "report_raid_dps",         OPT_INT8,   &( report_raid_dps         ) },
    { "report_scaling",          OPT_INT8,   &( report_scaling          ) },
    { "report_spell_stats",      OPT_INT8,   &( report_spell_stats      ) },
    { "report_statistics",       OPT_INT8,   &( report_statistics       ) },
    { "report_tag",              OPT_INT8,   &( report_tag              ) },
    { "report_uptime",           OPT_INT8,   &( report_uptime           ) },
    { "report_waiting",          OPT_INT8,   &( report_waiting          ) },
    { NULL, OPT_UNKNOWN }
  };

  if( name.empty() )
  {
    option_t::print( sim, options );
    return false;
  }

  return option_t::parse( sim, options, name, value );
}

// report_t::print_action ====================================================

void report_t::print_action( stats_t* s )
{
  if( s -> total_dmg == 0 ) return;

  double total_dmg;

  if( s -> player -> is_pet() )
  {
    total_dmg = s -> player -> cast_pet() -> owner ->  total_dmg;
  }
  else 
  {
    total_dmg = s -> player -> total_dmg;
  }

  fprintf( sim -> output_file, 
	   "    %-20s  Count=%5.1f|%4.1fsec  DPE=%4.0f|%2.0f%%  DPET=%4.0f  DPR=%4.1f", 
	   s -> name_str.c_str(),
	   s -> num_executes,
	   s -> frequency,
	   s -> dpe, 
	   s -> total_dmg * 100.0 / total_dmg, 
	   s -> dpet,
	   s -> dpr );

  if( report_miss ) fprintf( sim -> output_file, "  Miss=%.1f%%", s -> execute_results[ RESULT_MISS ].count * 100.0 / s -> num_executes );
      
  if( s -> execute_results[ RESULT_HIT ].avg_dmg > 0 )
  {
    fprintf( sim -> output_file, "  Hit=%4.0f", s -> execute_results[ RESULT_HIT ].avg_dmg );
  }
  if( s -> execute_results[ RESULT_CRIT ].avg_dmg > 0 )
  {
    fprintf( sim -> output_file, 
	     "  CritHit=%4.0f|%4.0f|%.1f%%", 
	     s -> execute_results[ RESULT_CRIT ].avg_dmg, 
	     s -> execute_results[ RESULT_CRIT ].max_dmg, 
	     s -> execute_results[ RESULT_CRIT ].count * 100.0 / s -> num_executes );
  }
  if( s -> execute_results[ RESULT_GLANCE ].avg_dmg > 0 )
  {
    fprintf( sim -> output_file, 
	     "  GlanceHit=%4.0f|%.1f%%", 
	     s -> execute_results[ RESULT_GLANCE ].avg_dmg, 
	     s -> execute_results[ RESULT_GLANCE ].count * 100.0 / s -> num_executes );
  }
  if( s -> execute_results[ RESULT_DODGE ].count > 0 )
  {
    fprintf( sim -> output_file, 
	     "  Dodge=%.1f%%", 
	     s -> execute_results[ RESULT_DODGE ].count * 100.0 / s -> num_executes );
  }

  if( s -> tick_results[ RESULT_HIT ].avg_dmg > 0 )
  {
    fprintf( sim -> output_file, 
	     "  Tick=%.0f", s -> tick_results[ RESULT_HIT ].avg_dmg );
  }
  if( s -> tick_results[ RESULT_CRIT ].avg_dmg > 0 )
  {
    fprintf( sim -> output_file, 
	     "  CritTick=%.0f|%.0f|%.1f%%", 
	     s -> tick_results[ RESULT_CRIT ].avg_dmg, 
	     s -> tick_results[ RESULT_CRIT ].max_dmg, 
	     s -> tick_results[ RESULT_CRIT ].count * 100.0 / s -> num_ticks );
  }

  fprintf( sim -> output_file, "\n" );
}

// report_t::print_actions ===================================================

void report_t::print_actions( player_t* p )
{
  fprintf( sim -> output_file, "  Actions:\n" );

  for( stats_t* s = p -> stats_list; s; s = s -> next )
  {
    if( s -> total_dmg > 0 )
    {
      print_action( s );
    }
  }

  for( pet_t* pet = p -> pet_list; pet; pet = pet -> next_pet )
  {
    bool first=true;
    for( stats_t* s = pet -> stats_list; s; s = s -> next )
    {
      if( s -> total_dmg > 0 )
      {
	if( first )
	{
	  fprintf( sim -> output_file, "   %s\n", pet -> name_str.c_str() );
	  first = false;
	}
	print_action( s );
      }
    }
  }
}

// report_t::print_core_stats =================================================

void report_t::print_core_stats( player_t* p )
{
  fprintf( sim -> output_file, 
	   "%s  %s%.0f  %s%.0f  %s%.0f  %s%.0f  %s%.0f  %s%.0f  %s%.0f\n", 
	   report_tag ? "  Core Stats:" : "", 
	   report_tag ? "strength=" : "",
	   p -> strength(),
	   report_tag ? "agility=" : "",
	   p -> agility(),
	   report_tag ? "stamina=" : "",
	   p -> stamina(),
	   report_tag ? "intellect=" : "",
	   p -> intellect(),
	   report_tag ? "spirit=" : "",
	   p -> spirit(),
	   report_tag ? "health=" : "",
	   p -> resource_max[ RESOURCE_HEALTH ], 
	   report_tag ? "mana=" : "",
	   p -> resource_max[ RESOURCE_MANA ] );
}

// report_t::print_spell_stats ================================================

void report_t::print_spell_stats( player_t* p )
{
  fprintf( sim -> output_file, 
	   "%s  %s%.0f  %s%.1f%%  %s%.1f%%  %s%.0f  %s%.1f%%  %s%.0f\n", 
	   report_tag ? "  Spell Stats:" : "", 
	   report_tag ? "power="       : "", p -> composite_spell_power( SCHOOL_MAX ),
	   report_tag ? "hit="         : "", p -> composite_spell_hit()  * 100.0, 
	   report_tag ? "crit="        : "", p -> composite_spell_crit() * 100.0,
	   report_tag ? "penetration=" : "", p -> composite_spell_penetration(),
	   report_tag ? "haste="       : "", ( 1.0 / p -> haste - 1 ) * 100.0,
	   report_tag ? "mp5="         : "", p -> initial_mp5 );
}

// report_t::print_attack_stats ===============================================

void report_t::print_attack_stats( player_t* p )
{
  fprintf( sim -> output_file, 
	   "%s  %s%.0f  %s%.1f%%  %s%.1f%%  %s%.1f  %s%.0f  %s%.1f%%\n", 
	   report_tag ? "  Attack Stats:" : "",
	   report_tag ? "power="       : "", p -> composite_attack_power(),
	   report_tag ? "hit="         : "", p -> composite_attack_hit()       * 100.0, 
	   report_tag ? "crit="        : "", p -> composite_attack_crit()      * 100.0, 
	   report_tag ? "expertise="   : "", p -> composite_attack_expertise() * 100.0,
	   report_tag ? "penetration=" : "", p -> composite_attack_penetration(),
	   report_tag ? "haste="       : "", ( 1.0 / p -> haste - 1 ) * 100.0 );
}

// report_t::print_gains =====================================================

void report_t::print_gains()
{
  fprintf( sim -> output_file, "\nGains:\n" );

  for( player_t* p = sim -> player_list; p; p = p -> next )
  {
    if( p -> quiet ) 
      continue;

    bool first=true;
    for( gain_t* g = p -> gain_list; g; g = g -> next )
    {
      if( g -> amount > 0 ) 
      {
	if( first )
        {
	  fprintf( sim -> output_file, "\n    %s:\n", p -> name() );
	  first = false;
	}
	fprintf( sim -> output_file, "        %s=%.1f\n", g -> name(), g -> amount );
      }
    }
  }
}

// report_t::print_procs =====================================================

void report_t::print_procs()
{
  fprintf( sim -> output_file, "\nProcs:\n" );

  for( player_t* player = sim -> player_list; player; player = player -> next )
  {
    if( player -> quiet ) 
      continue;

    bool first=true;
    for( proc_t* p = player -> proc_list; p; p = p -> next )
    {
      if( p -> count > 0 ) 
      {
	if( first )
        {
	  fprintf( sim -> output_file, "\n    %s:\n", player -> name() );
	  first = false;
	}
	fprintf( sim -> output_file, "        %s=%.1f|%.1fsec\n", p -> name(), p -> count, p -> frequency );
      }
    }
  }
}

// report_t::print_uptime =====================================================

void report_t::print_uptime()
{
  fprintf( sim -> output_file, "\nUp-Times:\n" );

  bool first=true;
  for( uptime_t* u = sim -> target -> uptime_list; u; u = u -> next )
  {
    if( u -> up > 0 ) 
    {
      if( first )
      {
	fprintf( sim -> output_file, "\n    Global:\n" );
	first = false;
      }
      fprintf( sim -> output_file, "        %4.1f%% : %s\n", u -> percentage(), u -> name() );
    }
  }

  for( player_t* p = sim -> player_list; p; p = p -> next )
  {
    if( p -> quiet ) 
      continue;

    first=true;
    for( uptime_t* u = p -> uptime_list; u; u = u -> next )
    {
      if( u -> up > 0 ) 
      {
	if( first )
        {
	  fprintf( sim -> output_file, "\n    %s:\n", p -> name() );
	  first = false;
	}
	fprintf( sim -> output_file, "        %4.1f%% : %s\n", u -> percentage(), u -> name() );
      }
    }
  }
}

// report_t::print_waiting =====================================================

void report_t::print_waiting()
{
  fprintf( sim -> output_file, "\nWaiting:\n" );

  bool nobody_waits = true;

  for( player_t* p = sim -> player_list; p; p = p -> next )
  {
    if( p -> quiet ) 
      continue;
    
    if( p -> total_waiting )
    {
      nobody_waits = false;
      fprintf( sim -> output_file, "    %4.1f%% : %s\n", 100.0 * p -> total_waiting / p -> total_seconds,  p -> name() );
    }
  }

  if( nobody_waits ) fprintf( sim -> output_file, "    All players active 100%% of the time.\n" );
}

// report_t::print_performance ================================================

void report_t::print_performance()
{
  fprintf( sim -> output_file, 
	   "\nBaseline Performance:\n"
	   "  TotalEvents   = %d\n"
	   "  MaxEventQueue = %d\n"
	   "  SimSeconds    = %.0f\n"
	   "  CpuSeconds    = %.0f\n"
	   "  SpeedUp       = %.0f\n", 
	   sim -> total_events_processed,
	   sim -> max_events_remaining,
	   sim -> iterations * sim -> total_seconds,
	   sim -> elapsed_cpu_seconds,
	   sim -> iterations * sim -> total_seconds / sim -> elapsed_cpu_seconds );
}

// report_t::print_scale_factors ==============================================

void report_t::print_scale_factors()
{
  if( ! sim -> scaling -> calculate_scale_factors ) return;

  fprintf( sim -> output_file, "\nScale Factors:\n" );

  int num_players = sim -> players_by_name.size();

  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_name[ i ];

    fprintf( sim -> output_file, "  %-25s", p -> name() );

    for( int j=0; j < ATTRIBUTE_MAX; j++ )
    {
      if( sim -> scaling -> gear.attribute[ j ] ) 
      {
	fprintf( sim -> output_file, "  %s=%.2f", util_t::attribute_type_string( j ), p -> scaling.attribute[ j ] );
      }
    }
    if( sim -> scaling -> gear.spell_power              ) fprintf( sim -> output_file,              "  spell_power=%.2f", p -> scaling.spell_power              );
    if( sim -> scaling -> gear.attack_power             ) fprintf( sim -> output_file,       "  attack_power_power=%.2f", p -> scaling.attack_power             );
    if( sim -> scaling -> gear.expertise_rating         ) fprintf( sim -> output_file,         "  expertise_rating=%.2f", p -> scaling.expertise_rating         );
    if( sim -> scaling -> gear.armor_penetration_rating ) fprintf( sim -> output_file, "  armor_penetrating_rating=%.2f", p -> scaling.armor_penetration_rating );
    if( sim -> scaling -> gear.hit_rating               ) fprintf( sim -> output_file,               "  hit_rating=%.2f", p -> scaling.hit_rating               );
    if( sim -> scaling -> gear.crit_rating              ) fprintf( sim -> output_file,              "  crit_rating=%.2f", p -> scaling.crit_rating              );
    if( sim -> scaling -> gear.haste_rating             ) fprintf( sim -> output_file,             "  haste_rating=%.2f", p -> scaling.haste_rating             );

    fprintf( sim -> output_file, "\n" );
  }
}

// report_t::print ============================================================

void report_t::print()
{
  if( sim -> total_seconds == 0 ) return;

  int num_players = sim -> players_by_rank.size();

  if( report_raid_dps ) 
  {
    if( report_tag ) fprintf( sim -> output_file, "\nDPS Ranking:\n" );
    fprintf( sim -> output_file, "%7.0f 100.0%%  Raid\n", sim -> raid_dps );
    for( int i=0; i < num_players; i++ )
    {
      player_t* p = sim -> players_by_rank[ i ];
      fprintf( sim -> output_file, "%7.0f  %4.1f%%  %s\n", p -> dps, 100 * p -> total_dmg / sim -> total_dmg, p -> name() );
    }
  }

  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_name[ i ];

    if( report_tag  ) fprintf( sim -> output_file, "\n" );
    if( report_name ) fprintf( sim -> output_file, "%s%s",     report_tag ? "Player="  : "", p -> name() );
    if( report_dps  ) 
    {
      fprintf( sim -> output_file, "  %s%.1f", report_tag ? "DPS="     : "", p -> dps );

      if( report_statistics ) 
      {
        fprintf( sim -> output_file, " (Error=+/-%.1f Range=+/-%.0f)", p -> dps_error, ( p -> dps_max - p -> dps_min ) / 2.0 );
      }
    }

    if( p -> rps_loss > 0 )
    {
      if( report_dpr  ) fprintf( sim -> output_file, "  %s%.1f",      report_tag ? "DPR="    : "", p -> dpr );
      if( report_rps  ) fprintf( sim -> output_file, "  %s%.1f/%.1f", report_tag ? "RPS="    : "", p -> rps_loss, p -> rps_gain );

      if( report_dpr || report_rps )
      fprintf( sim -> output_file, "  (%s)", util_t::resource_type_string( p -> primary_resource() ) );
    }

    if( report_name ) fprintf( sim -> output_file, "\n" );

    if( report_core_stats   ) print_core_stats  ( p );
    if( report_spell_stats  ) print_spell_stats ( p );
    if( report_attack_stats ) print_attack_stats( p );

    if( report_actions ) print_actions( p );

  }

  if( report_gains   ) print_gains();
  if( report_procs   ) print_procs();
  if( report_uptime  ) print_uptime();
  if( report_waiting ) print_waiting();

  if( report_performance ) print_performance();

  if( report_scaling ) print_scale_factors();

  fprintf( sim -> output_file, "\n" );
}

// report_t::chart_raid_dps ==================================================

const char* report_t::chart_raid_dps( std::string& s )
{
  int num_players = sim -> players_by_rank.size();
  assert( num_players != 0 );

  char buffer[ 1024 ];

  s = "http://chart.apis.google.com/chart?";
  snprintf( buffer, sizeof(buffer), "chs=450x%d", num_players * 20 + 30 ); s += buffer;
  s += "&";
  s += "cht=bhg";
  s += "&";
  s += "chbh=15";
  s += "&";
  s += "chd=t:";
  double max_dps=0;
  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_rank[ i ];
    snprintf( buffer, sizeof(buffer), "%s%.0f", (i?"|":""), p -> dps ); s += buffer;
    if( p -> dps > max_dps ) max_dps = p -> dps;
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chds=0,%.0f", max_dps * 2 ); s += buffer;
  s += "&";
  s += "chco=";
  for( int i=0; i < num_players; i++ )
  {
    if( i ) s += ",";
    s += get_color( sim -> players_by_rank[ i ] );
  }
  s += "&";
  s += "chm=";
  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_rank[ i ];
    snprintf( buffer, sizeof(buffer), "%st++%.0f++%s,%s,%d,0,15", (i?"|":""), p -> dps, p -> name(), get_color( p ), i ); s += buffer;
  }
  s += "&";
  s += "chtt=DPS+Ranking";
  s += "&";
  s += "chts=000000,20";

  return s.c_str();
}

// report_t::chart_raid_gear =================================================

const char* report_t::chart_raid_gear( std::string& s )
{
  int num_players = sim -> players_by_rank.size();
  assert( num_players != 0 );

  const int NUM_CATEGORIES = 11;
  std::vector<double> data_points[ NUM_CATEGORIES ];

  for( int i=0; i < NUM_CATEGORIES; i++ ) 
  {
    data_points[ i ].insert( data_points[ i ].begin(), num_players, 0 );
  }

  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_rank[ i ];

    data_points[ 0 ][ i ] = ( p -> gear.attribute[ ATTR_STRENGTH  ] + p -> gear.attribute_enchant[ ATTR_STRENGTH  ] );
    data_points[ 1 ][ i ] = ( p -> gear.attribute[ ATTR_AGILITY   ] + p -> gear.attribute_enchant[ ATTR_AGILITY   ] );
    data_points[ 2 ][ i ] = ( p -> gear.attribute[ ATTR_INTELLECT ] + p -> gear.attribute_enchant[ ATTR_INTELLECT ] );

    data_points[ 3 ][ i ] = ( ( p -> gear.attribute[ ATTR_SPIRIT ] + p -> gear.attribute_enchant[ ATTR_SPIRIT ] ) +
			      ( p -> gear.mp5 + p -> gear.mp5_enchant ) * 2.5 );

    data_points[ 4 ][ i ] = ( p -> gear.attack_power              + p -> gear.attack_power_enchant              ) * 0.5;
    data_points[ 5 ][ i ] = ( p -> gear.spell_power[ SCHOOL_MAX ] + p -> gear.spell_power_enchant[ SCHOOL_MAX ] ) * 0.86;

    data_points[ 6 ][ i ] = ( p -> gear.hit_rating       + p -> gear.hit_rating_enchant       );
    data_points[ 7 ][ i ] = ( p -> gear.crit_rating      + p -> gear.crit_rating_enchant      );
    data_points[ 8 ][ i ] = ( p -> gear.haste_rating     + p -> gear.haste_rating_enchant     );
    data_points[ 9 ][ i ] = ( p -> gear.expertise_rating + p -> gear.expertise_rating_enchant );

    data_points[ 10 ][ i ] = ( p -> gear.armor_penetration_rating + p -> gear.armor_penetration_rating_enchant ) +
                             ( p -> gear.spell_penetration        + p -> gear.spell_penetration_enchant        ) * 0.80 ;
  }

  double max_total=0;
  for( int i=0; i < num_players; i++ )
  {
    double total=0;
    for( int j=0; j < NUM_CATEGORIES; j++ )
    {
      total += data_points[ j ][ i ];
    }
    if( total > max_total ) max_total = total;
  }

  const char* colors[] = {
    class_color( WARRIOR ), class_color( HUNTER ), class_color( MAGE    ), class_color( DRUID  ), class_color( ROGUE        ), 
    class_color( WARLOCK ), class_color( PRIEST ), class_color( PALADIN ), class_color( SHAMAN ), class_color( DEATH_KNIGHT ), "00FF00"
  };

  char buffer[ 1024 ];
  
  s = "http://chart.apis.google.com/chart?";
  snprintf( buffer, sizeof(buffer), "chs=450x%d", num_players * 20 + 30 ); s += buffer;
  s += "&";
  s += "cht=bhs";
  s += "&";
  s += "chbh=15";
  s += "&";
  s += "chd=t:";
  for( int i=0; i < NUM_CATEGORIES; i++ )
  {
    if( i ) s += "|";
    for( int j=0; j < num_players; j++ )
    {
      snprintf( buffer, sizeof(buffer), "%s%.0f", (j?",":""), data_points[ i ][ j ] ); s += buffer;
    }
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chds=0,%.0f", max_total ); s += buffer;
  s += "&";
  s += "chco=";
  for( int i=0; i < NUM_CATEGORIES; i++ )
  {
    if( i ) s += ",";
    s += colors[ i ];
  }
  s += "&";
  s += "chxt=y";
  s += "&";
  s += "chxl=0:";
  for( int i = num_players-1; i >= 0; i-- )
  {
    s += "|";
    s += sim -> players_by_rank[ i ] -> name();
  }
  s += "&";
  s += "chxs=0,000000,15";
  s += "&";
  s += "chdl=Strength|Agility|Intellect|Spirit/MP5|Attack+Power|Spell+Power|Hit+Rating|Crit+Rating|Haste+Rating|Expertise+Rating|Penetration";
  s += "&";
  s += "chtt=Gear+Overview";
  s += "&";
  s += "chts=000000,20";

  return s.c_str();
}

// report_t::chart_raid_downtime ==============================================

const char* report_t::chart_raid_downtime( std::string& s )
{
  int num_players = sim -> players_by_name.size();
  assert( num_players != 0 );
  
  std::vector<player_t*> waiting_list;

  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_name[ i ];
    if( ( p -> total_waiting / p -> total_seconds ) > 0.01 ) 
    {
      waiting_list.push_back( p );
    }
  }

  int num_waiting = waiting_list.size();
  if( num_waiting == 0 ) return 0;

  char buffer[ 1024 ];

  s = "http://chart.apis.google.com/chart?";
  snprintf( buffer, sizeof(buffer), "chs=500x%d", num_waiting * 30 + 30 ); s += buffer;
  s += "&";
  s += "cht=bhg";
  s += "&";
  s += "chd=t:";
  double max_waiting=0;
  for( int i=0; i < num_waiting; i++ )
  {
    player_t* p = waiting_list[ i ];
    double waiting = 100.0 * p -> total_waiting / p -> total_seconds;
    if( waiting > max_waiting ) max_waiting = waiting;
    snprintf( buffer, sizeof(buffer), "%s%.0f", (i?"|":""), waiting ); s += buffer;
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chds=0,%.0f", max_waiting * 2 ); s += buffer;
  s += "&";
  s += "chco=";
  for( int i=0; i < num_waiting; i++ )
  {
    if( i ) s += ",";
    s += get_color( waiting_list[ i ] );
  }
  s += "&";
  s += "chm=";
  for( int i=0; i < num_waiting; i++ )
  {
    player_t* p = waiting_list[ i ];
    snprintf( buffer, sizeof(buffer), "%st++%.0f%%++%s,%s,%d,0,15", (i?"|":""), 100.0 * p -> total_waiting / p -> total_seconds, p -> name(), get_color( p ), i ); s += buffer;
  }
  s += "&";
  s += "chtt=Raid+Down-Time";
  s += "&";
  s += "chts=000000,20";

  return s.c_str();
}

// report_t::chart_raid_uptimes =================================================

const char* report_t::chart_raid_uptimes( std::string& s )
{
  std::vector<uptime_t*> uptime_list;

  for( uptime_t* u = sim -> target -> uptime_list; u; u = u -> next )
  {
    if( u -> up <= 0 ) continue;
    uptime_list.push_back( u );
  }

  int num_uptimes = uptime_list.size();
  if( num_uptimes == 0 ) return 0;

  char buffer[ 1024 ];

  s = "http://chart.apis.google.com/chart?";
  snprintf( buffer, sizeof(buffer), "chs=500x%d", num_uptimes * 30 + 30 ); s += buffer;
  s += "&";
  s += "cht=bhs";
  s += "&";
  s += "chd=t:";
  for( int i=0; i < num_uptimes; i++ )
  {
    uptime_t* u = uptime_list[ i ];
    snprintf( buffer, sizeof(buffer), "%s%.0f", (i?",":""), u -> percentage() ); s += buffer;
  }
  s += "&";
  s += "chds=0,200";
  s += "&";
  s += "chm=";
  for( int i=0; i < num_uptimes; i++ )
  {
    uptime_t* u = uptime_list[ i ];
    snprintf( buffer, sizeof(buffer), "%st++%.0f%%++%s,000000,0,%d,15", (i?"|":""), u -> percentage(), u -> name(), i ); s += buffer;
  }
  s += "&";
  s += "chtt=Global+Up-Times";
  s += "&";
  s += "chts=000000,20";
  
  return s.c_str();
}

// report_t::chart_raid_dpet =================================================

struct compare_dpet {
  bool operator()( stats_t* l, stats_t* r ) const
  {
    return l -> dpet > r -> dpet;
  }
};

int report_t::chart_raid_dpet( std::string& s, std::vector<std::string>& images )
{
  int num_players = sim -> players_by_rank.size();
  assert( num_players != 0 );

  std::vector<stats_t*> stats_list;

  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_rank[ i ];

    for( stats_t* st = p -> stats_list; st; st = st -> next )
    {
      if( st -> total_dmg <= 0 ) continue;
      if( ! st -> channeled && st -> total_execute_time <= 0 ) continue;
      if( st -> num_executes < 5 ) continue;

      stats_list.push_back( st );
    }
  }

  int num_stats = stats_list.size();
  if( num_stats == 0 ) return 0;

  std::sort( stats_list.begin(), stats_list.end(), compare_dpet() );

  double max_dpet = stats_list[ 0 ] -> dpet;

  int max_actions_per_chart = 25;
  int max_charts = 2;

  char buffer[ 1024 ];

  for( int chart=0; chart < max_charts; chart++ )
  {
    if( num_stats > max_actions_per_chart ) num_stats = max_actions_per_chart;

    s = "http://chart.apis.google.com/chart?";
    snprintf( buffer, sizeof(buffer), "chs=500x%d", num_stats * 15 + 30 ); s += buffer;
    s += "&";
    s += "cht=bhg";
    s += "&";
    s += "chbh=10";
    s += "&";
    s += "chd=t:";
    for( int i=0; i < num_stats; i++ )
    {
      stats_t* st = stats_list[ i ];
      snprintf( buffer, sizeof(buffer), "%s%.0f", (i?"|":""), st -> dpet ); s += buffer;
    }
    s += "&";
    snprintf( buffer, sizeof(buffer), "chds=0,%.0f", max_dpet * 2.5 ); s += buffer;
    s += "&";
    s += "chco=";
    for( int i=0; i < num_stats; i++ )
    {
      if( i ) s += ",";
      s += get_color( stats_list[ i ] -> player );
    }
    s += "&";
    s += "chm=";
    for( int i=0; i < num_stats; i++ )
    {
      stats_t* st = stats_list[ i ];
      snprintf( buffer, sizeof(buffer), "%st++%.0f++%s+(%s),%s,%d,0,10", (i?"|":""), 
	       st -> dpet, st -> name_str.c_str(), st -> player -> name(), get_color( st -> player ), i ); s += buffer;
    }
    s += "&";
    s += "chtt=Raid+Damage+Per+Execute+Time";
    s += "&";
    s += "chts=000000,20";

    images.push_back( s );

    stats_list.erase( stats_list.begin(), stats_list.begin()+num_stats );
    num_stats = stats_list.size();
    if( num_stats == 0 ) break;
  }

  return images.size();
}

// report_t::chart_action_dpet ===============================================

const char* report_t::chart_action_dpet( std::string& s, player_t* p )
{
  std::vector<stats_t*> stats_list;

  for( stats_t* st = p -> stats_list; st; st = st -> next )
  {
    if( st -> total_dmg <= 0 ) continue;
    if( ! st -> channeled && st -> total_execute_time <= 0 ) continue;
    if( st -> dpet > ( 5 * p -> dps ) ) continue;

    stats_list.push_back( st );
  }

  for( pet_t* pet = p -> pet_list; pet; pet = pet -> next_pet )
  {
    for( stats_t* st = pet -> stats_list; st; st = st -> next )
    {
      if( st -> total_dmg <= 0 ) continue;
      if( ! st -> channeled && st -> total_execute_time <= 0 ) continue;
      if( st -> dpet > ( 10 * p -> dps ) ) continue;
      
      stats_list.push_back( st );
    }
  }

  int num_stats = stats_list.size();
  if( num_stats == 0 ) return 0;

  std::sort( stats_list.begin(), stats_list.end(), compare_dpet() );

  char buffer[ 1024 ];

  s = "http://chart.apis.google.com/chart?";
  snprintf( buffer, sizeof(buffer), "chs=500x%d", num_stats * 30 + 65 ); s += buffer;
  s += "&";
  s += "cht=bhg";
  s += "&";
  s += "chd=t:";
  double max_dpet=0;
  for( int i=0; i < num_stats; i++ )
  {
    stats_t* st = stats_list[ i ];
    snprintf( buffer, sizeof(buffer), "%s%.0f", (i?"|":""), st -> dpet ); s += buffer;
    if( st -> dpet > max_dpet ) max_dpet = st -> dpet;
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chds=0,%.0f", max_dpet * 2 ); s += buffer;
  s += "&";
  s += "chco=";
  for( int i=0; i < num_stats; i++ )
  {
    if( i ) s += ",";
    s += school_color( stats_list[ i ] -> school );
  }
  s += "&";
  s += "chm=";
  for( int i=0; i < num_stats; i++ )
  {
    stats_t* st = stats_list[ i ];
    snprintf( buffer, sizeof(buffer), "%st++%.0f++%s,%s,%d,0,15", (i?"|":""), st -> dpet, st -> name_str.c_str(), school_color( st -> school ), i ); s += buffer;
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chtt=%s|Damage+Per+Execute+Time", p -> name() ); s += buffer;
  s += "&";
  s += "chts=000000,20";

  return s.c_str();
}

// report_t::chart_action_dmg ================================================

struct compare_dmg {
  bool operator()( stats_t* l, stats_t* r ) const
  {
    return l -> total_dmg > r -> total_dmg;
  }
};

const char* report_t::chart_action_dmg( std::string& s, player_t* p )
{
  std::vector<stats_t*> stats_list;

  for( stats_t* st = p -> stats_list; st; st = st -> next )
  {
    if( st -> total_dmg <= 0 ) continue;
    stats_list.push_back( st );
  }

  for( pet_t* pet = p -> pet_list; pet; pet = pet -> next_pet )
  {
    for( stats_t* st = pet -> stats_list; st; st = st -> next )
    {
      if( st -> total_dmg <= 0 ) continue;
      stats_list.push_back( st );
    }
  }

  int num_stats = stats_list.size();
  if( num_stats == 0 ) return 0;

  std::sort( stats_list.begin(), stats_list.end(), compare_dmg() );

  char buffer[ 1024 ];

  s = "http://chart.apis.google.com/chart?";
  snprintf( buffer, sizeof(buffer), "chs=500x%d", 200 + num_stats * 10 ); s += buffer;
  s += "&";
  s += "cht=p";
  s += "&";
  s += "chd=t:";
  for( int i=0; i < num_stats; i++ )
  {
    stats_t* st = stats_list[ i ];
    snprintf( buffer, sizeof(buffer), "%s%.0f", (i?",":""), 100.0 * st -> total_dmg / p -> total_dmg ); s += buffer;
  }
  s += "&";
  s += "chds=0,100";
  s += "&";
  s += "chco=";
  for( int i=0; i < num_stats; i++ )
  {
    if( i ) s += ",";
    s += school_color( stats_list[ i ] -> school );
  }
  s += "&";
  s += "chl=";
  for( int i=0; i < num_stats; i++ )
  {
    if( i ) s += "|";
    s += stats_list[ i ] -> name_str.c_str();
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chtt=%s+Damage+Sources", p -> name() ); s += buffer;
  s += "&";
  s += "chts=000000,20";

  return s.c_str();
}

// report_t::chart_gains =====================================================

struct compare_gain {
  bool operator()( gain_t* l, gain_t* r ) const
  {
    return l -> amount > r -> amount;
  }
};

const char* report_t::chart_gains( std::string& s, player_t* p )
{
  std::vector<gain_t*> gains_list;

  double total_gain=0;
  for( gain_t* g = p -> gain_list; g; g = g -> next )
  {
    if( g -> amount <= 0 ) continue;
    total_gain += g -> amount;
    gains_list.push_back( g );
  }

  int num_gains = gains_list.size();
  if( num_gains == 0 ) return 0;

  std::sort( gains_list.begin(), gains_list.end(), compare_gain() );

  char buffer[ 1024 ];

  s = "http://chart.apis.google.com/chart?";
  snprintf( buffer, sizeof(buffer), "chs=550x%d", 200 + num_gains * 10 ); s += buffer;
  s += "&";
  s += "cht=p";
  s += "&";
  s += "chd=t:";
  for( int i=0; i < num_gains; i++ )
  {
    gain_t* g = gains_list[ i ];
    snprintf( buffer, sizeof(buffer), "%s%.0f", (i?",":""), 100.0 * g -> amount / total_gain ); s += buffer;
  }
  s += "&";
  s += "chds=0,100";
  s += "&";
  s += "chco=";  
  s += get_color( p );
  s += "&";
  s += "chl=";
  for( int i=0; i < num_gains; i++ )
  {
    if( i ) s += "|";
    s += gains_list[ i ] -> name();
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chtt=%s+Resource+Gains", p -> name() ); s += buffer;
  s += "&";
  s += "chts=000000,20";

  return s.c_str();
}

// report_t::chart_uptimes_and_procs ===========================================

const char* report_t::chart_uptimes_and_procs( std::string& s, player_t* p )
{
  std::vector<uptime_t*> uptime_list;
  std::vector<proc_t*>     proc_list;

  for( uptime_t* u = p -> uptime_list; u; u = u -> next )
  {
    if( u -> up <= 0 ) continue;
    if( floor( u -> percentage() ) <= 0 ) continue;
    uptime_list.push_back( u );
  }

  double max_proc_count=0;
  for( proc_t* proc = p -> proc_list; proc; proc = proc -> next )
  {
    if( floor( proc -> count ) <= 0 ) continue;
    if( proc -> count > max_proc_count ) max_proc_count = proc -> count;
    proc_list.push_back( proc );
  }

  int num_uptimes = uptime_list.size();
  int num_procs   =   proc_list.size();

  if( num_uptimes == 0 && num_procs == 0 ) return 0;

  char buffer[ 1024 ];

  s = "http://chart.apis.google.com/chart?";
  snprintf( buffer, sizeof(buffer), "chs=500x%d", ( num_uptimes + num_procs ) * 30 + 65 ); s += buffer;
  s += "&";
  s += "cht=bhg";
  s += "&";
  s += "chd=t:";
  for( int i=0; i < num_uptimes; i++ )
  {
    uptime_t* u = uptime_list[ i ];
    snprintf( buffer, sizeof(buffer), "%s%.0f", (i?"|":""), u -> percentage() ); s += buffer;
  }
  for( int i=0; i < num_procs; i++ )
  {
    proc_t* proc = proc_list[ i ];
    snprintf( buffer, sizeof(buffer), "%s%.0f", ((num_uptimes+i)?"|":""), 100 * proc -> count / max_proc_count ); s += buffer;
  }
  s += "&";
  s += "chds=0,200";
  s += "&";
  s += "chco=";
  for( int i=0; i < num_uptimes; i++ )
  {
    if( i ) s += ",";
    s += "00FF00";
  }
  for( int i=0; i < num_procs; i++ )
  {
    if( num_uptimes+i) s += ",";
    s += "FF0000";
  }
  s += "&";
  s += "chm=";
  for( int i=0; i < num_uptimes; i++ )
  {
    uptime_t* u = uptime_list[ i ];
    snprintf( buffer, sizeof(buffer), "%st++%.0f%%++%s,000000,%d,0,15", (i?"|":""), u -> percentage(), u -> name(), i ); s += buffer;
  }
  for( int i=0; i < num_procs; i++ )
  {
    proc_t* proc = proc_list[ i ];
    snprintf( buffer, sizeof(buffer), "%st++%.0f+(%.1fsec)++%s,000000,%d,0,15", ((num_uptimes+i)?"|":""), 
	     proc -> count, proc -> frequency, proc -> name(), num_uptimes+i ); s += buffer;
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chtt=%s|Up-Times+and+Procs+(%.0fsec)", p -> name(), sim -> total_seconds ); s += buffer;
  s += "&";
  s += "chts=000000,20";

  return s.c_str();
}

// report_t::chart_timeline_dps ==============================================

const char* report_t::chart_timeline_dps( std::string& s, player_t* p )
{
  int max_buckets = p -> timeline_dps.size();
  int max_points  = 600;
  int increment   = 1;

  if( max_buckets <= max_points )
  {
    max_points = max_buckets;
  }
  else
  {
    increment = ( (int) floor( max_buckets / (double) max_points ) ) + 1;
  }

  double dps_max=0;
  for( int i=0; i < max_buckets; i++ ) 
  {
    if( p -> timeline_dps[ i ] > dps_max ) 
    {
      dps_max = p -> timeline_dps[ i ];
    }
  }
  double dps_range  = 60.0;
  double dps_adjust = dps_range / dps_max;

  char buffer[ 1024 ];

  s = "http://chart.apis.google.com/chart?";
  s += "chs=425x130";
  s += "&";
  s += "cht=lc";
  s += "&";
  s += "chd=s:";
  for( int i=0; i < max_buckets; i += increment ) 
  {
    s += simple_encoding( (int) ( p -> timeline_dps[ i ] * dps_adjust ) );
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chds=0,%.0f", dps_range ); s += buffer;
  s += "&";
  s += "chxt=x,y";
  s += "&";
  snprintf( buffer, sizeof(buffer), "chxl=0:|0|sec=%d|1:|0|avg=%.0f|max=%.0f", max_buckets, p -> dps, dps_max ); s += buffer;
  s += "&";
  snprintf( buffer, sizeof(buffer), "chxp=1,1,%.0f,100", 100.0 * p -> dps / dps_max ); s += buffer;
  s += "&";
  snprintf( buffer, sizeof(buffer), "chtt=%s+DPS+Timeline", p -> name() ); s += buffer;
  s += "&";
  s += "chts=000000,20";

  return s.c_str();
}

// report_t::chart_timeline_resource =========================================

const char* report_t::chart_timeline_resource( std::string& s, player_t* p )
{
  if( p -> primary_resource() == RESOURCE_NONE ) return 0;

  int max_buckets = p -> timeline_resource.size();
  int max_points  = 600;
  int increment   = 1;

  if( max_buckets <= 0 ) return 0;

  if( max_buckets <= max_points )
  {
    max_points = max_buckets;
  }
  else
  {
    increment = ( (int) floor( max_buckets / (double) max_points ) ) + 1;
  }

  double resource_max=0;
  for( int i=0; i < max_buckets; i++ ) 
  {
    if( p -> timeline_resource[ i ] > resource_max ) 
    {
      resource_max = p -> timeline_resource[ i ];
    }
  }
  double resource_range  = 60.0;
  double resource_adjust = resource_range / resource_max;

  char buffer[ 1024 ];

  s = "http://chart.apis.google.com/chart?";
  s += "chs=425x150";
  s += "&";
  s += "cht=lc";
  s += "&";
  s += "chd=s:";
  for( int i=0; i < max_buckets; i += increment ) 
  {
    s += simple_encoding( (int) ( p -> timeline_resource[ i ] * resource_adjust ) );
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chds=0,%.0f", resource_range ); s += buffer;
  s += "&";
  s += "chxt=x,y";
  s += "&";
  snprintf( buffer, sizeof(buffer), "chxl=0:|0|sec=%d|1:|0|max=%.0f", max_buckets, resource_max ); s += buffer;
  s += "&";
  snprintf( buffer, sizeof(buffer), "chtt=%s|Resource+(%s)+Timeline", p -> name(), util_t::resource_type_string( p -> primary_resource() ) ); s += buffer;
  s += "&";
  s += "chts=000000,20";

  return s.c_str();
}

// report_t::chart_distribution_dps ===========================================

const char* report_t::chart_distribution_dps( std::string& s, player_t* p )
{
  int max_buckets = p -> distribution_dps.size();

  int32_t count_max=0;
  for( int i=0; i < max_buckets; i++ ) 
  {
    if( p -> distribution_dps[ i ] > count_max ) 
    {
      count_max = p -> distribution_dps[ i ];
    }
  }

  char buffer[ 1024 ];

  s = "http://chart.apis.google.com/chart?";
  s += "chs=525x130";
  s += "&";
  s += "cht=bvs";
  s += "&";
  s += "chd=t:";
  for( int i=0; i < max_buckets; i++ ) 
  {
    snprintf( buffer, sizeof(buffer), "%s%d", (i?",":""), p -> distribution_dps[ i ] ); s += buffer;
  }
  s += "&";
  snprintf( buffer, sizeof(buffer), "chds=0,%d", count_max ); s += buffer;
  s += "&";
  s += "chbh=5";
  s += "&";
  s += "chxt=x";
  s += "&";
  snprintf( buffer, sizeof(buffer), "chxl=0:|min=%.0f|avg=%.0f|max=%.0f", p -> dps_min, p -> dps, p -> dps_max ); s += buffer;
  s += "&";
  snprintf( buffer, sizeof(buffer), "chxp=0,1,%.0f,100", 100.0 * ( p -> dps - p -> dps_min ) / ( p -> dps_max - p -> dps_min ) ); s += buffer;
  s += "&";
  snprintf( buffer, sizeof(buffer), "chtt=%s+DPS+Distribution", p -> name() ); s += buffer;
  s += "&";
  s += "chts=000000,20";

  return s.c_str();
}

// report_t::gear_weights_lootrank ============================================

const char* report_t::gear_weights_lootrank( std::string& s,
					     player_t*    p )
{
  char buffer[ 1024 ];

  s = "http://www.lootrank.com/wow/wr.asp?";

  switch( p -> type )
  {
  case DEATH_KNIGHT: s += "Cla=2048"; break;
  case DRUID:        s += "Cla=1024"; break;
  case HUNTER:       s += "Cla=4";    break; 
  case MAGE:         s += "Cla=128";  break;
  case PALADIN:      s += "Cla=2";    break;
  case PRIEST:       s += "Cla=16";   break;
  case ROGUE:        s += "Cla=8";    break;
  case SHAMAN:       s += "Cla=64";   break;
  case WARLOCK:      s += "Cla=256";  break;
  case WARRIOR:      s += "Cla=1";    break;
  default: assert(0);
  }

  const char* attr_prefix[] = { "None", "Str", "Agi", "Sta", "Int", "Spi" };

  for( int j=0; j < ATTRIBUTE_MAX; j++ )
  {
    if( sim -> scaling -> gear.attribute[ j ] ) 
    {
      snprintf( buffer, sizeof(buffer), "&%s=%.2f", attr_prefix[ j ], p -> scaling.attribute[ j ] );
      s += buffer;
    }
  }
  if( sim -> scaling -> gear.spell_power ) 
  { 
    snprintf( buffer, sizeof(buffer), "&spd=%.2f", p -> scaling.spell_power ); 
    s += buffer; 
  }
  if( sim -> scaling -> gear.attack_power ) 
  { 
    snprintf( buffer, sizeof(buffer), "&map=%.2f", p -> scaling.attack_power );
    s += buffer; 
  }
  if( sim -> scaling -> gear.expertise_rating ) 
  {
    snprintf( buffer, sizeof(buffer), "&Exp=%.2f", p -> scaling.expertise_rating );
    s += buffer; 
  }
  if( sim -> scaling -> gear.armor_penetration_rating )
  {
    snprintf( buffer, sizeof(buffer), "&arp=%.2f", p -> scaling.armor_penetration_rating );
    s += buffer;
  }
  if( sim -> scaling -> gear.hit_rating )
  {
    snprintf( buffer, sizeof(buffer), "&mhit=%.2f", p -> scaling.hit_rating );
    s += buffer;
  }
  if( sim -> scaling -> gear.crit_rating )
  {
    snprintf( buffer, sizeof(buffer), "&mcr=%.2f", p -> scaling.crit_rating );
    s += buffer;
  }
  if( sim -> scaling -> gear.haste_rating )
  {
    snprintf( buffer, sizeof(buffer), "&mh=%.2f", p -> scaling.haste_rating );
    s += buffer;
  }

  s += "&Ver=6&usr=&ser=&grp=www";

  return s.c_str();
}

// report_t::gear_weights_wowhead =============================================

const char* report_t::gear_weights_wowhead( std::string& s,
					    player_t*    p )
{
  char buffer[ 1024 ];

  s = "http://www.wowhead.com/?items&filter=";

  switch( p -> type )
  {
  case DEATH_KNIGHT: s += "ub=6;";  break;
  case DRUID:        s += "ub=11;"; break;
  case HUNTER:       s += "ub=3;";  break; 
  case MAGE:         s += "ub=8;";  break;
  case PALADIN:      s += "ub=2;";  break;
  case PRIEST:       s += "ub=5;";  break;
  case ROGUE:        s += "ub=4;";  break;
  case SHAMAN:       s += "ub=7;";  break;
  case WARLOCK:      s += "ub=9;";  break;
  case WARRIOR:      s += "ub=1;";  break;
  default: assert(0);
  }

  s += "gm=4;gb=1;";

  std::vector<int> prefix_list;
  std::vector<double> value_list;

  int attr_prefix[] = { -1, 20, 21, 22, 23, 24 };

  for( int j=0; j < ATTRIBUTE_MAX; j++ )
  {
    if( sim -> scaling -> gear.attribute[ j ] ) 
    {
      prefix_list.push_back( attr_prefix[ j ] );
       value_list.push_back( p -> scaling.attribute[ j ] );
    }
  }
  if( sim -> scaling -> gear.spell_power ) 
  { 
    prefix_list.push_back( 123 );
     value_list.push_back( p -> scaling.spell_power ); 
  }
  if( sim -> scaling -> gear.attack_power ) 
  { 
    prefix_list.push_back( 77 );
     value_list.push_back( p -> scaling.attack_power );
  }
  if( sim -> scaling -> gear.expertise_rating ) 
  {
    prefix_list.push_back( 117 );
     value_list.push_back( p -> scaling.expertise_rating );
  }
  if( sim -> scaling -> gear.armor_penetration_rating )
  {
    prefix_list.push_back( 114 );
     value_list.push_back( p -> scaling.armor_penetration_rating );
  }
  if( sim -> scaling -> gear.hit_rating )
  {
    prefix_list.push_back( 119 );
     value_list.push_back( p -> scaling.hit_rating );
  }
  if( sim -> scaling -> gear.crit_rating )
  {
    prefix_list.push_back( 96 );
     value_list.push_back( p -> scaling.crit_rating );
  }
  if( sim -> scaling -> gear.haste_rating )
  {
    prefix_list.push_back( 103 );
     value_list.push_back( p -> scaling.haste_rating );
  }

  s += "wt=";
  for( unsigned i=0; i < prefix_list.size(); i++ )
  {
    snprintf( buffer, sizeof(buffer), "%s%d", ( i ? ":" : "" ), prefix_list[ i ] );
    s += buffer;
  }
  s += ";";

  s += "wtv=";
  for( unsigned i=0; i < value_list.size(); i++ )
  {
    snprintf( buffer, sizeof(buffer), "%s%.2f", ( i ? ":" : "" ), value_list[ i ] );
    s += buffer;
  }
  s += ";";

  return s.c_str();
}

// report_t::html_scale_factors ===============================================

void report_t::html_scale_factors()
{
  if( ! sim -> scaling -> calculate_scale_factors ) return;

  fprintf( sim -> html_file, "<h1>DPS Scale Factors (dps increase per unit stat)</h1>\n" );

  fprintf( sim -> html_file, "<TABLE BORDER CELLPADDING=4>\n" );
  fprintf( sim -> html_file, "<TR> <TH>profile</TH>" );
  for( int j=0; j < ATTRIBUTE_MAX; j++ )
  {
    if( sim -> scaling -> gear.attribute[ j ] ) 
    {
      fprintf( sim -> html_file, " <TH>%s</TH>", util_t::attribute_type_string( j ) );
    }
  }
  if( sim -> scaling -> gear.spell_power              ) fprintf( sim -> html_file, " <TH>spell power</TH>" );
  if( sim -> scaling -> gear.attack_power             ) fprintf( sim -> html_file, " <TH>attack power</TH>" );
  if( sim -> scaling -> gear.expertise_rating         ) fprintf( sim -> html_file, " <TH>expertise</TH>" );
  if( sim -> scaling -> gear.armor_penetration_rating ) fprintf( sim -> html_file, " <TH>armor pen</TH>" );
  if( sim -> scaling -> gear.hit_rating               ) fprintf( sim -> html_file, " <TH>hit</TH>" );
  if( sim -> scaling -> gear.crit_rating              ) fprintf( sim -> html_file, " <TH>crit</TH>" );
  if( sim -> scaling -> gear.haste_rating             ) fprintf( sim -> html_file, " <TH>haste</TH>" );
  fprintf( sim -> html_file, " <TH>lootrank</TH> <TH>wowhead</TH> </TR>\n" );

  std::string buffer;
  int num_players = sim -> players_by_name.size();

  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_name[ i ];

    fprintf( sim -> html_file, "<TR> <TD>%s</TD>", p -> name() );

    for( int j=0; j < ATTRIBUTE_MAX; j++ )
    {
      if( sim -> scaling -> gear.attribute[ j ] ) 
      {
	fprintf( sim -> html_file, " <TD>%.2f</TD>", p -> scaling.attribute[ j ] );
      }
    }
    if( sim -> scaling -> gear.spell_power              ) fprintf( sim -> html_file, " <TD>%.2f</TD>", p -> scaling.spell_power              );
    if( sim -> scaling -> gear.attack_power             ) fprintf( sim -> html_file, " <TD>%.2f</TD>", p -> scaling.attack_power             );
    if( sim -> scaling -> gear.expertise_rating         ) fprintf( sim -> html_file, " <TD>%.2f</TD>", p -> scaling.expertise_rating         );
    if( sim -> scaling -> gear.armor_penetration_rating ) fprintf( sim -> html_file, " <TD>%.2f</TD>", p -> scaling.armor_penetration_rating );
    if( sim -> scaling -> gear.hit_rating               ) fprintf( sim -> html_file, " <TD>%.2f</TD>", p -> scaling.hit_rating               );
    if( sim -> scaling -> gear.crit_rating              ) fprintf( sim -> html_file, " <TD>%.2f</TD>", p -> scaling.crit_rating              );
    if( sim -> scaling -> gear.haste_rating             ) fprintf( sim -> html_file, " <TD>%.2f</TD>", p -> scaling.haste_rating             );

    fprintf( sim -> html_file, " <TD><a href=\"%s\"> lootrank</a></TD>", gear_weights_lootrank( buffer, p ) );
    fprintf( sim -> html_file, " <TD><a href=\"%s\"> wowhead </a></TD>", gear_weights_wowhead ( buffer, p ) );

    fprintf( sim -> html_file, " </TR>\n" );
  }
  fprintf( sim -> html_file, "</TABLE>\n" );
}

void report_t::html_trigger_menu()
{
  fprintf( sim -> html_file, "<a href=\"javascript:hideElement(document.getElementById('trigger_menu'));\">-</a> " );
  fprintf( sim -> html_file, "<a href=\"javascript:showElement(document.getElementById('trigger_menu'));\">+</a> " );
  fprintf( sim -> html_file, "Menu\n" );

  fprintf( sim -> html_file, "<div id=\"trigger_menu\" style=\"display:none;\">" );
  
  fprintf( sim -> html_file, "<a href=\"javascript:hideElements(document.getElementById('players').getElementsByTagName('img'));\">-</a> " );
  fprintf( sim -> html_file, "<a href=\"javascript:showElements(document.getElementById('players').getElementsByTagName('img'));\">+</a> " );
  fprintf( sim -> html_file, "All Charts<br/>\n" );

  fprintf( sim -> html_file, "<a href=\"javascript:hideElements(document.getElementsByName('chart_dpet'));\">-</a> " );
  fprintf( sim -> html_file, "<a href=\"javascript:showElements(document.getElementsByName('chart_dpet'));\">+</a> " );
  fprintf( sim -> html_file, "DamagePerExecutionTime<br/>\n" );

  fprintf( sim -> html_file, "<a href=\"javascript:hideElements(document.getElementsByName('chart_uptimes'));\">-</a> " );
  fprintf( sim -> html_file, "<a href=\"javascript:showElements(document.getElementsByName('chart_uptimes'));\">+</a> " );
  fprintf( sim -> html_file, "Up-Times and Procs<br/>\n" );

  fprintf( sim -> html_file, "<a href=\"javascript:hideElements(document.getElementsByName('chart_sources'));\">-</a> " );
  fprintf( sim -> html_file, "<a href=\"javascript:showElements(document.getElementsByName('chart_sources'));\">+</a> " );
  fprintf( sim -> html_file, "Damage Sources<br/>\n" );

  fprintf( sim -> html_file, "<a href=\"javascript:hideElements(document.getElementsByName('chart_gains'));\">-</a> " );
  fprintf( sim -> html_file, "<a href=\"javascript:showElements(document.getElementsByName('chart_gains'));\">+</a> " );
  fprintf( sim -> html_file, "Resource Gains<br/>\n" );

  fprintf( sim -> html_file, "<a href=\"javascript:hideElements(document.getElementsByName('chart_dps_timeline'));\">-</a> " );
  fprintf( sim -> html_file, "<a href=\"javascript:showElements(document.getElementsByName('chart_dps_timeline'));\">+</a> " );
  fprintf( sim -> html_file, "DPS Timeline<br/>\n" );

  fprintf( sim -> html_file, "<a href=\"javascript:hideElements(document.getElementsByName('chart_dps_distribution'));\">-</a> " );
  fprintf( sim -> html_file, "<a href=\"javascript:showElements(document.getElementsByName('chart_dps_distribution'));\">+</a> " );
  fprintf( sim -> html_file, "DPS Distribution<br/>\n" );

  fprintf( sim -> html_file, "<a href=\"javascript:hideElements(document.getElementsByName('chart_resource_timeline'));\">-</a> " );
  fprintf( sim -> html_file, "<a href=\"javascript:showElements(document.getElementsByName('chart_resource_timeline'));\">+</a> " );
  fprintf( sim -> html_file, "Resource Timeline<br/>\n" );
  
  fprintf( sim -> html_file, "</div>" );//trigger_menu
  
  fprintf( sim -> html_file, "<hr>\n" );
}

// report_t::chart_html ======================================================

void report_t::chart_html()
{
  int num_players = sim -> players_by_name.size();
  assert( num_players != 0 );

  std::string buffer;
  const char* img;

  fprintf( sim -> html_file, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n" );
  fprintf( sim -> html_file, "<html>\n" );

  fprintf( sim -> html_file, "<head>\n" );
  fprintf( sim -> html_file, "<title>Simulationcraft results</title>\n" );
  fprintf( sim -> html_file, "<script type=\"text/javascript\">\n"
      "function hideElement(el) {if (el) el.style.display='none';}\n"
      "function showElement(el) {if (el) el.style.display='';}\n"
      "function hideElements(els) {if (els) {"
      "for (var i = 0; i < els.length; i++) hideElement(els[i]);"
      "}}\n"
      "function showElements(els) {if (els) {"
      "for (var i = 0; i < els.length; i++) showElement(els[i]);"
      "}}\n"
      "</script>\n" );
  fprintf( sim -> html_file, "</head>\n" );
  fprintf( sim -> html_file, "<body>\n" );

  fprintf( sim -> html_file, "<h1>Raid</h1>\n" );

  fprintf( sim -> html_file, "\n<! DPS Ranking: >\n" );
  fprintf( sim -> html_file, "<img src=\"%s\" />\n", chart_raid_dps( buffer ) );

  fprintf( sim -> html_file, "\n<! Gear Overview: >\n" );
  fprintf( sim -> html_file, "<img src=\"%s\" />", chart_raid_gear( buffer ) );

  img = chart_raid_downtime( buffer );
  if( img )
  {
    fprintf( sim -> html_file, "\n<! Raid Downtime: >\n" );
    fprintf( sim -> html_file, "<img src=\"%s\" />", img );
  }

  img = chart_raid_uptimes( buffer );
  if( img )
  {
    fprintf( sim -> html_file, "\n<! Global Up-Times: >\n" );
    fprintf( sim -> html_file, "<img src=\"%s\" />", img );
  }

  std::vector<std::string> images;
  int count = chart_raid_dpet( buffer, images );
  for( int i=0; i < count; i++ )
  {
    fprintf( sim -> html_file, "\n<! Raid Damage Per Execute Time: >\n" );
    fprintf( sim -> html_file, "<img src=\"%s\" />", images[ i ].c_str() );
  }
  fprintf( sim -> html_file, "<hr>\n" );

  html_scale_factors();
  fprintf( sim -> html_file, "<hr>\n" );

  html_trigger_menu();

  fprintf( sim -> html_file, "<div id=\"players\">\n");
  
  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_name[ i ];

    fprintf( sim -> html_file, "<h1><a href=\"%s\">%s</a></h1>\n", p -> talents_str.c_str(), p -> name() );

    img = chart_action_dpet( buffer, p );
    if( img )
    {
      fprintf( sim -> html_file, "\n<! %s Damage Per Execute Time: >\n", p -> name() );
      fprintf( sim -> html_file, "<img name=\"chart_dpet\" src=\"%s\" />", img );
    }

    img = chart_uptimes_and_procs( buffer, p );
    if( img )
    {
      fprintf( sim -> html_file, "\n<! %s Up-Times and Procs: >\n", p -> name() );
      fprintf( sim -> html_file, "<img name=\"chart_uptimes\" src=\"%s\" />", img );
    }

    fprintf( sim -> html_file, "<br>\n" );

    img = chart_action_dmg( buffer, p );
    if( img )
    {
      fprintf( sim -> html_file, "\n<! %s Damage Sources: >\n", p -> name() );
      fprintf( sim -> html_file, "<img name=\"chart_sources\" src=\"%s\" />", img );
    }
    
    img = chart_gains( buffer, p );
    if( img )
    {
      fprintf( sim -> html_file, "\n<! %s Resource Gains: >\n", p -> name() );
      fprintf( sim -> html_file, "<img name=\"chart_gains\" src=\"%s\" />", img );
    }
    
    fprintf( sim -> html_file, "<br>\n" );

    fprintf( sim -> html_file, "\n<! %s DPS Timeline: >\n", p -> name() );
    fprintf( sim -> html_file, "<img name=\"chart_dps_timeline\" src=\"%s\" />\n", chart_timeline_dps( buffer, p ) );

    fprintf( sim -> html_file, "\n<! %s DPS Distribution: >\n", p -> name() );
    fprintf( sim -> html_file, "<img name=\"chart_dps_distribution\" src=\"%s\" />\n", chart_distribution_dps( buffer, p ) );

    fprintf( sim -> html_file, "\n<! %s Resource Timeline: >\n", p -> name() );
    fprintf( sim -> html_file, "<img name=\"chart_resource_timeline\" src=\"%s\" />\n", chart_timeline_resource( buffer, p ) );

    fprintf( sim -> html_file, "<hr>\n" );
  }
  
  fprintf( sim -> html_file, "</div>\n" ); //players
 
  fprintf( sim -> html_file, "</body>\n" );
  fprintf( sim -> html_file, "</html>" );
}

// report_t::wiki_scale_factors ===============================================

void report_t::wiki_scale_factors()
{
  if( ! sim -> scaling -> calculate_scale_factors ) return;

  fprintf( sim -> wiki_file, "----\n" );
  fprintf( sim -> wiki_file, "----\n" );
  fprintf( sim -> wiki_file, "----\n" );
  fprintf( sim -> wiki_file, "== DPS Scale Factors (dps increase per unit stat) ==\n" );

  fprintf( sim -> wiki_file, "|| ||" );
  for( int j=0; j < ATTRIBUTE_MAX; j++ )
  {
    if( sim -> scaling -> gear.attribute[ j ] ) 
    {
      fprintf( sim -> wiki_file, "  %s ||", util_t::attribute_type_string( j ) );
    }
  }
  if( sim -> scaling -> gear.spell_power              ) fprintf( sim -> wiki_file, " spell power  ||" );
  if( sim -> scaling -> gear.attack_power             ) fprintf( sim -> wiki_file, " attack power ||" );
  if( sim -> scaling -> gear.expertise_rating         ) fprintf( sim -> wiki_file, " expertise    ||" );
  if( sim -> scaling -> gear.armor_penetration_rating ) fprintf( sim -> wiki_file, " armor pen    ||" );
  if( sim -> scaling -> gear.hit_rating               ) fprintf( sim -> wiki_file, " hit          ||" );
  if( sim -> scaling -> gear.crit_rating              ) fprintf( sim -> wiki_file, " crit         ||" );
  if( sim -> scaling -> gear.haste_rating             ) fprintf( sim -> wiki_file, " haste        ||" );
  fprintf( sim -> wiki_file, " lootrank || wowhead ||\n" );

  std::string buffer;
  int num_players = sim -> players_by_name.size();

  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_name[ i ];

    fprintf( sim -> wiki_file, "|| %-25s ||", p -> name() );

    for( int j=0; j < ATTRIBUTE_MAX; j++ )
    {
      if( sim -> scaling -> gear.attribute[ j ] ) 
      {
	fprintf( sim -> wiki_file, " %.2f ||", p -> scaling.attribute[ j ] );
      }
    }
    if( sim -> scaling -> gear.spell_power              ) fprintf( sim -> wiki_file, " %.2f ||", p -> scaling.spell_power              );
    if( sim -> scaling -> gear.attack_power             ) fprintf( sim -> wiki_file, " %.2f ||", p -> scaling.attack_power             );
    if( sim -> scaling -> gear.expertise_rating         ) fprintf( sim -> wiki_file, " %.2f ||", p -> scaling.expertise_rating         );
    if( sim -> scaling -> gear.armor_penetration_rating ) fprintf( sim -> wiki_file, " %.2f ||", p -> scaling.armor_penetration_rating );
    if( sim -> scaling -> gear.hit_rating               ) fprintf( sim -> wiki_file, " %.2f ||", p -> scaling.hit_rating               );
    if( sim -> scaling -> gear.crit_rating              ) fprintf( sim -> wiki_file, " %.2f ||", p -> scaling.crit_rating              );
    if( sim -> scaling -> gear.haste_rating             ) fprintf( sim -> wiki_file, " %.2f ||", p -> scaling.haste_rating             );

    fprintf( sim -> wiki_file, " [%s lootrank] ||", gear_weights_lootrank( buffer, p ) );
    fprintf( sim -> wiki_file, " [%s wowhead ] ||", gear_weights_wowhead ( buffer, p ) );

    fprintf( sim -> wiki_file, "\n" );
  }
}

// report_t::chart_wiki ======================================================

void report_t::chart_wiki()
{
  int num_players = sim -> players_by_name.size();
  assert( num_players != 0 );

  std::string raid_dps      = "No chart for Raid DPS";
  std::string raid_gear     = "No chart for Raid Gear Overview";
  std::string raid_downtime = "No chart for Raid Down-Time";
  std::string raid_uptimes  = "No chart for Raid Up-Times";
  std::string raid_dpet_1   = "No chart for Raid Damage Per Execute Time";
  std::string raid_dpet_2   = "No chart for Raid Damage Per Execute Time";
  std::string buffer;
  const char* img;

  img = chart_raid_dps( buffer );
  if( img )
  {
    raid_dps = img;
    raid_dps += "&dummy=dummy.png";
  }
  img = chart_raid_gear( buffer );
  if( img )
  {
    raid_gear = img;
    raid_gear += "&dummy=dummy.png";
  }
  img = chart_raid_downtime( buffer );
  if( img )
  {
    raid_downtime = img;
    raid_downtime += "&dummy=dummy.png";
  }
  img = chart_raid_uptimes( buffer );
  if( img )
  {
    raid_uptimes = img;
    raid_uptimes += "&dummy=dummy.png";
  }

  std::vector<std::string> images;
  int count = chart_raid_dpet( buffer, images );
  img = ( count >= 1 ? images[ 0 ].c_str() : 0 );
  if( img )
  {
    raid_dpet_1 = img;
    raid_dpet_1 += "&dummy=dummy.png";
  }
  img = ( count >= 2 ? images[ 1 ].c_str() : 0 );
  if( img )
  {
    raid_dpet_2 = img;
    raid_dpet_2 += "&dummy=dummy.png";
  }

  fprintf( sim -> wiki_file, "----\n" );
  fprintf( sim -> wiki_file, "----\n" );
  fprintf( sim -> wiki_file, "----\n" );
  fprintf( sim -> wiki_file, "= Raid Charts =\n" );
  fprintf( sim -> wiki_file, "|| %s || %s ||\n", raid_dps.c_str(),      raid_gear.c_str() );
  fprintf( sim -> wiki_file, "|| %s || %s ||\n", raid_downtime.c_str(), raid_uptimes.c_str() );
  fprintf( sim -> wiki_file, "|| %s || %s ||\n", raid_dpet_1.c_str(),   raid_dpet_2.c_str() );
  fprintf( sim -> wiki_file, "\n" );

  wiki_scale_factors();

  for( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_name[ i ];

    std::string action_dpet       = "No chart for Damage Per Execute Time";
    std::string uptimes_and_procs = "No chart for Up-Times and Procs";
    std::string action_dmg        = "No chart for Damage Sources";
    std::string gains             = "No chart for Resource Gains";
    std::string timeline_dps      = "No chart for DPS Timeline";
    std::string distribution_dps  = "No chart for DPS Distribution";
    std::string timeline_resource = "No chart for Resource Timeline";

    img = chart_action_dpet( buffer, p );
    if( img )
    {
      action_dpet = img;
      action_dpet += "&dummy=dummy.png";
    }
    img = chart_uptimes_and_procs( buffer, p );
    if( img )
    {
      uptimes_and_procs = img;
      uptimes_and_procs += "&dummy=dummy.png";
    }
    img = chart_action_dmg( buffer, p );
    if( img )
    {
      action_dmg = img;
      action_dmg += "&dummy=dummy.png";
    }
    img = chart_gains( buffer, p );
    if( img )
    {
      gains = img;
      gains += "&dummy=dummy.png";
    }
    img = chart_timeline_dps( buffer, p );
    if( img )
    {
      timeline_dps = img;
      timeline_dps += "&dummy=dummy.png";
    }
    img = chart_distribution_dps( buffer, p );
    if( img )
    {
      distribution_dps = img;
      distribution_dps += "&dummy=dummy.png";
    }
    img = chart_timeline_resource( buffer, p );
    if( img )
    {
      timeline_resource = img;
      timeline_resource += "&dummy=dummy.png";
    }

    fprintf( sim -> wiki_file, "\n" );
    fprintf( sim -> wiki_file, "----\n" );
    fprintf( sim -> wiki_file, "----\n" );
    fprintf( sim -> wiki_file, "----\n" );
    fprintf( sim -> wiki_file, "= %s =\n", p -> name() );
    fprintf( sim -> wiki_file, "[%s Talents]\n", p -> talents_str.c_str() );
    fprintf( sim -> wiki_file, "\n" );
    fprintf( sim -> wiki_file, "|| %s || %s ||\n", action_dpet.c_str(), uptimes_and_procs.c_str() );
    fprintf( sim -> wiki_file, "|| %s || %s ||\n", action_dmg.c_str(),  gains.c_str() );
    fprintf( sim -> wiki_file, "|| %s || %s ||\n", timeline_dps.c_str(), distribution_dps.c_str() );
    fprintf( sim -> wiki_file, "|| %s || ||\n", timeline_resource.c_str() );
  }
}

// report_t::chart ===========================================================

void report_t::chart()
{
  if( ! report_chart ) return;

  int num_players = sim -> players_by_name.size();
  if( num_players == 0 ) return;

  if( sim -> total_seconds == 0 ) return;

  if( ! sim -> html_file_str.empty() )
  {
    sim -> html_file = fopen( sim -> html_file_str.c_str(), "w" );
    if( ! sim -> html_file )
    {
      fprintf( stderr, "simcraft: Unable to open html file '%s'\n", sim -> html_file_str.c_str() );
      exit(0);
    }
    chart_html();
    fclose( sim -> html_file );
  }

  if( ! sim -> wiki_file_str.empty() )
  {
    sim -> wiki_file = fopen( sim -> wiki_file_str.c_str(), "w" );
    if( ! sim -> wiki_file )
    {
      fprintf( stderr, "simcraft: Unable to open wiki file '%s'\n", sim -> wiki_file_str.c_str() );
      exit(0);
    }
    chart_wiki();
    fclose( sim -> wiki_file );
  }
}

// report_t::timestamp ======================================================

void report_t::timestamp( sim_t* sim )
{
  if( sim -> timestamp ) 
  {
    fprintf( sim -> output_file, "%-8.2f ", sim -> current_time );
  }
}

// report_t::va_printf ======================================================

void report_t::va_printf( sim_t*      sim, 
			  const char* format,
			  va_list     vap )
{
  timestamp( sim );
  vfprintf( sim -> output_file, format, vap );
  fprintf( sim -> output_file, "\n" );
  fflush( sim -> output_file );
}

