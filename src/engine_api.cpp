#include <cstdlib>
#include <future>
#include <glog/logging.h>
#include <memory>
#include <utility>
#include <vector>
#include <iostream>

#include "character.hpp"
#include "engine_api.hpp"
#include "event_manager.hpp"
#include "game_time.hpp"
#include "map_viewer.hpp"
#include "object.hpp"
#include "object_manager.hpp"
#include "dispatcher.hpp"


#define TILE_SIZE 32

void Engine::move_object(int id, Vec2D move_by) {
    // TODO: Make sure std::promise garbage collects correctly
    Engine::move_object(id, move_by, std::make_shared<std::promise<bool>>());
}

//TODO: This needs to work with renderable objects
void Engine::move_object(int id,
                         Vec2D move_by,
                         std::shared_ptr<std::promise<bool>> succeeded_promise_ptr) {

    std::shared_ptr<Character> character = ObjectManager::get_instance().get_object<Character>(id);

    if (!character || character->moving) {
        succeeded_promise_ptr->set_value(false);
        return;
    }

    Vec2D location = Vec2D(int(character->get_x_position()),
                           int(character->get_y_position()));
    Vec2D target = location + move_by;

    VLOG(2) << "Trying to walk to " << target.x << " " << target.y << ".\n"
            << "Tile blocker count is " << get_map_viewer()->get_map()->blocker.at(target.x).at(target.y);

    // TODO: animate walking in-place
    if (!walkable(target)) {
        succeeded_promise_ptr->set_value(false);
        return;
    }

    character->set_state_on_moving_start(target);

    // Step-off events
    EventManager::get_instance().add_event([location, id] () {
        get_map_viewer()->get_map()->event_step_off.trigger(location, id);
    });

    // move text with charcters
    Vec2D pixel_target = get_map_viewer()->get_map()->tile_to_pixel(target);
    character->get_character_text()->move(pixel_target.x, pixel_target.y);

    // Motion
    EventManager::get_instance().add_timed_event(
        GameTime::duration(0.07),
        [succeeded_promise_ptr, location, target, id] (double completion) {
            std::shared_ptr<Character> character = ObjectManager::get_instance().get_object<Character>(id);
            if (!character) {
                succeeded_promise_ptr->set_value(false);
                return false;
            }

            character->set_x_position(location.x * (1-completion) + target.x * completion);
            character->set_y_position(location.y * (1-completion) + target.y * completion);

            if (completion == 1.0) {
                character->set_state_on_moving_finish();

                // TODO: Make this only focus if the character
                // is the main character.
                if (Engine::map_viewer) {
                    Engine::map_viewer->refocus_map();
                }

                // Step-on events
                EventManager::get_instance().add_event([target, id] () {
                    get_map_viewer()->get_map()->event_step_on.trigger(target, id);
                });

                succeeded_promise_ptr->set_value(true);
            }

            // Run to completion
            return true;
        }
    );
}

MapViewer* Engine::map_viewer = nullptr;
Text* Engine::dialogue_box = nullptr;


bool Engine::walkable(Vec2D location) {
    int map_width = map_viewer->get_map()->get_width();
    int map_height = map_viewer->get_map()->get_height();

    //Check bounds
    if(location.x < 0 || location.x >= map_width || location.y < 0 || location.y >= map_height) {
        VLOG(2) << "Cannot move to requested tile due to map bounds";
        return false;
    }

    //Check for collidable objects
    if(!Engine::map_viewer->get_map()->is_walkable(location.x, location.y)) {
        VLOG(2) << "Cannot move to requested tile due to collidable objects";
        return false;
    }

    //check for tile blockers
    if (get_map_viewer()->get_map()->blocker.at(location.x).at(location.y) != 0) {
        VLOG(2) << "Cannot move to requested tile due to tile blocker";
        return false;
    }

    return true;
}

bool Engine::change_tile(int, Vec2D, int) {
    return false;
}

std::vector<int> Engine::get_tiles(Vec2D) {
    std::vector<int> tiles;
    return tiles;
}

int Engine::get_tile_size() {
    throw std::runtime_error("get_tile_size not supported");
}

std::vector<int> Engine::get_objects(Vec2D) {
    std::vector<int> objects;
    return objects;
}

bool Engine::load_map(int) {
    return false;
}


Vec2D Engine::find_object(int id) {
    if (!Engine::map_viewer) {
        throw std::runtime_error("Object Requested could not be found");
    }

    Map* map = map_viewer->get_map();
    if (!map) {
        throw std::runtime_error("Object Requested could not be found");
    }

    //Check the object is on the map
    auto objects = map->get_characters();
    for(auto object_id : objects) {
        if(object_id == id) {
            //Object is on the map so now get its locationg
            auto object = ObjectManager::get_instance().get_object<Object>(id);
            return Vec2D(int(object->get_x_position()),
                         int(object->get_y_position()));
        }
    }

    //Not on the map
    throw std::runtime_error("Object is not in the map");
}

bool Engine::open_editor(std::string filename) {
    LOG(INFO) << "Opening editor";
    //TODO remove this function in the final version
    std::string bash_command = editor + std::string(" python_embed/scripts/") + filename;
    std::thread TEXT_EDIT([] (std::string command) { system(command.c_str()); }, bash_command);
    TEXT_EDIT.detach();
    return true;
}


std::vector<int> Engine::get_objects_at(Vec2D location) {
    Map* map = map_viewer->get_map();
    if (!map) {
        throw std::runtime_error("Map not avalaible");
    }

    std::vector<int> results;
    //Check the object is on the map

    auto objects = map->get_characters();
    for(int object_id : objects) {
        //Object is on the map so now get its locationg
        auto object = ObjectManager::get_instance().get_object<Object>(object_id);
        Vec2D current_location(int(object->get_x_position()),
                               int(object->get_y_position()));

        if (current_location == location) {
            results.push_back(object_id);
        }
    }
    return results;
}

std::string Engine::editor = DEFAULT_PY_EDITOR;

void Engine::print_dialogue(std::string name, std::string text) {
    std::string text_to_display = name + " : " + text;
    EventManager::get_instance().add_event(
        [text_to_display] () {
            Engine::get_dialogue_box()->set_text(text_to_display);
            std::cout << text_to_display << std::endl;
        }
    );
}

void Engine::text_displayer() {

    Map* map = map_viewer->get_map();
    if (!map) {
        throw std::runtime_error("Map not avalaible");
    }

    auto objects = map->get_characters();
    for(int object_id : objects) {
        //Object is on the map so now get its locationg
        auto character = ObjectManager::get_instance().get_object<Character>(object_id);
        if (character->get_character_text() != nullptr) {
            character->get_character_text()->display();
        }
    }
}

void Engine::text_updater() {

     Map* map = map_viewer->get_map();
    if (!map) {
        throw std::runtime_error("Map not avalaible");
    }

    auto objects = map->get_characters();
    for(int object_id : objects) {
        //Object is on the map so now get its locationg
        auto character = ObjectManager::get_instance().get_object<Character>(object_id);
        auto object = ObjectManager::get_instance().get_object<Object>(object_id);
        Vec2D tile = Vec2D(object->get_x_position(), object->get_y_position());
        Vec2D pixel_position = Engine::get_map_viewer()->get_map()->tile_to_pixel(tile);
        character->get_character_text()->move(pixel_position.x, pixel_position.y);
    }

}

