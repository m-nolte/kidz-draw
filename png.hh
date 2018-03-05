#ifndef PNG_HH
#define PNG_HH

#include <cassert>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>

#include <png.h>

namespace png
{

  // Types
  // -----

  typedef png_byte byte_t;
  typedef png_uint_32 uint32_t;



  // color_type
  // ----------

  enum class color_type_t
    : int
  {
    gray = PNG_COLOR_TYPE_GRAY,
    palette = PNG_COLOR_TYPE_PALETTE,
    rgb = PNG_COLOR_TYPE_RGB,
    rgb_alpha = PNG_COLOR_TYPE_RGB_ALPHA,
    gray_alpha = PNG_COLOR_TYPE_GRAY_ALPHA
  };



  // info_t
  // ------

  class info_t
  {
    struct deleter
    {
      explicit deleter ( const png_struct *p = nullptr ) : png( p ) {}

      void operator() ( png_info *p ) { if( p ) png_destroy_info_struct( png, &p ); }

      const png_struct *png;
    };

  public:
    info_t () = default;

    explicit info_t ( const png_struct *png )
      : info_( png_create_info_struct( png ), deleter( png ) )
    {}

    uint32_t bit_depth () const { return png_get_bit_depth( get_png(), get_info() ); }
    uint32_t channels () const { return png_get_channels( get_png(), get_info() ); }
    color_type_t color_type () const { return static_cast< color_type_t >( png_get_color_type( get_png(), get_info() ) ); }

    uint32_t image_width () const { return png_get_image_width( get_png(), get_info() ); }
    uint32_t image_height () const { return png_get_image_height( get_png(), get_info() ); }

    std::array< uint32_t, 2 > image_size () const { return { image_width(), image_height() }; }

    png_info *get_info () const { return info_.get(); }
    const png_struct *get_png () const { return info_.get_deleter().png; }

  private:
    std::unique_ptr< png_info, deleter > info_;
  };



  // input
  // -----

  class input
  {
    struct deleter
    {
      void operator() ( png_struct *p ) { if( p ) png_destroy_read_struct( &p, nullptr, nullptr ); }
    };

  public:
    input ( std::istream &in )
      : png_( png_create_read_struct( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr ) )
    {
      png_set_read_fn( get_png(), &in, read_data );
    }

    info_t read_info ()
    {
      info_t info( get_png() );
      png_read_info( get_png(), info.get_info() );
      return info;
    }

    void read_image ( byte_t **rows ) { png_read_image( get_png(), rows ); }

    std::unique_ptr< byte_t[] > read_image ( std::size_t pitch, std::size_t height, bool flip = false )
    {
      std::unique_ptr< byte_t[] > image( new byte_t[ pitch * height ] );
      std::unique_ptr< byte_t *[] > rows( new byte_t *[ height ] );
      for( std::size_t i = 0; i < height; ++i )
        rows[ i ] = image.get() + pitch * (flip ? height - (i+1) : i);
      read_image( rows.get() );
      return image;
    }

    png_struct *get_png () const { return png_.get(); }

  private:
    static void read_data ( png_struct *png, byte_t *data, png_size_t length )
    {
      std::istream &in = *static_cast< std::istream * >( png_get_io_ptr( png ) );
      in.read( reinterpret_cast< char * >( data ), length );
    }

    std::unique_ptr< png_struct, deleter > png_;
  };



  // output
  // ------

  class output
  {
    struct deleter
    {
      void operator() ( png_struct *p ) { if( p ) png_destroy_write_struct( &p, nullptr ); }
    };

  public:
    output ( std::ostream &out )
      : png_( png_create_write_struct( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr ) )
    {
      png_set_write_fn( get_png(), &out, write_data, flush_data );
    }

    info_t create_info ( uint32_t width, uint32_t height, int bit_depth, color_type_t color_type )
    {
      info_t info( get_png() );
      png_set_IHDR( get_png(), info.get_info(), width, height, bit_depth, static_cast< int >( color_type ), PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
      return info;
    }

    void write_info ( info_t info )
    {
      assert( info.get_png() == get_png() );
      png_write_info( get_png(), info.get_info() );
    }

    void write_info( uint32_t width, uint32_t height, int bit_depth, color_type_t color_type )
    {
      write_info( create_info( width, height, bit_depth, color_type ) );
    }

    void write_image ( byte_t **rows ) { png_write_image( get_png(), rows ); }

    void write_image ( byte_t *image, std::size_t pitch, std::size_t height, bool flip = false )
    {
      std::unique_ptr< byte_t *[] > rows( new byte_t *[ height ] );
      for( std::size_t i = 0; i < height; ++i )
        rows[ i ] = image + pitch * (flip ? height - (i+1) : i);
      write_image( rows.get() );
    }

    void write_image ( std::unique_ptr< byte_t[] > image, std::size_t pitch, std::size_t height, bool flip = false )
    {
      write_image( image.get(), pitch, height, flip );
    }

    void write_end () { png_write_end( get_png(), nullptr ); }

    png_struct *get_png () const { return png_.get(); }

  private:
    static void write_data ( png_struct *png, byte_t *data, png_size_t length )
    {
      std::ostream &out = *static_cast< std::ostream * >( png_get_io_ptr( png ) );
      out.write( reinterpret_cast< char * >( data ), length );
    }

    static void flush_data ( png_struct *png )
    {
      std::ostream &out = *static_cast< std::ostream * >( png_get_io_ptr( png ) );
      out.flush();
    }

    std::unique_ptr< png_struct, deleter > png_;
  };

} // namespace png

#endif // #ifndef PNG_HH
