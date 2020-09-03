#include "renderer.hpp"

#include "engine/utility/extra.hpp"
#include "engine/graphics/interface/window.hpp"
#include "engine/graphics/opengl/gl_texture.hpp"
#include "engine/interfacegen_renderer.hpp"

#include "engine/assets/texture_set/texture_set.hpp"



namespace oe::graphics
{
	class NullSpritePatcher
	{
	private:
		static NullSpritePatcher* singleton;
		NullSpritePatcher(const NullSpritePatcher& copy) = delete;
		NullSpritePatcher()
		{
			auto checker_img = oe::assets::TextureSet::generate_checkerboard();
			TextureInfo ti;
			ti.data = checker_img.data;
			ti.data_format = checker_img.format;
			ti.size = { static_cast<size_t>(checker_img.width), static_cast<size_t>(checker_img.height) };
			ti.offset = { 0, 0 };
			ti.wrap = oe::texture_wrap::repeat;

			m_sprite = {
				{ ti }, { 0.0f, 0.0f }, { 1.0f, 1.0f }
			};
		}

	public:
		static NullSpritePatcher& getSingleton() { if (!singleton) singleton = new NullSpritePatcher(); return *singleton; }

		oe::graphics::Sprite m_sprite;
	};
	NullSpritePatcher* NullSpritePatcher::singleton = nullptr;



	void nullopt_fn(float, glm::vec2&) {}

	void gen_points(const glm::vec2& position, const glm::vec2& size, const glm::vec2& align, float angle, glm::vec2& pointA, glm::vec2& pointB, glm::vec2& pointC, glm::vec2& pointD)
	{
		// setup points
		const glm::vec2 alignment = alignmentOffset(size, align);
		pointA = size * glm::vec2(0.0f, 0.0f) - alignment;
		pointB = size * glm::vec2(0.0f, 1.0f) - alignment;
		pointC = size * glm::vec2(1.0f, 1.0f) - alignment;
		pointD = size * glm::vec2(1.0f, 0.0f) - alignment;

		// rotate fn
		using rotate_fn = void(*)(float, glm::vec2&);
		constexpr std::array<rotate_fn, 2> branchless_if = {
			&oe::utils::rotatePoint,
			&nullopt_fn
		};
		const uint8_t idx = angle != 0.0f ? 0 : 1;
		
		// rotate if non 0 angle - branchless
		branchless_if[idx](angle, pointA);
		branchless_if[idx](angle, pointB);
		branchless_if[idx](angle, pointC);
		branchless_if[idx](angle, pointD);

		// move to wanted offset pos
		pointA += position;
		pointB += position;
		pointC += position;
		pointD += position;
	}



	Quad::Quad(Renderer& host)
		: m_renderer(host)
	{}

	Quad::~Quad()
	{
		m_renderer.m_quads.erase(m_id);
	}

	std::shared_ptr<Quad> Quad::shared()
	{
		return shared_from_this();
	}

	void Quad::gen_vertices(VertexData* ref) const
	{
		glm::vec2 pointA, pointB, pointC, pointD;
		gen_points(m_position, m_size, m_rotation_alignment, m_rotation, pointA, pointB, pointC, pointD);

		const glm::vec2 sprite_pos = m_sprite.position;
		const glm::vec2 sprite_size = m_sprite.size * (m_toggled ? 1.0f : 0.0f);

		ref[0] = VertexData(glm::vec3(pointA, m_position.z), sprite_pos + sprite_size * glm::vec2(0.0f, 0.0f), m_color);
		ref[1] = VertexData(glm::vec3(pointB, m_position.z), sprite_pos + sprite_size * glm::vec2(0.0f, 1.0f), m_color);
		ref[2] = VertexData(glm::vec3(pointC, m_position.z), sprite_pos + sprite_size * glm::vec2(1.0f, 1.0f), m_color);
		ref[3] = VertexData(glm::vec3(pointD, m_position.z), sprite_pos + sprite_size * glm::vec2(1.0f, 0.0f), m_color);
	}

	void Quad::update()
	{
		update(shared());
	}
	
	void Quad::update(const std::shared_ptr<Quad>& ptr)
	{
		m_renderer.update(ptr);
	}

	size_t Quad::getQuadIndex() const {
		return m_quad_index;
	}



	SubRenderer::SubRenderer(Renderer& host)
			: m_renderer(host)
		{}

	SubRenderer::~SubRenderer()
	{
		m_renderer.m_renderers.erase((size_t)m_texture.get());
	}

	std::shared_ptr<SubRenderer> SubRenderer::shared() {
		return shared_from_this();
	}
	
	void SubRenderer::attempt_map()
	{
		if(m_buffer_bound) return;

		m_primitive_renderer->begin();
		m_buffer_bound = true;
	}
	
	void SubRenderer::attempt_unmap()
	{
		if(!m_buffer_bound) return;

		m_primitive_renderer->end();
		m_buffer_bound = false;
	}
	
	void SubRenderer::render()
	{
		attempt_unmap();
		m_primitive_renderer->render();
	}

	void SubRenderer::assign_new_quad(const std::shared_ptr<Quad>& quad)
	{
		quad->m_assigned = true;
		quad->m_current_subrenderer = shared();
		quad->m_quad_index = m_quad_index;
		
		// submit the quad to the selected subrenderer
		attempt_map();
		quad->gen_vertices(m_primitive_renderer->modifyVertex(4, m_quad_index * 4));
		m_primitive_renderer->vertexCount() += 4;
		m_quad_index++;
	}

	void SubRenderer::reassign_quad(const std::shared_ptr<Quad>& quad)
	{
		quad->m_current_subrenderer->remove_quad(quad);
		assign_new_quad(quad);
	}
	
	void SubRenderer::modify_quad(const std::shared_ptr<Quad>& quad)
	{
		attempt_map();
		quad->gen_vertices(m_primitive_renderer->modifyVertex(4, quad->m_quad_index * 4));
	}
	
	void SubRenderer::remove_quad(const std::shared_ptr<Quad>& quad)
	{
		quad->m_current_subrenderer.reset();
		quad->m_assigned = false;

		if (!quad->m_current_subrenderer) // was the only quad on the stack
		{
			return; // this = nullptr
		}
		else if (quad->m_quad_index + 1 == m_quad_index) // if last quad on stack
		{
			// reduce drawn vertices
			m_primitive_renderer->vertexCount() -= 4;
			m_quad_index--;
		}
		else // somewhere in the middle of stack, or front
		{
			// else fill with empty data
			const static std::array<VertexData, 4> empty_filler = {
				VertexData({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }),
				VertexData({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }),
				VertexData({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }),
				VertexData({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f })
			}; // this could just be array of 0 bytes, but this looks nicer

			attempt_map();
			m_primitive_renderer->submitVertex(empty_filler.data(), empty_filler.size(), quad->m_quad_index * 4);
		}
	}



	Renderer::Renderer(const RendererInfo& renderer_info)
		: m_renderer_info(renderer_info)
	{}

	Renderer::~Renderer()
	{}

	std::shared_ptr<SubRenderer> Renderer::select_subrenderer(const Texture& texture)
	{
		auto find_iter = m_renderers.find((size_t)texture.get());
		if (find_iter == m_renderers.end())
		{
			auto sr = std::make_shared<SubRenderer>(*this);
			sr->m_primitive_renderer = PrimitiveRenderer(m_renderer_info);
			sr->m_quad_index = 0;
			sr->m_buffer_bound = false;
			sr->m_texture = texture;

			// create new subrenderer
			m_renderers.insert({reinterpret_cast<size_t>(texture.get()), static_cast<std::weak_ptr<SubRenderer>>(sr)});

			return sr;
		}
		else
		{
			return find_iter->second.lock();
		}
	}

	void Renderer::clear()
	{
		m_quads.clear();
		m_renderers.clear();

		m_quads_forgotten.clear();
	}
	
	void Renderer::update(const std::shared_ptr<Quad>& quad)
	{
		if (!quad->m_sprite.m_owner) quad->m_sprite = NullSpritePatcher::getSingleton().m_sprite;

		if (!quad->m_current_subrenderer)
		{
			// new quad
			quad->m_updated = false;
			quad->m_sprite_updated = false;
			auto new_subrenderer = select_subrenderer(quad);
			if (new_subrenderer->m_quad_index >= m_renderer_info.max_primitive_count)
			{
				spdlog::error("subrenderer primitive overflow!"); // TODO: add another subrenderer
				return;
			}
			new_subrenderer->assign_new_quad(quad);
		}
		else if (quad->m_sprite_updated)
		{


			// move quad to another subrenderer
			quad->m_sprite_updated = false;
			auto new_subrenderer = select_subrenderer(quad);
			auto& old_subrenderer = quad->m_current_subrenderer;

			if (old_subrenderer == new_subrenderer)
			{
				// modified quad vertices
				quad->m_updated = false;
				old_subrenderer->modify_quad(quad);
			}

			old_subrenderer->remove_quad(quad);
			
			if (new_subrenderer->m_quad_index >= m_renderer_info.max_primitive_count)
			{
				spdlog::error("subrenderer primitive overflow!");
				return;
			}
			new_subrenderer->assign_new_quad(quad);
		}
		else if (quad->m_updated)
		{
			// modified quad vertices
			quad->m_updated = false;
			quad->m_current_subrenderer->modify_quad(quad);
		}
		else
		{
			// nothing has changed
		}
	}

	void Renderer::forget(std::shared_ptr<Quad>&& quad)
	{
		m_quads_forgotten.insert({quad->m_id, std::move(quad)});
	}

	std::shared_ptr<Quad> Renderer::create()
	{
		auto shared = std::make_shared<Quad>(*this);
		shared->m_id = std::hash<Quad*>{}(shared.get());
		
		m_quads.insert({ shared->m_id, static_cast<std::weak_ptr<Quad>>(shared) });

		return shared;
	}
	
	void Renderer::render()
	{
		for (auto& subrenderer : m_renderers)
		{
			if (subrenderer.second.expired())
			{
				spdlog::warn("subrenderer leaked");
				return;
			}

			auto shared = subrenderer.second.lock();
			if (shared->m_texture) shared->m_texture->bind();
			shared->render();
		}
	}

}
