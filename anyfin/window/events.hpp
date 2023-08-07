
#pragma once

struct User_Quit_Event {};

struct Key_Code {
  u32 value;
};

enum struct Key_Press_State: u32 {
  Undefined,
  Pressed,
  Released,
  Pressed_Repeated
};

struct Key_Modifiers {
  union {
    struct {
      u16 control;
      u16 shift;
      u16 alt;
      u16 unassigned;
    };

    u64 value;
  };
};

struct Keyboard_Input_Event {
  u8 key_code;

  // union {
  //   u64 value;
  //   struct {
  //     u64 modifiers: 3;
  //     u64 press_state: 3;
  //     u64 reserved: 18;
  //     u64 virtual_key: 8;
  //     u64 codepoint: 32;  // UTF-32BE Unicode codepoint // TODO: #encoding I should store this in utf-8
  //   };
  // };
};

struct System_Event {
  enum struct Value: u32 {
    Quit,
    Keyboard,
  };

  using enum Value;

  Value tag;

  union {
    User_Quit_Event      quit;
    Keyboard_Input_Event keyboard;
  };
};
