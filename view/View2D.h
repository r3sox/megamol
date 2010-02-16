/*
 * View2D.h
 *
 * Copyright (C) 2008 - 2009 by VISUS (Universitaet Stuttgart). 
 * Alle Rechte vorbehalten.
 */

#ifndef MEGAMOLCORE_VIEW2D_H_INCLUDED
#define MEGAMOLCORE_VIEW2D_H_INCLUDED
#if (defined(_MSC_VER) && (_MSC_VER > 1000))
#pragma once
#endif /* (defined(_MSC_VER) && (_MSC_VER > 1000)) */

#include "view/AbstractView.h"
#include "view/MouseFlags.h"
#include "Module.h"
#include "CallerSlot.h"
#include "param/ParamSlot.h"


namespace megamol {
namespace core {
namespace view {


    /**
     * Base class of rendering graph calls
     */
    class View2D: public AbstractView, public Module {
    public:

        /**
         * Answer the name of this module.
         *
         * @return The name of this module.
         */
        static const char *ClassName(void) {
            return "View2D";
        }

        /**
         * Answer a human readable description of this module.
         *
         * @return A human readable description of this module.
         */
        static const char *Description(void) {
            return "2D View Module";
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
        View2D(void);

        /** Dtor. */
        virtual ~View2D(void);

        /**
         * Renders this AbstractView3D in the currently active OpenGL context.
         */
        virtual void Render(void);

        /**
         * Resets the view. This normally sets the camera parameters to
         * default values.
         */
        virtual void ResetView(void);

        /**
         * Resizes the AbstractView3D.
         *
         * @param width The new width.
         * @param height The new height.
         */
        virtual void Resize(unsigned int width, unsigned int height);

        /**
         * Sets the button state of a button of the 2d cursor. See
         * 'vislib::graphics::Cursor2D' for additional information.
         *
         * @param button The button.
         * @param down Flag whether the button is pressed, or not.
         */
        virtual void SetCursor2DButtonState(unsigned int btn, bool down);

        /**
         * Sets the position of the 2d cursor. See 'vislib::graphics::Cursor2D'
         * for additional information.
         *
         * @param x The x coordinate
         * @param y The y coordinate
         */
        virtual void SetCursor2DPosition(float x, float y);

        /**
         * Sets the state of an input modifier.
         *
         * @param mod The input modifier to be set.
         * @param down The new state of the input modifier.
         */
        virtual void SetInputModifier(mmcInputModifier mod, bool down);

        /**
         * Callback requesting a rendering of this view
         *
         * @param call The calling call
         *
         * @return The return value
         */
        virtual bool OnRenderView(Call& call);

        /**
         * Freezes, updates, or unfreezes the view onto the scene (not the
         * rendering, but camera settings, timing, etc).
         *
         * @param freeze true means freeze or update freezed settings,
         *               false means unfreeze
         */
        virtual void UpdateFreeze(bool freeze);

    private:

        /**
         * Implementation of 'Create'.
         *
         * @return 'true' on success, 'false' otherwise.
         */
        virtual bool create(void);

        /**
         * Implementation of 'Release'.
         */
        virtual void release(void);

        /**
         * Resets the view
         *
         * @param p Must be resetViewSlot
         *
         * @return true
         */
        bool onResetView(param::ParamSlot& p);

        /** The background colour */
        param::ParamSlot backCol;

        /** The background colour for the view */
        float bkgndCol[3];

        /**
         * Flag if this is the first time an image gets created. Used for 
         * initial camera reset
         */
        bool firstImg;

        /** The viewport height */
        float height;

        /** The mouse drag mode */
        unsigned int mouseMode;

        /** The mouse x coordinate */
        float mouseX;

        /** The mouse y coordinate */
        float mouseY;

        /** The mouse flags */
        MouseFlags mouseFlags;

        /** Slot to call the renderer to render */
        CallerSlot rendererSlot;

        /** Triggers the reset of the view */
        param::ParamSlot resetViewSlot;

        /** Shows/hides the bounding box */
        param::ParamSlot showBBoxSlot;

        /** The view focus x coordinate */
        float viewX;

        /** The view focus y coordinate */
        float viewY;

        /** The view zoom factor */
        float viewZoom;

        /** the viewport width */
        float width;

    };


} /* end namespace view */
} /* end namespace core */
} /* end namespace megamol */

#endif /* MEGAMOLCORE_VIEW2D_H_INCLUDED */
