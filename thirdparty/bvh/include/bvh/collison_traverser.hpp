#pragma once

#include "bvh/bvh.hpp"
#include "bvh/sphere.hpp"
#include "bvh/node_colliders.hpp"
#include "bvh/utilities.hpp"

namespace bvh {

template <typename Bvh, size_t StackSize = 64, typename NodeCollider = NodeCollider<Bvh>>
class CollisionTraverser
{
public:
	static constexpr size_t stack_size = StackSize;

private:
	using Scalar = typename Bvh::ScalarType;

    struct Stack {
        using Element = typename Bvh::IndexType;

        Element elements[stack_size];
        size_t size = 0;

        void push(const Element& t) {
            assert(size < stack_size);
            elements[size++] = t;
        }

        Element pop() {
            assert(!empty());
            return elements[--size];
        }

        bool empty() const { return size == 0; }
    };

    template <typename PrimitiveIntersector>
    bvh_always_inline
    std::optional<typename PrimitiveIntersector::Result>& collide_leaf(
        const typename Bvh::Node& node,
        Sphere<Scalar>& sphere, 
        std::optional<typename PrimitiveIntersector::Result>& best_hit,
        PrimitiveIntersector& primitive_intersector) const
    {
        size_t begin = node.first_child_or_primitive;
        size_t end = begin + node.primitive_count;

        for (size_t i = begin; i < end; ++i) 
        {
            if (auto hit = primitive_intersector.collide(i, sphere))
            {
                best_hit = hit;
                sphere.radius = hit->distance();
            }           
        }

        return best_hit;
    }

    template <typename PrimitiveIntersector>
    bvh_always_inline
        std::optional<typename PrimitiveIntersector::Result>
        collide(Sphere<Scalar> sphere, PrimitiveIntersector& primitive_intersector) const
    {
        auto best_hit = std::optional<typename PrimitiveIntersector::Result>(std::nullopt);

        if (bvh_unlikely(bvh.nodes[0].is_leaf()))
        {
            return collide_leaf(bvh.nodes[0], sphere, best_hit, primitive_intersector);
        }


        NodeCollider node_collider;

        Stack stack;
        auto* left_child = &bvh.nodes[bvh.nodes[0].first_child_or_primitive];        
        while (true)
        {
            auto* right_child = left_child + 1;
            auto distance_left = node_collider.collide(*left_child, sphere);
            auto distance_right = node_collider.collide(*right_child, sphere);

            if (distance_left.second < sphere.radius)
            {
                sphere.radius = distance_left.second;
            }

            if (distance_right.second < sphere.radius)
            {
                sphere.radius = distance_right.second;
            }

            if (distance_left.first <= sphere.radius)
            {                
                if (bvh_unlikely(left_child->is_leaf())) 
                {
                    collide_leaf(*left_child, sphere, best_hit, primitive_intersector);
                    left_child = nullptr;
                }
            }
            else
            {
                left_child = nullptr;
            }

            if (distance_right.first <= sphere.radius)
            {                
                if (bvh_unlikely(right_child->is_leaf())) 
                {
                    collide_leaf(*right_child, sphere, best_hit, primitive_intersector);                        
                    right_child = nullptr;
                }
            }
            else
            {
                right_child = nullptr;
            }

            if (left_child) 
            {
                if (right_child) 
                {
                    if (distance_left.first > distance_right.first)
                    {
                        std::swap(left_child, right_child);
                    }
                    stack.push(right_child->first_child_or_primitive);
                }
                left_child = &bvh.nodes[left_child->first_child_or_primitive];
            }
            else if (right_child) 
            {
                left_child = &bvh.nodes[right_child->first_child_or_primitive];
            }
            else 
            {
                if (stack.empty()) break;
                left_child = &bvh.nodes[stack.pop()];
            }
        }                
        return best_hit;
    }

    const Bvh& bvh;

public:
    CollisionTraverser(const Bvh& bvh) : bvh(bvh) {}

    template <typename PrimitiveIntersector>
    bvh_always_inline
    std::optional<typename PrimitiveIntersector::Result>
    traverse(const Sphere<Scalar>& sphere, PrimitiveIntersector& primitive_intersector) const
    {
        return collide(sphere, primitive_intersector);
    }
};

}

