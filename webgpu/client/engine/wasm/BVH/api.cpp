#include <bvh/bvh.hpp>
#include <bvh/vector.hpp>
#include <bvh/triangle.hpp>
#include <bvh/sweep_sah_builder.hpp>

#include <bvh/ray.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>

#include <memory.h>
#include <glm.hpp>
#include <emscripten.h>


extern "C"
{
	EMSCRIPTEN_KEEPALIVE void* alloc(unsigned size);
	EMSCRIPTEN_KEEPALIVE void dealloc(void* ptr);
	EMSCRIPTEN_KEEPALIVE void zero(void* ptr, unsigned size);
    EMSCRIPTEN_KEEPALIVE void CreateSH(void* p_coeffs, const void* p_fp16);
    EMSCRIPTEN_KEEPALIVE void *CreateBLAS(int num_face, int num_pos, const void* p_indices, int type_indices, const void* p_pos, const void* p_matrix);
    EMSCRIPTEN_KEEPALIVE void DestroyBLAS(void* p_blas);
    EMSCRIPTEN_KEEPALIVE int IntersectBLAS(void* p_blas, float origin_x, float origin_y, float origin_z, float dir_x, float dir_y, float dir_z, float tmin, float tmax, int culling, void* p_interection);
    EMSCRIPTEN_KEEPALIVE void *CreateTLAS(int num_primitives, const void* p_lst_BLAS);
    EMSCRIPTEN_KEEPALIVE void DestroyTLAS(void* p_tlas);
    EMSCRIPTEN_KEEPALIVE int IntersectTLAS(void* p_tlas, float origin_x, float origin_y, float origin_z, float dir_x, float dir_y, float dir_z, float tmin, float tmax, int culling, void* p_interection);
}

void* alloc(unsigned size)
{
	return malloc(size);
}

void dealloc(void* ptr)
{
	free(ptr);
}

void zero(void* ptr, unsigned size)
{
	memset(ptr, 0, size);
}

class BLAS
{
public:
    struct Intersection 
    {
        int triangle_index = -1;
        float t = -1.0f;
        float u = 0.0f;
        float v = 0.0f;
        float distance() const { return t; }
    };

    using ScalarType = float;
	using IntersectionType = Intersection;		

    BLAS(BLAS&&) = default;
    BLAS(int num_face, int num_pos, const void* p_indices, int type_indices, const glm::vec3* p_pos, const glm::mat4& matrix);
    ~BLAS();

    bvh::Vector3<float> center() const;
    bvh::BoundingBox<float> bounding_box() const;
    std::optional<Intersection> intersect(const bvh::Ray<float>& ray, int culling) const;

    BLAS& operator =(BLAS&&) = default;    

private:
    typedef bvh::Triangle<float, true, true> PrimitiveType;
    typedef bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, PrimitiveType> IntersectorType;
    typedef bvh::SingleRayTraverser<bvh::Bvh<float>> TraversorType;

    std::vector<PrimitiveType> m_triangles;
    bvh::BoundingBox<float> m_bounding_box;
    std::unique_ptr<bvh::Bvh<float>> m_bvh;
    std::unique_ptr<IntersectorType> m_intersector[3];
    std::unique_ptr<TraversorType> m_traverser;    


};


template <typename T>
inline void t_get_indices(const T* indices, int face_id, unsigned& i0, unsigned& i1, unsigned& i2)
{
	i0 = indices[face_id * 3];
	i1 = indices[face_id * 3 + 1];
	i2 = indices[face_id * 3 + 2];
}

inline void get_indices(const void* indices, int type_indices, int face_id, unsigned& i0, unsigned& i1, unsigned& i2)
{
	if (type_indices == 1)
	{
		t_get_indices((const uint8_t*)indices, face_id, i0, i1, i2);
	}
	else if (type_indices == 2)
	{
		t_get_indices((const uint16_t*)indices, face_id, i0, i1, i2);
	}
	else if (type_indices == 4)
	{
		t_get_indices((const uint32_t*)indices, face_id, i0, i1, i2);
	}
}

BLAS::BLAS(int num_face, int num_pos, const void* p_indices, int type_indices, const glm::vec3* p_pos, const glm::mat4& matrix)
{
    if (p_indices!=nullptr)
    {
        for (int i=0; i< num_face; i++)
        {
            unsigned i0, i1, i2;
			get_indices(p_indices, type_indices, i, i0, i1, i2);

            glm::vec4 v0, v1, v2;
			v0 = glm::vec4(p_pos[i0], 1.0f);
			v1 = glm::vec4(p_pos[i1], 1.0f);
			v2 = glm::vec4(p_pos[i2], 1.0f);

			v0 = matrix * v0;
			v1 = matrix * v1;
			v2 = matrix * v2;

            m_triangles.emplace_back(PrimitiveType(
				bvh::Vector3<float>(v0.x, v0.y, v0.z),
				bvh::Vector3<float>(v1.x, v1.y, v1.z),
				bvh::Vector3<float>(v2.x, v2.y, v2.z)
			));
        }
    }
    else
	{
		for (int i = 0; i < num_pos / 3; i++)
		{
			glm::vec4 v0, v1, v2;
            v0 = glm::vec4(p_pos[i * 3], 1.0f);
			v1 = glm::vec4(p_pos[i * 3 + 1], 1.0f);
			v2 = glm::vec4(p_pos[i * 3 + 2], 1.0f);

			v0 = matrix * v0;
			v1 = matrix * v1;
			v2 = matrix * v2;

			m_triangles.emplace_back(PrimitiveType(
				bvh::Vector3<float>(v0.x, v0.y, v0.z),
				bvh::Vector3<float>(v1.x, v1.y, v1.z),
				bvh::Vector3<float>(v2.x, v2.y, v2.z)
			));
		}
	}

    auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(m_triangles.data(), m_triangles.size());
	m_bounding_box = bvh::compute_bounding_boxes_union(bboxes.get(), m_triangles.size());

	m_bvh = std::unique_ptr<bvh::Bvh<float>>(new bvh::Bvh<float>);
	bvh::SweepSahBuilder<bvh::Bvh<float>> builder(*m_bvh);
	builder.build(m_bounding_box, bboxes.get(), centers.get(), m_triangles.size());

	m_intersector[0] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_triangles.data()));
	m_intersector[0]->culling = 0;
	m_intersector[1] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_triangles.data()));
	m_intersector[1]->culling = 1;
	m_intersector[2] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_triangles.data()));
	m_intersector[2]->culling = 2;
	m_traverser = std::unique_ptr<TraversorType>(new TraversorType(*m_bvh));
}

BLAS::~BLAS()
{


}

bvh::Vector3<float> BLAS::center() const
{
	return m_bounding_box.center();
}

bvh::BoundingBox<float> BLAS::bounding_box() const
{
	return m_bounding_box;
}

std::optional<BLAS::Intersection> BLAS::intersect(const bvh::Ray<float>& ray, int culling) const
{
	auto hit = m_traverser->traverse(ray, *m_intersector[culling]);
	if (hit.has_value())
	{
		auto intersection = hit->intersection;
		Intersection ret;
		ret.triangle_index = hit->primitive_index;
		ret.t = intersection.t;
		ret.u = intersection.u;
		ret.v = intersection.v;
		return std::make_optional<Intersection>(ret);
	}
	return std::nullopt;		
}

void *CreateBLAS(int num_face, int num_pos, const void* p_indices, int type_indices, const void* p_pos, const void* p_matrix)
{
    const glm::vec3* pos = (const glm::vec3*)p_pos;
    const glm::mat4* matrix = (const glm::mat4*)p_matrix;
    return new BLAS(num_face, num_pos, p_indices, type_indices, pos, *matrix);    
}

void DestroyBLAS(void* p_blas)
{
    delete (BLAS*)p_blas;
}

int IntersectBLAS(void* p_blas, float origin_x, float origin_y, float origin_z, float dir_x, float dir_y, float dir_z, float tmin, float tmax, int culling, void* p_interection)
{
    BLAS* blas = (BLAS*)p_blas;
    bvh::Ray<float> ray(
        bvh::Vector3<float>(origin_x, origin_y, origin_z),
        bvh::Vector3<float>(dir_x, dir_y, dir_z),
        tmin, tmax
    );
    auto intersection = blas->intersect(ray, culling);
    if (intersection.has_value())
    {
        BLAS::Intersection* ret = (BLAS::Intersection*)p_interection;
        *ret = *intersection;
        return 1;
    }
    else
    {
        return 0;
    }
}

class RefBLAS
{
public:
    using Intersection = BLAS::Intersection;
    using ScalarType = float;
	using IntersectionType = Intersection;		

    RefBLAS(RefBLAS&&) = default;
    RefBLAS(const BLAS* blas): m_blas(blas) {}
    ~RefBLAS() {}

    bvh::Vector3<float> center() const
    {
        return m_blas->center();
    }

    bvh::BoundingBox<float> bounding_box() const
    {
        return m_blas->bounding_box();
    }

    std::optional<Intersection> intersect(const bvh::Ray<float>& ray, int culling) const
    {
        return m_blas->intersect(ray, culling);
    }

    RefBLAS& operator =(RefBLAS&&) = default;    

private:
    const BLAS* m_blas;

};

class TLAS
{
public:
    struct Intersection {		
		int primitive_index = -1;
		int triangle_index = -1;
		float t = -1.0f;
		float u = 0.0f;
		float v = 0.0f;
        float distance() const { return t; }
	};   

    TLAS(int num_primitives, const BLAS** lst_BLAS);
    ~TLAS();

    std::optional<Intersection> intersect(const bvh::Ray<float>& ray, int culling = 0) const;

private:
    typedef bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, RefBLAS> IntersectorType;
    typedef bvh::SingleRayTraverser<bvh::Bvh<float>> TraversorType;

    std::vector<RefBLAS> m_primitives;
    std::unique_ptr<bvh::Bvh<float>> m_bvh;
	std::unique_ptr<IntersectorType> m_intersector[3];
	std::unique_ptr<TraversorType> m_traverser;

};

TLAS::TLAS(int num_primitives, const BLAS** lst_BLAS)
{
    for (int i=0; i<num_primitives; i++)
    {
        m_primitives.emplace_back(RefBLAS(lst_BLAS[i]));
    }
    auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(m_primitives.data(), m_primitives.size());
    bvh::BoundingBox<float> global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), m_primitives.size());

    m_bvh = std::unique_ptr<bvh::Bvh<float>>(new bvh::Bvh<float>);
	bvh::SweepSahBuilder<bvh::Bvh<float>> builder(*m_bvh);
	builder.build(global_bbox, bboxes.get(), centers.get(), m_primitives.size());

	m_intersector[0] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_primitives.data()));
	m_intersector[0]->culling = 0;
	m_intersector[1] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_primitives.data()));
	m_intersector[1]->culling = 1;
	m_intersector[2] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_primitives.data()));
	m_intersector[2]->culling = 2;
	m_traverser = std::unique_ptr<TraversorType>(new TraversorType(*m_bvh));
}

TLAS::~TLAS()
{


}

std::optional<TLAS::Intersection> TLAS::intersect(const bvh::Ray<float>& ray, int culling) const
{
    auto hit = m_traverser->traverse(ray, *m_intersector[culling]);
    if (hit.has_value())
	{
        auto intersection = hit->intersection;	
        Intersection ret;
        ret.primitive_index = hit->primitive_index;
        ret.triangle_index = intersection.triangle_index;
		ret.t = intersection.t;
		ret.u = intersection.u;
		ret.v = intersection.v;
		return std::make_optional<Intersection>(ret);        
    }
    return std::nullopt;	
}

void *CreateTLAS(int num_primitives, const void* p_lst_BLAS)
{
    const BLAS** lst_BLAS = (const BLAS**)p_lst_BLAS;
    return new TLAS(num_primitives, lst_BLAS);
}

void DestroyTLAS(void* p_tlas)
{
    delete (TLAS*)p_tlas;
}

int IntersectTLAS(void* p_tlas, float origin_x, float origin_y, float origin_z, float dir_x, float dir_y, float dir_z, float tmin, float tmax, int culling, void* p_interection)
{
    TLAS* tlas = (TLAS*)p_tlas;
    bvh::Ray<float> ray(
        bvh::Vector3<float>(origin_x, origin_y, origin_z),
        bvh::Vector3<float>(dir_x, dir_y, dir_z),
        tmin, tmax
    );
    auto intersection = tlas->intersect(ray, culling);
    if (intersection.has_value())
    {
        TLAS::Intersection* ret = (TLAS::Intersection*)p_interection;
        *ret = *intersection;
        return 1;
    }
    else
    {
        return 0;
    }
}



