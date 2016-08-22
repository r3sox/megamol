//
// ProteinExploder.h
//
// Copyright (C) 2016 by University of Stuttgart (VISUS).
// All rights reserved.
//

#ifndef MMPROTEINPLUGIN_PROTEINEXPLODER_H_INCLUDED
#define MMPROTEINPLUGIN_PROTEINEXPLODER_H_INCLUDED

#include "mmcore/Module.h"
#include "mmcore/CallerSlot.h"
#include "mmcore/CalleeSlot.h"
#include "protein_calls/MolecularDataCall.h"
#include "mmcore/param/ParamSlot.h"
#include <chrono>

namespace megamol {
namespace protein {
	
	class ProteinExploder : public megamol::core::Module
	{
	public:

		/**
         * Answer the name of this module.
         *
         * @return The name of this module.
         */
		static const char *ClassName(void) {
			return "ProteinExploder";
		}

		/**
         * Answer a human readable description of this module.
         *
         * @return A human readable description of this module.
         */
		static const char *Description(void) {
			return "Pulls proteins apart to have a better view on single components.";
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
		ProteinExploder(void);

		/** Dtor. */
		virtual ~ProteinExploder(void);

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

		/**
         * Call for get data.
         */
		bool getData(megamol::core::Call& call);

		/**
         * Call for get extent.
         */
		bool getExtent(megamol::core::Call& call);
	private:

		/**
		 *	Enum for the mode of explosion
		 */
		enum ExplosionMode {
			SPHERICAL_MIDDLE = 0,
			SPHERICAL_MASS = 1,
			MAIN_DIRECTION = 2,
			MAIN_DIRECTION_CIRCULAR = 3
		};

		/**
		 *	Function controlling the explosion timing
		 *
		 *	@param exVal The linear explosion factor.
		 *	@return The new explosion factor.
		 */
		float explosionFunction(float exVal);

		/**
		 *	Returns the name of the given explosion mode
		 *
		 *	@param mode The explosion mode
		 *	@return The name of the explosion mode as string
		 */
		std::string getModeName(ExplosionMode mode);

		/*
		 *	Returns the explosion mode with the given index
		 *	
		 *	@param idx The index of the explosion mode
		 *	@return The explosion mode with the given index
		 */
		ExplosionMode getModeByIndex(unsigned int idx);

		/**
		 *	Returns the number of explosion modes.
		 */
		int getModeNumber();

		/**
		 *	Callback function for the animation play button.
		 *
		 *	@param p The button parameter
		 *	@return true
		 */
		bool onPlayToggleButton(megamol::core::param::ParamSlot& p);

		/**
		 *	Displaces the atom positions of a molecule dependant from the given parameters. 
		 *	The middle point of the data set has to be computed beforehand
		 *
		 *	@param call The call containing the molecule data
		 *	@param mode The explosion mode
		 *	@param exFactor The explosion factor
		 *	@param computeBoundingBox Should the bounding box be computed? Standard: false
		 */
		void explodeMolecule(megamol::protein_calls::MolecularDataCall& call, ExplosionMode mode, float exFactor, bool computeBoundingBox = false);

		/**
		 *	Computes the main directions of the molecules in the data set via principal component analysis
		 *
		 *	@param call The call containing the molecule data
		 */
		void computeMainDirectionPCA(megamol::protein_calls::MolecularDataCall& call);

		/**
		 *	Computes the middle point of the data set (a.k.a. the average position)
		 *
		 *	@param call The call containing the molecule data
		 *	@param call The explosion mode
		 */
		void computeMidPoint(megamol::protein_calls::MolecularDataCall& call, ExplosionMode mode);

		/** data caller slot */
		megamol::core::CallerSlot getDataSlot;

		/** slot for outgoing data */
		megamol::core::CalleeSlot dataOutSlot;

		/** minimal distance between two exploded components */
		megamol::core::param::ParamSlot minDistanceParam;

		/** the maximal explosion displacement factor */
		megamol::core::param::ParamSlot maxExplosionFactorParam;

		/** the current explosion displacement*/
		megamol::core::param::ParamSlot explosionFactorParam;

		/** The explosion mode */
		megamol::core::param::ParamSlot explosionModeParam;

		/** toggles the play of the animation */
		megamol::core::param::ParamSlot playParam;

		/** toggle callback param for the playParam slot */
		megamol::core::param::ParamSlot togglePlayParam;

		/** parameter to allow restarting of the animation */
		megamol::core::param::ParamSlot replayParam;

		/** the duration of the animation in seconds */
		megamol::core::param::ParamSlot playDurationParam;

		/** the middle point of the explosion origins from */
		megamol::core::param::ParamSlot midPointParam;

		/** force the middle point of the explosion */
		megamol::core::param::ParamSlot forceMidPointParam;

		/** The current atom positions */
		float * atomPositions;

		/** The size of the atom positions array */
		unsigned int atomPositionsSize;

		/** The current bounding box */
		vislib::math::Cuboid<float> currentBoundingBox;

		/** Has the current playthrough of the animation ended? */
		bool playDone;

		/** The last point in time that was captured during animation playback */
		std::chrono::high_resolution_clock::time_point lastTime;

		/** Accumulator for the animation time*/
		float timeAccum;

		/** Is this the first time request?*/
		bool firstRequest;

		/** The main directions of the data set ordered by significance*/
		std::vector<vislib::math::Vector<float, 3>> mainDirections;

		/** Does the data set have a significant main direction? */
		bool significantDirection;

		/** The hash of the last dataset rendered */
		SIZE_T lastDataHash;

		/** The middle point of the data set */
		vislib::math::Vector<float, 3> midpoint;

		/** The middle point value set by the gui*/
		vislib::math::Vector<float, 3> guiMidpoint;
	};
} /* namespace protein */
} /* namespace megamol */

#endif // MMPROTEINPLUGIN_PROTEINEXPLODER_H_INCLUDED