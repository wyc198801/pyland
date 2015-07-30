#include <glog/logging.h>

#include "audio_engine.hpp"
#include "button.hpp"
#include "config.hpp"
#include "engine.hpp"
#include "event_manager.hpp"
#include "game_engine.hpp"
#include "game_main.hpp"
#include "text_font.hpp"


/*
boost::python::object GameEngine::addObject(std::string name, std::string class_location, int x, int y) {
    use the challenge instance to make and object and load it into the challenge
    wrap the object an entity instance
    grab the class from the class location
    create a new instance of that class and use it to wrap the entity instance
    return that intance, that instance will also be located at the coordinates given in-game
}

boost::python::list GameEngine::get_objectsAt(int x, int y) {
    return a python list of all the correctly wrapped game-object instances at the location given.
}

*/


void GameEngine::change_level(std::string level_location) {
    //TODO: run the finish.py script of a level.
    LOG(INFO) << "Changing level to " << level_location;
    return;
}


boost::python::object GameEngine::add_object(std::string name, std::string class_location, int x, int y) {
    LOG(INFO) << "Creating an instance of " << class_location << " at (" << x << ", " << y << ") called " << name;
    //use the challenge instance to make and object and load it into the challenge
    //wrap the object an entity instance
    //grab the class from the class location
    //create a new instance of that class and use it to wrap the entity instance
    //return that intance, that instance will also be located at the coordinates given in-game
    return boost::python::list();
}

std::string GameEngine::get_level_location() {
    nlohmann::json j = Config::get_instance();
    std::string map_name = j["files"]["level_location"];
    //return "test_world/test_level/test_one";
    return map_name;
}

void GameEngine::print_debug(std::string debug_message) {
    LOG(INFO) << debug_message; // TODO: work out properly how python messages should be debugged.
}


void GameEngine::add_button(std::string file_path, std::string name, PyObject* callback) {

    LOG(INFO) << "Got here";
    boost::python::object boost_callback(boost::python::handle<>(boost::python::borrowed(callback)));

    std::shared_ptr<Button> new_button;
    new_button = std::make_shared<Button>(ButtonType::SpriteHead);
    game_main->get_buttons().push_back(new_button);
    LOG(INFO) << file_path;
    LOG(INFO) << name;
    new_button->set_picture(file_path);
    new_button->set_text(name);
    new_button->set_on_click([boost_callback]() {
        EventManager::get_instance()->add_event(boost_callback);
    });
    new_button->set_width(0.30f);
    new_button->set_height(0.50f);
    game_main->get_sprite_window()->add(new_button);
    game_main->refresh_gui();
}

void GameEngine::register_input_callback(int input_key, PyObject *py_input_callback) {
    boost::python::object input_callback(boost::python::handle<>(boost::python::borrowed(py_input_callback)));
    InputHandler::get_instance()->register_input_callback(input_key, input_callback);
    return;
}

void GameEngine::play_music() {
    AudioEngine::get_instance()->play_song("test.ogg");
}
