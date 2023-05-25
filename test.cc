#include <chrono>
#include <cstdio>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include "util.h"

using namespace std::chrono_literals;

using Solution = std::map<User, std::pair<Sat, Color>>;

#define BOLD    "\u001b[1m"
#define GRAY    "\u001b[38;5;248m"
#define CYAN    "\u001b[36m"
#define RED     "\u001b[31m"
#define GREEN   "\u001b[32m"
#define YELLOW  "\u001b[33m"
#define RESET   "\u001b[0m"

#define TIMEOUT 60s

#define FAIL(...)                                       \
    {                                                   \
        printf(RED BOLD "FAIL: " RESET __VA_ARGS__);    \
        printf("\n");                                   \
        std::exit(1);                                   \
    }

#define CHECK(condition, ...)                           \
    if (!(condition))                                   \
    {                                                   \
        FAIL(__VA_ARGS__);                              \
    }

/**
 * Parse a space-separated 3D vector.
 */
std::istream& operator>>(std::istream &is, Vector3 &vector)
{
    return is >> vector.x >> vector.y >> vector.z;
}

/**
 * Check color validity.
 */
void check_color(Color color)
{
    for (const Color c : colors)
    {
        if (color == c)
        {
            return;
        }
    }

    FAIL("Invalid color: '%c'", color);
}

/**
 * A single test case.
 */
struct Scenario
{
    /**
     * User positions.
     */
    std::map<User, Vector3> users;

    /**
     * Satellite positions.
     */
    std::map<Sat, Vector3> sats;

    /**
     * Minimum passing coverage.
     */
    float min_coverage = 1.0;

    /**
     * Load a scenario from a file.
     */
    Scenario(const std::string& path)
    {
        std::string line;
        std::ifstream file(path);
        CHECK(file.is_open(), "Could not open file.")
        while (std::getline(file, line))
        {
            if (line.rfind("#", 0) == 0 || line.size() == 0)
            {
                continue;
            }
            std::stringstream to_parse{line};

            int id;
            Vector3 pos;
            std::string token;
            to_parse >> token;
            if (token == "min_coverage")
            {
                to_parse >> min_coverage;
            }
            else
            {
                to_parse >> id >> pos;
                if (token == "sat")
                {
                    sats[id] = pos;
                }
                else if (token == "user")
                {
                    users[id] = pos;
                }
                else
                {
                    FAIL("Invalid token '%s'.", token.c_str());
                }
            }
        }

        file.close();
    }

    /**
     * Check solution validity.
     */
    void check(const Solution &solution) const
    {
        std::map<Sat, std::vector<std::pair<Color, User>>> beams;

        for (const std::pair<User, std::pair<Sat, Color>> it : solution)
        {
            const User user = it.first;
            const Sat sat = it.second.first;
            const Color color = it.second.second;
            const Vector3 user_pos = users.at(user);
            const Vector3 sat_pos = sats.at(sat);
            check_color(color);

            const float angle = acos(user_pos.unit().dot((sat_pos - user_pos).unit())) / PI * 180;
            CHECK(angle <= 45, "User %d cannot see satellite %d (%.2f degrees from vertical)", user, sat, angle);

            beams[sat].emplace_back(std::make_pair(color, user));
        }

        for (const std::pair<Sat, std::vector<std::pair<Color, User>>>& it : beams)
        {
            const Sat sat = it.first;
            const Vector3 sat_pos = sats.at(sat);
            const int sat_beams = it.second.size();
            CHECK(sat_beams <= 32, "Satellite %d cannot serve more than 32 users (%d assigned)", sat, sat_beams);

            for (const std::pair<Color, User>& u1 : it.second)
            {
                const Color color1 = u1.first;
                const User user1 = u1.second;

                for (const std::pair<Color, User>& u2 : it.second)
                {
                    const Color color2 = u2.first;
                    const User user2 = u2.second;

                    if (color1 == color2 && user1 != user2)
                    {
                        const Vector3 user1_pos = users.at(user1);
                        const Vector3 user2_pos = users.at(user2);
                        const float angle = sat_pos.angle_between(user1_pos, user2_pos);
                        CHECK(angle >= 10.0,
                              "Users %d and %d on satellite %d color %c are too close (%.2f degrees)",
                              user1, user2, sat, color1, angle);
                    }
                }
            }
        }

        auto coverage = float(solution.size()) / float(users.size());
        CHECK(coverage >= min_coverage, "Too few users served");
    }
};

/**
 * Main.
 */
int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("USAGE: %s TEST_CASE OUT_PATH\n", argv[0]);
        return 1;
    }

    printf(BOLD CYAN "============================================================\n" RESET);
    printf(BOLD CYAN "%s" RESET "\n", argv[1]);
    printf(BOLD CYAN "============================================================\n" RESET);

    const Scenario scenario(argv[1]);
    printf(GRAY "Scenario: " RESET "%.2f%% coverage (%zu users, %zu sats)" RESET "\n",
           100 * scenario.min_coverage,
           scenario.users.size(),
           scenario.sats.size());

    const auto start = std::chrono::high_resolution_clock::now();
    const Solution solution = solve(scenario.users, scenario.sats);
    const auto duration = std::chrono::high_resolution_clock::now() - start;
    const float covered = 1.0 * solution.size() / scenario.users.size();
    const float duration_s = duration / 1ns / 1e9;
    printf(GRAY "Solution: " RESET BOLD "%s%.2f%%" RESET " coverage (%zu users) in %s" BOLD "%.2fs" RESET "\n",
           covered >= scenario.min_coverage ? GREEN : RED,
           100.0 * covered,
           solution.size(),
           duration > TIMEOUT ? RED : duration > TIMEOUT / 2 ? YELLOW : GREEN,
           duration_s);

    FILE* out = fopen(argv[2], "a");
    fprintf(out, "%-44s %6.2f%% %6.2fs\n", argv[1], 100.0 * covered, duration_s);

    CHECK(duration < TIMEOUT, "Took too long to produce a solution");
    scenario.check(solution);
}
