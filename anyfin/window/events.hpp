
#pragma once

struct User_Quit_Event {};

struct Platform_Event {
  enum struct Value: u32 {
    Quit,
  };

  using enum Value;

  Value tag;

  union {
    User_Quit_Event quit;
  };

  Platform_Event (User_Quit_Event event)
    : tag  { Quit },
      quit { event }
  {}
  
};
