#include "util.h"
#include <iostream>
static Vector3 center_of_earth;

using namespace std;

float angle_from_user_vertical(Vector3 &u, Vector3 &s){
    Vector3 user_to_sat = s - u;
    return center_of_earth.angle_between(user_to_sat, u);
}
std::map<User, std::pair<Sat, Color>> solve(
    const std::map<User, Vector3>& users,
    const std::map<Sat, Vector3>& sats)
{
    std::map<User, std::pair<Sat, Color>> solution;
    center_of_earth.x = 0;
    center_of_earth.y = 0;
    center_of_earth.z = 0;
    /*
     * TODO: Implement.
     */

    std::map<Sat, int> users_covered_by_sat; // key: satellite #, value: # of users assigned so far
    std::map<Sat, std::vector<std::vector<User>>> color_map; // key: sat #, value : 2D array of beams
    for(auto sat : sats){
        for(auto user : users){
            if(users_covered_by_sat[sat.first] == 32) break;
            if(solution.find(user.first) != solution.end()) continue;
            float angle = angle_from_user_vertical(user.second, sat.second);
            if(angle <= 45){
                if(color_map[sat.first].size() < 4){ // not every color is taken
                    std::vector<User> v = {user.first};
                    color_map[sat.first].push_back(v);
                    
                    users_covered_by_sat[sat.first]++;
                    char c = color_map[sat.first].size() - 1 + 65;
                    solution[user.first] = {sat.first, c};
                }
                else{
                    for(int i = 0; i < 4; i++){
                        bool violation_found = false;
                        for(int j = 0; j < color_map[sat.first][i].size(); j++){
                            angle = sat.second.angle_between(user.second, users.at(color_map[sat.first][i][j]));
                            if(angle <= 10){
                                violation_found = true;
                                continue;
                            }
                        }
                        if(!violation_found){
                            color_map[sat.first][i].push_back(user.first);
                            users_covered_by_sat[sat.first]++;
                            char c = i + 65;
                            solution[user.first] = {sat.first, c};
                            break;
                        }
                    }
                }
            }
        }
    }
    
    return solution;
}
