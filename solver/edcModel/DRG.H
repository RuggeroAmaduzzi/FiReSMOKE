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

#ifndef OpenSMOKE_DRG
#define OpenSMOKE_DRG

// OpenSMOKE++ Definitions
#include "OpenSMOKEpp"

// CHEMKIN maps
#include "maps/Maps_CHEMKIN"

// OpenSMOKE++ Dictionaries
#include "dictionary/OpenSMOKE_Dictionary"

namespace OpenSMOKE
{
	//!  A class to perform DRG (Direct Relation Graph)
	/*!
	This class provides the tools to perform DRG on detailed kinetic mechanisms with arbitrary number of
	species and reactions
	*/

	class DRG
	{
	public:

		/**
		*@brief Default constructor
		*@param thermodynamicsMapXML thermodynamic map
		*@param kineticsMapXML kinetics map
		*/
		DRG(OpenSMOKE::ThermodynamicsMap_CHEMKIN* thermodynamicsMapXML, OpenSMOKE::KineticsMap_CHEMKIN* kineticsMapXML);

		/**
		*@brief Sets the key-species (i.e. the target species which are considered important)
		*@param names_key_species vector containing the names of target or key species (0-index based)
		*/
		void SetKeySpecies(const std::vector<std::string> names_key_species);

		/**
		*@brief Sets the key-species (i.e. the target species which are considered important)
		*@param names_key_species vector containing the indices of target or key species (0-index based)
		*/
		void SetKeySpecies(const std::vector<unsigned int> indices_key_species);

		/**
		*@brief Sets the threshold
		*@param epsilon the desired threshold (default 1.e-2)
		*/
		void SetEpsilon(const double epsilon);

		/**
		*@brief Performs the DRG analysis for the given conditions
		*@param T temperature in K
		*@param P_Pa pressure in Pa
		*@param c vector of concentrations in kmol/m3 (1-index based)
		*/
		void Analysis(const double T, const double P_Pa, const OpenSMOKE::OpenSMOKEVectorDouble&);

		/**
		*@brief Returns a boolean vector for each species: true means important (0-index based)
		*/
		const std::vector<bool>& important_species() const { return important_species_; }

		/**
		*@brief Returns a boolean vector for each reaction: true means important (0-index based)
		*/
		const std::vector<bool>& important_reactions() const { return important_reactions_; }

		/**
		*@brief Returns the number of important species
		*/
		unsigned int number_important_species() const { return number_important_species_; }

		/**
		*@brief Returns the number of unimportant species
		*/
		unsigned int number_unimportant_species() const { return number_unimportant_species_; }

		/**
		*@brief Returns the number of important reactions
		*/
		unsigned int number_important_reactions() const { return (NR_ - number_unimportant_reactions_); }

		/**
		*@brief Returns the indices of important species (zero-based)
		*/
		const std::vector<unsigned int>& indices_important_species() const { return indices_important_species_; }

		/**
		*@brief Returns the indices of unimportant species (zero-based)
		*/
		const std::vector<unsigned int>& indices_unimportant_species() const { return indices_unimportant_species_; }

		/**
		*@brief Returns the indices of unimportant reactions (zero-based)
		*/
		const std::vector<unsigned int>& indices_unimportant_reactions() const { return indices_unimportant_reactions_; }

		/**
		*@brief Returns epsilon
		*/
		double epsilon() const { return epsilon_; }

		/**
		*@brief Write cpu times on the screen
		*/
		void WriteCpuTimesOnTheScreen();

		/**
		*@brief Reset counters for calculating cpu time
		*/
		void ResetCounters();

		/**
		*@brief Return the overall cpu time [s] since ResetCounters()
		*/
		double cpuTimeOverall() const { return cpuTimeCumulative_PairWiseErrorMatrix_ + cpuTimeCumulative_ParsePairWiseErrorMatrix_; }

		/**
		*@brief Return the overall number of important species since ResetCounters()
		*/
		unsigned int numberImportantSpeciesOverall() const { return number_important_species_cumulative_; }

		/**
		*@brief Return the overall number of important reactions since ResetCounters()
		*/
		unsigned int numberImportantReactionsOverall() const { return number_important_reactions_cumulative_; }
                
	private:

		/**
		*@brief Builds the pair wise error matrix
		*@param T temperature in K
		*@param P_Pa pressure in Pa
		*@param c vector of concentrations in kmol/m3 (1-index based)
		*/
		void PairWiseErrorMatrix(const double T, const double P_Pa, const OpenSMOKE::OpenSMOKEVectorDouble& c);

		/**
		*@brief Analyzes the pair wise error matrix and calculates (internally) the important_species vector
		*/
		void ParsePairWiseErrorMatrix();

	private:

		OpenSMOKE::ThermodynamicsMap_CHEMKIN& thermodynamicsMapXML_;	/**< reference to the thermodynamic map */
		OpenSMOKE::KineticsMap_CHEMKIN& kineticsMapXML_;		/**< reference to the kinetics map */

		std::vector<unsigned int> index_key_species_;				/**< indices of target or key species (zero-based) */

		std::vector<bool> important_species_;					/**< boolean vector indicating important and unimportant species (zero-based) */
		std::vector<bool> important_reactions_;					/**< boolean vector indicating important and unimportant reactions (zero-based) */

		OpenSMOKE::OpenSMOKEVectorDouble rNet_;					/**< vector containing the net reaction rates (one-based) */
		unsigned int NR_;							/**< total number of reactions */
		unsigned int NS_;							/**< total number of species */

		double epsilon_;							/**< threshold value */

		unsigned int number_important_species_;					/**< current number of important species */
		unsigned int number_unimportant_species_;				/**< current number of unimportant species */
		unsigned int number_unimportant_reactions_;				/**< number of unimportant reactions */

		std::vector<unsigned int> indices_unimportant_reactions_;		/**< indices of unimportant reactions (zero-based) */
		std::vector<unsigned int> indices_important_species_;			/**< indices of important species (zero-based) */
		std::vector<unsigned int> indices_unimportant_species_;			/**< indices of unimportant species (zero-based) */

		Eigen::MatrixXd					r_;			/**< full matrix, pair wise error matrix, (NS x NS) */
		Eigen::SparseMatrix<double> 			nu_sparse_;		/**< sparse matrix containing the net stoichiometric coefficients, (NS x NR) */
		std::vector< Eigen::SparseMatrix<double> >	nu_times_delta_;	/**< vector of sparse matrices (NS x NS) */
		Eigen::SparseMatrix<double> 			delta_sparse_;		/**< delta matrix (NS x NR) */

		unsigned int counter_analysis_;
		double cpuTimeCumulative_PairWiseErrorMatrix_;
		double cpuTimeAverage_PairWiseErrorMatrix_;
		double cpuTimeCumulative_PairWiseErrorMatrix_Step2_;
		double cpuTimeAverage_PairWiseErrorMatrix_Step2_;
		double cpuTimeLocal_ParsePairWiseErrorMatrix_;
		double cpuTimeCumulative_ParsePairWiseErrorMatrix_;
		double cpuTimeAverage_ParsePairWiseErrorMatrix_;

		unsigned int number_important_species_cumulative_;
		double number_important_species_average_;
		unsigned int number_important_reactions_cumulative_;
		double number_important_reactions_average_;
		unsigned int number_important_species_min_;
		unsigned int number_important_species_max_;
		unsigned int number_important_reactions_min_;
		unsigned int number_important_reactions_max_;
	};
}

#include "DRG.hpp"

#endif /* OpenSMOKE_DRG */
