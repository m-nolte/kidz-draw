#ifndef CURSOR_HH
#define CURSOR_HH

// Cursor
// ------

class Cursor
{
public:
  explicit Cursor ( const std::string &fileName );
  ~Cursor ();

private:
  Cursor ( const Cursor & ) = delete;
  Cursor &operator= ( const Cursor & ) = delete;

  friend void setCursor ( const Cursor &cursor );

  SDL_Cursor *cursor_;
};



// Auxiliary Functions
// -------------------

void setCursor ( const Cursor &cursor );

void hideCursor ();
void showCursor ();

#endif // #ifndef CURSOR_HH
