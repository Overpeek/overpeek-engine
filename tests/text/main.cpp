#include <engine/include.hpp>

#include <string>



oe::graphics::Window window;
oe::graphics::Renderer* renderer;
oe::assets::DefaultShader* shader;
oe::graphics::SpritePack* pack;
oe::graphics::Font* font;

oe::graphics::TextLabel* label;

void render(float update_fraction)
{
	shader->bind();
	renderer->render();
}

void update_2() {
	auto& gameloop = window->getGameloop();
	spdlog::debug("frametime: {:3.3f} ms ({} fps)", gameloop.getFrametimeMS(), gameloop.getAverageFPS());
}

void resize(const oe::ResizeEvent& event) {
	float aspect = event.aspect;
	glm::mat4 pr_matrix = glm::ortho(-aspect, aspect, 1.0f, -1.0f);
	shader->setProjectionMatrix(pr_matrix);
	shader->useTexture(true);
}

int main(int argc, char** argv) {
	auto& engine = oe::Engine::getSingleton();
	// engine
	oe::EngineInfo engine_info = {};
	engine_info.api = oe::graphics_api::OpenGL;
	engine_info.debug_messages = true;
	engine.init(engine_info);

	// window
	oe::WindowInfo window_info;
	window_info.title = "Colored Text";
	window_info.multisamples = 4;
	window = oe::graphics::Window(window_info);

	// connect events
	window->connect_listener<oe::ResizeEvent, &resize>();
	window->connect_render_listener<&render>();
	window->connect_update_listener<2, &update_2>();
	
	// instance settings
	engine.culling(oe::culling_modes::back);
	engine.swapInterval(0);
	engine.blending();

	// renderer
	oe::RendererInfo renderer_info = {};
	renderer_info.arrayRenderType = oe::types::dynamic_type;
	renderer_info.indexRenderType = oe::types::static_type;
	renderer_info.max_primitive_count = 1000;
	renderer_info.staticVBOBuffer_data = nullptr;
	renderer = new oe::graphics::Renderer(renderer_info);

	// shader
	shader = new oe::assets::DefaultShader();

	// sprites
	pack = new oe::graphics::SpritePack();
	font = new oe::graphics::Font(pack);
	oe::graphics::Text::setFont(font);
	pack->construct();

	
	// submitting
	label = new oe::graphics::TextLabel(font);
	// label->generate("<#1020ff>The quick brown fox <#ff2020>jumps <#ffffff>over the lazy dog.", window);
	label->generate(L"The quick brown fox", window, oe::colors::pink);
	auto quad = renderer->createQuad();
	quad->setPosition({ label->getSize().x * -0.05, 0.25f });
	quad->setSize(label->getSize() * 0.1f);
	quad->setColor(oe::colors::white);
	quad->setSprite(label->getSprite());
	quad->update();
	spdlog::info(label->getSize());
	
	oe::graphics::Sprite sprite;
	sprite.m_owner = pack->getTexture();
	quad = renderer->createQuad();
	quad->setPosition({ -0.5f, -1.0f });
	quad->setSize({ 1.0f, 1.0f });
	quad->setColor(oe::colors::white);
	quad->setSprite(sprite);
	quad->update();
	
	// blue: <#0000ff>
	// incomplete: <#542>
	// faulty: <#5f>>>>>>>>>
	// with 0x: <#0x4354>
	// negative: <#-43531>

	
	// start
	window->getGameloop().start(); // print the average frametime 30 times in a second

	// closing
	delete font;
	delete pack;
	delete shader;

	return 0;
}
