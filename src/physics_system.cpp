// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include "world_system.hpp"

WorldSystem world;
std::vector<std::vector<int>> map = world.get_current_map();
const float TILE_SIZE = 100.0f;

#include <iostream>

// Returns the local bounding coordinates (AABB)
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { motion.position.x - (abs(motion.scale.x) / 2), motion.position.y - (abs(motion.scale.y) / 2) };
}

bool collides(const Motion& motion1, const Motion& motion2) {
	float bottom_1 = motion1.position.y - (abs(motion1.scale.y) / 2);
	float top_1 = motion1.position.y + (abs(motion1.scale.y) / 2);
	float left_1 = motion1.position.x - (abs(motion1.scale.x) / 2);
	float right_1 = motion1.position.x + (abs(motion1.scale.x) / 2);

	float bottom_2 = motion2.position.y - (abs(motion2.scale.y) / 2);
	float top_2 = motion2.position.y + (abs(motion2.scale.y) / 2);
	float left_2 = motion2.position.x - (abs(motion2.scale.x) / 2);
	float right_2 = motion2.position.x + (abs(motion2.scale.x) / 2);

	// check for vertical overlap
	if (((top_1 >= bottom_2) && (top_1 <= top_2)) || ((top_2 >= bottom_1) && (top_2 <= top_1))) {
		// horiz overlap
		if (((left_1 <= right_2) && (left_1 >= left_2)) || ((left_2 <= right_1) && (left_2 >= left_1))) {
			return true;
		}
	}
	return false;
}

// Check if point is in the triangle with vertices v0, v1, v2
bool point_in_triangle(vec2 point, vec2 v0, vec2 v1, vec2 v2) {
    // Calculate barycentric coordinates
    float denom = (v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y);
    float a = ((v1.y - v2.y) * (point.x - v2.x) + (v2.x - v1.x) * (point.y - v2.y)) / denom;
    float b = ((v2.y - v0.y) * (point.x - v2.x) + (v0.x - v2.x) * (point.y - v2.y)) / denom;
    float c = 1 - a - b;

    // test collision
    return 0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1;
}

// Transform vertex on mesh to its in-game position
vec2 transform(vec2 vertex, const Motion& mesh_motion) {
    // rotate
    float c = cosf(mesh_motion.angle);
    float s = sinf(mesh_motion.angle);
    mat2 R = { { c, s },{ -s, c } };
    vertex = vertex * R;

        // scale
    vertex = vertex * mesh_motion.scale;

        // translate to mesh entity position position
    vertex = vertex + mesh_motion.position;

    return vertex;
}

// For mesh-box collision
bool mesh_collides(const Entity& mesh_entity, const Motion& mesh_motion, const Motion& box_motion)
{    
    // Getting AABB bounding box of object box bounding
	vec2 box_top_left = get_bounding_box(box_motion);

	// Getting mesh
	auto mesh = registry.meshPtrs.get(mesh_entity);
	std::vector<ColoredVertex> vertices = mesh->vertices;

    // Check if any vertex of the mesh is in box bounding box
	for(uint i = 0; i < vertices.size(); i++) {
		vec3 relative_pos = vertices[i].position;
		vec2 pos = vec2( relative_pos.x, relative_pos.y );

        pos = transform(pos, mesh_motion);

		if (pos.y < box_top_left.y + abs(box_motion.scale.y)
		&& pos.y > box_top_left.y
		&& pos.x < box_top_left.x + abs(box_motion.scale.x)
		&& pos.x > box_top_left.x)
			return true;
	}

    // Check if any of the box bounding box corners are within the triangles that make up the mesh
    std::vector<uint16_t> vertex_indices = mesh->vertex_indices;
    for(uint i = 0; i < vertex_indices.size(); i+=3) {
        // Get vertices of the trangle face
        uint16_t v0_index = vertex_indices[i];
        uint16_t v1_index = vertex_indices[i+1];
        uint16_t v2_index = vertex_indices[i+2];
		vec2 v0 = vec2( vertices[v0_index].position.x , vertices[v0_index].position.y );
        vec2 v1 = vec2( vertices[v1_index].position.x , vertices[v1_index].position.y );
        vec2 v2 = vec2( vertices[v2_index].position.x , vertices[v2_index].position.y );
        v0 = transform(v0, mesh_motion);
        v1 = transform(v1, mesh_motion);
        v2 = transform(v2, mesh_motion);

        // Get corner position of the box BB
        // box_top_left already exists
        vec2 box_top_right = box_top_left + vec2( abs(box_motion.scale.x) , 0 );
        vec2 box_bottom_left = box_top_left + vec2( 0 , abs(box_motion.scale.y) );
        vec2 box_bottom_right = box_top_left + vec2( abs(box_motion.scale.x) , abs(box_motion.scale.y) );

        // Check if point is in triangle
        if (point_in_triangle(box_top_left, v0, v1, v2)) return true;
        if (point_in_triangle(box_top_right, v0, v1, v2)) return true;
        if (point_in_triangle(box_bottom_left, v0, v1, v2)) return true;
        if (point_in_triangle(box_bottom_right, v0, v1, v2)) return true;
	}

	return false;
}


// Node in A* path
struct Node {
    vec2 position;
    float g_cost = 0;
    float h_cost = 0;
    float f_cost = 0;
    Node* parent = nullptr;

    Node(vec2 pos) : position(pos) {}

    bool operator==(const Node& other) const {
        return position.x == other.position.x && position.y == other.position.y;
    }
};

// Calculate h cost using diagonal distance
float calculate_h_cost(const vec2& start, const vec2& goal) {
    float dx = abs(start.x - goal.x);
    float dy = abs(start.y - goal.y);
    float c = sqrt(TILE_SIZE * TILE_SIZE + TILE_SIZE * TILE_SIZE);
    return TILE_SIZE * (dx + dy) + (c - 2 * TILE_SIZE) * min(dx, dy);
}

// Check if a certain tile can be moved onto by an enemy
bool is_walkable(const vec2& pos, vec2 dir) {
    const float GRID_OFFSET_X = (640 - (25 * TILE_SIZE));
    const float GRID_OFFSET_Y = (640 - (44 * TILE_SIZE));

    // Convert to grid coordinates
    int grid_x = static_cast<int>((pos.x - GRID_OFFSET_X) / TILE_SIZE);
    int grid_y = static_cast<int>((pos.y - GRID_OFFSET_Y) / TILE_SIZE);

    // Boundary check
    if (grid_x < 0 || grid_y < 0 || grid_y >= map.size() || grid_x >= map[0].size()) {
        return false;
    }
    const std::vector<vec2> diagonals = {
            {TILE_SIZE, TILE_SIZE}, {-TILE_SIZE, TILE_SIZE},
            {TILE_SIZE, -TILE_SIZE}, {-TILE_SIZE, -TILE_SIZE}
    };

    // Check for clipping through walls when moving diagonally
    if (dir == diagonals[0]) {  // Moving top-right
        if (map[grid_y][grid_x - 1] == 0 || map[grid_y - 1][grid_x] == 0) return false;
    }
    else if (dir == diagonals[1]) {  // Moving top-left
        if (map[grid_y][grid_x + 1] == 0 || map[grid_y - 1][grid_x] == 0) return false;
    }
    else if (dir == diagonals[2]) {  // Moving bottom-right
        if (map[grid_y + 1][grid_x] == 0 || map[grid_y][grid_x - 1] == 0) return false;
    }
    else if (dir == diagonals[3]) {  // Moving bottom-left
        if (map[grid_y + 1][grid_x] == 0 || map[grid_y][grid_x + 1] == 0) return false;
    }

    return map[grid_y][grid_x] != 0;
}

// Find A* path for enemy
std::vector<vec2> find_path(const Motion& enemy, const Motion& player) {
    vec2 start = enemy.position;

    const float GRID_OFFSET_X = (640 - (25 * TILE_SIZE));
    const float GRID_OFFSET_Y = (640 - (44 * TILE_SIZE));

    // Convert to grid coordinates
    int grid_x = static_cast<int>((player.position.x - GRID_OFFSET_X) / TILE_SIZE);
    int grid_y = static_cast<int>((player.position.y - GRID_OFFSET_Y) / TILE_SIZE);
    vec2 goal = { (640 - (25 * 100)) + (grid_x * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (grid_y * TILE_SIZE) + (TILE_SIZE / 2) };

    std::vector<Node*> open_list;
    std::vector<Node*> closed_list;

    Node* start_node = new Node(start);
    start_node->h_cost = calculate_h_cost(start, goal);
    start_node->f_cost = start_node->h_cost;

    open_list.push_back(start_node);

    while (!open_list.empty()) {
        // Find node with lowest f_cost
        auto current_it = std::min_element(open_list.begin(), open_list.end(),
            [](const Node* a, const Node* b) { return a->f_cost < b->f_cost; });

        Node* current = *current_it;

        // Check if we reached the goal
        const float GOAL_THRESHOLD = TILE_SIZE * 0.5f;
        if (calculate_h_cost(current->position, goal) < GOAL_THRESHOLD) {
            std::vector<vec2> path;
            Node* current_path = current;
            while (current_path != nullptr) {
                path.push_back(current_path->position);
                current_path = current_path->parent;
            }

            for (auto node : open_list) delete node;
            for (auto node : closed_list) delete node;

            std::reverse(path.begin(), path.end());
            return path;
        }

        open_list.erase(current_it);
        closed_list.push_back(current);

        // Generate neighboring positions: 4 cardinal directions and 4 diagonal directions
        const std::vector<vec2> directions = {
            {TILE_SIZE, 0}, {-TILE_SIZE, 0},
            {0, TILE_SIZE}, {0, -TILE_SIZE},
            {TILE_SIZE, TILE_SIZE}, {-TILE_SIZE, TILE_SIZE},
            {TILE_SIZE, -TILE_SIZE}, {-TILE_SIZE, -TILE_SIZE}
        };

        // Check if each direction is walkable
        for (const auto& dir : directions) {
            vec2 neighbor_pos = {
                current->position.x + dir.x,
                current->position.y + dir.y
            };

            if (!is_walkable(neighbor_pos, dir)) continue;

            Node* neighbor = new Node(neighbor_pos);

            // Skip if in closed list
            if (std::find_if(closed_list.begin(), closed_list.end(),
                [&](const Node* n) { return *n == *neighbor; }) != closed_list.end()) {
                delete neighbor;
                continue;
            }

            // Diagonals cost less since they do 2 tiles of movement in one move
            float g_cost;
            if (dir.x == 0 || dir.y == 0) {
                g_cost = current->g_cost + TILE_SIZE;
            }
            else {
                g_cost = current->g_cost + TILE_SIZE / 2;
            }
            
            float h_cost = calculate_h_cost(neighbor_pos, goal);
            float f_cost = g_cost + h_cost;

            auto existing = std::find_if(open_list.begin(), open_list.end(),
                [&](const Node* n) { return *n == *neighbor; });

            if (existing != open_list.end() && g_cost >= (*existing)->g_cost) {
                delete neighbor;
                continue;
            }

            neighbor->g_cost = g_cost;
            neighbor->h_cost = h_cost;
            neighbor->f_cost = f_cost;
            neighbor->parent = current;

            if (existing != open_list.end()) {
                delete* existing;
                *existing = neighbor;
            }
            else {
                open_list.push_back(neighbor);
            }
        }
    }

    for (auto node : open_list) delete node;
    for (auto node : closed_list) delete node;
    return std::vector<vec2>();
}

//A* pathfinding code
//--------------------
// 1. Calculate path for enemy based on tilemap coordinates
// 2. Move enemy based on screen coordinates and prevent path recalculation to ensure enemy reaches the centre of the tile (or else bad things happen)
void update_enemy_movement(Entity enemy, float step_seconds) {
    const float PATH_UPDATE_TIME = 0.5f;

    Motion& motion = registry.motions.get(enemy);
    const Motion& player_motion = registry.motions.get(registry.players.entities[0]);

    // Update or create path timer
    if (!registry.pathTimers.has(enemy)) {
        registry.pathTimers.emplace(enemy, PathTimer{ 0.f });
    }
    PathTimer& timer = registry.pathTimers.get(enemy);
    timer.timer += step_seconds;


    const float GRID_OFFSET_X = (640 - (25 * TILE_SIZE)) - TILE_SIZE / 2;
    const float GRID_OFFSET_Y = (640 - (44 * TILE_SIZE)) - TILE_SIZE / 2;

    // Convert to grid coordinates
    Motion& enemy_motion = registry.motions.get(enemy);
    int grid_x = static_cast<int>((enemy_motion.position.x - GRID_OFFSET_X) / TILE_SIZE);
    int grid_y = static_cast<int>((enemy_motion.position.y - GRID_OFFSET_Y) / TILE_SIZE);
    float raw_x = (enemy_motion.position.x - GRID_OFFSET_X) / TILE_SIZE;
    float raw_y = (enemy_motion.position.y - GRID_OFFSET_Y) / TILE_SIZE;

    // Recalculate path if update time has elapsed AND enemy entity is at the centre of the tile in screen xy coordinates
    if ((!registry.paths.has(enemy) || timer.timer >= PATH_UPDATE_TIME) && (grid_x == raw_x && grid_y == raw_y)) {
        std::vector<vec2> new_path = find_path(motion, player_motion);

        if (!new_path.empty()) {
            if (registry.paths.has(enemy)) {
                registry.paths.remove(enemy);
            }
            registry.paths.emplace(enemy, Path{ new_path, 0 });
            timer.timer = 0.f;
        }
    }

    // Move along path
    if (registry.paths.has(enemy)) {
        Path& path = registry.paths.get(enemy);
        if (path.points.size() > 1 && path.current_index < path.points.size() - 1) {
            vec2 target = path.points[path.current_index + 1];
            vec2 direction = {
                target.x - motion.position.x,
                target.y - motion.position.y
            };

            float length = sqrt(direction.x * direction.x + direction.y * direction.y);
            if (length > 0) {
                direction.x /= length;
                direction.y /= length;

                // If movement is diagonal, apply diagonal scaling to align with movement increments for cardinal directions
                if (direction.x != 0 && direction.y != 0) {
                    const float diagonal_scale = 1 / sqrt(2);
                    motion.position.x += direction.x * motion.velocity.x * diagonal_scale;
                    motion.position.y += direction.y * motion.velocity.y * diagonal_scale;
                }
                else {
                    motion.position.x += direction.x * motion.velocity.x / 2;
                    motion.position.y += direction.y * motion.velocity.y / 2;
                }

                // Centre of tile reached, move onto next tile in the path
                if (length == 0.f) {
                    path.current_index++;
                }
            }
        }
    }
}
			

void PhysicsSystem::step(float elapsed_ms)
{
	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];

                bool is_colliding = true;

                // if player is entity_i do mesh-based collision with entity_i
				if (registry.stickies.has(entity_i) && registry.players.has(entity_j)) {
					is_colliding = mesh_collides(entity_i, motion_i, motion_j);
                    if (is_colliding) std::cout << "player slowed down (mesh collision)" << std::endl;
				} else if (registry.stickies.has(entity_j) && registry.players.has(entity_i)) {
                    is_colliding = mesh_collides(entity_j, motion_j, motion_i);
                    if (is_colliding) std::cout << "player slowed down (mesh collision)" << std::endl;
                }

                if (is_colliding) {
                    // Create a collisions event
                    // We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
                    registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                    registry.collisions.emplace_with_duplicates(entity_j, entity_i);
                }
			} else {
                if (registry.players.has(entity_i)) registry.players.get(entity_i).last_pos = motion_i.position;
            }
		}
	}
    
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		float step_seconds = elapsed_ms / 1000.f;
    if (registry.players.has(entity)) {
		motion.position[0] += motion.velocity[0] * step_seconds;
		motion.position[1] += motion.velocity[1] * step_seconds;
    } else {
			//Handle contact damage enemies
			if (registry.deadlys.has(entity)) {
				if (registry.deadlys.get(entity).enemy_type != ENEMY_TYPES::PROJECTILE) {

                    //A* pathfinding
                    update_enemy_movement(entity, step_seconds);

				}
				else {
					motion.position.x -= cos(motion.angle) * motion.velocity.x * step_seconds;
					motion.position.y -= sin(motion.angle) * motion.velocity.y * step_seconds;
				}
			}
		}
	}

	for (Entity entity : registry.blockedTimers.entities) {
		// progress timer
		BlockedTimer& counter = registry.blockedTimers.get(entity);
		counter.counter_ms -= elapsed_ms;
		if (counter.counter_ms < 0){
			registry.blockedTimers.remove(entity);
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE EGG UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}