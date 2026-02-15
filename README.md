# CyrexVoxels - *Modern C++ meets Voxels... and it's epic.*
## Design features
- Storage agnostic
- Data oriented
- Concept-driven
- Sampling decoupled from storage

## Brief Example
### 1. Design your voxel structure
The structure must implement `is_solid` and `colorized` to be regarded as a Voxel type
```c++
struct Voxel {
    bool solid{};
    glm::vec3 color{1.0f};

    [[nodiscard]] constexpr bool is_solid() const noexcept {
        return solid;
    }

    [[nodiscard]] constexpr glm::vec3 colorized() const noexcept {
        return color;
    }
};
```
### 2. Code a sampler
A sampler is a generic invokable object that produces a voxel from a `glm::ivec3` input
```c++
const auto sampler = [&](const glm::ivec3 p) -> Voxel {
    constexpr float Radius = 10.0f;
    constexpr glm::vec3 Color = glm::vec3(1.0f, 0.0f, 0.0f); 
    const auto p_float = glm::vec3(p);
    const float distance = glm::length(p_float);
    return {
        .solid = distance < Radius,
        .color = Color
    };
};
```
### 3. Create a mesh generator
`vox::make_blocky_mesher` is suitable for minecraft-style voxels.
```c++
const auto mesher = vox::make_blocky_mesher<decltype(sampler)>({
    .from = {0, 0, 0},
    .to   = {W, H, D},
});
```

### 4. Generate the mesh
```c++
const auto mesh = mesher(sampler);
```

## Graphics Library
CyrexVoxels comes with a functional graphics library based on `glbindings` (Minimal GL_XYZ header pollution)

This means you can get stuck right into coding your voxel game / simulation without slowdown.

```c++

// init shader
const auto program = gfx::Shader::from_source(
    vertex_source,
    fragment_source
);

if (!program) {
    std::cerr << program.error() << '\n';
    return 1;
}

// prepare renderer
const auto [projection, view] = rend::FirstPersonCamera::make_render_matrices(camera);
program->uniform_matrix("projection", projection);
program->uniform_matrix("view", view);

// make buffers
const auto& [vertices, indices] = mesh; // our mesh from before
const auto vbo = gfx::VertexBuffer::make_fixed(std::span(vertices));
const auto ibo = gfx::IndexBuffer::make_fixed(std::span(indices));
const auto vbo = gfx::VertexArray::make_voxel(vbo, ibo);

// rendering ...
gfx::Shader::bind(*program);
gfx::VertexArray::bind(frame.vao);
gfx::draw_triangles_indexed<gfx::IndexType::Uint>({0, static_cast<GLuint>(indices.size())});
```
