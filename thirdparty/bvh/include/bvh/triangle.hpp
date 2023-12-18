#ifndef BVH_TRIANGLE_HPP
#define BVH_TRIANGLE_HPP

#include <optional>
#include <cassert>

#include "bvh/utilities.hpp"
#include "bvh/vector.hpp"
#include "bvh/bounding_box.hpp"
#include "bvh/ray.hpp"
#include "bvh/sphere.hpp"

namespace bvh {

/// Triangle primitive, defined by three points, and using the Moeller-Trumbore test.
/// By default, the normal is left-handed, which minimizes the number of operations in
/// the intersection routine.
/// When encountering precision problems, a small tolerance can be added to the intersector,
/// such that rays that hit the edges can still report an intersection.
template <typename Scalar,
    bool LeftHandedNormal = true,
    bool NonZeroTolerance = false>
struct Triangle {
    struct Intersection {
        Scalar t, u, v;
        Scalar distance() const { return t; }
    };

    using ScalarType       = Scalar;
    using IntersectionType = Intersection;

    Vector3<Scalar> p0, e1, e2, n;

    Triangle() = default;
    Triangle(const Vector3<Scalar>& p0, const Vector3<Scalar>& p1, const Vector3<Scalar>& p2)
        : p0(p0), e1(p0 - p1), e2(p2 - p0)
    {
        n = LeftHandedNormal ? cross(e1, e2) : cross(e2, e1);
    }

    Vector3<Scalar> p1() const { return p0 - e1; }
    Vector3<Scalar> p2() const { return p0 + e2; }

    BoundingBox<Scalar> bounding_box() const {
        BoundingBox<Scalar> bbox(p0);
        bbox.extend(p1());
        bbox.extend(p2());
        return bbox;
    }

    Vector3<Scalar> center() const {
        return (p0 + p1() + p2()) * (Scalar(1.0) / Scalar(3.0));
    }

    std::pair<Vector3<Scalar>, Vector3<Scalar>> edge(size_t i) const {
        assert(i < 3);
        Vector3<Scalar> p[] = { p0, p1(), p2() };
        return std::make_pair(p[i], p[(i + 1) % 3]);
    }

    Scalar area() const {
        return length(n) * Scalar(0.5);
    }

    std::pair<BoundingBox<Scalar>, BoundingBox<Scalar>> split(size_t axis, Scalar position) const {
        Vector3<Scalar> p[] = { p0, p1(), p2() };
        auto left  = BoundingBox<Scalar>::empty();
        auto right = BoundingBox<Scalar>::empty();
        auto split_edge = [=] (const Vector3<Scalar>& a, const Vector3<Scalar>& b) {
            auto t = (position - a[axis]) / (b[axis] - a[axis]);
            return a + t * (b - a);
        };
        auto q0 = p[0][axis] <= position;
        auto q1 = p[1][axis] <= position;
        auto q2 = p[2][axis] <= position;
        if (q0) left.extend(p[0]);
        else    right.extend(p[0]);
        if (q1) left.extend(p[1]);
        else    right.extend(p[1]);
        if (q2) left.extend(p[2]);
        else    right.extend(p[2]);
        if (q0 ^ q1) {
            auto m = split_edge(p[0], p[1]);
            left.extend(m);
            right.extend(m);
        }
        if (q1 ^ q2) {
            auto m = split_edge(p[1], p[2]);
            left.extend(m);
            right.extend(m);
        }
        if (q2 ^ q0) {
            auto m = split_edge(p[2], p[0]);
            left.extend(m);
            right.extend(m);
        }
        return std::make_pair(left, right);
    }

    std::optional<Intersection> intersect(const Ray<Scalar>& ray, int culling) const {
        auto negate_when_right_handed = [] (Scalar x) { return LeftHandedNormal ? x : -x; };

        auto c = p0 - ray.origin;
        auto r = cross(ray.direction, c);
        auto inv_det = negate_when_right_handed(1.0) / dot(n, ray.direction);
        if (culling == 1 && inv_det <= 0) return std::nullopt;
        if (culling == 2 && inv_det >= 0) return std::nullopt;

        auto u = dot(r, e2) * inv_det;
        auto v = dot(r, e1) * inv_det;
        auto w = Scalar(1.0) - u - v;

        // These comparisons are designed to return false
        // when one of t, u, or v is a NaN
        static constexpr auto tolerance =
            NonZeroTolerance ? -std::numeric_limits<Scalar>::epsilon() : Scalar(0);
        if (u >= tolerance && v >= tolerance && w >= tolerance) {
            auto t = negate_when_right_handed(dot(n, c)) * inv_det;
            if (t >= ray.tmin && t <= ray.tmax) {
                if constexpr (NonZeroTolerance) {
                    u = robust_max(u, Scalar(0));
                    v = robust_max(v, Scalar(0));
                }
                return std::make_optional(Intersection{ t, u, v });
            }
        }

        return std::nullopt;
    }

    std::optional<Intersection> collide(const Sphere<Scalar>& sphere) const {

        auto p1 = p0 + e1;
        auto p2 = p0 + e2;
        auto pos = sphere.origin;
        Scalar t = FLT_MAX;
        Scalar u = 0;
        Scalar v = 0;
        {
            auto diff = pos - p0;
            Scalar d = length(diff);
            if (d < t)
            {
                t = d;
            }
        }
        {
            auto diff = pos - p1;
            Scalar d = length(diff);
            if (d < t)
            {
                t = d;
                u = 1.0;
            }
        }
        {
            auto diff = pos - p2;
            Scalar d = length(diff);
            if (d < t)
            {
                t = d;
                v = 1.0;
            }
        }
        {
            Scalar k = dot(pos - p0, e1) / dot(e1, e1);
            if (k >= 0.0 && k < 1.0)
            {
                auto diff = pos - (p0 + e1 * k);
                Scalar d = length(diff);
                if (d < t)
                {
                    t = d;
                    u = k;
                }
            }
        }
        {
            Scalar k = dot(pos - p0, e2) / dot(e2, e2);
            if (k >= 0.0 && k < 1.0)
            {
                auto diff = pos - (p0 + e2 * k);
                Scalar d = length(diff);
                if (d < t)
                {
                    t = d;
                    v = k;
                }
            }
        }
        {
            auto e = e2 - e1;
            Scalar k = dot(pos - p1, e) / dot(e, e);
            if (k >= 0.0 && k < 1.0)
            {
                auto diff = pos - (p1 + e * k);
                Scalar d = length(diff);
                if (d < t)
                {
                    t = d;
                    u = 1.0 - k;
                    v = k;
                }
            }
        }
        {
            auto diff0 = pos - p0;
            Scalar a11 = dot(e1, e1);
            Scalar a22 = dot(e2, e2);
            Scalar a12 = dot(e1, e2);
            Scalar b1 = dot(diff0, e1);
            Scalar b2 = dot(diff0, e2);
            
            Scalar delta = a11 * a22 - a12 * a12;
            Scalar k1 = (b1 * a22 - b2 * a12) / delta;
            Scalar k2 = (b2 * a11 - b1 * a12) / delta;

            if (k1 >= 0.0 && k2 >= 0.0 && (k1 + k2) <= 1.0)
            {
                auto diff = pos - (p0 + k1 * e1 + k2 * e2);
                Scalar d = length(diff);
                if (d < t)
                {
                    t = d;
                    u = k1;
                    v = k2;
                }
            }
        }

        if (t <= sphere.radius)
        {
            return std::make_optional(Intersection{ t, u, v });
        }
        return std::nullopt;
    }
};

} // namespace bvh

#endif
