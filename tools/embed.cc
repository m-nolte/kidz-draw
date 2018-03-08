#include <iostream>
#include <fstream>

int main ( int argc, char **argv )
{
  if( argc != 4 )
  {
    std::cerr << "Usage: " << argv[ 0 ] << " <binary> <symbol> <output>" << std::endl;
    return 1;
  }

  std::ifstream input( argv[ 1 ], std::ios::binary );
  if( !input )
  {
    std::cerr << "Unable to open binary file: '" << argv[ 1 ] << "'." << std::endl;
    return 1;
  }

  std::ofstream output( argv[ 3 ] );
  output << "#include <cstddef>" << std::endl;
  output << "#include <cstdint>" << std::endl;
  output << std::endl;
  output << "extern const std::uint8_t " << argv[ 2 ] << "_data[];" << std::endl;
  output << "extern const std::size_t " << argv[ 2 ] << "_size;" << std::endl;
  output << std::endl;
  output << "const std::uint8_t " << argv[ 2 ] << "_data[] = {" << std::endl;

  const char *hex = "0123456789ABCDEF";
  while( input )
  {
    char buffer[ 32 ];
    input.read( buffer, sizeof( buffer ) );
    const int count = input.gcount();
    output << "   ";
    for( int i = 0; i < count; ++i )
      output << " 0x" << hex[ (buffer[ i ] >> 4) & 0xf ] << hex[ buffer[ i ] & 0xf ] << ",";
    output << std::endl;
  }

  output << "  };" << std::endl;
  output << "const std::size_t " << argv[ 2 ] << "_size = sizeof( " << argv[ 2 ] << "_data );" << std::endl;
  return 0;
}
