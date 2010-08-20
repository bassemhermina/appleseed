
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010 Francois Beaune
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Interface header.
#include "aosurfaceshader.h"

// appleseed.renderer headers.
#include "renderer/kernel/shading/ambientocclusion.h"
#include "renderer/kernel/shading/shadingcontext.h"
#include "renderer/kernel/shading/shadingpoint.h"
#include "renderer/kernel/shading/shadingresult.h"
#include "renderer/modeling/input/inputarray.h"
#include "renderer/modeling/input/source.h"
#include "renderer/modeling/surfaceshader/surfaceshader.h"

// appleseed.foundation headers.
#include "foundation/image/colorspace.h"

using namespace foundation;
using namespace std;

namespace renderer
{

namespace
{

    //
    // Ambient occlusion surface shader.
    //

    class AOSurfaceShader
      : public SurfaceShader
    {
      public:
        // Constructor.
        AOSurfaceShader(
            const char*             name,
            const ParamArray&       params)
          : SurfaceShader(params)
          , m_name(name)
          , m_samples(m_params.get_required<size_t>("samples", 16))
          , m_max_distance(m_params.get_required<double>("max_distance", 1.0))
        {
        }

        // Delete this instance.
        virtual void release()
        {
            delete this;
        }

        // Return a string identifying the model of this surface shader.
        virtual const char* get_model() const
        {
            return AOSurfaceShaderFactory::get_model();
        }

        // Return the name of this surface shader.
        virtual const char* get_name() const
        {
            return m_name.c_str();
        }

        // Evaluate the shading at a given point.
        virtual void evaluate(
            SamplingContext&        sampling_context,
            const ShadingContext&   shading_context,
            const ShadingPoint&     shading_point,
            ShadingResult&          shading_result) const
        {
            // Set color space to linear RGB.
            shading_result.m_color_space = ColorSpaceLinearRGB;

            // Set alpha channel to full opacity.
            shading_result.m_alpha = Alpha(1.0);

            // Compute ambient occlusion.
            const double occlusion =
                compute_ambient_occlusion(
                    sampling_context,
                    shading_context.get_intersector(),
                    shading_point.get_point(),
                    shading_point.get_geometric_normal(),
                    shading_point.get_shading_basis(),
                    m_max_distance,
                    m_samples,
                    &shading_point);

            // Return a gray scale value proportional to the accessibility.
            const float accessibility = static_cast<float>(1.0 - occlusion);
            shading_result.m_color[0] = accessibility;
            shading_result.m_color[1] = accessibility;
            shading_result.m_color[2] = accessibility;
        }

      private:
        const string    m_name;
        const size_t    m_samples;
        const double    m_max_distance;
    };

}   // anonymous namespace


//
// AOSurfaceShaderFactory class implementation.
//

// Return a string identifying this surface shader model.
const char* AOSurfaceShaderFactory::get_model()
{
    return "ao_surface_shader";
}

// Return a new ambient occlusion surface shader.
auto_release_ptr<SurfaceShader> AOSurfaceShaderFactory::create(
    const char*         name,
    const ParamArray&   params)
{
    return
        auto_release_ptr<SurfaceShader>(
            new AOSurfaceShader(name, params));
}

}   // namespace renderer
