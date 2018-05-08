#ifndef PX_CG_UTIL_SHAPE_GENERATOR_HPP
#define PX_CG_UTIL_SHAPE_GENERATOR_HPP

namespace px { namespace generator
{
std::pair<std::vector<float>, std::vector<unsigned short> >
        sphere(unsigned int n_grid, float radius);
std::tuple<
        // vertex            vertex indices
        std::vector<float>, std::vector<unsigned short>,
        // uv                norm
        std::vector<float>, std::vector<float>,
        // tangent
        std::vector<float>
>
sphereWithNormUVTangle(unsigned int n_grid, float radius);
}}

std::pair<std::vector<float>, std::vector<unsigned short> >
px::generator::sphere(unsigned int n_grid, float radius)
{
    auto n_theta = static_cast<int>(n_grid+1);
    auto n_phi = static_cast<int>(n_grid + n_grid);
    auto tot_point = n_phi*(n_theta-1)+1;
    auto n_indices = (n_phi+1)*(n_theta-1) * 6 + 3*n_phi;

    std::vector<float> sphere(tot_point * 3);
    std::vector<unsigned short> vertex_order(n_indices);

    auto d = static_cast<float>(M_PI) / n_grid;
    sphere[0] = 0.f; sphere[1] = 0.f; sphere[2] = radius;
    auto theta = d;
    auto idx = 0;
    for (auto i = 1; i < n_theta; ++i)
    {
        auto phi = 0.f;
        auto c = std::cos(theta);
        auto s = std::sin(theta);
        for (auto j = 0; j < n_phi; ++j)
        {
            sphere[idx++] = radius * s*std::cos(phi);
            sphere[idx++] = radius * s*std::sin(phi);
            sphere[idx++] = radius * c;

            phi += d;
        }
        theta += d;
    }

    auto n = 0;
    idx = 1;
    for (auto j = 0; j < n_phi-1; ++j)
    {
        vertex_order[n++] = 0;
        vertex_order[n++] = 1+j;
        vertex_order[n++] = 2+j;
    }
    vertex_order[n++] = 0;
    vertex_order[n++] = n_phi-1;
    vertex_order[n++] = 1;
    for (auto i = 1; i < n_theta; ++i)
    {
        for (auto j = 0; j < n_phi; ++j)
        {
            vertex_order[n++] = idx;
            vertex_order[n++] = idx+1;
            vertex_order[n++] = idx+1+n_phi;

            vertex_order[n++] = idx+1+n_phi;
            vertex_order[n++] = idx;
            vertex_order[n++] = idx+n_phi;
            ++idx;
        }
        vertex_order[n++] = idx-1;
        vertex_order[n++] = idx-n_phi;
        vertex_order[n++] = idx;

        vertex_order[n++] = idx;
        vertex_order[n++] = idx-1;
        vertex_order[n++] = idx-1+n_phi;
    }

    return {sphere, vertex_order};
};

std::tuple<
        // vertex            vertex indices
        std::vector<float>, std::vector<unsigned short>,
        // uv                norm
        std::vector<float>, std::vector<float>,
        // tangent
        std::vector<float>
>
px::generator::sphereWithNormUVTangle(unsigned int n_grid, float radius)
{
    auto n_theta = static_cast<int>(n_grid+1);
    auto n_phi = static_cast<int>(n_grid + n_grid);
    auto tot_point = n_phi*(n_theta-1)+1;
    auto n_indices = (n_phi+1)*(n_theta-1) * 6 + 3*n_phi;

    std::vector<float> sphere(tot_point * 3);
    std::vector<float> norm(tot_point * 3);
    std::vector<float> tangent(tot_point * 3);
    std::vector<float> uv(tot_point * 2);
    std::vector<unsigned short> vertex_order(n_indices);

    auto d = static_cast<float>(M_PI) / n_grid;
    auto n = 1;
    uv[0] = 0.f; uv[1] = 0.f;
    norm[0] = 0.f; norm[1] = 0.f; norm[2] = 1.f;
    sphere[0] = 0.f; sphere[1] = 0.f; sphere[2] = radius;
    tangent[0] = -1.f; tangent[1] = 0.f, tangent[2] = 0.f;
    auto theta = d;
    for (auto i = 1; i < n_theta; ++i)
    {
        auto phi = 0.f;
        auto c = std::cos(theta);
        auto s = std::sin(theta);
        auto v = static_cast<float>(i) / n_grid;
        for (auto j = 0; j < n_phi; ++j, ++n)
        {
            auto idx = n+n;

            uv[idx] = static_cast<float>(j)/n_grid;
            if (uv[idx] > 1) uv[idx] = 2.f - uv[idx];
            uv[idx+1] = v;

            idx += n;

            norm[idx]   = s*std::cos(phi);
            norm[idx+1] = s*std::sin(phi);
            norm[idx+2] = c;

            sphere[idx] = radius * norm[idx];
            sphere[idx+1] = radius * norm[idx+1];
            sphere[idx+2] = radius * norm[idx+2];

            auto den = std::sqrt(norm[idx+1]*norm[idx+1] + norm[idx]*norm[idx]);
            tangent[idx]   = -norm[idx+1] / den;
            tangent[idx+1] = norm[idx] /den ;
            tangent[idx+2] = 0.f;

//            uv[idx-n] = std::atan2(norm[idx+1], norm[idx])/ static_cast<float>(M_PI);
//            uv[idx-n+1] = std::atan2(norm[idx+1], norm[idx+2]) / static_cast<float>(M_PI);

            phi += d;
        }
        theta += d;
    }

    n = 0;
    auto idx = 1;

    for (auto j = 0; j < n_phi-1; ++j)
    {
        vertex_order[n++] = 0;
        vertex_order[n++] = 1+j;
        vertex_order[n++] = 2+j;
    }
    vertex_order[n++] = 0;
    vertex_order[n++] = n_phi-1;
    vertex_order[n++] = 1;
    for (auto i = 1; i < n_theta; ++i)
    {
        for (auto j = 0; j < n_phi; ++j)
        {
            vertex_order[n++] = idx;
            vertex_order[n++] = idx+1;
            vertex_order[n++] = idx+1+n_phi;

            vertex_order[n++] = idx+1+n_phi;
            vertex_order[n++] = idx;
            vertex_order[n++] = idx+n_phi;
            ++idx;
        }
        vertex_order[n++] = idx-1;
        vertex_order[n++] = idx-n_phi;
        vertex_order[n++] = idx;

        vertex_order[n++] = idx;
        vertex_order[n++] = idx-1;
        vertex_order[n++] = idx-1+n_phi;
    }

    return {sphere, vertex_order, uv, norm, tangent};
};


#endif // PX_CG_UTIL_SHAPE_GENERATOR_HPP
