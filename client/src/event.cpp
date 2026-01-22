#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

using json = nlohmann::json;


Event::Event(std::string team_a_name, std::string team_b_name, std::string name, int time,
             std::map<std::string, std::string> game_updates, std::map<std::string, std::string> team_a_updates,
             std::map<std::string, std::string> team_b_updates, std::string discription) : 
    team_a_name(team_a_name),
    team_b_name(team_b_name), 
    name(name),
    time(time),
    game_updates(game_updates),
    team_a_updates(team_a_updates),
    team_b_updates(team_b_updates),
    description(discription) {}

Event::~Event() {
}

Event::Event(const std::string &frame_body) : 
    team_a_name(""),
    team_b_name(""),
    name(""),
    time(0), 
    game_updates(),
    team_a_updates(),
    team_b_updates(),
    description("") 
{
    std::stringstream ss(frame_body);
    std::string line;
    std::string current_section = ""; 

    while (std::getline(ss, line)) {

        if (line.find("team a:") == 0 && current_section == "") {
            int skip = (line.length() > 7 && line[7] == ' ') ? 8 : 7;
            team_a_name = line.substr(skip);
        }
        
        if (line.find("team b:") == 0 && current_section == "") {
            int skip = (line.length() > 7 && line[7] == ' ') ? 8 : 7;
            team_b_name = line.substr(skip);
        }
        
        if (line.find("event name:") == 0 && current_section == "") 
            name = line.substr(11);
        
        if (line.find("time:") == 0 && current_section == "") {
            try {
                time = std::stoi(line.substr(5));
            } catch (...) { time = 0; }
        }

        if (line == "general game updates:") 
            current_section = "general";
        
        if (line == "team a updates:") 
            current_section = "team_a";
        
        if (line == "team b updates:") 
            current_section = "team_b";
        
        if (line == "description:") 
            current_section = "description";
        
        if (!line.empty()) {
            if (current_section == "description") {
                description += line + "\n";
            } else {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string key = line.substr(0, colonPos);
                    std::string value = line.substr(colonPos + 1);
                    
                    if (current_section == "general") 
                        game_updates[key] = value;
                    else if (current_section == "team_a") 
                        team_a_updates[key] = value;
                    else if (current_section == "team_b") 
                        team_b_updates[key] = value;
                }
            }
        }
    }
}


const std::string& Event::get_team_a_name() const { return this->team_a_name; }
const std::string &Event::get_team_b_name() const { return this->team_b_name; }
const std::string &Event::get_name() const { return this->name; }
int Event::get_time() const { return this->time; }
const std::map<std::string, std::string> &Event::get_game_updates() const { return this->game_updates; }
const std::map<std::string, std::string> &Event::get_team_a_updates() const { return this->team_a_updates; }
const std::map<std::string, std::string> &Event::get_team_b_updates() const { return this->team_b_updates; }
const std::string &Event::get_discription() const { return this->description; }



names_and_events parseEventsFile(std::string json_path)
{
    std::ifstream f(json_path);

    if (!f.is_open()) {
        std::cerr << "Error: Could not open file " << json_path << std::endl;
        return names_and_events{"", "", {}}; 
    }

    json data = json::parse(f);

    std::string team_a_name = data["team a"];
    std::string team_b_name = data["team b"];

    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event name"];
        int time = event["time"];
        std::string description = event["description"];
        std::map<std::string, std::string> game_updates;
        std::map<std::string, std::string> team_a_updates;
        std::map<std::string, std::string> team_b_updates;

        for (auto &update : event["general game updates"].items())
        {
            if (update.value().is_string())
                game_updates[update.key()] = update.value();
            else
                game_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team a updates"].items())
        {
            if (update.value().is_string())
                team_a_updates[update.key()] = update.value();
            else
                team_a_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team b updates"].items())
        {
            if (update.value().is_string())
                team_b_updates[update.key()] = update.value();
            else
                team_b_updates[update.key()] = update.value().dump();
        }
        

        events.push_back(Event(team_a_name, team_b_name, name, time, game_updates, team_a_updates, team_b_updates, description));
    }
    names_and_events events_and_names{team_a_name, team_b_name, events};

    return events_and_names;
}