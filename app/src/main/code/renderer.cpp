#include "renderer.h"

#include "asset_loader.h"
#include "bundle.h"
#include "mat3.h"
#include "rect.h"
#include "vec2.h"

#include <android/native_window.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

namespace {

    const GLuint ATTRIBUTE_POSITION = 0;
    const GLuint ATTRIBUTE_TEXCOORD = 1;

    const char* UNIFORM_MATRIX = "Matrix";
    const char* UNIFORM_TEXTURE = "Texture";

    using blend_func = void (*)();

    struct blend_applicators
    {
        static void none() { glDisable(GL_BLEND); }

        static void alpha()
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    };

    struct vertex
    {
        vec2 position;
        vec2 texcoord;
    };

    struct material_unit
    {
        blend_func apply_blend;
        GLint matrix_uniform;
        GLint texture_uniform;
        size_t program;
        size_t texture;
    };

    struct texture_unit
    {
        GLuint handle;
        uint32_t width;
        uint32_t height;
    };

    GLuint create_shader(GLenum type, const char* source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint compileStatus = GL_TRUE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

        if (compileStatus == GL_TRUE) { return shader; }

        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

        std::vector<char> info(static_cast<size_t>(len) + 1);
        glGetShaderInfoLog(shader, len, &len, &info[0]);

        glDeleteShader(shader);

        throw std::runtime_error(&info[0]);
    }

}

class renderer::impl
{
public:
    explicit impl(ANativeWindow* w);
    ~impl();

    void add_program(std::string vert, std::string frag);
    void add_texture(std::vector<uint8_t> bytes);
    void add_material(const material_source& source);

    void begin_frame();
    void end_frame();

    void set_screen_width(float width);

    void draw(const sprite& s, const mat3& matrix);

private:
    ANativeWindow* window_;
    EGLDisplay display_ = EGL_NO_DISPLAY;
    EGLSurface surface_ = EGL_NO_SURFACE;
    EGLContext context_ = EGL_NO_CONTEXT;

    std::vector<GLuint> programs_;
    std::vector<texture_unit> textures_;
    std::vector<material_unit> materials_;

    mat3 view_matrix_;
    vec2 view_size_;

    struct {
        size_t material;
        std::vector<vertex> vertices;
        std::vector<uint16_t> indices;
    } batch_;

    void clear_resources();
    void use_material(size_t m);
    uint16_t push_vertex(vec2 p, vec2 t);
    void push_triangle(uint16_t i1, uint16_t i2, uint16_t i3);
    void flush_batch();
};

renderer::impl::impl(ANativeWindow* w)
    : window_(w)
{
    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };

    EGLint contextAttrib[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display_, 0, 0);

    EGLint numConfigs;
    EGLConfig config;
    eglChooseConfig(display_, attribs, &config, 1, &numConfigs);

    EGLint format;
    eglGetConfigAttrib(display_, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(window_, 0, 0, format);

    surface_ = eglCreateWindowSurface(display_, config, window_, nullptr);
    context_ = eglCreateContext(display_, config, nullptr, contextAttrib);

    if (eglMakeCurrent(display_, surface_, surface_, context_) == EGL_FALSE)
    {
        return; // throw
    }

    eglSwapInterval(display_, 1);

    glDisable(GL_CULL_FACE);
}

renderer::impl::~impl()
{
    clear_resources();

    if (display_ == EGL_NO_DISPLAY) { return; }

    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (context_ != EGL_NO_CONTEXT) { eglDestroyContext(display_, context_); }
    if (surface_ != EGL_NO_SURFACE) { eglDestroySurface(display_, surface_); }

    eglTerminate(display_);
}

void renderer::impl::begin_frame()
{
    int32_t w = ANativeWindow_getWidth(window_);
    int32_t h = ANativeWindow_getHeight(window_);

    glViewport(0, 0, w, h);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    view_size_ = vec2 { static_cast<float>(w), static_cast<float>(h) };
    view_matrix_ = mat3_scaling(2.0f / w, 2.0f / h);
    batch_.vertices.clear();
    batch_.indices.clear();
}

void renderer::impl::end_frame()
{
    flush_batch();
    eglSwapBuffers(display_, surface_);
}

void renderer::impl::set_screen_width(float width)
{
    view_matrix_ = mat3_scaling(2.0f / width, 2.0f * view_size_.x / (view_size_.y * width));
}

void renderer::impl::draw(const sprite& s, const mat3& m)
{
    use_material(s.material);
    const auto& texture = textures_[materials_[s.material].texture];

    const auto center = vec2 { s.rect.left, s.rect.bottom } + s.origin;
    const auto rect = s.rect - center;

    const vec2 vcs[] = {
        m * vec2 { rect.left, rect.bottom },
        m * vec2 { rect.left, rect.top },
        m * vec2 { rect.right, rect.bottom },
        m * vec2 { rect.right, rect.top }
    };

    const struct rect uv {
        s.rect.left / texture.width,
        s.rect.right / texture.width,
        s.rect.bottom / texture.height,
        s.rect.top / texture.height
    };

    const uint16_t ics[] = {
        push_vertex(vcs[0], vec2 { uv.left, uv.bottom }),
        push_vertex(vcs[1], vec2 { uv.left, uv.top }),
        push_vertex(vcs[2], vec2 { uv.right, uv.bottom }),
        push_vertex(vcs[3], vec2 { uv.right, uv.top })
    };

    push_triangle(ics[0], ics[1], ics[3]);
    push_triangle(ics[0], ics[3], ics[2]);
}

void renderer::impl::use_material(size_t m)
{
    if (batch_.material != m)
    {
        flush_batch();
        batch_.material = m;
    }
}

uint16_t renderer::impl::push_vertex(vec2 p, vec2 t)
{
    batch_.vertices.emplace_back(vertex { p, t });
    return static_cast<uint16_t>(batch_.vertices.size() - 1);
}

void renderer::impl::push_triangle(uint16_t i1, uint16_t i2, uint16_t i3)
{
    batch_.indices.insert(batch_.indices.end(), { i1, i2, i3 });
}

void renderer::impl::flush_batch()
{
    if (batch_.indices.empty() || materials_.empty()) { return; }

    const auto& material = materials_[batch_.material];

    material.apply_blend();

    glUseProgram(programs_[material.program]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures_[material.texture].handle);
    glUniform1i(material.texture_uniform, 0);

    glUniformMatrix3fv(material.matrix_uniform, 1, GL_FALSE, view_matrix_.m);

    glVertexAttribPointer(ATTRIBUTE_POSITION, 2,
        GL_FLOAT, GL_FALSE, sizeof(vertex),
        (const void*)&batch_.vertices[0].position
    );
    glEnableVertexAttribArray(ATTRIBUTE_POSITION);

    glVertexAttribPointer(ATTRIBUTE_TEXCOORD, 2,
        GL_FLOAT, GL_FALSE, sizeof(vertex),
        (const void*)&batch_.vertices[0].texcoord
    );
    glEnableVertexAttribArray(ATTRIBUTE_TEXCOORD);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(batch_.indices.size()),
        GL_UNSIGNED_SHORT, (const void*)&batch_.indices[0]);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    batch_.indices.clear();
    batch_.vertices.clear();
}

void renderer::impl::add_program(std::string vert, std::string frag)
{
    GLuint vshader = create_shader(GL_VERTEX_SHADER, vert.c_str());
    if (vshader == 0) { return; }

    GLuint fshader = create_shader(GL_FRAGMENT_SHADER, frag.c_str());
    if (fshader == 0) { return; }

    GLuint program = glCreateProgram();
    if (program == 0) { return; } // throw

    glBindAttribLocation(program, ATTRIBUTE_POSITION, "transform");
    glBindAttribLocation(program, ATTRIBUTE_TEXCOORD, "texcoord");

    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);

    GLint linkStatus = GL_TRUE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        GLint len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

        std::vector<char> info(static_cast<size_t>(len) + 1);
        glGetProgramInfoLog(program, len, &len, &info[0]);

        glDeleteProgram(program);

        throw std::runtime_error(&info[0]);
    }

    programs_.push_back(program);
}

void renderer::impl::add_texture(std::vector<uint8_t> bytes)
{
    static const uint32_t BMP_WIDTH_OFFSET = 18;
    static const uint32_t BMP_HEIGHT_OFFSET = 22;
    static const uint32_t BMP_DATA_OFFSET = 54;
    static const uint32_t CHANNELS_COUNT = 4;

    texture_unit unit;
    unit.width = *reinterpret_cast<const uint32_t*>(&bytes[BMP_WIDTH_OFFSET]);
    unit.height = *reinterpret_cast<const uint32_t*>(&bytes[BMP_HEIGHT_OFFSET]);

    std::vector<uint8_t> texels(CHANNELS_COUNT * unit.width * unit.height);

    const uint32_t data_size = 3 * unit.width * unit.height;
    for (uint32_t i = 0, t = 0; i < data_size; i += 3, t += CHANNELS_COUNT)
    {
        uint8_t r = texels[t] = bytes[BMP_DATA_OFFSET + i + 2];
        uint8_t g = texels[t + 1] = bytes[BMP_DATA_OFFSET + i + 1];
        uint8_t b = texels[t + 2] = bytes[BMP_DATA_OFFSET + i];

        texels[t + 3] = (uint8_t)((r == 0xff && g == 0x00 && b == 0xff) ? 0x00 : 0xff);
    }

    glGenTextures(1, &unit.handle);
    glBindTexture(GL_TEXTURE_2D, unit.handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, unit.width, unit.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texels[0]);

    glBindTexture(GL_TEXTURE_2D, 0);

    textures_.emplace_back(unit);
}

void renderer::impl::add_material(const material_source& source)
{
    material_unit unit;
    unit.texture = source.texture;
    unit.program = source.shader;
    unit.matrix_uniform = glGetUniformLocation(programs_[source.shader], UNIFORM_MATRIX);
    unit.texture_uniform = glGetUniformLocation(programs_[source.shader], UNIFORM_TEXTURE);

    switch (source.blend)
    {
        case blend_mode::none: unit.apply_blend = blend_applicators::none;
        case blend_mode::alpha: unit.apply_blend = blend_applicators::alpha;
    }

    materials_.emplace_back(unit);
}

void renderer::impl::clear_resources()
{
    materials_.clear();

    for (auto texture: textures_) { glDeleteTextures(1, &texture.handle); };
    textures_.clear();

    for (GLuint program: programs_) { glDeleteProgram(program); };
    programs_.clear();
}


renderer::renderer(ANativeWindow* w): impl_(new impl(w)) {}

renderer::~renderer() {}

void renderer::load_assets(const bundle& b, const asset_loader& loader)
{
    for (auto& shader: b.shaders())
    {
        impl_->add_program(loader.load_string(shader.vert), loader.load_string(shader.frag));
    }

    for (auto& texture: b.textures())
    {
        impl_->add_texture(loader.load_bytes(texture.path));
    }

    for (auto& material: b.materials())
    {
        impl_->add_material(material);
    }
}

void renderer::begin_frame(float interpolation, int64_t delta)
{
    frame_interpolation_ = interpolation;
    frame_delta_ = delta;
    impl_->begin_frame();
}

void renderer::end_frame()
{
    impl_->end_frame();
}

void renderer::set_screen_width(float width)
{
    impl_->set_screen_width(width);
}

void renderer::draw(const sprite& s, const mat3& matrix)
{
    impl_->draw(s, matrix);
}

void renderer::draw(const sprite& s, vec2 position)
{
    impl_->draw(s, mat3_translation(position.x, position.y));
}

void renderer::draw(const sprite& s)
{
    impl_->draw(s, mat3_translation(0.0f, 0.0f));
}
