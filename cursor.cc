#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <SDL.h>

#include "cursor.hh"


static SDL_Cursor *loadCursor ( const std::vector< std::string > &lines )
{
  std::array< int, 2 > size = { { 0, static_cast< int >( lines.size() ) } };

  for( const std::string &line : lines )
    size[ 0 ] = std::max( size[ 0 ], int( line.length() ) );

  size[ 0 ] = (size[ 0 ] + 7) & ~7;
  const int bytePerLine = size[ 0 ] / 8;
  std::unique_ptr< Uint8[] > data( new Uint8[ size[ 1 ] * bytePerLine ] );
  std::unique_ptr< Uint8[] > mask( new Uint8[ size[ 1 ] * bytePerLine ] );

  bool hasHotSpot = false;
  std::array< int, 2 > hotSpot = { { 0, 0 } };
  for( int j = 0; j < size[ 1 ]; ++j )
  {
    const std::string &line = lines[ j ];
    for( int i = 0; i < bytePerLine; ++i )
    {
      Uint8 &data_ij = data[ j*bytePerLine + i ];
      Uint8 &mask_ij = mask[ j*bytePerLine + i ];

      data_ij = mask_ij = Uint8( 0 );
      for( int k = 0; k < 8; ++k )
      {
        if( i*8 + k >= int( line.length() ) )
          break;

        const Uint8 bit = Uint8( 0x80 ) >> k;
        char c = line[ i*8 + k ];
        bool setHotSpot = false;
        switch( c )
        {
        case 'X':
          setHotSpot = true;
        case 'x':
          data_ij |= bit;
          mask_ij |= bit;
          break;

        case ':':
          setHotSpot = true;
        case '.':
          mask_ij |= bit;
          break;

        case '^':
          setHotSpot = true;
        case ' ':
          break;

        default:
          if( (int( c ) & 0xff) < ' ' )
            std::cerr << "Error in cursor: Invalid control character (" << int( c ) << ")." << std::endl;
          else
            std::cerr << "Error in cursor: Invalid character (" << c << ")." << std::endl;
          exit( 1 );
        }

        if( setHotSpot )
        {
          if( hasHotSpot )
          {
            std::cerr << "Error in cursor: Multiple hot spots." << std::endl;
            exit( 1 );
          }
          else
            hotSpot = { { i*8 + k, j } };
          hasHotSpot = true;
        }
      }
    }
  }

  if( !hasHotSpot )
  {
    std::cerr << "Error in cursor: No hot spot defined, using (0,0)." << std::endl;
    for( int i = 0; i < 2; ++i )
      hotSpot[ i ] = (hotSpot[ i ] >= size[ i ] ? 0 : hotSpot[ i ]);
  }

  return SDL_CreateCursor( data.get(), mask.get(), size[ 0 ], size[ 1 ], hotSpot[ 0 ], hotSpot[ 1 ] );
}


static SDL_Cursor *loadCursor ( std::istream &fin )
{
  std::vector< std::string > lines;
  while( true )
  {
    std::string line;
    std::getline( fin, line );
    if( !fin.good() )
      break;

    const std::size_t cpos = line.find( '#' );
    if( cpos == 0 )
      continue;
    if( cpos != line.npos )
      line = line.substr( 0, line.rfind( ' ', cpos ) );

    lines.push_back( std::move( line ) );
  }

  return loadCursor( lines );
}



// Implementation of Cursor
// ------------------------

Cursor::Cursor ( const std::string &fileName )
{
  std::ifstream fin( fileName );
  if( !fin )
  {
    std::cerr << "Unable to open '" << fileName << "'." << std::endl;
    exit( 1 );
  }

  cursor_ = loadCursor( fin );
}


Cursor::Cursor ( const std::vector< std::string > &lines )
  : cursor_( loadCursor( lines ) )
{}


Cursor::Cursor ( std::istream &input )
  : cursor_( loadCursor( input ) )
{}


Cursor::~Cursor ()
{
  SDL_FreeCursor( cursor_ );
}



// Auxiliary Functions
// -------------------

void setCursor ( const Cursor &cursor )
{
  SDL_SetCursor ( cursor.cursor_ );
}


void hideCursor ()
{
  SDL_ShowCursor( SDL_DISABLE );
}


void showCursor ()
{
  SDL_ShowCursor( SDL_ENABLE );
}
