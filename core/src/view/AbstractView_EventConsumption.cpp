/*
 * AbstractView_EventConsumption.cpp
 *
 * Copyright (C) 2020 by Universitaet Stuttgart (VIS).
 * Alle Rechte vorbehalten.
 */

#include "mmcore/view/AbstractView_EventConsumption.h"

#include "Framebuffer_Events.h"
#include "KeyboardMouse_Events.h"
#include "Window_Events.h"

#include <chrono>

namespace megamol {
namespace core {
namespace view {

using namespace megamol::frontend_resources;

// shorthand notation to unpack a FrontendResource to some type.
// if the type is present in the resource is made available as an 'events' variable in the if statemtnt.
// note that when using this macro there is no visible opening bracket { for the if statements because it is hidden inside the macro
#define GET_RESOURCE(TYPENAME) \
    {                          \
        TYPENAME const& events = resource.getResource<TYPENAME>();


void view_consume_keyboard_events(AbstractView& view, megamol::frontend::FrontendResource const& resource) {
    GET_RESOURCE(KeyboardEvents) //{
    for (auto& e : events.key_events)
        view.OnKey(std::get<0>(e), std::get<1>(e), std::get<2>(e));

    for (auto& e : events.codepoint_events)
        view.OnChar(e);
}
}

void view_consume_mouse_events(AbstractView& view, megamol::frontend::FrontendResource const& resource) {
    GET_RESOURCE(MouseEvents) //{
    for (auto& e : events.buttons_events)
        view.OnMouseButton(std::get<0>(e), std::get<1>(e), std::get<2>(e));

    for (auto& e : events.position_events)
        view.OnMouseMove(std::get<0>(e), std::get<1>(e));

    for (auto& e : events.scroll_events)
        view.OnMouseScroll(std::get<0>(e), std::get<1>(e));

    //for (auto& e: events.enter_events) {}
}
}

void view_consume_window_events(AbstractView& view, megamol::frontend::FrontendResource const& resource) {
    GET_RESOURCE(WindowEvents) //{
    events.is_focused_events;
}
}

// this is a weird place to measure passed program time, but we do it here so we satisfy _mmcRenderViewContext and nobody else needs to know
static std::chrono::high_resolution_clock::time_point render_view_context_timer_start;

void view_poke_rendering(AbstractView& view, megamol::frontend_resources::RenderInput const& render_input,
    megamol::frontend_resources::ImageWrapper& result_image) {
    static bool started_timer = false;
    if (!started_timer) {
        render_view_context_timer_start = std::chrono::high_resolution_clock::now();
        started_timer = true;
    }

    const double instanceTime_sec = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::high_resolution_clock::now() - render_view_context_timer_start)
                                        .count() /
                                    static_cast<double>(1000);

    const double time_sec = view.DefaultTime(instanceTime_sec);

    // copy render inputs from frontend so we can update time
    auto renderinput = render_input;

    renderinput.instanceTime_sec = instanceTime_sec;
    renderinput.time_sec = time_sec;

    view.Resize(renderinput.local_view_framebuffer_resolution.x, renderinput.local_view_framebuffer_resolution.y);

    auto camera = view.GetCamera();

    Camera::AspectRatio aspect = {
        renderinput.global_framebuffer_resolution.x / static_cast<float>(renderinput.global_framebuffer_resolution.y)};
    Camera::ImagePlaneTile tile = {renderinput.local_tile_relative_begin, renderinput.local_tile_relative_end};

    switch (camera.getProjectionType()) {
    case Camera::ProjectionType::ORTHOGRAPHIC: {
        auto intrinsics = camera.get<Camera::OrthographicParameters>();
        intrinsics.aspect = aspect;
        intrinsics.image_plane_tile = tile;
        camera.setOrthographicProjection(intrinsics);
        break;
    }
    case Camera::ProjectionType::PERSPECTIVE: {
        auto intrinsics = camera.get<Camera::PerspectiveParameters>();
        intrinsics.aspect = aspect;
        intrinsics.image_plane_tile = tile;
        camera.setPerspectiveProjection(intrinsics);
        break;
    }
    case Camera::ProjectionType::UNKNOWN:
    default:
        break;
    }

    view.SetCamera(camera);

    result_image = view.Render(renderinput.time_sec, renderinput.instanceTime_sec);
}

std::vector<std::string> get_view_runtime_resources_requests() {
    return {"ViewRenderInputs", "KeyboardEvents", "MouseEvents", "WindowEvents"};
}

bool view_rendering_execution(void* module_ptr, std::vector<megamol::frontend::FrontendResource> const& resources,
    megamol::frontend_resources::ImageWrapper& result_image) {
    megamol::core::view::AbstractView* view_ptr =
        dynamic_cast<megamol::core::view::AbstractView*>(static_cast<megamol::core::Module*>(module_ptr));

    if (!view_ptr) {
        std::cout << "error. module is not a view module. could not use as rendering entry point." << std::endl;
        return false;
    }

    megamol::core::view::AbstractView& view = *view_ptr;

    // resources are in order of initial requests from get_view_runtime_resources_requests()
    megamol::core::view::view_consume_keyboard_events(view, resources[1]);
    megamol::core::view::view_consume_mouse_events(view, resources[2]);
    megamol::core::view::view_consume_window_events(view, resources[3]);
    megamol::core::view::view_poke_rendering(
        view, resources[0].getResource<megamol::frontend_resources::RenderInput>(), result_image);

    return true;
}

} /* end namespace view */
} /* end namespace core */
} /* end namespace megamol */
