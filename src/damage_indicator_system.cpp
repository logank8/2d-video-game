// internal
#include "damage_indicator_system.hpp"
#include "world_init.hpp"
#include <iostream>

const float RESIZE_SCALE = 1.0f;

mat3 createProjectionMatrix(vec2 position)
{
    // Fake projection matrix, scales with respect to window coordinates
    float left = 0.f;
    float top = 0.f;

    gl_has_errors();
    float right = (float)window_width_px;
    float bottom = (float)window_height_px;

    float sx = 2.f / (right - left);
    float sy = 2.f / (top - bottom);
    float tx = -(right + left) / (right - left) - (position.x - window_width_px / 2.f) * sx;
    float ty = -(top + bottom) / (top - bottom) - (position.y - window_height_px / 2.f) * sy;
    return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

vec2 convertToScreenSpace(vec2 position)
{
    vec2 cameraPosition;

    // sus code : assumes only one camera and that its always present
    for (Entity entity : registry.cameras.entities)
    {
        auto &cameraMotion = registry.motions.get(entity);
        cameraPosition = cameraMotion.position;
    }

    // converts to normalized device coordinates
    vec2 screenPosition = createProjectionMatrix(cameraPosition) * vec3(position.x, position.y, 1.0f);

    // converts ndc to screen coordinates
    screenPosition.x = (screenPosition.x + 1.0f) * window_width_px / 2;
    screenPosition.y = (screenPosition.y + 1.0f) * window_height_px / 2;

    return screenPosition;
}

void DamageIndicatorSystem::step(float elapsed_ms)
{
    for (Entity entity : registry.damageIndicators.entities)
    {
        auto &damageIndicatorComponent = registry.damageIndicators.get(entity);
        auto &damageMotion = registry.motions.get(entity);

        damageIndicatorComponent.time_elapsed_ms -= elapsed_ms;

        float time_frac = (damageIndicatorComponent.time_elapsed_ms / damageIndicatorComponent.time_total_ms);

        float scale = sin(time_frac * M_PI * RESIZE_SCALE);

        if (damageIndicatorComponent.time_elapsed_ms < 0)
        {
            registry.remove_all_components_of(entity);
        }

        // std::cout << damageMotion.position.x << ',' << damageMotion.position.y << std::endl;

        damageIndicatorComponent.text = createText(convertToScreenSpace(damageMotion.position), scale, std::to_string(damageIndicatorComponent.damage), vec3(1.f, 1.f, 1.f));
    }
}