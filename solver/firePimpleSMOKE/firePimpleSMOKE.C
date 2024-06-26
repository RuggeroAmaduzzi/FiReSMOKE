/*-----------------------------------------------------------------------*\
|                  _       _____ __  __  ____  _  ________                |
|                 | |     / ____|  \/  |/ __ \| |/ /  ____|               |
|          ___  __| | ___| (___ | \  / | |  | | ' /| |__                  |
|         / _ \/ _` |/ __|\___ \| |\/| | |  | |  < |  __|                 |
|        |  __/ (_| | (__ ____) | |  | | |__| | . \| |____                |
|         \___|\__,_|\___|_____/|_|  |_|\____/|_|\_\______|               |
|                                                                         |
|                                                                         |
|   Authors: A. Cuoci, M.R. Malik, Z. Li, A. Parente                      |
|                                                                         |
|   Contacts: Alberto Cuoci                                               |
|   email: alberto.cuoci@polimi.it                                        |
|   Department of Chemistry, Materials and Chemical Engineering           |
|   Politecnico di Milano                                                 |
|   P.zza Leonardo da Vinci 32, 20133 Milano (Italy)                      |
|                                                                         |
|   Contacts: Mohammad Rafi Malik, Zhiyi Li, Alessandro Parente           |
|   Aero-Thermo-Mechanical Department                                     |
|   Université Libre de Bruxelles                                         |
|   Avenue F. D. Roosevelt 50, 1050 Bruxelles (Belgium)                   |
|                                                                         |
|-------------------------------------------------------------------------|
|                                                                         |
|   This file is part of edcSMOKE solver.                                 |
|                                                                         |
|	License                                                           |
|                                                                         |
|   Copyright(C) 2017-2014 A. Cuoci, A. Parente                           |
|   edcSMOKE is free software: you can redistribute it and/or modify      |
|   it under the terms of the GNU General Public License as published by  |
|   the Free Software Foundation, either version 3 of the License, or     |
|   (at your option) any later version.                                   |
|                                                                         |
|   edcSMOKE is distributed in the hope that it will be useful,           |
|   but WITHOUT ANY WARRANTY; without even the implied warranty of        |
|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
|   GNU General Public License for more details.                          |
|                                                                         |
|   You should have received a copy of the GNU General Public License     |
|   along with edcSMOKE. If not, see <http://www.gnu.org/licenses/>.      |
|                                                                         |
\*-----------------------------------------------------------------------*/

// This is a unsteady simulation
#define STEADYSTATE 0

// OpenSMOKE++ Definitions
#include "OpenSMOKEpp"

// CHEMKIN maps
#include "maps/Maps_CHEMKIN"

// OpenSMOKE++ Dictionaries
#include "dictionary/OpenSMOKE_Dictionary"

// ODE solvers
#include "math/native-ode-solvers/MultiValueSolver"
#include "math/external-ode-solvers/ODE_Parameters.h"

// NLS solvers
#include "math/native-nls-solvers/NonLinearSystemSolver"
#include "math/native-nls-solvers/parameters/NonLinearSolver_Parameters.h"

// OpenFOAM
#include "fvCFD.H"
#if OPENFOAM_VERSION >= 40
#include "turbulentFluidThermoModel.H"
#else
#include "turbulenceModel.H"
#include "compressible/LES/LESModel/LESModel.H"
#endif
#if OPENFOAM_VERSION >=60
#include "psiReactionThermo.H"
#include "CombustionModel.H"
#else
#include "psiCombustionModel.H"
#endif
#include "multivariateScheme.H"
#include "pimpleControl.H"
#if OPENFOAM_VERSION >= 40
#if DEVVERSION==1
#include "pressureControl.H"
#endif
#include "fvOptions.H"
#include "localEulerDdtScheme.H"
#include "fvcSmooth.H"
#else
#include "fvIOoptionList.H"
#endif
#include "radiationModel.H"

// Utilities
#include "Utilities.H"

// DRG
#include "DRG.H"

// ODE systems
#include "ODE_PSR.H"
#include "ODE_PSR_Interface.H"
#include "ODE_PFR.H"
#include "ODE_PFR_Interface.H"

// NLS Systems
#include "NLS_PSR.H"
#include "NLS_PSR_Interface.H"

// Characteristic chemical times
#include "CharacteristicChemicalTimes.H"

// ISAT
#if FIRESMOKE_USE_ISAT == 1
    #include "ISAT.h"
    #include "numericalJacobian4ISAT.H"
    #include "mappingGradients/mappingGradient4OpenFOAM.h"
#endif

// SPARC
#if SPARC==1
	#include "myNeuralNetworkBasedOnPCA.h"
	#include "classifyPoint.h"
	#include "classifyPoint_initialize.h"
	#include "classifyPoint_terminate.h"
	#include "extensions/sparc/SPARC_classifier_VQ2.H"
	#include "extensions/sparc/SPARC_classifier_SOFTMAX.H"
	#include "extensions/sparc/ODE_PFR_SPARC.H"
	#include "extensions/sparc/ODE_PFR_SPARC_Interface.H"
	#include "extensions/sparc/SPARC_classifier_NEURAL.H"
	#include "extensions/sparc/SPARC_predictor_NEURAL.H"
#endif


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    unsigned int runTimeStep = 0;

	#include "postProcess.H"

	#include "setRootCase.H"
	#include "createTime.H"
	#include "createMesh.H"
	#include "readGravitationalAcceleration.H"
	#include "createControl.H"
	#include "createTimeControls.H"
	#include "initContinuityErrs.H"
	#include "createFields.H"
	#include "createOpenSMOKEFields.H"
	#include "createFvOptions.H"
	#include "createRadiationModel.H"

	turbulence->validate();

	if (!LTS)
	{
		#include "compressibleCourantNo.H"
		#include "setInitialDeltaT.H"
	}

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;

    while (runTime.run())
    {
        #include "readTimeControls.H"
        #include "compressibleCourantNo.H"
        #include "setDeltaT.H"

        runTime++;
		runTimeStep++;
        Info<< "Time = " << runTime.timeName() << nl << endl;

	if (momentumEquations == true)
	{
		#include "rhoEqn.H"

		while (pimple.loop())
		{
		    #include "UEqn.H"
		    #include "properties.H"
		    if (dynamicCmixEquations == true)
		    {	
				#include "fEqn.H"
		    	#include "varfEqn.H"
		    	#include "ChiEqn.H"
		    }
		    #include "YEqn.H"
		    #include "EEqn.H"

		    // --- Pressure corrector loop
		    while (pimple.correct())
		    {
		        if (pimple.consistent())
		        {
		            #include "pcEqn.H"
		        }
		        else
		        {
		            #include "pEqn.H"
		        }
		    }

		    if (pimple.turbCorr())
		    {
		        turbulence->correct();
		    }
		}
	}
	else
	{
		    #include "properties.H"
		    #include "YEqn.H"
		    #include "EEqn.H"
		    turbulence->correct();
	}

	if (SPARCswitch == true)
	{
    	#include "extensions/sparc/SPARC_local_post_processing.H"
	}
	
	runTime.write();

	Pav << runTime.timeName() << "\t" << p.weightedAverage(mesh.V()).value() << endl;

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}

// ************************************************************************* //
