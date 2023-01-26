/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(KeyCode, nkS32)
{
    KeyCode_Invalid,
    KeyCode_F1, KeyCode_F2, KeyCode_F3, KeyCode_F4, KeyCode_F5, KeyCode_F6,
    KeyCode_F7, KeyCode_F8, KeyCode_F9, KeyCode_F10, KeyCode_F11, KeyCode_F12,
    KeyCode_0, KeyCode_1, KeyCode_2, KeyCode_3, KeyCode_4,
    KeyCode_5, KeyCode_6, KeyCode_7, KeyCode_8, KeyCode_9,
    KeyCode_Q, KeyCode_W, KeyCode_E, KeyCode_R, KeyCode_T, KeyCode_Y, KeyCode_U,
    KeyCode_I, KeyCode_O, KeyCode_P, KeyCode_A, KeyCode_S, KeyCode_D, KeyCode_F,
    KeyCode_G, KeyCode_H, KeyCode_J, KeyCode_K, KeyCode_L, KeyCode_Z, KeyCode_X,
    KeyCode_C, KeyCode_V, KeyCode_B, KeyCode_N, KeyCode_M,
    KeyCode_Escape,
    KeyCode_Tab,
    KeyCode_CapsLock,
    KeyCode_Backspace,
    KeyCode_Delete,
    KeyCode_Home,
    KeyCode_End,
    KeyCode_Enter,
    KeyCode_Space,
    KeyCode_Grave,
    KeyCode_Up, KeyCode_Right, KeyCode_Down, KeyCode_Left,
    KeyCode_LeftShift, KeyCode_RightShift,
    KeyCode_LeftCtrl, KeyCode_RightCtrl,
    KeyCode_LeftAlt, KeyCode_RightAlt,
    KeyCode_TOTAL
};

NK_ENUM(MouseButton, nkS32)
{
    MouseButton_Invalid,
    MouseButton_Left,
    MouseButton_Middle,
    MouseButton_Right,
    MouseButton_TOTAL
};

NK_ENUM(GamepadButton, nkS32)
{
    GamepadButton_Invalid,
    GamepadButton_DPadUp, GamepadButton_DPadRight, GamepadButton_DPadDown, GamepadButton_DPadLeft,
    GamepadButton_A, GamepadButton_B, GamepadButton_X, GamepadButton_Y,
    GamepadButton_LeftStick, GamepadButton_RightStick,
    GamepadButton_LeftShoulder, GamepadButton_RightShoulder,
    GamepadButton_Select,
    GamepadButton_Start,
    GamepadButton_TOTAL
};

NK_ENUM(GamepadAxis, nkS32)
{
    GamepadAxis_Invalid,
    GamepadAxis_LeftStickX, GamepadAxis_RightStickX,
    GamepadAxis_LeftStickY, GamepadAxis_RightStickY,
    GamepadAxis_LeftTrigger, GamepadAxis_RightTrigger,
    GamepadAxis_TOTAL
};

static void init_input_system   (void);
static void quit_input_system   (void);
static void process_input_events(void* event);
static void update_input_state  (void);
static void reset_input_state   (void);

// Text Input
static nkChar* get_current_text_input(void);

// Keyboard
static nkBool is_key_down        (KeyCode code);
static nkBool is_key_up          (KeyCode code);
static nkBool is_key_pressed     (KeyCode code);
static nkBool is_key_released    (KeyCode code);
static nkBool is_any_key_down    (void);
static nkBool is_any_key_up      (void);
static nkBool is_any_key_pressed (void);
static nkBool is_any_key_released(void);
static nkBool is_key_mod_active  (void); // Are any of the key modifiers pressed Ctrl/Shift/Alt.

// Mouse
static nkBool is_mouse_button_down        (MouseButton button);
static nkBool is_mouse_button_up          (MouseButton button);
static nkBool is_mouse_button_pressed     (MouseButton button);
static nkBool is_mouse_button_released    (MouseButton button);
static nkBool is_any_mouse_button_down    (void);
static nkBool is_any_mouse_button_up      (void);
static nkBool is_any_mouse_button_pressed (void);
static nkBool is_any_mouse_button_released(void);
static void   set_mouse_to_relative       (nkBool enable);
static nkBool is_mouse_relative           (void);
static nkVec2 get_window_mouse_pos        (void);
static nkVec2 get_relative_mouse_pos      (void);

// Gamepad
static nkBool is_gamepad_button_down         (GamepadButton button);
static nkBool is_gamepad_button_up           (GamepadButton button);
static nkBool is_gamepad_button_pressed      (GamepadButton button);
static nkBool is_gamepad_button_released     (GamepadButton button);
static nkBool is_any_gamepad_button_down     (void);
static nkBool is_any_gamepad_button_up       (void);
static nkBool is_any_gamepad_button_pressed  (void);
static nkBool is_any_gamepad_button_released (void);
static nkS16  get_gamepad_axis               (GamepadAxis axis);
static nkBool is_gamepad_rstick_up           (void);
static nkBool is_gamepad_rstick_right        (void);
static nkBool is_gamepad_rstick_down         (void);
static nkBool is_gamepad_rstick_left         (void);
static nkBool is_gamepad_rstick_pressed_up   (void);
static nkBool is_gamepad_rstick_pressed_right(void);
static nkBool is_gamepad_rstick_pressed_down (void);
static nkBool is_gamepad_rstick_pressed_left (void);
static nkBool is_gamepad_lstick_up           (void);
static nkBool is_gamepad_lstick_right        (void);
static nkBool is_gamepad_lstick_down         (void);
static nkBool is_gamepad_lstick_left         (void);
static nkBool is_gamepad_lstick_pressed_up   (void);
static nkBool is_gamepad_lstick_pressed_right(void);
static nkBool is_gamepad_lstick_pressed_down (void);
static nkBool is_gamepad_lstick_pressed_left (void);

/*////////////////////////////////////////////////////////////////////////////*/
