/*
 * MMPLDWriter.h
 *
 * Copyright (C) 2010-2021 by VISUS (Universitaet Stuttgart).
 * Alle Rechte vorbehalten.
 */

#pragma once

#include "mmcore/AbstractDataWriter.h"
#include "mmcore/CallerSlot.h"
#include "geometry_calls/MultiParticleDataCall.h"
#include "mmcore/param/ParamSlot.h"
#include "vislib/sys/File.h"


namespace megamol::moldyn::io {


    /**
     * MMPLD (MegaMol Particle List Dump) file writer
     */
    class MMPLDWriter : public core::AbstractDataWriter {
    public:

        /**
         * Answer the name of this module.
         *
         * @return The name of this module.
         */
        static const char *ClassName(void) {
            return "MMPLDWriter";
        }

        /**
         * Answer a human readable description of this module.
         *
         * @return A human readable description of this module.
         */
        static const char *Description(void) {
            return "MMPLD file writer";
        }

        /**
         * Answers whether this module is available on the current system.
         *
         * @return 'true' if the module is available, 'false' otherwise.
         */
        static bool IsAvailable(void) {
            return true;
        }

        /**
         * Disallow usage in quickstarts
         *
         * @return false
         */
        static bool SupportQuickstart(void) {
            return false;
        }

        /** Ctor. */
        MMPLDWriter(void);

        /** Dtor. */
        virtual ~MMPLDWriter(void);

    protected:

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
         * The main function
         *
         * @return True on success
         */
        virtual bool run(void);

        /**
         * Function querying the writers capabilities
         *
         * @param call The call to receive the capabilities
         *
         * @return True on success
         */
        virtual bool getCapabilities(core::DataWriterCtrlCall& call);

    private:

        /**
         * Writes the data of one frame to the file
         *
         * @param file The output data file
         * @param data The data of the current frame
         *
         * @return True on success
         */
        bool writeFrame(vislib::sys::File& file, geocalls::MultiParticleDataCall& data);

        /** The file name of the file to be written */
        core::param::ParamSlot filenameSlot;

        /** The file format version to be written */
        core::param::ParamSlot versionSlot;

        core::param::ParamSlot startFrameSlot;
        core::param::ParamSlot endFrameSlot;
        core::param::ParamSlot subsetSlot;

        /** The slot asking for data */
        core::CallerSlot dataSlot;

    };


} /* end namespace */

