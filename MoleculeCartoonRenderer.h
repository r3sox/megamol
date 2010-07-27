/*
 * MoleculeCartoonRenderer.h
 *
 * Copyright (C) 2008 by Universitaet Stuttgart (VIS). 
 * Alle Rechte vorbehalten.
 */

#ifndef MMPROTEINPLUGIN_MOLECULECARTOONRENDERER_H_INCLUDED
#define MMPROTEINPLUGIN_MOLECULECARTOONRENDERER_H_INCLUDED
#if (defined(_MSC_VER) && (_MSC_VER > 1000))
#pragma once
#endif /* (defined(_MSC_VER) && (_MSC_VER > 1000)) */

#include "MolecularDataCall.h"
#include "CallFrame.h"
#include "param/ParamSlot.h"
#include "BSpline.h"
#include "CallerSlot.h"
#include "view/Renderer3DModule.h"
#include "view/CallRender3D.h"
#include "vislib/GLSLShader.h"
#include "vislib/GLSLGeometryShader.h"
#include "vislib/SimpleFont.h"
#include "vislib/Array.h"
#include <vector>

namespace megamol {
namespace protein {

    /*
     * Protein Renderer class
     *
     * TODO:
     * - add Parameter:
     *    o number of segments per amino acids
     *    o number of tube segments for CARTOON_CPU
     * - add RenderMode CARTOON_GPU
     */

    class MoleculeCartoonRenderer : public megamol::core::view::Renderer3DModule {
    public:
        /**
         * Answer the name of this module.
         *
         * @return The name of this module.
         */
        static const char *ClassName(void) {
            return "MoleculeCartoonRenderer";
        }

        /**
         * Answer a human readable description of this module.
         *
         * @return A human readable description of this module.
         */
        static const char *Description(void) {
            return "Offers protein cartoon renderings.";
        }

        /**
         * Answers whether this module is available on the current system.
         *
         * @return 'true' if the module is available, 'false' otherwise.
         */
        static bool IsAvailable(void) {
            return true;
        }

        /** Ctor. */
        MoleculeCartoonRenderer(void);

        /** Dtor. */
        virtual ~MoleculeCartoonRenderer(void);

        enum CartoonRenderMode {
            CARTOON        = 0,
            CARTOON_SIMPLE = 1,
            CARTOON_CPU    = 2,
            CARTOON_GPU    = 3
        };

        /** The names of the coloring modes */
        enum ColoringMode {
            ELEMENT     = 0,
            RESIDUE     = 1,
            STRUCTURE   = 2,
            BFACTOR     = 3,
            CHARGE      = 4,
            OCCUPANCY   = 5,
            CHAIN       = 6,
            MOLECULE    = 7,
            RAINBOW     = 8,
            CHAINBOW    = 9     // TODO
        };

        /**********************************************************************
        * 'get'-functions
        **********************************************************************/

        /** Get radius for cartoon rendering mode */
        inline float GetRadiusCartoon(void) const { return radiusCartoon; };

        /** Get number of spline segments per amino acid for cartoon rendering mode */
        inline unsigned int GetNumberOfSplineSegments(void) const { return numberOfSplineSeg; };

        /** Get number of tube segments per 390 degrees in CPU cartoon rendering mode */
        inline unsigned int GetNumberOfTubeSegments(void) const { return numberOfTubeSeg; };

        /** Get the color of a certain atom of the protein. */
        const float* GetProteinAtomColor( unsigned int idx);

        /**********************************************************************
         * 'set'-functions
         **********************************************************************/

        /** Set current render mode */
        void SetRenderMode( CartoonRenderMode rm) { currentRenderMode = rm; RecomputeAll(); };

        /** Set current coloring mode */
        void SetColoringMode( ColoringMode cm) { currentColoringMode = cm; RecomputeAll(); };

        /** Set radius for cartoon rendering mode */
        inline void SetRadiusCartoon( float rad ) { radiusCartoon = rad; RecomputeAll(); };

        /** Set number of spline segments per amino acid for cartoon rendering mode */
        inline void SetNumberOfSplineSegments( unsigned int numSeg ) { numberOfSplineSeg = numSeg; RecomputeAll(); };

        /** Set number of tube segments per 390 degrees in CPU cartoon rendering mode */
        inline void SetNumberOfTubeSegments( unsigned int numSeg ) { numberOfTubeSeg = numSeg; RecomputeAll(); };

    protected:

        /**
         * Implementation of 'Create'.
         *
         * @return 'true' on success, 'false' otherwise.
         */
        virtual bool create(void);

        /**
         * Implementation of 'release'.
         */
        virtual void release(void);

    private:

        /**********************************************************************
         * 'render'-functions
         **********************************************************************/

        /**
         * The get capabilities callback. The module should set the members
         * of 'call' to tell the caller its capabilities.
         *
         * @param call The calling call.
         *
         * @return The return value of the function.
         */
        virtual bool GetCapabilities( megamol::core::Call& call);

        /**
         * The get extents callback. The module should set the members of
         * 'call' to tell the caller the extents of its data (bounding boxes
         * and times).
         *
         * @param call The calling call.
         *
         * @return The return value of the function.
         */
        virtual bool GetExtents( megamol::core::Call& call);

        /**
         * The Open GL Render callback.
         *
         * @param call The calling call.
         * @return The return value of the function.
         */
        virtual bool Render( megamol::core::Call& call);

        /**
         * Draw label for current loaded RMS frame.
         *
         * @param call Ths calling CallFrame.
         */
        void DrawLabel(unsigned int frameID);

        /**
         * The CallFrame callback.
         *
         * @param call The calling call.
         * @return The return value of the function.
         */
        bool ProcessFrameRequest( megamol::core::Call& call);

        /**
         * Render protein in hybrid CARTOON mode using the Geometry Shader.
         *
         * @param prot The data interface.
         */
        void RenderCartoonHybrid( const MolecularDataCall *mol, float* atomPos);

        /**
         * Render protein in CPU CARTOON mode using OpenGL primitives.
         *
         * @param prot The data interface.
         */
        void RenderCartoonCPU( const MolecularDataCall *mol);

         /** 
         * Render protein in GPU CARTOON mode using OpenGL primitives.
         *
         * @param prot The data interface.
         */
        void RenderCartoonGPU( const MolecularDataCall *mol);

        /**
         * Render the molecular data in stick mode.
         */
        void RenderStick( const MolecularDataCall *mol, const float *atomPos);

         /** 
          * Recompute all values.
          * This function has to be called after every change rendering attributes,
          * e.g. coloring or render mode.
          */
         void RecomputeAll(void);

        /**
         * Read color table from file.
         *
         * @param filename The filename of the color table file.
         */
        void ReadColorTableFromFile( vislib::StringA filename);

         /**
         * Creates a rainbow color table with 'num' entries.
         *
         * @param num The number of color entries.
         */
        void MakeRainbowColorTable( unsigned int num);

        /**
         * Make color table for all atoms acoording to the current coloring mode.
         * The color table is only computed if it is empty or if the recomputation 
         * is forced by parameter.
         *
         * @param mol               The data interface.
         * @param forceRecompute    Force recomputation of the color table.
         */
        void MakeColorTable( const MolecularDataCall *mol, bool forceRecompute = false);
 
        /**
         * Update all parameter slots.
         *
         * @param mol   Pointer to the data call.
         */
        void UpdateParameters( const MolecularDataCall *mol);

        /**********************************************************************
         * variables
         **********************************************************************/

        // caller slot
        megamol::core::CallerSlot molDataCallerSlot;
		// caller slot
		megamol::core::CallerSlot molRendererCallerSlot;

        // label with id of current loaded frame
        vislib::graphics::AbstractFont *frameLabel;

        /** camera information */
        vislib::SmartPtr<vislib::graphics::CameraParameters> cameraInfo;

        megamol::core::param::ParamSlot renderingModeParam;
        megamol::core::param::ParamSlot coloringModeParam;
        megamol::core::param::ParamSlot stickColoringModeParam;
        megamol::core::param::ParamSlot smoothCartoonColoringParam;
        /** parameter slot for color table filename */
        megamol::core::param::ParamSlot colorTableFileParam;
        /** parameter slot for min color of gradient color mode */
        megamol::core::param::ParamSlot minGradColorParam;
        /** parameter slot for mid color of gradient color mode */
        megamol::core::param::ParamSlot midGradColorParam;
        /** parameter slot for max color of gradient color mode */
        megamol::core::param::ParamSlot maxGradColorParam;
        /** parameter slot for stick radius */
        megamol::core::param::ParamSlot stickRadiusParam;

        // shader for per pixel lighting (polygonal view)
        vislib::graphics::gl::GLSLShader lightShader;
        // shader for tube generation (cartoon view)
        vislib::graphics::gl::GLSLGeometryShader cartoonShader;
        vislib::graphics::gl::GLSLGeometryShader tubeShader;
        vislib::graphics::gl::GLSLGeometryShader arrowShader;
        vislib::graphics::gl::GLSLGeometryShader helixShader;
        vislib::graphics::gl::GLSLGeometryShader tubeSimpleShader;
        vislib::graphics::gl::GLSLGeometryShader arrowSimpleShader;
        vislib::graphics::gl::GLSLGeometryShader helixSimpleShader;
        vislib::graphics::gl::GLSLGeometryShader tubeSplineShader;
        vislib::graphics::gl::GLSLGeometryShader arrowSplineShader;
        vislib::graphics::gl::GLSLGeometryShader helixSplineShader;

        vislib::graphics::gl::GLSLShader sphereShader;
        vislib::graphics::gl::GLSLShader cylinderShader;

        // current render mode
        CartoonRenderMode currentRenderMode;
        // current coloring mode
        ColoringMode currentColoringMode;
        // smooth coloring of cartoon mode
        bool smoothCartoonColoringMode;

        // attribute locations for GLSL-Shader
        GLint attribLocInParams;
        GLint attribLocQuatC;
        GLint attribLocColor1;
        GLint attribLocColor2;

        // is the geometry shader (and OGL V2) supported?
        bool geomShaderSupported;

        // has the hybrid CARTOON render mode to be prepared?
        bool prepareCartoonHybrid;
        // has the CPU CARTOON render mode to be prepared?
        bool prepareCartoonCPU;

        // counters, vertex- and color-arrays for cartoon mode
        float *vertTube;
        float *normalTube;
        float *colorsParamsTube;
        unsigned int totalCountTube;
        float *vertArrow;
        float *normalArrow;
        float *colorsParamsArrow;
        unsigned int totalCountArrow;
        float *vertHelix;
        float *normalHelix;
        float *colorsParamsHelix;
        unsigned int totalCountHelix;

        // number of spline segments per amino acid
        unsigned int numberOfSplineSeg;
        // number of tube segments per 390 degrees (only used with cartoon GPU)
        unsigned int numberOfTubeSeg;
        // radius for secondary structure elements with CARTOON render modes
        float radiusCartoon;

        /** The color lookup table (for chains, amino acids,...) */
        vislib::Array<vislib::math::Vector<float, 3> > colorLookupTable;
        /** The color lookup table which stores the rainbow colors */
        vislib::Array<vislib::math::Vector<float, 3> > rainbowColors;

        /** The atom color table for rendering */
        vislib::Array<vislib::math::Vector<float, 3> > atomColorTable;
        
        // the Id of the current frame (for dynamic data)
        unsigned int currentFrameId;

        unsigned int atomCount;

	};


} /* end namespace protein */
} /* end namespace megamol */

#endif // MMPROTEINPLUGIN_MOLECULECARTOONRENDERER_H_INCLUDED
