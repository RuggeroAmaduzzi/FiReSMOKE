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

#ifndef NLS_PSR_H
#define NLS_PSR_H

class NLS_PSR
{
public:

	NLS_PSR(	OpenSMOKE::ThermodynamicsMap_CHEMKIN& 		thermodynamicsMapXML, 
			OpenSMOKE::KineticsMap_CHEMKIN& 		kineticsMapXML);

	void Set(	const double T0, const double P0_Pa, 
			const OpenSMOKE::OpenSMOKEVectorDouble& omegaMean, const OpenSMOKE::OpenSMOKEVectorDouble& omegaStar, 
			const double hMean, const double gammaStar, const double mDotStar, const int index_max_species )
	{
		TStar_ 			= T0;
		P_Pa_ 			= P0_Pa;
		gammaStar_ 		= gammaStar;
		mDotStar_ 		= mDotStar;
		omegaMean_  		= omegaMean;
		hMean_ 			= hMean;
		omegaStar_		= omegaStar;
		index_max_species_	= index_max_species;
	}

	void SetCheckMassFractions( const bool flag ) { checkMassFractions_ = flag; };
	unsigned int NumberOfEquations() const { return number_of_equations_; }

	int Equations(const double t, const OpenSMOKE::OpenSMOKEVectorDouble& y, OpenSMOKE::OpenSMOKEVectorDouble& dy);
	int Print(const double t, const OpenSMOKE::OpenSMOKEVectorDouble& y);

	void SetDRG(OpenSMOKE::DRG* drg) { drg_ = drg; drgAnalysis_ = true; }

	int index_max_species() const { return index_max_species_; }
	unsigned int number_of_gas_species() const { return number_of_gas_species_; }
	void ReconstructData(const Eigen::VectorXd& y, double& TStar, double& omegaReconstructed);

private:

	unsigned int number_of_equations_;
	unsigned int number_of_gas_species_;
	unsigned int number_of_reactions_;
	         int index_max_species_;

	double TStar_;
	double P_Pa_;
	double MWStar_;
	double cTotStar_;
	double rhoStar_;
	double mDotStar_;
	double gammaStar_;
	
	double hMean_;


	OpenSMOKE::ThermodynamicsMap_CHEMKIN& 	thermodynamicsMapXML_;
	OpenSMOKE::KineticsMap_CHEMKIN& 	kineticsMapXML_;

	OpenSMOKE::OpenSMOKEVectorDouble omegaMean_;
	OpenSMOKE::OpenSMOKEVectorDouble omegaStar_;
	OpenSMOKE::OpenSMOKEVectorDouble omegaSurr_;
	OpenSMOKE::OpenSMOKEVectorDouble xStar_;
	OpenSMOKE::OpenSMOKEVectorDouble cStar_;
	OpenSMOKE::OpenSMOKEVectorDouble RStar_;
	OpenSMOKE::OpenSMOKEVectorDouble rStar_;

	bool checkMassFractions_;

	OpenSMOKE::DRG* drg_;
	bool drgAnalysis_;
};

#include "NLS_PSR.C"

#endif // NLS_PSR_H

