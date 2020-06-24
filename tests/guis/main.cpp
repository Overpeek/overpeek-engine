#include <engine/include.hpp>
#include <spdlog/fmt/bundled/format.h>

#include <string>



oe::gui::GUI* gui;
oe::gui::TextInput* textbox;
oe::gui::TextPanel* textpanel;
oe::gui::SpritePanel* box;
oe::gui::List* list;

oe::graphics::Window window;
oe::assets::DefaultShader* shader;
oe::graphics::PrimitiveRenderer renderer;
oe::graphics::SpritePack* pack;
const oe::graphics::Sprite* sprite;
oe::graphics::Font* font;

glm::vec4 color = oe::colors::orange;
glm::vec3 rotate(0.0f, 1.0f, 0.0f);
float speed = 0.0f;



static glm::mat4 ml_matrix = glm::mat4(1.0f);
static oe::gui::TextPanelInfo tpi;
class Checkpoint : public oe::gui::TextPanel
{
public:
	glm::quat quaternion;

	Checkpoint(const glm::mat4& point)
		: TextPanel(tpi)
	{
		quaternion = glm::quat_cast(point);
		text_panel_info.text = fmt::format(L"{:.1f}", quaternion);
		spdlog::info("mat: {}", glm::vec4(1.0f));
	}
};

// render cube
void cube() {
	// vertices
	const std::vector<oe::graphics::VertexData> box_vertices = 
	{
		// front
		{ { -1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f }, color },
		{ { -1.0f,  1.0f,  1.0f }, { 0.0f, 1.0f }, color },
		{ {  1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f }, color },
		{ {  1.0f, -1.0f,  1.0f }, { 1.0f, 0.0f }, color },
		
		// back
		{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 1.0f }, color },
		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f }, color },
		{ {  1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f }, color },
		{ {  1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f }, color },
		
		// top
		{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f }, color },
		{ { -1.0f,  1.0f,  1.0f }, { 0.0f, 1.0f }, color },
		{ {  1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f }, color },
		{ {  1.0f,  1.0f, -1.0f }, { 1.0f, 0.0f }, color },
		
		// bottom
		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f }, color },
		{ { -1.0f, -1.0f,  1.0f }, { 0.0f, 1.0f }, color },
		{ {  1.0f, -1.0f,  1.0f }, { 1.0f, 1.0f }, color },
		{ {  1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f }, color },
		
		// left
		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f }, color },
		{ { -1.0f, -1.0f,  1.0f }, { 0.0f, 1.0f }, color },
		{ { -1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f }, color },
		{ { -1.0f,  1.0f, -1.0f }, { 1.0f, 0.0f }, color },
		
		// right
		{ {  1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f }, color },
		{ {  1.0f, -1.0f,  1.0f }, { 0.0f, 1.0f }, color },
		{ {  1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f }, color },
		{ {  1.0f,  1.0f, -1.0f }, { 1.0f, 0.0f }, color },
	};

	// submit
	renderer->begin();
	renderer->clear();
	renderer->submitVertex(box_vertices);
	renderer->end();
	
	// shader and model matrix
	ml_matrix = glm::rotate(ml_matrix, speed * -0.02f * window->getGameloop().getFrametimeMS(), rotate);
	shader->bind();
	shader->setModelMatrix(ml_matrix);

	// render
	oe::Engine::getSingleton().polygonMode(oe::polygon_mode::lines);
	renderer->render();
	oe::Engine::getSingleton().polygonMode(oe::polygon_mode::fill);
}

// render event
void render(float update_fraction) {
	// submitting
	cube();

	// gui
	gui->render();
}

// framebuffer resize
void resize(const oe::ResizeEvent& event) {
	glm::mat4 pr_matrix = glm::perspectiveFov(30.0f, (float)event.framebuffer_size.x, (float)event.framebuffer_size.y, 0.0f, 1000.0f);
	glm::mat4 vw_matrix = glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	shader->bind();
	shader->useTexture(false);
	shader->setProjectionMatrix(pr_matrix);
	shader->setViewMatrix(vw_matrix);
}

// update 30 times per second
void update_30() {
	auto& gameloop = window->getGameloop(); 
	std::wstring str = fmt::format(L"frametime: {:3.3f} ms ({} fps)", gameloop.getFrametimeMS(), gameloop.getAverageFPS());
	textpanel->text_panel_info.text = str;
	// spdlog::info(str);
}

void append_list(const std::wstring& str)
{
	Checkpoint* checkpoint = new Checkpoint(ml_matrix);
	list->add(checkpoint);
}

// gui
void setup_gui() {
	{
		oe::gui::DecoratedButtonInfo button_info = {};
		button_info.size = { 175, 50 };
		button_info.offset_position = { 0, 10 };
		button_info.align_parent = oe::alignments::top_center;
		button_info.align_render = oe::alignments::top_center;
		button_info.sprite = pack->empty_sprite();
		button_info.text = L"new dir";
		button_info.callback = [](oe::mouse_buttons button, oe::actions action) {
			if (action == oe::actions::release && button == oe::mouse_buttons::button_left) {
				// spdlog::info("Button pressed"); 
				std::swap(rotate.x, rotate.y);
				std::swap(rotate.y, rotate.z);
			}
			else if (action == oe::actions::none || button == oe::mouse_buttons::none) {
				// spdlog::info("Button hovered");
			}
		};
		auto button = new oe::gui::DecoratedButton(button_info);
		gui->addSubWidget(button);
	}
	{
		oe::gui::DecoratedButtonInfo button_info = {};
		button_info.size = { 175, 50 };
		button_info.offset_position = { 0, 70 };
		button_info.align_parent = oe::alignments::top_center;
		button_info.align_render = oe::alignments::top_center;
		button_info.sprite = pack->empty_sprite();
		button_info.text = L"reverse";
		button_info.callback = [](oe::mouse_buttons button, oe::actions action) {
			if (action == oe::actions::release && button == oe::mouse_buttons::button_left) {
				// spdlog::info("Button pressed"); 
				rotate *= -1;
			}
			else if (action == oe::actions::none || button == oe::mouse_buttons::none) {
				// spdlog::info("Button hovered");
			}
		};
		auto button = new oe::gui::DecoratedButton(button_info);
		gui->addSubWidget(button);
	}
	{
		oe::gui::SpritePanelInfo sprite_panel_info = {};
		sprite_panel_info.size = { 150, 150 };
		sprite_panel_info.align_parent = oe::alignments::bottom_left;
		sprite_panel_info.align_render = oe::alignments::bottom_left;
		sprite_panel_info.offset_position = { 10, -10 };
		sprite_panel_info.sprite = sprite;
		box = new oe::gui::SpritePanel(sprite_panel_info);
		gui->addSubWidget(box);
	}
	{
		oe::gui::TextInputInfo text_input_info = {};
		text_input_info.size = { 200, 80 };
		text_input_info.align_parent = oe::alignments::bottom_right;
		text_input_info.align_render = oe::alignments::bottom_right;
		text_input_info.font_size = 14;
		text_input_info.sprite = pack->empty_sprite();
		textbox = new oe::gui::TextInput(text_input_info);
		gui->addSubWidget(textbox);
	}
	{
		oe::gui::DecoratedButtonInfo button_info = {};
		button_info.size = { 175, 50 };
		button_info.offset_position = { 0, -10 };
		button_info.align_parent = oe::alignments::top_center;
		button_info.align_render = oe::alignments::bottom_center;
		button_info.sprite = pack->empty_sprite();
		button_info.text = L"log";
		button_info.callback = [](oe::mouse_buttons button, oe::actions action) {
			if (action == oe::actions::release && button == oe::mouse_buttons::button_left) {
				spdlog::info(oe::utils::convertUTF(textbox->text_input_info.text));
				append_list(textbox->text_input_info.text);
				textbox->text_input_info.text = L"";
			}
			else if (action == oe::actions::none || button == oe::mouse_buttons::none) {
				// spdlog::info("Button hovered");
			}
		};
		auto button = new oe::gui::DecoratedButton(button_info);
		textbox->addSubWidget(button);
	}
	{
		oe::gui::SliderInfo slider_info = {};
		slider_info.slider_size = { 400, 30 };
		slider_info.knob_size = { 45, 45 };
		slider_info.align_parent = oe::alignments::bottom_center;
		slider_info.align_render = oe::alignments::bottom_center;
		slider_info.slider_sprite = pack->empty_sprite();
		slider_info.knob_sprite = sprite;
		slider_info.min_value = -1.0f;
		slider_info.max_value =  1.0f;
		slider_info.initial_value = 0.5f;
		slider_info.callback = [](float val) { speed = val; };
		slider_info.draw_value = true;
		auto slider = new oe::gui::Slider(slider_info);
		gui->addSubWidget(slider);
	}
	{
		oe::gui::TextPanelInfo text_panel_info = {};
		text_panel_info.font_size = 20;
		text_panel_info.align_parent = oe::alignments::top_left;
		text_panel_info.align_render = oe::alignments::top_left;
		text_panel_info.text = L"placeholder";
		textpanel = new oe::gui::TextPanel(text_panel_info);
		gui->addSubWidget(textpanel);
	}
	{
		oe::gui::ColorPickerInfo color_picker_info;
		color_picker_info.size = { 200, 100 };
		color_picker_info.align_parent = oe::alignments::center_left;
		color_picker_info.align_render = oe::alignments::center_left;
		color_picker_info.callback = [&](glm::vec4 value) { color = value; };
		color_picker_info.sprite = pack->empty_sprite();
		auto color_picker = new oe::gui::ColorPicker(color_picker_info);
		gui->addSubWidget(color_picker);
	}
	{
		oe::gui::ListInfo list_info;
		list_info.size = { 200, 400 };
		list_info.align_parent = oe::alignments::top_right;
		list_info.align_render = oe::alignments::top_right;
		list_info.sprite = pack->empty_sprite();
		list_info.title = L"Loglist";
		list_info.title_height = 28;
		list = new oe::gui::List(list_info);
		gui->addSubWidget(list);
	}

	tpi = {};
	tpi.font_size = 14;
	tpi.align_parent = oe::alignments::top_left;
	tpi.align_render = oe::alignments::top_left;
	tpi.text = L"placeholder";

	append_list(L"xyz");
}

void main()
{
	auto& engine = oe::Engine::getSingleton();

	// engine
	oe::EngineInfo engine_info = {};
	engine_info.api = oe::graphics_api::OpenGL;
	engine_info.debug_messages = true;
	engine_info.ignore_errors = false;
	engine.init(engine_info);

	// window
	oe::WindowInfo window_config = {};
	window_config.title = "GUIs";
	window_config.multisamples = 4;
	window = oe::graphics::Window(window_config);

	// connect events
	window->connect_listener<oe::ResizeEvent, &resize>();
	window->connect_render_listener<&render>();
	window->connect_update_listener<30, &update_30>();

	// instance settings
	engine.swapInterval(0);
	engine.culling(oe::culling_modes::neither);
	engine.blending(oe::modes::enable);

	// renderer
	oe::RendererInfo renderer_info = {};
	renderer_info.arrayRenderType = oe::types::dynamic_type;
	renderer_info.indexRenderType = oe::types::static_type;
	renderer_info.max_primitive_count = 6;
	renderer_info.staticVBOBuffer_data = nullptr;
	renderer = oe::graphics::PrimitiveRenderer(renderer_info);
	
	// shader
	shader = new oe::assets::DefaultShader();

	// spritepack
	auto img = oe::assets::TextureSet::oe_logo_img;
	pack = new oe::graphics::SpritePack();
	sprite = pack->addSprite(img);

	// font
	font = new oe::graphics::Font(pack);
	oe::graphics::Text::setFont(font);
	pack->construct();

	// gui
	gui = new oe::gui::GUI(window);
	setup_gui();

	// start
	window->getGameloop().start();

	// cleanup
	delete gui;
	delete font;
	delete pack;
	delete shader;
}