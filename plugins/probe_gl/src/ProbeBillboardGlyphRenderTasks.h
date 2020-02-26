/*
 * ProbeBillboardGlyphRenderTasks.h
 *
 * Copyright (C) 2019 by Universitaet Stuttgart (VISUS).
 * All rights reserved.
 */

#ifndef PROBE_BILLBOARD_GLYPH_RENDER_TASK_H_INCLUDED
#define PROBE_BILLBOARD_GLYPH_RENDER_TASK_H_INCLUDED

#include "mesh/AbstractGPURenderTaskDataSource.h"

#include "ProbeCollection.h"

namespace megamol {
namespace probe_gl {

class ProbeBillboardGlyphRenderTasks : public mesh::AbstractGPURenderTaskDataSource {
public:
    /**
     * Answer the name of this module.
     *
     * @return The name of this module.
     */
    static const char* ClassName(void) { return "ProbeBillboardGlyphRenderTasks"; }

    /**
     * Answer a human readable description of this module.
     *
     * @return A human readable description of this module.
     */
    static const char* Description(void) { return "..."; }

    /**
     * Answers whether this module is available on the current system.
     *
     * @return 'true' if the module is available, 'false' otherwise.
     */
    static bool IsAvailable(void) { return true; }

    ProbeBillboardGlyphRenderTasks();
    ~ProbeBillboardGlyphRenderTasks();

protected:
    bool getDataCallback(core::Call& caller);

    bool getMetaDataCallback(core::Call& caller);

    core::param::ParamSlot m_billboard_size_slot;

private:
    uint32_t m_version;

    core::CallerSlot m_probes_slot;

    std::shared_ptr<glowl::Mesh> m_billboard_dummy_mesh;

    /**
     *
     */

    struct TexturedGlyphData {
        glm::vec4 position;
        GLuint64 texture_handle;
        float slice_idx;
        float scale;
    };

    struct GlyphVectorProbeData {
        glm::vec4 position;
        float scale;

        float padding0;
        float padding1;

        float sample_cnt;
        std::array<float, 4> samples[32];
    };

    struct GlyphScalarProbeData {
        glm::vec4 position;
        GLuint64 texture_handle;
        float slice_idx;
        float scale;

        float sample_cnt;
        float samples[51];
    };

    template <typename ProbeType> TexturedGlyphData createTexturedGlyphData(
        ProbeType const& probe,
        GLuint64 texture_handle,
        float slice_idx,
        float scale);

     GlyphScalarProbeData createScalarProbeGlyphData(
        probe::FloatProbe const& probe,
        float scale);

    GlyphVectorProbeData createVectorProbeGlyphData(
        probe::Vec4Probe const& probe,
        float scale);
};

template <typename ProbeType>
inline ProbeBillboardGlyphRenderTasks::TexturedGlyphData ProbeBillboardGlyphRenderTasks::createTexturedGlyphData(
    ProbeType const& probe, GLuint64 texture_handle, float slice_idx, float scale)
{
    TexturedGlyphData glyph_data;
    glyph_data.position = glm::vec4(
        probe.m_position[0] + probe.m_direction[0] * (probe.m_begin * 1.1f),
        probe.m_position[1] + probe.m_direction[1] * (probe.m_begin * 1.1f),
        probe.m_position[2] + probe.m_direction[2] * (probe.m_begin * 1.1f),
        1.0f);
    glyph_data.texture_handle = texture_handle;
    glyph_data.slice_idx = slice_idx;
    glyph_data.scale = scale;

    return glyph_data;
}

} // namespace probe_gl
} // namespace megamol


#endif // !PROBE_BILLBOARD_GLYPH_RENDER_TASK_H_INCLUDED
