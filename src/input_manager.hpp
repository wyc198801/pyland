#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <set>

extern "C" {
#include <SDL2/SDL.h>
}



class GameWindow;

///
/// Handles SDL input such that it can be accessed reliably and on
/// demand.
///
class InputManager {
private:
    friend class GameWindow;

    ///
    /// The associate GameWindow to handle events for.
    ///
    GameWindow* window;
    
    ///
    /// Set of currently depressed keys (scancodes).
    ///
    /// Use this when the key's layout is important.
    ///
    std::set<int> held_keys;
    ///
    /// Set of recently pressed keys (scancodes).
    ///
    std::set<int> pressed_keys;
    ///
    /// Set of recently released keys (scancodes).
    ///
    std::set<int> released_keys;

    ///
    /// The current x coordinate of the mouse.
    ///
    /// Stored in pixels from left of render-area.
    ///
    int mouse_x;
    ///
    /// The current y coordinate of the mouse.
    ///
    /// Stored in pixels from top of render-area.
    ///
    int mouse_y;

    ///
    /// Set of currently depressed mouse buttons.
    ///
    std::set<int> held_buttons;
    ///
    /// Set of recently pressed mouse buttons.
    ///
    std::set<int> pressed_buttons;
    ///
    /// Set of recently released mouse buttons.
    ///
    std::set<int> released_buttons;

    ///
    /// Code to be run before handling event loops.
    ///
    /// Clears pressed and released keys and mouse buttons.
    ///
    void clean();
    
    ///
    /// Update the state using the given SDL_Event.
    ///
    /// Called by the friend class GameWindow's update static method.
    ///
    /// @param event An SDL_Event holding the event to process.
    ///
    static void handle_event(SDL_Event* event);

public:
    ///
    /// Prepares a new input manager.
    ///
    /// @param window The associated window to receive events from.
    ///
    InputManager(GameWindow* window);

    ~InputManager();

    ///
    /// Query whether a key is down, given scancode.
    ///
    /// This should be used for keys where layout is important.
    ///
    /// @param key The scancode of the key.
    ///
    bool is_key_held(int key);
    ///
    /// Query whether a key is down, given keycode.
    ///
    /// This should be used for keys where symbol is important.
    ///
    /// @param key The keycode of the key.
    ///
    bool is_char_held(int key);
    ///
    /// Query whether a key was recently pressed, given scancode.
    ///
    /// @param key The scancode of the key.
    ///
    bool is_key_pressed(int key);
    ///
    /// Query whether a key was recently pressed, given keycode.
    ///
    /// @param key The keycode of the key.
    ///
    bool is_char_pressed(int key);
    ///
    /// Query whether a key was recently released, given scancode.
    ///
    /// @param key The scancode of the key.
    ///
    bool is_key_released(int key);
    ///
    /// Query whether a key was recently released, given keycode.
    ///
    /// @param key The keycode of the key.
    ///
    bool is_char_released(int key);
    
    ///
    /// Query whether a mouse button is down.
    ///
    /// @param button The button number.
    ///
    bool is_mouse_down(int button);
    ///
    /// Query whether a mouse button was recently pressed.
    ///
    /// @param button The button number.
    ///
    bool is_mouse_pressed(int button);
    ///
    /// Query whether a mouse button was recently released.
    ///
    /// @param button The button number.
    ///
    bool is_mouse_released(int button);
};

#endif
