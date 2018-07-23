#ifndef SNAPSHOTS_HH
#define SNAPSHOTS_HH

#include <ctime>

#include <chrono>
#include <mutex>
#include <set>
#include <string>
#include <vector>



// Day
// ---

struct Date
{
  Date ()
    : Date( std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() ) )
  {}

  explicit Date ( std::time_t time )
  {
    const std::tm *tm = std::localtime( &time );
    year = tm->tm_year + 1900;
    month = tm->tm_mon + 1;
    day = tm->tm_mday;
  }

  Date ( int year, int month, int day )
    : year( year ), month( month ), day( day )
  {}

  Date ( const std::string &year, const std::string &month, const std::string &day )
    : year( std::atoi( year.c_str() ) ), month( std::atoi( month.c_str() ) ), day( std::atoi( day.c_str() ) )
  {}

  bool operator== ( const Date &other ) const
  {
    return (year == other.year) && (month == other.month) && (day == other.day);
  }

  bool operator!= ( const Date &other ) const { return !(*this == other); }

  bool operator< ( const Date &other ) const
  {
    if( year != other.year )
      return (year < other.year);
    if( month != other.month )
      return (month < other.month);
    return day < other.day;
  }

  bool operator<= ( const Date &other ) const
  {
    if( year != other.year )
      return (year < other.year);
    if( month != other.month )
      return (month < other.month);
    return day <= other.day;
  }

  bool operator>= ( const Date &other ) const { return !(*this < other); }
  bool operator> ( const Date &other ) const { return !(*this <= other); }

  static Date today () { return Date( std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() ) ); }
  static Date yesterday () { return Date( std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() - std::chrono::hours( 24 ) ) ); }

  std::string toQuery () const
  {
    return "year=" + std::to_string( year ) + "&month=" + std::to_string( month ) + "&day=" + std::to_string( day );
  }

  int year, month, day;
};



// TimeStamp
// ---------

struct TimeStamp
{
  TimeStamp ()
    : TimeStamp( std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() ) )
  {}

  explicit TimeStamp ( std::time_t time )
  {
    const std::tm *tm = std::localtime( &time );
    date = Date( tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday );
    hour = tm->tm_hour;
    minute = tm->tm_min;
  }

  TimeStamp ( int year, int month, int day, int hour, int minute )
    : date( year, month, day ), hour( hour ), minute( minute )
  {}

  TimeStamp ( const std::string &year, const std::string &month, const std::string &day, const std::string &hour, const std::string &minute )
    : date( year, month, day ), hour( std::atoi( hour.c_str() ) ), minute( std::atoi( minute.c_str() ) )
  {}

  TimeStamp ( const Date &date, int hour, int minute )
    : date( date ), hour( hour ), minute( minute )
  {}

  bool operator== ( const TimeStamp &other ) const
  {
    return (date == other.date) && (hour == other.hour) && (minute == other.minute);
  }

  bool operator!= ( const TimeStamp &other ) const { return !(*this == other); }

  bool operator< ( const TimeStamp &other ) const
  {
    if( date != other.date )
      return (date < other.date);
    if( hour != other.hour )
      return (hour < other.hour);
    return (minute < other.minute);
  }

  bool operator<= ( const TimeStamp &other ) const
  {
    if( date != other.date )
      return (date < other.date);
    if( hour != other.hour )
      return (hour < other.hour);
    return (minute <= other.minute);
  }

  bool operator>= ( const TimeStamp &other ) const { return !(*this < other); }
  bool operator> ( const TimeStamp &other ) const { return !(*this <= other); }

  std::string toQuery () const
  {
    return date.toQuery() + "&hour=" + std::to_string( hour ) + "&minute=" + std::to_string( minute );
  }

  Date date;
  int hour, minute;
};



// TooFrequentSnapShots
// --------------------

struct TooFrequentSnapShots
  : public std::exception
{
  virtual const char *what () const noexcept override final;
};



// SnapShots
// ---------

class SnapShots
{
public:
  typedef std::set< TimeStamp >::const_iterator Iterator;

  SnapShots ();

  bool exists ( const TimeStamp &timeStap ) const;

  std::string newSnapShot ();

  std::vector< TimeStamp > timeStamps () const;

  std::vector< TimeStamp > timeStamps ( const Date &date ) const;

  std::string toFileName ( const TimeStamp &timeStamp ) const;

private:
  std::set< TimeStamp > timeStamps_;
  mutable std::mutex mutex_;
};



// to_string
// ---------

std::string to_string ( const Date &date );
std::string to_string ( const TimeStamp &timeStamp );

#endif // #ifndef SNAPSHOTS_HH
