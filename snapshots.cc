#include <iomanip>
#include <regex>
#include <sstream>

#include <experimental/filesystem>

#include "snapshots.hh"


namespace filesystem = std::experimental::filesystem;


// Implementation of TooFrequentSnapShots
// --------------------------------------

const char *TooFrequentSnapShots::what () const noexcept
{
  return "too frequent snapshots, wait a second...";
}



// Implementation of SnapShots
// ---------------------------

SnapShots::SnapShots ()
{
  const std::regex pattern( "snapshot-([0-9]{4})-([0-9]{2})-([0-9]{2})-at-([0-9]{2})-([0-9]{2})[.]png" );
  for( const auto &entry : filesystem::directory_iterator( "." ) )
  {
    std::smatch subMatch;
    const std::string filename = entry.path().filename().string();
    if( !std::regex_match( filename, subMatch, pattern ) )
      continue;
    timeStamps_.emplace( subMatch[ 1 ].str(), subMatch[ 2 ].str(), subMatch[ 3 ].str(), subMatch[ 4 ].str(), subMatch[ 5 ].str() );
  }
}


bool SnapShots::exists ( const TimeStamp &timeStamp ) const
{
  std::lock_guard< std::mutex > lock( mutex_ );
  return (timeStamps_.find( timeStamp ) != timeStamps_.end());
}


std::string SnapShots::newSnapShot ()
{
  std::lock_guard< std::mutex > lock( mutex_ );
  auto result = timeStamps_.emplace();
  if( result.second )
    return toFileName( *result.first );
  else
    throw TooFrequentSnapShots();
}


std::vector< TimeStamp > SnapShots::timeStamps () const
{
  std::lock_guard< std::mutex > lock( mutex_ );
  std::vector< TimeStamp > result;
  result.reserve( timeStamps_.size() );
  for( const auto &timeStamp : timeStamps_ )
    result.push_back( timeStamp );
  return result;
}


std::vector< TimeStamp > SnapShots::timeStamps ( const Date &date ) const
{
  std::lock_guard< std::mutex > lock( mutex_ );
  const auto lower_bound = timeStamps_.lower_bound( TimeStamp( date, 0, 0 ) );
  const auto upper_bound = timeStamps_.upper_bound( TimeStamp( date, 23, 59 ) );
  return std::vector< TimeStamp >( lower_bound, upper_bound );
}


std::string SnapShots::toFileName ( const TimeStamp &timeStamp ) const
{
  return "snapshot-" + to_string( timeStamp ) + ".png";
}



// Implementation of to_string
// ---------------------------

std::string to_string ( const Date &date )
{
  std::ostringstream s;
  s << std::setfill( '0' )
    << std::setw( 4 ) << date.year << "-"
    << std::setw( 2 ) << date.month << "-"
    << std::setw( 2 ) << date.day;
  return s.str();
}


std::string to_string ( const TimeStamp &timeStamp )
{
  std::ostringstream s;
  s << to_string( timeStamp.date ) << "-at-"
    << std::setfill( '0' )
    << std::setw( 2 ) << timeStamp.hour << "-"
    << std::setw( 2 ) << timeStamp.minute;
  return s.str();
}
