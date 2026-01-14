#pragma once

#include <string>
#include <iostream>
#include <map>
#include <vector>

class Event
{
private:
    std::string team_a_name;
    std::string team_b_name;
    std::string name;
    int time;
    std::map<std::string, std::string> game_updates;
    std::map<std::string, std::string> team_a_updates;
    std::map<std::string, std::string> team_b_updates;
    std::string description;

public:
    // FIXED: Changed order to match .cpp: (team_a, team_b, name, ...)
    Event(std::string team_a_name, std::string team_b_name, std::string name, int time, 
          std::map<std::string, std::string> game_updates, 
          std::map<std::string, std::string> team_a_updates, 
          std::map<std::string, std::string> team_b_updates, 
          std::string description);
          
    Event(const std::string & frame_body);
    
    virtual ~Event(); // Don't forget to implement this in .cpp!

    const std::string &get_team_a_name() const;
    const std::string &get_team_b_name() const;
    const std::string &get_name() const;
    int get_time() const;
    const std::map<std::string, std::string> &get_game_updates() const;
    const std::map<std::string, std::string> &get_team_a_updates() const;
    const std::map<std::string, std::string> &get_team_b_updates() const;
    const std::string &get_discription() const;
};

struct names_and_events {
    std::string team_a_name;
    std::string team_b_name;
    std::vector<Event> events;
};

names_and_events parseEventsFile(std::string json_path);