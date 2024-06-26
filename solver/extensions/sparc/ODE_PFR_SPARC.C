/*——————————————————————————————————————————————————————————————————————-*\
|                              _____ __  __  ____  _  ________            |
|            ___              / ____|  \/  |/ __ \| |/ /  ____|           |
|           /  _| _  ___  ___| (___ | \  / | |  | | ' /| |__              |
|           | |_ | ||  _|/ _ \\___ \| |\/| | |  | |  < |  __|             |
|           |  _|| || | |  __/ ___) | |  | | |__| | . \| |____.           |
|           |_|  |_||_|  \___|_____/|_|  |_|\____/|_|\_\______|           |
|                                                                         |
|   Authors: A. Cuoci, R. Amaduzzi, A. Péquin, A. Parente                 |
|                                                                         |
|   Contacts: Alberto Cuoci                                               |
|   email: alberto.cuoci@polimi.it                                        |
|   Department of Chemistry, Materials and Chemical Engineering           |
|   Politecnico di Milano                                                 |
|   P.zza Leonardo da Vinci 32, 20133 Milano (Italy)                      |
|                                                                         |
|   Contacts: Ruggero Amaduzzi, Arthur Péquin, Alessandro Parente         |
|	email: alessandro.parente@ulb.be			                          |
|   Aero-Thermo-Mechanical Department                                     |
|   Université Libre de Bruxelles                                         |
|   Avenue F. D. Roosevelt 50, 1050 Bruxelles (Belgium)                   |
|                                                                         |
|-------------------------------------------------------------------------|
|                                                                         |
|   This file is part of fireSMOKE solver.                                |
|                                                                         |
|       License                                                           |
|                                                                         |
|   Copyright(C) 2017-2014 A. Cuoci, A. Parente                           |
|   fireSMOKE is free software: you can redistribute it and/or modify     |
|   it under the terms of the GNU General Public License as published by  |
|   the Free Software Foundation, either version 3 of the License, or     |
|   (at your option) any later version.                                   |
|                                                                         |
|   fireSMOKE is distributed in the hope that it will be useful,          |
|   but WITHOUT ANY WARRANTY; without even the implied warranty of        |
|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
|   GNU General Public License for more details.                          |
|                                                                         |
|   You should have received a copy of the GNU General Public License     |
|   along with fireSMOKE. If not, see <http://www.gnu.org/licenses/>.     |
|                                                                         |
\*------------------------------------------------------------------------*/

// OpenSMOKE
#include "OpenSMOKE_Definitions.h"
#include <string>
#include <iostream>
#include <numeric>
#include <Eigen/Dense>

// Base classes
#include "kernel/thermo/ThermoPolicy_CHEMKIN.h"
#include "kernel/kinetics/ReactionPolicy_CHEMKIN.h"
#include "math/PhysicalConstants.h"
#include "math/OpenSMOKEUtilities.h"

// Maps
#include "maps/ThermodynamicsMap_CHEMKIN.h"
#include "maps/KineticsMap_CHEMKIN.h"


ODE_PFR_SPARC::ODE_PFR_SPARC(
	OpenSMOKE::ThermodynamicsMap_CHEMKIN& thermodynamicsMapXML, 
	OpenSMOKE::KineticsMap_CHEMKIN& kineticsMapXML) :
	thermodynamicsMapXML_(thermodynamicsMapXML),
	kineticsMapXML_(kineticsMapXML)
{
	number_of_gas_species_ = thermodynamicsMapXML_.NumberOfSpecies();
	number_of_reactions_ = kineticsMapXML_.NumberOfReactions();
	number_of_equations_ = number_of_gas_species_ + 1 + 2;	// species and temperature + 2 dummy variables

	ChangeDimensions(number_of_gas_species_, &omegaStar_, true);
	ChangeDimensions(number_of_gas_species_, &xStar_, true);
	ChangeDimensions(number_of_gas_species_, &cStar_, true);
	ChangeDimensions(number_of_gas_species_, &RStar_, true);
	ChangeDimensions(number_of_reactions_, 	 &rStar_, true);

	checkMassFractions_ = false;	
	energyEquation_ = true;
}

int ODE_PFR_SPARC::Equations(const double t, const OpenSMOKE::OpenSMOKEVectorDouble& y, OpenSMOKE::OpenSMOKEVectorDouble& dy)
{
	// Recover mass fractions
	if (checkMassFractions_ == true)
	{	for(unsigned int i=1;i<=number_of_gas_species_;++i)
			omegaStar_[i] = max(y[i], 0.);
	}
	else
	{
		for(unsigned int i=1;i<=number_of_gas_species_;++i)
			omegaStar_[i] = y[i];
	}
	// Recover temperature
	const double TStar_ = y[number_of_gas_species_+1];

	// Recover dummy variables
	// There are 2 additional dummy variables (not needed to recover them)

	// Calculates the pressure and the concentrations of species
	thermodynamicsMapXML_.MoleFractions_From_MassFractions(xStar_.GetHandle(), MWStar_, omegaStar_.GetHandle());
	cTotStar_ = P_Pa_/(PhysicalConstants::R_J_kmol * TStar_);
	rhoStar_ = cTotStar_*MWStar_;
	Product(cTotStar_, xStar_, &cStar_);

	// Calculates thermodynamic properties
	thermodynamicsMapXML_.SetTemperature(TStar_);
	thermodynamicsMapXML_.SetPressure(P_Pa_);
	cpStar_ = thermodynamicsMapXML_.cpMolar_Mixture_From_MoleFractions(xStar_.GetHandle());
	cpStar_/=MWStar_;

	// Calculates kinetics
	kineticsMapXML_.SetTemperature(TStar_);
	kineticsMapXML_.SetPressure(P_Pa_);
	kineticsMapXML_.ReactionEnthalpiesAndEntropies();
	kineticsMapXML_.KineticConstants();
	kineticsMapXML_.ReactionRates(cStar_.GetHandle());
	kineticsMapXML_.FormationRates(RStar_.GetHandle());

	// Recovering residuals
	for (unsigned int i=1;i<=number_of_gas_species_;++i)	
		dy[i] = thermodynamicsMapXML_.MW(i-1)*RStar_[i]/rhoStar_;

	if (energyEquation_ == true)
	{	
		const double Q = 0.; // radiation contribution
		const double QRStar_ = kineticsMapXML_.HeatRelease(RStar_.GetHandle());
		dy[number_of_gas_species_+1] = (QRStar_ - Q)/(rhoStar_*cpStar_);
	}
	else		
	{
		dy[number_of_gas_species_+1] = 0.;
	}

	// Dummy equations
	dy[number_of_gas_species_+2] = 0.;
	dy[number_of_gas_species_+3] = 0.;

	return 0;
}

int ODE_PFR_SPARC::Print(const double t, const OpenSMOKE::OpenSMOKEVectorDouble& y)
{
	//std::cout << t << std::endl;
	return 0;
}

