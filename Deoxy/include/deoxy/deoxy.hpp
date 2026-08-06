#pragma once

#include <deoxy/core/application.hpp>
#include <deoxy/math/math.hpp>

#include <deoxy/input/input.hpp>
#include <deoxy/input/key.hpp>
#include <deoxy/input/mouse_buttons.hpp>

#include <deoxy/graphics/renderer.hpp>
#include <deoxy/graphics/color.hpp>
#include <deoxy/graphics/mesh_data.hpp>
#include <deoxy/graphics/mesh_generator.hpp>
#include <deoxy/graphics/graphical_handles.hpp>
#include <deoxy/graphics/image_data.hpp>
#include <deoxy/graphics/image_loader.hpp>
#include <deoxy/graphics/material.hpp>
#include <deoxy/graphics/texture.hpp>
#include <deoxy/graphics/vertex.hpp>
#include <deoxy/graphics/lighting/ambient_light.hpp>
#include <deoxy/graphics/lighting/directional_light.hpp>
#include <deoxy/graphics/lighting/point_light.hpp>
#include <deoxy/graphics/lighting/spot_light.hpp>

#include <deoxy/platform/window.hpp>
#include <deoxy/platform/message_box.hpp>
#include <deoxy/platform/logger.hpp>

using namespace deoxy;
using namespace deoxy::core;
using namespace deoxy::math;
using namespace deoxy::graphics;
using namespace deoxy::input;
using namespace deoxy::platform;
