#include <engine/include.hpp>

#include <string>



const oe::graphics::Sprite* sprite;
const oe::graphics::Sprite* sprite_white;
oe::graphics::SpritePack* pack;
oe::graphics::Window* window;
oe::assets::DefaultShader* shader;
oe::graphics::Renderer* renderer;
std::array<oe::graphics::Quad*, 2> quads;



void create_scene() {
	quads[0] = renderer->createQuad();
	quads[0]->setPosition({ 0.0f, 0.0f });
	quads[0]->setSize({ 1.0f, 1.0f });
	quads[0]->setRotationAlignment(oe::alignments::center_center);
	quads[0]->setSprite(sprite_white);
	quads[0]->setColor(oe::colors::blue);
	quads[0]->update();
	
	quads[1] = renderer->createQuad();
	quads[1]->setPosition({ 0.0f, 0.0f });
	quads[1]->setSize({ 0.75f, 0.75f });
	quads[1]->setRotationAlignment(oe::alignments::center_center);
	quads[1]->setSprite(sprite);
	quads[1]->setColor(oe::colors::white);
	quads[1]->update();
}

void resize(const glm::vec2& window_size) {
	window->active_context();
	float aspect = window->aspect();
	glm::mat4 pr = glm::ortho(-aspect, aspect, 1.0f, -1.0f);
	shader->setProjectionMatrix(pr);
	shader->useTexture(true);
}

void render(float update_fraction) {
	// modify scene
	float t = oe::utils::Clock::getSingleton().getSessionMillisecond() / 300.0f;
	quads[0]->setRotation(std::sin(t));
	quads[0]->update();
	quads[1]->setRotation(t);
	quads[1]->update();

	// render scene
	shader->bind();
	pack->bind();
	renderer->render();
}

void update() {
	auto& gameloop = window->getGameloop();
	spdlog::info("frametime: {:3.3f} ms ({} fps)", gameloop.getFrametimeMS(), gameloop.getAverageFPS());
}

void keyboard(oe::keys key, oe::actions action, oe::modifiers mods) {
	if (action == oe::actions::press) {
		if (key == oe::keys::key_escape) {
			window->getGameloop().stop();
		}
		else if (key == oe::keys::key_enter) {
			window->setFullscreen(!window->getFullscreen());
		}
	}
}

void init() {
	auto& engine = oe::Engine::getSingleton();

	// engine
	oe::EngineInfo engine_info = {};
	engine_info.api = oe::graphics_api::OpenGL;
	engine_info.debug_messages = true;
	engine.init(engine_info);

	// window
	oe::WindowInfo window_info;
	window_info.title = "Test 1 - Renderer";
	window_info.multisamples = 4;
	window_info.resize_callback = resize;
	window_info.key_callback = keyboard;
	window_info.update_callback = update;
	window_info.render_callback = render;
	window_info.size = { 900, 600 };
	window = engine.createWindow(window_info);
	
	// instance settings
	engine.culling(oe::culling_modes::back);
	engine.swapInterval(0);
	engine.blending();

	// renderer
	oe::RendererInfo renderer_info = {};
	renderer_info.arrayRenderType = oe::types::dynamicrender;
	renderer_info.indexRenderType = oe::types::staticrender;
	renderer_info.max_primitive_count = 100;
	renderer_info.staticVBOBuffer_data = nullptr;
	renderer = engine.createRenderer(renderer_info);

	// shader
	shader = new oe::assets::DefaultShader();
	
	// sprites
	auto img = oe::utils::image_data(oe::assets::TextureSet::oe_logo, oe::formats::rgba, 5, 5);
	pack = new oe::graphics::SpritePack();
	sprite = pack->addSprite(img);
	sprite_white = pack->empty_sprite();
	pack->construct();
	
	// start
	create_scene();
	resize(window->getSize());
	window->getGameloop().start(30); // print the average frametime 30 times in a second

	// closing
	delete pack;
	delete shader;
	delete renderer;
	delete window;
}

int main(int argc, char** argv) {
	try {
		init();
		__debugbreak();
		return 0;
	} catch (const std::exception& e) {
		spdlog::error(e.what());
		assert(e.what());
		__debugbreak();
		return -1;
	}
}
